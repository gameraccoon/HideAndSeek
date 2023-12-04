#pragma once

#include <unordered_map>

#include "GameData/Geometry/Vector2D.h"
#include "GameData/Input/ControllerState.h"
#include "GameData/Input/GameplayInputConstants.h"
#include "GameData/Input/PlayerControllerStates.h"

namespace HAL
{
	class InputControllersData
	{
	public:
		void resetLastFrameStates()
		{
			controllerStates.clearLastFrameState();
		}

	public:
		Input::PlayerControllerStates controllerStates;
	};
}
