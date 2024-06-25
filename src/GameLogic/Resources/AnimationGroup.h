#pragma once

#ifndef DISABLE_SDL

#include <vector>
#include <map>

#include "EngineCommon/Types/String/StringId.h"

#include "GameData/Resources/ResourceHandle.h"
#include "GameData/Resources/Resource.h"

class RelativeResourcePath;

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

		static std::string GetUniqueId(const RelativeResourcePath& filename);
		static InitSteps GetInitSteps();
		DeinitSteps getDeinitSteps() const override;

	private:
		std::map<StringId, std::vector<ResourceHandle>> mAnimationClips;
		StringId mStateMachineId;
		StringId mDefaultState;
	};
}

#endif // !DISABLE_SDL
