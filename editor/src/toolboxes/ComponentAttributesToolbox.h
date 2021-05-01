#pragma once

#include <optional>

#include <QString>
#include <QWidget>

#include "ECS/Delegates.h"

#include "src/editorcommands/editorcommand.h"
#include "src/editorutils/componentreference.h"

class MainWindow;

namespace ads
{
	class CDockManager;
}

class ComponentAttributesToolbox : public QWidget
{
public:
	ComponentAttributesToolbox(MainWindow* mainWindow, ads::CDockManager* dockManager);
	~ComponentAttributesToolbox();
	void show();
	bool isShown() const;

	static const QString WidgetName;
	static const QString ToolboxName;
	static const QString ContainerName;
	static const QString ContainerContentName;

private:
	void updateContent(EditorCommand::EffectBitset effects, bool originalCall);
	void clearContent();
	void onSelectedComponentChange(const std::optional<ComponentReference>& componentReference);

private:
	MainWindow* mMainWindow;
	ads::CDockManager* mDockManager;
	std::optional<ComponentReference> mLastSelectedComponent;

	Ecs::Delegates::Handle mOnComponentChangedHandle;
	Ecs::Delegates::Handle mOnCommandEffectHandle;
};
