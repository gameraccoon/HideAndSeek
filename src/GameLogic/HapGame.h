#pragma once

#include <thread>

#include "GameLogic/Game.h"

#ifdef IMGUI_ENABLED
#include "GameLogic/Imgui/ImguiDebugData.h"
#endif

class HapGame : public Game
{
public:
	HapGame(int width, int height);

	void start(ArgumentsParser& arguments);
	void initResources() override;

private:
	void initSystems();

#ifdef IMGUI_ENABLED
	ImguiDebugData mImguiDebugData{getWorldHolder(), getTime(), getComponentFactory(), {}};
#endif // IMGUI_ENABLED
};
