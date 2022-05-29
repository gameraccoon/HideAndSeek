#pragma once

#include <thread>

#include "GameLogic/Game/Game.h"

#ifdef IMGUI_ENABLED
#include "GameLogic/Imgui/ImguiDebugData.h"
#endif

class RenderAccessorGameRef;

class HapGame : public Game
{
public:
	using Game::Game;

	void preStart(ArgumentsParser& arguments, const RenderAccessorGameRef& renderAccessor);
	void initResources() override;

	void quitGame() override { mShouldQuit = true; }
	bool shouldQuitGame() const override { return mShouldQuit; }

private:
	void initSystems();

private:
	bool mShouldQuit = false;

#ifdef IMGUI_ENABLED
	ImguiDebugData mImguiDebugData{getWorldHolder(), getTime(), getComponentFactory(), {}};
#endif // IMGUI_ENABLED
};
