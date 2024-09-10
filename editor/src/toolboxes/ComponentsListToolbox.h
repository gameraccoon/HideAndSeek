#pragma once

#include <QString>
#include <QWidget>

#include "src/editorutils/componentreference.h"
#include <raccoon-ecs/delegates.h>

class MainWindow;
class QPushButton;

namespace ads
{
	class CDockManager;
}

class QListWidgetItem;

class ComponentsListToolbox final : public QWidget
{
public:
	ComponentsListToolbox(MainWindow* mainWindow, ads::CDockManager* dockManager);
	~ComponentsListToolbox() override;
	void show();

	static const QString WidgetName;
	static const QString ToolboxName;
	static const QString ContainerName;
	static const QString ContainerContentName;
	static const QString ListName;

private:
	void updateContent();
	void onCurrentItemChanged(QListWidgetItem* current, QListWidgetItem* previous);
	void onSelectedComponentSourceChanged(const std::optional<ComponentSourceReference>& newSource);
	void showContextMenu(const QPoint& pos);
	void removeSelectedComponent();
	void addComponentCommand();
	void addComponent(const QString& typeName);

private:
	MainWindow* mMainWindow = nullptr;
	ads::CDockManager* mDockManager = nullptr;
	QPushButton* mAddComponentButton = nullptr;
	std::optional<ComponentSourceReference> mLastSelectedComponentSource;

	RaccoonEcs::Delegates::Handle mOnWorldChangedHandle;
	RaccoonEcs::Delegates::Handle mOnComponentSourceChangedHandle;
};
