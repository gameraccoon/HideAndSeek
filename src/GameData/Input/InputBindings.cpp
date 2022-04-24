#include "Base/precomp.h"

#include "GameData/Input/InputBindings.h"

#include <algorithm>

namespace Input
{
	PressSingleButtonKeyBinding::PressSingleButtonKeyBinding(ControllerType controllerType, int button)
		: mControllerType(controllerType)
		, mButton(button)
	{

	}

	bool PressSingleButtonKeyBinding::isActive(const PlayerControllerStates& controllerStates) const
	{
		const auto& it = controllerStates.find(mControllerType);
		if (it == controllerStates.end())
		{
			return false;
		}

		return it->second.isButtonPressed(mButton);
	}

	PressButtonChordKeyBinding::PressButtonChordKeyBinding(ControllerType controllerType, const std::vector<int>& buttons)
		: mControllerType(controllerType)
		, mButtons(buttons)
	{}

	bool PressButtonChordKeyBinding::isActive(const PlayerControllerStates& controllerStates) const
	{
		if (mButtons.empty())
		{
			return false;
		}

		const auto& it = controllerStates.find(mControllerType);
		if (it == controllerStates.end())
		{
			return false;
		}

		return std::all_of(mButtons.begin(), mButtons.end(), [&state = it->second](int button)
		{
			return state.isButtonPressed(button);
		});
	}

	PositiveButtonAxisBinding::PositiveButtonAxisBinding(ControllerType controllerType, int button)
		: mControllerType(controllerType)
		, mButton(button)
	{}

	float PositiveButtonAxisBinding::getAxisValue(const PlayerControllerStates& controllerStates) const
	{
		const auto& it = controllerStates.find(mControllerType);
		return (it != controllerStates.end() && it->second.isButtonPressed(mButton)) ? 1.0f : 0.0f;
	}

	NegativeButtonAxisBinding::NegativeButtonAxisBinding(ControllerType controllerType, int button)
		: mControllerType(controllerType)
		, mButton(button)
	{}

	float NegativeButtonAxisBinding::getAxisValue(const PlayerControllerStates& controllerStates) const
	{
		const auto& it = controllerStates.find(mControllerType);
		return (it != controllerStates.end() && it->second.isButtonPressed(mButton)) ? -1.0f : 0.0f;
	}

	DirectAxisToAxisBinding::DirectAxisToAxisBinding(ControllerType controllerType, int axis)
		: mControllerType(controllerType)
		, mAxis(axis)
	{}

	float DirectAxisToAxisBinding::getAxisValue(const PlayerControllerStates& controllerStates) const
	{
		const auto& it = controllerStates.find(mControllerType);
		return (it != controllerStates.end()) ? it->second.getAxisValue(mAxis) : 0.0f;
	}

	bool InputBindings::IsKeyPressed(const std::vector<std::unique_ptr<KeyBinding>>& bindings, const PlayerControllerStates& controllerStates)
	{
		return std::any_of(bindings.begin(), bindings.end(), [&controllerStates](const std::unique_ptr<KeyBinding>& binding)
		{
			return binding->isActive(controllerStates);
		});
	}

	float InputBindings::GetBlendedAxisValue(const std::vector<std::unique_ptr<AxisBinding>>& bindings, const PlayerControllerStates& controllerStates)
	{
		float accumulatedValue = 0.0f;
		int validBindings = 0;
		for (const std::unique_ptr<AxisBinding>& binding : bindings)
		{
			const float axisValue = binding->getAxisValue(controllerStates);
			if (axisValue != 0.0f)
			{
				accumulatedValue += axisValue;
				++validBindings;
			}
		}

		if (validBindings == 0)
		{
			return 0.0f;
		}

		return accumulatedValue / validBindings;
	}
}
