#pragma once

#include <optional>

#include <QString>
#include <QWidget>

#include "src/editorcommands/editorcommand.h"
#include "src/editorutils/componentreference.h"
#include <raccoon-ecs/delegates.h>

class MainWindow;

namespace ads
{
	class CDockManager;
}

class ComponentAttributesToolbox final : public QWidget
{
public:
	ComponentAttributesToolbox(MainWindow* mainWindow, ads::CDockManager* dockManager);
	~ComponentAttributesToolbox() override;
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

	RaccoonEcs::Delegates::Handle mOnComponentChangedHandle;
	RaccoonEcs::Delegates::Handle mOnCommandEffectHandle;
};
