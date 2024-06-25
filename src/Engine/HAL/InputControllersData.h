#pragma once

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
