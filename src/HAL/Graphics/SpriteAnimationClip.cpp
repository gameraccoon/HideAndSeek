#include "Base/precomp.h"

#include "HAL/Graphics/SpriteAnimationClip.h"

#include <algorithm>
#include <filesystem>

#include <nlohmann/json.hpp>

#include "Base/Types/String/Path.h"

#include "HAL/Base/Engine.h"
#include "HAL/Base/ResourceManager.h"
#include "HAL/Internal/SdlSurface.h"
#include "HAL/Graphics/Sprite.h"

namespace Graphics
{
	static std::vector<ResourcePath> LoadSpriteAnimClipData(const ResourcePath& path)
	{
		SCOPED_PROFILER("ResourceManager::loadSpriteAnimClipData");
		namespace fs = std::filesystem;
		fs::path atlasDescPath(static_cast<std::string>(path));

		std::vector<ResourcePath> result;
		ResourcePath pathBase;
		int framesCount = 0;

		try
		{
			std::ifstream animDescriptionFile(atlasDescPath);
			nlohmann::json animJson;
			animDescriptionFile >> animJson;

			animJson.at("path").get_to(pathBase);
			animJson.at("frames").get_to(framesCount);
		}
		catch(const std::exception& e)
		{
			LogError("Can't open animation data '%s': %s", path.c_str(), e.what());
		}

		for (int i = 0; i < framesCount; ++i)
		{
			result.emplace_back(pathBase + std::to_string(i) + ".png");
		}

		return result;
	}

	static UniqueAny CreateAnimationClip(UniqueAny&& resource, HAL::ResourceManager& resourceManager, ResourceHandle handle)
	{
		SCOPED_PROFILER("CreateAnimationClip");

		const ResourcePath* pathPtr = resource.cast<ResourcePath>();

		if (!pathPtr)
		{
			ReportFatalError("We got an incorrect type of value when loading a sprite in CreateAnimationClip (expected ResourcePath)");
			return {};
		}

		const ResourcePath& path = *pathPtr;

		std::vector<ResourcePath> framePaths = LoadSpriteAnimClipData(path);
		std::vector<ResourceHandle> frames;
		for (const auto& animFramePath : framePaths)
		{
			ResourceHandle spriteHandle = resourceManager.lockResource<Sprite>(animFramePath);
			resourceManager.setFirstResourceDependOnSecond(handle, spriteHandle);
			frames.push_back(spriteHandle);
		}

		return UniqueAny::Create<HAL::Resource::Ptr>(std::make_unique<Graphics::SpriteAnimationClip>(std::move(frames)));
	}

	SpriteAnimationClip::SpriteAnimationClip(std::vector<ResourceHandle>&& sprites)
		: mSprites(std::move(sprites))
	{
	}

	bool SpriteAnimationClip::isValid() const
	{
		return !mSprites.empty();
	}

	const ResourceHandle& SpriteAnimationClip::getSprite(float progress) const
	{
		return mSprites[std::min(mSprites.size()*static_cast<size_t>(progress), mSprites.size() - 1)];
	}

	const std::vector<ResourceHandle> &SpriteAnimationClip::getSprites() const
	{
		return mSprites;
	}

	std::string SpriteAnimationClip::GetUniqueId(const std::string& filename)
	{
		return filename;
	}

	HAL::Resource::InitSteps SpriteAnimationClip::GetInitSteps()
	{
		return {
			InitStep{
				.thread = Thread::Any,
				.init = &CreateAnimationClip,
			},
		};
	}

	HAL::Resource::DeinitSteps SpriteAnimationClip::getDeinitSteps() const
	{
		return {};
	}
}
