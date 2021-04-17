#pragma once

#include <memory>
#include <vector>
#include <map>

#include "GameData/Core/ResourceHandle.h"

#include "HAL/EngineFwd.h"
#include "HAL/Base/Resource.h"
#include "HAL/Base/Types.h"
#include "HAL/Graphics/Sprite.h"

namespace Graphics
{
	class AnimationGroup : public HAL::Resource
	{
	public:
		AnimationGroup() = default;
		explicit AnimationGroup(std::map<StringId, std::vector<ResourceHandle>>&& animationClips, StringId stateMachineId, StringId defaultState);

		[[nodiscard]] bool isValid() const override;
		[[nodiscard]] StringId getStateMachineId() const { return mStateMachineId; }
		[[nodiscard]] std::map<StringId, std::vector<ResourceHandle>> getAnimationClips() const { return mAnimationClips; }
		[[nodiscard]] StringId getDefaultState() const { return mDefaultState; }

	private:
		std::map<StringId, std::vector<ResourceHandle>> mAnimationClips;
		StringId mStateMachineId;
		StringId mDefaultState;
	};
}
