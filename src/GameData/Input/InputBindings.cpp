#include "EngineCommon/precomp.h"

#include "GameData/Input/InputBindings.h"

#include <algorithm>

namespace Input
{
	PressSingleButtonKeyBinding::PressSingleButtonKeyBinding(const ControllerType controllerType, const int button)
		: mControllerType(controllerType)
		, mButton(button)
	{
		AssertFatal(mControllerType < ControllerType::Count, "Invalid controller type specified");
	}

	GameplayInput::KeyState PressSingleButtonKeyBinding::getState(const PlayerControllerStates& controllerStates) const
	{
		using namespace GameplayInput;

		const auto& state = controllerStates.getState(mControllerType);

		if (state.isButtonPressed(mButton))
		{
			return state.isButtonJustPressed(mButton) ? KeyState::JustActivated : KeyState::Active;
		}
		else
		{
			return state.isButtonJustReleased(mButton) ? KeyState::JustDeactivated : KeyState::Inactive;
		}
	}

	PressButtonChordKeyBinding::PressButtonChordKeyBinding(const ControllerType controllerType, const std::vector<int>& buttons)
		: mControllerType(controllerType)
		, mButtons(buttons)
	{}

	GameplayInput::KeyState PressButtonChordKeyBinding::getState(const PlayerControllerStates& controllerStates) const
	{
		using namespace GameplayInput;

		if (mButtons.empty())
		{
			return KeyState::Inactive;
		}

		const auto& state = controllerStates.getState(mControllerType);

		size_t countPressed = 0;
		// note that some keys can be justPressed and justReleased during the same frame
		int countJustPressed = 0;
		int countJustReleased = 0;
		int flipFloppedDownUp = 0;
		int flipFloppedUpDown = 0;

		for (const int button : mButtons)
		{
			// none of these three are exclusive
			const bool isPressed = state.isButtonPressed(button);
			const bool isJustPressed = state.isButtonJustPressed(button);
			const bool isJustReleased = state.isButtonJustReleased(button);

			// to manage special cases when one button changed state twice in one frame
			const bool flipFlopDownUp = isJustPressed && isJustReleased && !isPressed;
			const bool flipFlopUpDown = isJustPressed && isJustReleased && isPressed;

			countPressed += isPressed ? 1 : 0;
			countJustPressed += isJustPressed ? 1 : 0;
			countJustReleased += isJustReleased ? 1 : 0;
			flipFloppedDownUp += flipFlopDownUp ? 1 : 0;
			flipFloppedUpDown += flipFlopUpDown ? 1 : 0;
		}

		if (countPressed == mButtons.size()) // all pressed
		{
			if (countJustPressed - countJustReleased + flipFloppedUpDown > 0)
			{
				return KeyState::JustActivated;
			}
			return KeyState::Active;
		}
		else
		{
			// the chord was pressed last frame
			if (static_cast<size_t>(countPressed - countJustPressed + countJustReleased + flipFloppedDownUp) == mButtons.size())
			{
				return KeyState::JustDeactivated;
			}
			return KeyState::Inactive;
		}
	}

	PositiveButtonAxisBinding::PositiveButtonAxisBinding(const ControllerType controllerType, const int button)
		: mControllerType(controllerType)
		, mButton(button)
	{}

	float PositiveButtonAxisBinding::getAxisValue(const PlayerControllerStates& controllerStates) const
	{
		const auto& state = controllerStates.getState(mControllerType);
		return state.isButtonPressed(mButton) ? 1.0f : 0.0f;
	}

	NegativeButtonAxisBinding::NegativeButtonAxisBinding(const ControllerType controllerType, const int button)
		: mControllerType(controllerType)
		, mButton(button)
	{}

	float NegativeButtonAxisBinding::getAxisValue(const PlayerControllerStates& controllerStates) const
	{
		const auto& state = controllerStates.getState(mControllerType);
		return state.isButtonPressed(mButton) ? -1.0f : 0.0f;
	}

	DirectAxisToAxisBinding::DirectAxisToAxisBinding(const ControllerType controllerType, const int axis)
		: mControllerType(controllerType)
		, mAxis(axis)
	{}

	float DirectAxisToAxisBinding::getAxisValue(const PlayerControllerStates& controllerStates) const
	{
		const auto& state = controllerStates.getState(mControllerType);
		return state.getAxisValue(mAxis);
	}

	GameplayInput::KeyState InputBindings::GetKeyState(const std::vector<std::unique_ptr<KeyBinding>>& bindings, const PlayerControllerStates& controllerStates)
	{
		using namespace GameplayInput;

		// the most common case, also the easiest
		if (bindings.size() == 1)
		{
			return bindings.front()->getState(controllerStates);
		}

		std::array<int, 4> states{};
		for (const std::unique_ptr<KeyBinding>& binding : bindings)
		{
			++states[static_cast<size_t>(binding->getState(controllerStates))];
		}

		// we have at least one active or activated binding at this frame
		if ((states[static_cast<size_t>(KeyState::Active)] + states[static_cast<size_t>(KeyState::JustActivated)]) > 0)
		{
			// we didn't have active bindings last frame
			if ((states[static_cast<size_t>(KeyState::Active)] + states[static_cast<size_t>(KeyState::JustDeactivated)]) == 0)
			{
				return KeyState::JustActivated;
			}

			// at least one of the bindings was active last frame
			return KeyState::Active;
		}
		else
		{
			// at least one of the bindings was active last frame
			if (states[static_cast<size_t>(KeyState::JustDeactivated)] > 0)
			{
				return KeyState::JustDeactivated;
			}
			return KeyState::Inactive;
		}
	}

	float InputBindings::GetBlendedAxisValue(const std::vector<std::unique_ptr<AxisBinding>>& bindings, const PlayerControllerStates& controllerStates)
	{
		float maxPositive = 0.0f;
		float maxNegative = 0.0f;
		for (const std::unique_ptr<AxisBinding>& binding : bindings)
		{
			const float axisValue = binding->getAxisValue(controllerStates);
			maxPositive = std::max(maxPositive, axisValue);
			maxNegative = std::max(maxNegative, -axisValue);
		}

		return maxPositive - maxNegative;
	}
} // namespace Input
