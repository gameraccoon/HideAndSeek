#include "ComponentsListToolbox.h"

#include "src/mainwindow.h"

#include "DockManager.h"
#include "DockWidget.h"
#include "DockAreaWidget.h"

#include <QVBoxLayout>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QPushButton>

#include "src/editorcommands/removecomponentcommand.h"
#include "src/editorutils/componentreferenceutils.h"
#include "src/editorcommands/addcomponentcommand.h"

const QString ComponentsListToolbox::WidgetName = "ComponentsList";
const QString ComponentsListToolbox::ToolboxName = ComponentsListToolbox::WidgetName + "Toolbox";
const QString ComponentsListToolbox::ContainerName = ComponentsListToolbox::WidgetName + "Container";
const QString ComponentsListToolbox::ContainerContentName = ComponentsListToolbox::ContainerName + "Content";
const QString ComponentsListToolbox::ListName = "ComponentList";

ComponentsListToolbox::ComponentsListToolbox(MainWindow* mainWindow, ads::CDockManager* dockManager)
	: mMainWindow(mainWindow)
	, mDockManager(dockManager)
{
	mOnWorldChangedHandle = mMainWindow->OnWorldChanged.bind([this]{ updateContent(); });
	mOnComponentSourceChangedHandle = mMainWindow->OnSelectedComponentSourceChanged.bind([this](const auto& val){ onSelectedComponentSourceChanged(val); });
}

ComponentsListToolbox::~ComponentsListToolbox()
{
	mMainWindow->OnWorldChanged.unbind(mOnWorldChangedHandle);
	mMainWindow->OnSelectedComponentSourceChanged.unbind(mOnComponentSourceChangedHandle);
}

void ComponentsListToolbox::show()
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
	ads::CDockWidget* dockWidget = HS_NEW ads::CDockWidget(QString("Components List"));
	dockWidget->setObjectName(ToolboxName);
	dockWidget->setWidget(containerWidget);
	dockWidget->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
	dockWidget->setIcon(mMainWindow->style()->standardIcon(QStyle::SP_DialogOpenButton));
	dockWidget->setFeature(ads::CDockWidget::DockWidgetClosable, true);
	mDockManager->addDockWidget(ads::RightDockWidgetArea, dockWidget);

	containerWidget->setObjectName(ContainerName);

	QVBoxLayout* layout = HS_NEW QVBoxLayout();
	containerWidget->setLayout(layout);
	QListWidget* componentList = HS_NEW QListWidget();
	layout->addWidget(componentList);
	componentList->setObjectName(ListName);
	componentList->setContextMenuPolicy(Qt::CustomContextMenu);

	mAddComponentButton = HS_NEW QPushButton();
	mAddComponentButton->setText("Add Component");
	QObject::connect(mAddComponentButton, &QPushButton::pressed, this, &ComponentsListToolbox::addComponentCommand);
	mAddComponentButton->setEnabled(false);
	layout->addWidget(mAddComponentButton);

	QObject::connect(componentList, &QListWidget::currentItemChanged, this, &ComponentsListToolbox::onCurrentItemChanged);
	QObject::connect(componentList, &QListWidget::customContextMenuRequested, this, &ComponentsListToolbox::showContextMenu);
}

void ComponentsListToolbox::updateContent()
{
	onSelectedComponentSourceChanged(mLastSelectedComponentSource);
}

void ComponentsListToolbox::onSelectedComponentSourceChanged(const std::optional<ComponentSourceReference>& newSource)
{
	QListWidget* componentsList = mDockManager->findChild<QListWidget*>(ListName);
	if (componentsList == nullptr)
	{
		return;
	}

	componentsList->clear();

	World* currentWorld = mMainWindow->getCurrentWorld();

	if (currentWorld && newSource.has_value())
	{
		const std::vector<TypedComponent> components = Utils::GetComponents(*newSource, currentWorld);
		for (const auto& componentData : components)
		{
			componentsList->addItem(QString::fromStdString(ID_TO_STR(componentData.typeId)));
		}
		mAddComponentButton->setEnabled(true);
	}
	else
	{
		mAddComponentButton->setEnabled(false);
	}

	mLastSelectedComponentSource = newSource;
}

void ComponentsListToolbox::showContextMenu(const QPoint& pos)
{
	QListWidget* componentsList = mDockManager->findChild<QListWidget*>(ListName);
	if (componentsList == nullptr)
	{
		return;
	}

	if (!componentsList->currentItem())
	{
		return;
	}

	QMenu contextMenu(tr("Context menu"), this);

	QAction actionRemove("Remove Component", this);
	connect(&actionRemove, &QAction::triggered, this, &ComponentsListToolbox::removeSelectedComponent);
	contextMenu.addAction(&actionRemove);

	contextMenu.exec(componentsList->mapToGlobal(pos));
}

void ComponentsListToolbox::removeSelectedComponent()
{
	QListWidget* componentsList = mDockManager->findChild<QListWidget*>(ListName);
	if (componentsList == nullptr)
	{
		return;
	}

	QListWidgetItem* currentItem = componentsList->currentItem();
	if (currentItem == nullptr)
	{
		return;
	}

	World* currentWorld = mMainWindow->getCurrentWorld();
	if (currentWorld == nullptr)
	{
		return;
	}

	if (!mLastSelectedComponentSource.has_value())
	{
		return;
	}

	mMainWindow->getCommandStack().executeNewCommand<RemoveComponentCommand>(
		currentWorld,
		*mLastSelectedComponentSource,
		STR_TO_ID(currentItem->text().toStdString()),
		mMainWindow->getComponentSerializationHolder(),
		mMainWindow->getComponentFactory()
	);
	updateContent();
}

void ComponentsListToolbox::addComponentCommand()
{
	QInputDialog* dialog = HS_NEW QInputDialog();
	dialog->setLabelText("Select Component Type:");
	dialog->setCancelButtonText("Cancel");
	dialog->setComboBoxEditable(false);
	QStringList items;
	mMainWindow->getComponentFactory().forEachComponentType([&items](StringId name)
	{
		items.append(QString::fromStdString(ID_TO_STR(name)));
	});
	dialog->setComboBoxItems(items);
	connect(dialog, &QInputDialog::textValueSelected, this, &ComponentsListToolbox::addComponent);
	dialog->show();
}

void ComponentsListToolbox::addComponent(const QString& typeName)
{
	QListWidget* entitiesList = mDockManager->findChild<QListWidget*>(ListName);
	if (entitiesList == nullptr)
	{
		return;
	}

	World* currentWorld = mMainWindow->getCurrentWorld();
	if (currentWorld == nullptr)
	{
		return;
	}

	if (mLastSelectedComponentSource.has_value())
	{
		mMainWindow->getCommandStack().executeNewCommand<AddComponentCommand>(
			currentWorld,
			*mLastSelectedComponentSource,
			STR_TO_ID(typeName.toStdString()),
			mMainWindow->getComponentFactory()
		);
	}
	updateContent();
}

void ComponentsListToolbox::onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
	if (current && mLastSelectedComponentSource.has_value())
	{
		ComponentReference reference;
		reference.source = *mLastSelectedComponentSource;
		reference.componentTypeName = STR_TO_ID(current->text().toStdString());
		mMainWindow->OnSelectedComponentChanged.broadcast(reference);
	}
	else
	{
		mMainWindow->OnSelectedComponentChanged.broadcast(std::nullopt);
	}
}
