#include "Base/precomp.h"

#include "GameLogic/Resources/AnimationGroup.h"

#include <filesystem>

#include <nlohmann/json.hpp>

#include "Base/Types/String/Path.h"

#include "HAL/Base/Engine.h"
#include "HAL/Graphics/SdlSurface.h"

#include "GameLogic/Resources/SpriteAnimationClip.h"

#include "Utils/ResourceManagement/ResourceManager.h"

namespace Graphics
{
	struct AnimGroupData
	{
		std::map<StringId, ResourcePath> clips;
		StringId stateMachineID;
		StringId defaultState;
	};

	struct AnimGroupLoadData
	{
		std::map<StringId, ResourceHandle> clips;
		StringId stateMachineID;
		StringId defaultState;
	};

	static AnimGroupData LoadAnimGroupData(const ResourcePath& path)
	{
		SCOPED_PROFILER("ResourceManager::loadAnimGroupData");
		namespace fs = std::filesystem;
		fs::path atlasDescPath(static_cast<std::string>(path));

		AnimGroupData result;
		ResourcePath pathBase;

		try
		{
			std::ifstream animDescriptionFile(atlasDescPath);
			nlohmann::json animJson;
			animDescriptionFile >> animJson;

			animJson.at("clips").get_to(result.clips);
			animJson.at("stateMachineID").get_to(result.stateMachineID);
			animJson.at("defaultState").get_to(result.defaultState);
		}
		catch(const std::exception& e)
		{
			LogError("Can't open animation group data '%s': %s", path.c_str(), e.what());
		}

		return result;
	}

	static UniqueAny CalculateAnimationGroupDependencies(UniqueAny&& resource, ResourceManager& resourceManager, ResourceHandle handle)
	{
		SCOPED_PROFILER("CalculateSpriteDependencies");

		const ResourcePath* pathPtr = resource.cast<ResourcePath>();

		if (!pathPtr)
		{
			ReportFatalError("We got an incorrect type of value when loading a sprite in CalculateSpriteDependencies (expected ResourcePath)");
			return {};
		}

		const ResourcePath& path = *pathPtr;

		AnimGroupData animGroupData = LoadAnimGroupData(path);

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
			animClips.emplace(std::move(clipPath), resourceManager.tryGetResource<Graphics::SpriteAnimationClip>(clipHandle)->getSprites());
		}

		return UniqueAny::Create<Resource::Ptr>(std::make_unique<Graphics::AnimationGroup>(std::move(animClips), animGroupLoadData->stateMachineID, animGroupLoadData->defaultState));
	}

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

	std::string AnimationGroup::GetUniqueId(const std::string& filename)
	{
		return filename;
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
}
