#pragma once

#include <map>
#include <vector>

#include "Base/Types/String/StringId.h"

#include "GameData/Resources/ResourceHandle.h"

template <typename StateIdType>
class AnimationGroup
{
public:
	StateIdType currentState;
	std::map<StateIdType, std::vector<ResourceHandle>> animationClips;
	StringId stateMachineId;
	size_t animationClipIdx = 0;
};
