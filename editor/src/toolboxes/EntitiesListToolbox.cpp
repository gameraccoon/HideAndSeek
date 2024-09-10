#include "EntitiesListToolbox.h"

#include <QAction>
#include <QInputDialog>
#include <QMenu>
#include <QVBoxLayout>

#include "DockManager.h"
#include "DockWidget.h"
#include "src/editorcommands/removeentitiescommand.h"
#include "src/editorutils/editoridutils.h"
#include "src/mainwindow.h"
#include "src/toolboxes/PrefabListToolbox.h"

#include "GameData/Components/EditorIdComponent.generated.h"
#include "GameData/World.h"

const QString EntitiesListToolbox::WidgetName = "EntitiesList";
const QString EntitiesListToolbox::ToolboxName = WidgetName + "Toolbox";
const QString EntitiesListToolbox::ContainerName = WidgetName + "Container";
const QString EntitiesListToolbox::ContainerContentName = ContainerName + "Content";
const QString EntitiesListToolbox::ListName = "EntityList";

EntitiesListToolbox::EntitiesListToolbox(MainWindow* mainWindow, ads::CDockManager* dockManager)
	: mMainWindow(mainWindow)
	, mDockManager(dockManager)
{
	mOnWorldChangedHandle = mMainWindow->OnWorldChanged.bind([this] { bindEvents(); updateContent(); });
	mOnSelectedEntityChangedHandle = mMainWindow->OnSelectedEntityChanged.bind([this](const auto& entityRef) { onEntityChangedEvent(entityRef); });
}

EntitiesListToolbox::~EntitiesListToolbox()
{
	mMainWindow->OnWorldChanged.unbind(mOnWorldChangedHandle);
	mMainWindow->OnSelectedEntityChanged.unbind(mOnSelectedEntityChangedHandle);
	unbindEvents();
}

void EntitiesListToolbox::show()
{
	if (ads::CDockWidget* dockWidget = mDockManager->findDockWidget(ToolboxName))
	{
		if (dockWidget->isVisible())
		{
			return;
		}
		mDockManager->layout()->removeWidget(dockWidget);
	}

	QWidget* containerWidget = HS_NEW QWidget();
	ads::CDockWidget* dockWidget = HS_NEW ads::CDockWidget(QString("Entities List"));
	dockWidget->setObjectName(ToolboxName);
	dockWidget->setWidget(containerWidget);
	dockWidget->setToggleViewActionMode(ads::CDockWidget::ActionModeShow);
	dockWidget->setIcon(mMainWindow->style()->standardIcon(QStyle::SP_DialogOpenButton));
	dockWidget->setFeature(ads::CDockWidget::DockWidgetClosable, true);
	dockWidget->setFeature(ads::CDockWidget::DockWidgetMovable, true);
	mDockManager->addDockWidget(ads::RightDockWidgetArea, dockWidget);

	containerWidget->setObjectName(ContainerName);

	QVBoxLayout* layout = HS_NEW QVBoxLayout();
	containerWidget->setLayout(layout);
	QListWidget* entityList = HS_NEW QListWidget();
	layout->addWidget(entityList);
	entityList->setObjectName(ListName);
	entityList->setContextMenuPolicy(Qt::CustomContextMenu);

	QObject::connect(entityList, &QListWidget::currentItemChanged, this, &EntitiesListToolbox::onCurrentItemChanged);
	QObject::connect(entityList, &QListWidget::customContextMenuRequested, this, &EntitiesListToolbox::showContextMenu);

	bindEvents();
}

void EntitiesListToolbox::onWorldUpdated()
{
	bindEvents();
}

void EntitiesListToolbox::onEntityChangedEvent(const std::optional<EditorEntityReference>& entity)
{
	if (!entity.has_value())
	{
		return;
	}

	QListWidget* entitiesList = mDockManager->findChild<QListWidget*>(ListName);
	if (entitiesList == nullptr)
	{
		return;
	}

	const QString text = QString::number(entity->editorUniqueId);
	int i = 0;
	while (QListWidgetItem* item = entitiesList->item(i))
	{
		if (item->text() == text)
		{
			entitiesList->blockSignals(true);
			entitiesList->setCurrentItem(item);
			entitiesList->blockSignals(false);
			break;
		}
		++i;
	}
}

void EntitiesListToolbox::updateContent()
{
	World* currentWorld = mMainWindow->getCurrentWorld();
	if (currentWorld == nullptr)
	{
		return;
	}

	if (QListWidget* entitiesList = mDockManager->findChild<QListWidget*>(ListName))
	{
		currentWorld->getEntityManager().forEachComponentSet<const EditorIdComponent>(
			[entitiesList](const EditorIdComponent* editorIdComponent) {
				QListWidgetItem* newItem = HS_NEW QListWidgetItem(QString::number(editorIdComponent->getId()));
				newItem->setData(0, static_cast<unsigned long long>(editorIdComponent->getId()));
				newItem->setData(1, false);
				entitiesList->addItem(newItem);
			}
		);
	}
}

void EntitiesListToolbox::onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* /*previous*/)
{
	if (current)
	{
		const size_t editorId = current->data(0).toUInt();
		EditorEntityReference reference{ editorId };

		if (current->data(1).toBool())
		{
			CellPos cellPos{ current->data(2).toInt(), current->data(3).toInt() };
			reference.cellPos = cellPos;
		}
		mMainWindow->OnSelectedEntityChanged.broadcast(reference);
	}
	else
	{
		mMainWindow->OnSelectedEntityChanged.broadcast(std::nullopt);
	}
}

void EntitiesListToolbox::showContextMenu(const QPoint& pos)
{
	const QListWidget* entitiesList = mDockManager->findChild<QListWidget*>(ListName);
	if (entitiesList == nullptr)
	{
		return;
	}

	if (!entitiesList->currentItem())
	{
		return;
	}

	QMenu contextMenu(tr("Context menu"), this);

	QAction actionRemove("Remove Entity", this);
	connect(&actionRemove, &QAction::triggered, this, &EntitiesListToolbox::removeSelectedEntity);
	contextMenu.addAction(&actionRemove);

	QAction actionCreatePrefab("Create Prefab", this);
	connect(&actionCreatePrefab, &QAction::triggered, this, &EntitiesListToolbox::createPrefabRequested);
	contextMenu.addAction(&actionCreatePrefab);

	contextMenu.exec(entitiesList->mapToGlobal(pos));
}

void EntitiesListToolbox::removeSelectedEntity()
{
	const QListWidget* entitiesList = mDockManager->findChild<QListWidget*>(ListName);
	if (entitiesList == nullptr)
	{
		return;
	}

	const QListWidgetItem* currentItem = entitiesList->currentItem();
	if (currentItem == nullptr)
	{
		return;
	}

	const CommandExecutionContext commandExecutionContext = mMainWindow->getCommandExecutionContext();
	if (commandExecutionContext.world == nullptr)
	{
		return;
	}

	mMainWindow->getCommandStack().executeNewCommand<RemoveEntitiesCommand>(
		commandExecutionContext,
		std::vector<EditorEntityReference>{ EditorEntityReference{ currentItem->text().toUInt() } },
		mMainWindow->getComponentSerializationHolder()
	);
}

void EntitiesListToolbox::createPrefabRequested()
{
	QInputDialog* dialog = HS_NEW QInputDialog();
	dialog->setLabelText("Choose a name for the prefab:");
	dialog->setCancelButtonText("Cancel");
	connect(dialog, &QInputDialog::textValueSelected, this, &EntitiesListToolbox::createPrefab);
	dialog->show();
}

void EntitiesListToolbox::createPrefab(const QString& prefabName)
{
	const QListWidget* entitiesList = mDockManager->findChild<QListWidget*>(ListName);
	if (entitiesList == nullptr)
	{
		return;
	}

	const QListWidgetItem* currentItem = entitiesList->currentItem();
	if (currentItem == nullptr)
	{
		return;
	}

	PrefabListToolbox* prefabToolbox = mMainWindow->getPrefabToolbox();
	if (prefabToolbox == nullptr)
	{
		return;
	}

	prefabToolbox->show();

	const size_t editorId = currentItem->text().toUInt();

	const OptionalEntity entity = Utils::GetEntityFromId(editorId, mMainWindow->getCurrentWorld()->getEntityManager());

	if (!entity.isValid())
	{
		return;
	}

	prefabToolbox->createPrefabFromEntity(prefabName, entity.getEntity());
}

void EntitiesListToolbox::bindEvents()
{
	if (World* currentWorld = mMainWindow->getCurrentWorld())
	{
		EntityManager& worldEntityManager = currentWorld->getEntityManager();
		mOnEntityAddedHandle = worldEntityManager.onEntityAdded.bind([this] { updateContent(); });
		mOnEntityRemovedHandle = worldEntityManager.onEntityRemoved.bind([this] { updateContent(); });
	}
}

void EntitiesListToolbox::unbindEvents()
{
	if (World* currentWorld = mMainWindow->getCurrentWorld())
	{
		EntityManager& worldEntityManager = currentWorld->getEntityManager();
		worldEntityManager.onEntityAdded.unbind(mOnEntityAddedHandle);
		worldEntityManager.onEntityRemoved.unbind(mOnEntityRemovedHandle);
	}
}
