#include "Base/precomp.h"

#include "GameLogic/Systems/InputSystem.h"

#include <SDL_scancode.h>
#include <SDL_mouse.h>

#include "GameData/Components/GameplayInputComponent.generated.h"
#include "GameData/Components/ImguiComponent.generated.h"
#include "GameData/Components/RenderModeComponent.generated.h"
#include "GameData/Components/TimeComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/GameData.h"
#include "GameData/Input/InputBindings.h"
#include "GameData/World.h"

#include "HAL/InputControllersData.h"

#include "GameLogic/SharedManagers/WorldHolder.h"


InputSystem::InputSystem(WorldHolder& worldHolder, const HAL::InputControllersData& inputData) noexcept
	: mWorldHolder(worldHolder)
	, mInputData(inputData)
{
}

void InputSystem::update()
{
	SCOPED_PROFILER("InputSystem::update");

#ifdef IMGUI_ENABLED
	GameData& gameData = mWorldHolder.getGameData();
	if (auto [imgui] = gameData.getGameComponents().getComponents<ImguiComponent>(); imgui)
	{
		if (imgui->getIsImguiVisible())
		{
			// stop processing input if imgui is shown
			return;
		}
	}
#endif // IMGUI_ENABLED

	processGameplayInput();
}

static Input::InputBindings GetDebugInputBindings()
{
	using namespace Input;

	InputBindings result;

	// yes, this is not efficient, but this is a temporary solution
	result.keyBindings[GameplayInput::InputKey::Shoot].push_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Mouse, SDL_BUTTON_LEFT));
	result.keyBindings[GameplayInput::InputKey::Shoot].push_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Keyboard, SDL_SCANCODE_RCTRL));

	result.keyBindings[GameplayInput::InputKey::Sprint].push_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Keyboard, SDL_SCANCODE_LSHIFT));
	result.keyBindings[GameplayInput::InputKey::Sprint].push_back(std::make_unique<PressSingleButtonKeyBinding>(ControllerType::Keyboard, SDL_SCANCODE_RSHIFT));

	result.axisBindings[GameplayInput::InputAxis::MoveHorizontal].push_back(std::make_unique<NegativeButtonAxisBinding>(ControllerType::Keyboard, SDL_SCANCODE_LEFT));
	result.axisBindings[GameplayInput::InputAxis::MoveHorizontal].push_back(std::make_unique<NegativeButtonAxisBinding>(ControllerType::Keyboard, SDL_SCANCODE_A));
	result.axisBindings[GameplayInput::InputAxis::MoveHorizontal].push_back(std::make_unique<PositiveButtonAxisBinding>(ControllerType::Keyboard, SDL_SCANCODE_RIGHT));
	result.axisBindings[GameplayInput::InputAxis::MoveHorizontal].push_back(std::make_unique<PositiveButtonAxisBinding>(ControllerType::Keyboard, SDL_SCANCODE_D));

	result.axisBindings[GameplayInput::InputAxis::MoveVertical].push_back(std::make_unique<NegativeButtonAxisBinding>(ControllerType::Keyboard, SDL_SCANCODE_UP));
	result.axisBindings[GameplayInput::InputAxis::MoveVertical].push_back(std::make_unique<NegativeButtonAxisBinding>(ControllerType::Keyboard, SDL_SCANCODE_W));
	result.axisBindings[GameplayInput::InputAxis::MoveVertical].push_back(std::make_unique<PositiveButtonAxisBinding>(ControllerType::Keyboard, SDL_SCANCODE_DOWN));
	result.axisBindings[GameplayInput::InputAxis::MoveVertical].push_back(std::make_unique<PositiveButtonAxisBinding>(ControllerType::Keyboard, SDL_SCANCODE_S));

	result.axisBindings[GameplayInput::InputAxis::AimHorizontal].push_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Mouse, 0));
	result.axisBindings[GameplayInput::InputAxis::AimVertical].push_back(std::make_unique<DirectAxisToAxisBinding>(ControllerType::Mouse, 1));

	return result;
}

void InputSystem::processGameplayInput()
{
	SCOPED_PROFILER("InputSystem::processGameplayInput");

	using namespace Input;

	World& world = mWorldHolder.getWorld();
	const PlayerControllerStates& controllerStates = mInputData.controllerStates;

	const auto [time] = world.getWorldComponents().getComponents<const TimeComponent>();
	const GameplayTimestamp currentTimestamp = time->getValue().lastFixedUpdateTimestamp.getIncreasedByUpdateCount(1);

	GameplayInputComponent* gameplayInput = world.getWorldComponents().getOrAddComponent<GameplayInputComponent>();
	GameplayInput::FrameState& gameplayInputState = gameplayInput->getCurrentFrameStateRef();

	// Debug keybindings (static variable just for now, it should be data-driven and stored somewhere)
	static InputBindings gameplayBindings = GetDebugInputBindings();

	for (const auto& pair : gameplayBindings.keyBindings)
	{
		gameplayInputState.updateKey(pair.first, InputBindings::GetKeyState(pair.second, controllerStates), currentTimestamp);
	}

	for (const auto& pair : gameplayBindings.axisBindings)
	{
		gameplayInputState.updateAxis(pair.first, InputBindings::GetBlendedAxisValue(pair.second, controllerStates));
	}
}
