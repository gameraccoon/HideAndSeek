#include "EngineCommon/precomp.h"

#include "GameData/Input/InputBindings.h"

#include <algorithm>

namespace Input
{
	GameplayInput::KeyState InputBindings::GetKeyState(const std::vector<std::unique_ptr<KeyBinding>>& bindings, const PlayerControllerStates& controllerStates)
	{
		using namespace GameplayInput;

		// merge all key states, if any was activated, deactivated or is active, the result will reflect all of them
		KeyState result = KeyState::Inactive;
		for (const std::unique_ptr<KeyBinding>& binding : bindings)
		{
			// active but not activated/deactivated takes precedence over any other state
			const KeyState state = binding->getState(controllerStates);
			if (state == KeyState::Active)
			{
				result = state;
				break;
			}

			result = MergeKeyStates(result, state);
		}

		return result;
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
