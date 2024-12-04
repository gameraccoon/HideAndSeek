#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "EngineData/Input/InputBindingBase.h"

#include "GameData/Input/GameplayInputConstants.h"

namespace Input
{
	struct PlayerControllerStates;

	struct InputBindings
	{
		using KeyBindingList = std::vector<std::unique_ptr<KeyBinding>>;
		using AxisBindingList = std::vector<std::unique_ptr<AxisBinding>>;
		using KeyBindingMap = std::unordered_map<GameplayInput::InputKey, KeyBindingList>;
		using AxisBindingMap = std::unordered_map<GameplayInput::InputAxis, AxisBindingList>;

		KeyBindingMap keyBindings;
		AxisBindingMap axisBindings;

		/**
		 * returns a state which is a consensus between key states from across all bindings
		 */
		static GameplayInput::KeyState GetKeyState(const std::vector<std::unique_ptr<KeyBinding>>& bindings, const PlayerControllerStates& controllerStates);

		/**
		 * returns an average value from multiple bindings
		 * the axes with zero value don't participate in the calculation
		 */
		static float GetBlendedAxisValue(const std::vector<std::unique_ptr<AxisBinding>>& bindings, const PlayerControllerStates& controllerStates);
	};
} // namespace Input
