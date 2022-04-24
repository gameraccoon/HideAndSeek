#pragma once

#include <unordered_map>

#include "GameData/Geometry/Vector2D.h"
#include "GameData/Input/ControllerState.h"

class InputData
{
public:
	void resetLastFrameStates()
	{
		for (auto& controllerState : controllerStates)
		{
			controllerState.second.clearLastFrameState();
		}
	}

public:
	Input::PlayerControllerStates controllerStates;
};
