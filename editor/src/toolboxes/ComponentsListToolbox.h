#pragma once

#include <QString>
#include <QWidget>

#include "ECS/Delegates.h"
#include "ECS/Entity.h"

#include "src/editorutils/entityreference.h"

class MainWindow;

namespace ads
{
	class CDockManager;
}

class QListWidgetItem;

class ComponentsListToolbox : public QWidget
{
public:
	ComponentsListToolbox(MainWindow* mainWindow, ads::CDockManager* dockManager);
	~ComponentsListToolbox();
	void show();

	static const QString WidgetName;
	static const QString ToolboxName;
	static const QString ContainerName;
	static const QString ContainerContentName;
	static const QString ListName;

private:
	void updateContent();
	void onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
	void onSelectedEntityChanged(const std::optional<EntityReference>& newEntity);
	void showContextMenu(const QPoint& pos);
	void removeSelectedComponent();

	void bindEvents();
	void unbindEvents();

private:
	MainWindow* mMainWindow;
	ads::CDockManager* mDockManager;
	std::optional<EntityReference> mLastSelectedEntity;

	Delegates::Handle mOnComponentAddedHandle;
	Delegates::Handle mOnComponentRemovedHandle;
	Delegates::Handle mOnWorldChangedHandle;
	Delegates::Handle mOnEntityChangedHandle;
};
