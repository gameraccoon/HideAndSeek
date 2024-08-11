#pragma once

#ifndef DISABLE_SDL

#include <map>
#include <vector>

#include "EngineCommon/Types/String/StringId.h"

#include "EngineData/Resources/Resource.h"
#include "EngineData/Resources/ResourceHandle.h"

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
} // namespace Graphics

#endif // !DISABLE_SDL
