#include "ComponentAttributesToolbox.h"

#include "src/mainwindow.h"

#include "DockManager.h"
#include "DockWidget.h"
#include "DockAreaWidget.h"

#include <QVBoxLayout>

#include "src/editorutils/componentreferenceutils.h"

const QString ComponentAttributesToolbox::WidgetName = "ComponentAttributes";
const QString ComponentAttributesToolbox::ToolboxName = ComponentAttributesToolbox::WidgetName + "Toolbox";
const QString ComponentAttributesToolbox::ContainerName = ComponentAttributesToolbox::WidgetName + "Container";
const QString ComponentAttributesToolbox::ContainerContentName = ComponentAttributesToolbox::ContainerName + "Content";

ComponentAttributesToolbox::ComponentAttributesToolbox(MainWindow* mainWindow, ads::CDockManager* dockManager)
	: mMainWindow(mainWindow)
	, mDockManager(dockManager)
{
	mOnComponentChangedHandle = mMainWindow->OnSelectedComponentChanged.bind([this](const auto& val){ onSelectedComponentChange(val); });
	mOnCommandEffectHandle = mMainWindow->OnCommandEffectApplied.bind([this](EditorCommand::EffectBitset effects, bool originalCall){ updateContent(effects, originalCall); });
}

ComponentAttributesToolbox::~ComponentAttributesToolbox()
{
	mMainWindow->OnSelectedComponentChanged.unbind(mOnComponentChangedHandle);
	mMainWindow->OnCommandEffectApplied.unbind(mOnCommandEffectHandle);
}

void ComponentAttributesToolbox::show()
{
	if (ads::CDockWidget* dockWidget = mDockManager->findDockWidget(ToolboxName))
	{
		if (dockWidget->isVisible())
		{
			return;
		}
		else
		{
			mDockManager->layout()->removeWidget(dockWidget);
		}
	}

	QWidget* containerWidget = HS_NEW QWidget();
	ads::CDockWidget* dockWidget = HS_NEW ads::CDockWidget(QString("Component Attributes"));
	dockWidget->setObjectName(ToolboxName);
	dockWidget->setWidget(containerWidget);
	dockWidget->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
	dockWidget->setIcon(mMainWindow->style()->standardIcon(QStyle::SP_DialogOpenButton));
	dockWidget->setFeature(ads::CDockWidget::DockWidgetClosable, true);
	mDockManager->addDockWidget(ads::RightDockWidgetArea, dockWidget);

	containerWidget->setObjectName(ContainerName);

	QVBoxLayout* layout = HS_NEW QVBoxLayout();
	containerWidget->setLayout(layout);
}

bool ComponentAttributesToolbox::isShown() const
{
	if (ads::CDockWidget* dockWidget = mDockManager->findDockWidget(ToolboxName))
	{
		return dockWidget->isVisible();
	}
	else
	{
		return false;
	}
}

void ComponentAttributesToolbox::updateContent(EditorCommand::EffectBitset effects, bool originalCall)
{
	if (!originalCall || (effects.has(EditorCommand::EffectType::ComponentAttributes) && !effects.has(EditorCommand::EffectType::SkipLayoutUpdate)))
	{
		onSelectedComponentChange(mLastSelectedComponent);
	}
}

void ComponentAttributesToolbox::clearContent()
{
	QWidget* componentAttributesContainerWidget = mDockManager->findChild<QWidget*>(ContainerName);
	if (componentAttributesContainerWidget == nullptr)
	{
		return;
	}
	mMainWindow->getComponentContentFactory().removeEditContent(componentAttributesContainerWidget->layout());
}

void ComponentAttributesToolbox::onSelectedComponentChange(const std::optional<ComponentReference>& componentReference)
{
	QWidget* componentAttributesContainerWidget = mDockManager->findChild<QWidget*>(ContainerName);
	if (componentAttributesContainerWidget == nullptr)
	{
		return;
	}

	World* currentWorld = mMainWindow->getCurrentWorld();
	if (currentWorld == nullptr)
	{
		return;
	}

	bool validComponentIsSelected = false;

	if (componentReference.has_value())
	{
		if (BaseComponent* component = Utils::GetComponent(*componentReference, currentWorld))
		{
			mMainWindow->getComponentContentFactory().replaceEditContent(componentAttributesContainerWidget->layout(), componentReference->source, component, mMainWindow->getCommandStack(), currentWorld);
			validComponentIsSelected = true;
			mLastSelectedComponent = componentReference;
		}
	}

	if (!validComponentIsSelected)
	{
		clearContent();
	}
}
