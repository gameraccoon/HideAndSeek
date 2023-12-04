#pragma once

#include "GameData/Input/ControllerState.h"
#include "GameData/Input/ControllerType.h"

namespace Input
{
	class InputBindings;

	struct PlayerControllerStates
	{
		// these values are not universal, feel free to add more if needed

		// supported values (scancodes)
		static constexpr size_t KeyboardButtonCount = 512;
		static constexpr size_t MouseButtonCount = 6;
		static constexpr size_t GamepadButtonCount = 21;

		// supported axes count for each controller type
		static constexpr size_t KeyboardAxesCount = 0;
		static constexpr size_t MouseAxesCount = 2;
		static constexpr size_t GamepadAxesCount = 8;

		using KeyboardState = ControllerState<KeyboardButtonCount, KeyboardAxesCount>;
		using MouseState = ControllerState<MouseButtonCount, MouseAxesCount>;
		using GamepadState = ControllerState<GamepadButtonCount, GamepadAxesCount>;

		KeyboardState keyboardState;
		MouseState mouseState;
		GamepadState gamepadState;

		BaseControllerState& getState(ControllerType controllerType) noexcept
		{
			switch (controllerType)
			{
			case ControllerType::Keyboard:
				return keyboardState;
			case ControllerType::Mouse:
				return mouseState;
			case ControllerType::Gamepad:
				return gamepadState;
			default:
				AssertFatal(false, "Invalid controller type specified");
				return keyboardState;
			}
		}

		const BaseControllerState& getState(ControllerType controllerType) const noexcept
		{
			switch (controllerType)
			{
			case ControllerType::Keyboard:
				return keyboardState;
			case ControllerType::Mouse:
				return mouseState;
			case ControllerType::Gamepad:
				return gamepadState;
			default:
				AssertFatal(false, "Invalid controller type specified");
				return keyboardState;
			}
		}

		void clearLastFrameState() noexcept
		{
			keyboardState.clearLastFrameState();
			mouseState.clearLastFrameState();
			gamepadState.clearLastFrameState();
		}
	};
}
