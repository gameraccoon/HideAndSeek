#include "EngineCommon/precomp.h"

#include "GameLogic/Systems/DebugInputSystem.h"

#ifndef DISABLE_SDL

#include <SDL_scancode.h>

#include "EngineData/Input/ControllerState.h"

#include "GameData/Components/ImguiComponent.generated.h"
#include "GameData/Components/RenderModeComponent.generated.h"
#include "GameData/GameData.h"

#include "HAL/InputControllersData.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

DebugInputSystem::DebugInputSystem(
	WorldHolder& worldHolder,
	const HAL::InputControllersData& inputData,
	bool& shouldPauseGame
) noexcept
	: mWorldHolder(worldHolder)
	, mInputData(inputData)
	, mShouldPauseGame(shouldPauseGame)
{
}

static void UpdateRenderStateOnPressed(const Input::PlayerControllerStates::KeyboardState& keyboardState, const int button, bool& value)
{
	if (keyboardState.isButtonJustPressed(button))
	{
		value = !value;
	}
}

void DebugInputSystem::update()
{
	SCOPED_PROFILER("DebugInputSystem::update");

#ifdef IMGUI_ENABLED
	GameData& gameData = mWorldHolder.getGameData();
	if (auto [imgui] = gameData.getGameComponents().getComponents<ImguiComponent>(); imgui)
	{
		UpdateRenderStateOnPressed(mInputData.controllerStates.keyboardState, SDL_SCANCODE_F1, imgui->getIsImguiVisibleRef());
		if (imgui->getIsImguiVisible())
		{
			// stop processing input if imgui is shown
			return;
		}
	}
#endif // IMGUI_ENABLED

	processDebugInput();
}

void DebugInputSystem::processDebugInput() const
{
	SCOPED_PROFILER("DebugInputSystem::processDebugInput");

	GameData& gameData = mWorldHolder.getGameData();
	const Input::PlayerControllerStates::KeyboardState& keyboardState = mInputData.controllerStates.keyboardState;

	if (keyboardState.isButtonJustPressed(SDL_SCANCODE_ESCAPE))
	{
		mShouldPauseGame = !mShouldPauseGame;
	}

	if (auto [renderMode] = gameData.getGameComponents().getComponents<RenderModeComponent>(); renderMode)
	{
		UpdateRenderStateOnPressed(keyboardState, SDL_SCANCODE_F2, renderMode->getIsDrawDebugCollisionsEnabledRef());
		UpdateRenderStateOnPressed(keyboardState, SDL_SCANCODE_F3, renderMode->getIsDrawBackgroundEnabledRef());
		UpdateRenderStateOnPressed(keyboardState, SDL_SCANCODE_F4, renderMode->getIsDrawLightsEnabledRef());
		UpdateRenderStateOnPressed(keyboardState, SDL_SCANCODE_F5, renderMode->getIsDrawVisibleEntitiesEnabledRef());
		UpdateRenderStateOnPressed(keyboardState, SDL_SCANCODE_F7, renderMode->getIsDrawDebugCharacterInfoEnabledRef());
		UpdateRenderStateOnPressed(keyboardState, SDL_SCANCODE_F8, renderMode->getIsDrawDebugPrimitivesEnabledRef());
		UpdateRenderStateOnPressed(keyboardState, SDL_SCANCODE_F9, renderMode->getIsDrawDebugCellInfoEnabledRef());
	}
}

#endif // !DISABLE_SDL
