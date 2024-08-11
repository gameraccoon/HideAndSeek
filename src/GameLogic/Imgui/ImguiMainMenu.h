#pragma once

#ifdef IMGUI_ENABLED

#include "GameLogic/Imgui/ComponentInspector/ImguiComponentInspectorWindow.h"
#include "GameLogic/Imgui/ImguiRenderModeWindow.h"
#include "GameLogic/Imgui/ImguiSystemsTimeReportWindow.h"

class ImguiMainMenu
{
public:
	void update(struct ImguiDebugData& debugData);

private:
	bool mShowImguiDemoWindow = false;
	ImguiRenderModeWindow mRenderModeWindow;
	ImguiSystemsTimeReportWindow mSystemsTimeReportWindow;
	ImguiComponentInspectorWindow mComponentInspectorWindow;
};

#endif // IMGUI_ENABLED
