#include "EngineCommon/precomp.h"

#ifndef DISABLE_SDL

#include <filesystem>

#include <nlohmann/json.hpp>

#include "EngineCommon/Types/String/ResourcePath.h"

#include "HAL/Graphics/SdlSurface.h"

#include "GameUtils/ResourceManagement/ResourceManager.h"

#include "GameLogic/Resources/AnimationGroup.h"
#include "GameLogic/Resources/SpriteAnimationClip.h"

namespace Graphics
{
	struct AnimGroupData
	{
		std::map<StringId, RelativeResourcePath> clips;
		StringId stateMachineID;
		StringId defaultState;
	};

	struct AnimGroupLoadData
	{
		std::map<StringId, ResourceHandle> clips;
		StringId stateMachineID;
		StringId defaultState;
	};

	static AnimGroupData LoadAnimGroupData(const AbsoluteResourcePath& path)
	{
		SCOPED_PROFILER("ResourceManager::loadAnimGroupData");

		AnimGroupData result;
		AbsoluteResourcePath pathBase;

		try
		{
			std::ifstream animDescriptionFile(path.getAbsolutePath());
			nlohmann::json animJson;
			animDescriptionFile >> animJson;

			animJson.at("clips").get_to(result.clips);
			animJson.at("stateMachineID").get_to(result.stateMachineID);
			animJson.at("defaultState").get_to(result.defaultState);
		}
		catch (const std::exception& e)
		{
			LogError("Can't open animation group data '%s': %s", path.getAbsolutePath().c_str(), e.what());
		}

		return result;
	}

	static UniqueAny CalculateAnimationGroupDependencies(UniqueAny&& resource, ResourceManager& resourceManager, const ResourceHandle handle)
	{
		SCOPED_PROFILER("CalculateSpriteDependencies");

		const RelativeResourcePath* pathPtr = resource.cast<RelativeResourcePath>();

		if (!pathPtr)
		{
			ReportFatalError("We got an incorrect type of value when loading a sprite in CalculateSpriteDependencies (expected ResourcePath)");
			return {};
		}

		const RelativeResourcePath& path = *pathPtr;

		AnimGroupData animGroupData = LoadAnimGroupData(resourceManager.getAbsoluteResourcePath(path));

		AnimGroupLoadData animGroupLoadData;
		animGroupLoadData.defaultState = animGroupData.defaultState;
		animGroupLoadData.stateMachineID = animGroupData.stateMachineID;
		for (auto&& [animClipPath, resourcePath] : animGroupData.clips)
		{
			ResourceHandle clipHandle = resourceManager.lockResource<SpriteAnimationClip>(resourcePath);
			resourceManager.setFirstResourceDependOnSecond(handle, clipHandle);
			animGroupLoadData.clips.emplace(std::move(animClipPath), clipHandle);
		}

		return UniqueAny::Create<AnimGroupLoadData>(std::move(animGroupLoadData));
	}

	static UniqueAny CreateAnimationGroup(UniqueAny&& resource, ResourceManager& resourceManager, ResourceHandle)
	{
		SCOPED_PROFILER("CreateAnimationGroup");

		const AnimGroupLoadData* animGroupLoadData = resource.cast<AnimGroupLoadData>();

		if (!animGroupLoadData)
		{
			ReportFatalError("We got an incorrect type of value when loading a sprite in CreateAnimationGroup");
			return {};
		}

		std::map<StringId, std::vector<ResourceHandle>> animClips;
		for (auto&& [clipPath, clipHandle] : animGroupLoadData->clips)
		{
			animClips.emplace(std::move(clipPath), resourceManager.tryGetResource<SpriteAnimationClip>(clipHandle)->getSprites());
		}

		return UniqueAny::Create<Resource::Ptr>(std::make_unique<AnimationGroup>(std::move(animClips), animGroupLoadData->stateMachineID, animGroupLoadData->defaultState));
	}

	AnimationGroup::AnimationGroup(std::map<StringId, std::vector<ResourceHandle>>&& animationClips, const StringId stateMachineId, const StringId defaultState)
		: mAnimationClips(animationClips)
		, mStateMachineId(stateMachineId)
		, mDefaultState(defaultState)
	{
	}

	bool AnimationGroup::isValid() const
	{
		return mStateMachineId.isValid();
	}

	std::string AnimationGroup::GetUniqueId(const RelativeResourcePath& filename)
	{
		return filename.getRelativePathStr();
	}

	Resource::InitSteps AnimationGroup::GetInitSteps()
	{
		return {
			InitStep{
				.thread = Thread::Any,
				.init = &CalculateAnimationGroupDependencies,
			},
			InitStep{
				.thread = Thread::Any,
				.init = &CreateAnimationGroup,
			},
		};
	}

	Resource::DeinitSteps AnimationGroup::getDeinitSteps() const
	{
		return {};
	}
} // namespace Graphics

#endif // !DISABLE_SDL
