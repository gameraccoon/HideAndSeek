#pragma once

#include <thread>

#include "GameLogic/Game/Game.h"

#ifdef IMGUI_ENABLED
#include "GameLogic/Imgui/ImguiDebugData.h"
#endif

class RenderAccessor;

class HapGame : public Game
{
public:
	using Game::Game;

	void preStart(ArgumentsParser& arguments, RenderAccessor& renderAccessor);
	void initResources() override;

private:
	void initSystems();

#ifdef IMGUI_ENABLED
	ImguiDebugData mImguiDebugData{getWorldHolder(), getTime(), getComponentFactory(), {}};
#endif // IMGUI_ENABLED
};
