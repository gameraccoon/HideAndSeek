#include "EngineCommon/precomp.h"

#ifndef DISABLE_SDL

#include <algorithm>
#include <filesystem>

#include <nlohmann/json.hpp>

#include "EngineCommon/Types/String/ResourcePath.h"

#include "HAL/Graphics/Sprite.h"

#include "GameUtils/ResourceManagement/ResourceManager.h"

#include "GameLogic/Resources/SpriteAnimationClip.h"

namespace Graphics
{
	static std::vector<RelativeResourcePath> LoadSpriteAnimClipData(const AbsoluteResourcePath& path)
	{
		SCOPED_PROFILER("ResourceManager::loadSpriteAnimClipData");

		std::vector<RelativeResourcePath> result;
		RelativeResourcePath pathBase;
		int framesCount = 0;

		try
		{
			std::ifstream animDescriptionFile(path.getAbsolutePath());
			nlohmann::json animJson;
			animDescriptionFile >> animJson;

			animJson.at("path").get_to(pathBase);
			animJson.at("frames").get_to(framesCount);
		}
		catch (const std::exception& e)
		{
			LogError("Can't open animation data '%s': %s", path.getAbsolutePath().c_str(), e.what());
		}

		for (int i = 0; i < framesCount; ++i)
		{
			result.emplace_back(pathBase.getRelativePathStr() + std::to_string(i) + ".png");
		}

		return result;
	}

	static UniqueAny CreateAnimationClip(UniqueAny&& resource, ResourceManager& resourceManager, const ResourceHandle handle)
	{
		SCOPED_PROFILER("CreateAnimationClip");

		const RelativeResourcePath* pathPtr = resource.cast<RelativeResourcePath>();

		if (!pathPtr)
		{
			ReportFatalError("We got an incorrect type of value when loading a sprite in CreateAnimationClip (expected ResourcePath)");
			return {};
		}

		const RelativeResourcePath& path = *pathPtr;

		std::vector<RelativeResourcePath> framePaths = LoadSpriteAnimClipData(resourceManager.getAbsoluteResourcePath(path));
		std::vector<ResourceHandle> frames;
		for (const auto& animFramePath : framePaths)
		{
			ResourceHandle spriteHandle = resourceManager.lockResource<Sprite>(animFramePath);
			resourceManager.setFirstResourceDependOnSecond(handle, spriteHandle, ResourceDependencyType::Unload);
			frames.push_back(spriteHandle);
		}

		return UniqueAny::Create<Resource::Ptr>(std::make_unique<Graphics::SpriteAnimationClip>(std::move(frames)));
	}

	SpriteAnimationClip::SpriteAnimationClip(std::vector<ResourceHandle>&& sprites)
		: mSprites(std::move(sprites))
	{
	}

	bool SpriteAnimationClip::isValid() const
	{
		return !mSprites.empty();
	}

	const ResourceHandle& SpriteAnimationClip::getSprite(const float progress) const
	{
		return mSprites[std::min(mSprites.size() * static_cast<size_t>(progress), mSprites.size() - 1)];
	}

	const std::vector<ResourceHandle>& SpriteAnimationClip::getSprites() const
	{
		return mSprites;
	}

	std::string SpriteAnimationClip::GetUniqueId(const RelativeResourcePath& filename)
	{
		return filename.getRelativePathStr();
	}

	Resource::InitSteps SpriteAnimationClip::GetInitSteps()
	{
		return {
			InitStep{
				.thread = Thread::Any,
				.init = &CreateAnimationClip,
			},
		};
	}

	Resource::DeinitSteps SpriteAnimationClip::getDeinitSteps() const
	{
		return {};
	}
} // namespace Graphics

#endif // !DISABLE_SDL
