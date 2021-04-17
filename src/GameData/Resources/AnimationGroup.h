#pragma once

#include <map>
#include <vector>
#include <string>

#include "GameData/Core/ResourceHandle.h"
#include "GameData/FSM/Blackboard.h"

template <typename StateIdType>
class AnimationGroup
{
public:
	StateIdType currentState;
	std::map<StateIdType, std::vector<ResourceHandle>> animationClips;
	StringId stateMachineId;
	size_t animationClipIdx = 0;
};
