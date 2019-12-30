#include "ComponentsListToolbox.h"

#include "src/mainwindow.h"

#include "DockManager.h"
#include "DockWidget.h"
#include "DockAreaWidget.h"

#include <QVBoxLayout>
#include <QMenu>
#include <QAction>

#include "src/editorcommands/removecomponentcommand.h"

const QString ComponentsListToolbox::WidgetName = "ComponentsList";
const QString ComponentsListToolbox::ToolboxName = ComponentsListToolbox::WidgetName + "Toolbox";
const QString ComponentsListToolbox::ContainerName = ComponentsListToolbox::WidgetName + "Container";
const QString ComponentsListToolbox::ContainerContentName = ComponentsListToolbox::ContainerName + "Content";
const QString ComponentsListToolbox::ListName = "ComponentList";

ComponentsListToolbox::ComponentsListToolbox(MainWindow* mainWindow, ads::CDockManager* dockManager)
	: mMainWindow(mainWindow)
	, mDockManager(dockManager)
{
	mOnWorldChangedHandle = mMainWindow->OnWorldChanged.bind([this]{bindEvents(); updateContent();});
	mOnEntityChangedHandle = mMainWindow->OnSelectedEntityChanged.bind([this](const auto& val){onSelectedEntityChanged(val);});
}

ComponentsListToolbox::~ComponentsListToolbox()
{
	mMainWindow->OnWorldChanged.unbind(mOnWorldChangedHandle);
	mMainWindow->OnSelectedEntityChanged.unbind(mOnEntityChangedHandle);
	unbindEvents();
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

	QWidget* containerWidget = new QWidget();
	ads::CDockWidget* dockWidget = new ads::CDockWidget(QString("Components List"));
	dockWidget->setObjectName(ToolboxName);
	dockWidget->setWidget(containerWidget);
	dockWidget->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
	dockWidget->setIcon(mMainWindow->style()->standardIcon(QStyle::SP_DialogOpenButton));
	dockWidget->setFeature(ads::CDockWidget::DockWidgetClosable, true);
	mDockManager->addDockWidget(ads::RightDockWidgetArea, dockWidget);

	containerWidget->setObjectName(ContainerName);

	QVBoxLayout* layout = new QVBoxLayout();
	containerWidget->setLayout(layout);
	QListWidget* componentList = new QListWidget();
	layout->addWidget(componentList);
	componentList->setObjectName(ListName);
	componentList->setContextMenuPolicy(Qt::CustomContextMenu);

	QObject::connect(componentList, &QListWidget::currentItemChanged, this, &ComponentsListToolbox::onCurrentItemChanged);
	QObject::connect(componentList, &QListWidget::customContextMenuRequested, this, &ComponentsListToolbox::showContextMenu);
}

void ComponentsListToolbox::updateContent()
{
	onSelectedEntityChanged(mLastSelectedEntity);
}

void ComponentsListToolbox::onSelectedEntityChanged(const std::optional<EntityReference>& newEntity)
{
	QListWidget* componentsList = mDockManager->findChild<QListWidget*>(ListName);
	if (componentsList == nullptr)
	{
		return;
	}

	componentsList->clear();

	World* currentWorld = mMainWindow->getCurrentWorld();

	if (currentWorld && newEntity.has_value())
	{
		Entity::EntityID entityUid = newEntity->entity.getID();
		std::vector<BaseComponent*> components = currentWorld->getEntityManager().getAllEntityComponents(Entity(entityUid));
		for (auto& component : components)
		{
			componentsList->addItem(QString::fromStdString(ID_TO_STR(component->getComponentTypeName())));
		}
	}

	mLastSelectedEntity = newEntity;
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

	if (!mLastSelectedEntity.has_value())
	{
		return;
	}

	mMainWindow->getCommandStack().executeNewCommand<RemoveComponentCommand>(
		currentWorld,
		mLastSelectedEntity->entity,
		STR_TO_ID(currentItem->text().toStdString()),
		&mMainWindow->getComponentFactory()
	);
}

void ComponentsListToolbox::onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
	if (current && mLastSelectedEntity.has_value())
	{
		ComponentReference reference;
		reference.isWorld = true;
		reference.entity = mLastSelectedEntity->entity;
		reference.cellPos = mLastSelectedEntity->cellPos;
		reference.componentTypeName = STR_TO_ID(current->text().toStdString());
		mMainWindow->OnSelectedComponentChanged.broadcast(reference);
	}
	else
	{
		mMainWindow->OnSelectedComponentChanged.broadcast(std::nullopt);
	}
}

void ComponentsListToolbox::bindEvents()
{
	if (World* currentWorld = mMainWindow->getCurrentWorld())
	{
		mOnComponentAddedHandle = currentWorld->getEntityManager().OnComponentAdded.bind([this]{updateContent();});
		mOnComponentRemovedHandle = currentWorld->getEntityManager().OnComponentRemoved.bind([this]{updateContent();});
	}
}

void ComponentsListToolbox::unbindEvents()
{
	if (World* currentWorld = mMainWindow->getCurrentWorld())
	{
		currentWorld->getEntityManager().OnComponentAdded.unbind(mOnComponentAddedHandle);
		currentWorld->getEntityManager().OnComponentRemoved.unbind(mOnComponentRemovedHandle);
	}
}
