#pragma once

#include <vector>
#include <string>

#include "GameData/Geometry/ResourceHandle.h"
#include "GameData/FSM/Blackboard.h"

template <typename StateIDType>
class AnimationState
{
public:
	StateIDType currentState;
	std::vector<std::pair<StateIDType, ResourceHandle>> animations;
	std::string stateMachineId;
	int animationIdx;
};
