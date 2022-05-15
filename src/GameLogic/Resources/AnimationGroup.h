#pragma once

#include <memory>
#include <vector>
#include <map>

#include "GameData/Resources/ResourceHandle.h"
#include "GameData/Resources/Resource.h"

#include "HAL/Graphics/Sprite.h"

namespace Graphics
{
	class AnimationGroup : public Resource
	{
	public:
		AnimationGroup() = default;
		explicit AnimationGroup(std::map<StringId, std::vector<ResourceHandle>>&& animationClips, StringId stateMachineId, StringId defaultState);

		bool isValid() const override;
		StringId getStateMachineId() const { return mStateMachineId; }
		std::map<StringId, std::vector<ResourceHandle>> getAnimationClips() const { return mAnimationClips; }
		StringId getDefaultState() const { return mDefaultState; }

		static std::string GetUniqueId(const std::string& filename);
		static InitSteps GetInitSteps();
		DeinitSteps getDeinitSteps() const override;

	private:
		std::map<StringId, std::vector<ResourceHandle>> mAnimationClips;
		StringId mStateMachineId;
		StringId mDefaultState;
	};
}
