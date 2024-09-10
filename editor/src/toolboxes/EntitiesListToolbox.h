#pragma once

#include <QString>
#include <QWidget>

#include "src/editorutils/editorentityreference.h"
#include <raccoon-ecs/delegates.h>

class MainWindow;

namespace ads
{
	class CDockManager;
}

class QListWidgetItem;
class QMenu;
class QPoint;

class EntitiesListToolbox final : public QWidget
{
public:
	EntitiesListToolbox(MainWindow* mainWindow, ads::CDockManager* dockManager);
	~EntitiesListToolbox() override;
	void show();

	static const QString WidgetName;
	static const QString ToolboxName;
	static const QString ContainerName;
	static const QString ContainerContentName;
	static const QString ListName;

private:
	void onWorldUpdated();
	void onEntityChangedEvent(const std::optional<EditorEntityReference>& entity);
	void updateContent();
	void onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
	void showContextMenu(const QPoint& pos);
	void removeSelectedEntity();
	void createPrefabRequested();
	void createPrefab(const QString& prefabName);
	void bindEvents();
	void unbindEvents();

private:
	MainWindow* mMainWindow;
	ads::CDockManager* mDockManager;

	RaccoonEcs::Delegates::Handle mOnEntityAddedHandle;
	RaccoonEcs::Delegates::Handle mOnEntityRemovedHandle;
	RaccoonEcs::Delegates::Handle mOnWorldChangedHandle;
	RaccoonEcs::Delegates::Handle mOnSelectedEntityChangedHandle;
};
