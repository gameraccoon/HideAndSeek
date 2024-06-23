#pragma once

#ifdef IMGUI_ENABLED

class ImguiRenderModeWindow
{
public:
	void update(const struct ImguiDebugData& debugData);

	bool isVisible = false;
};

#endif // IMGUI_ENABLED
