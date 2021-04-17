#include "Base/precomp.h"

#include "HAL/Graphics/AnimationGroup.h"

#include "../Internal/SdlSurface.h"
#include "HAL/Base/Engine.h"

namespace Graphics
{
	AnimationGroup::AnimationGroup(std::map<StringId, std::vector<ResourceHandle>>&& animationClips, StringId stateMachineId, StringId defaultState)
		: mAnimationClips(animationClips)
		, mStateMachineId(stateMachineId)
		, mDefaultState(defaultState)
	{
	}

	bool AnimationGroup::isValid() const
	{
		return mStateMachineId.isValid();
	}
}
