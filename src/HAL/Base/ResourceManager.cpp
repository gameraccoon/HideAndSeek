#include "Base/precomp.h"

#include "HAL/Base/ResourceManager.h"

#include <fstream>
#include <vector>
#include <filesystem>
#include <string>

#include <nlohmann/json.hpp>

#include "HAL/Internal/SdlSurface.h"

#include "HAL/Audio/Music.h"
#include "HAL/Audio/Sound.h"
#include "HAL/Graphics/Font.h"
#include "HAL/Graphics/Sprite.h"
#include "HAL/Graphics/SpriteAnimationClip.h"
#include "HAL/Graphics/AnimationGroup.h"

namespace HAL
{
	ResourceManager::ResourceManager() noexcept = default;

	ResourceHandle ResourceManager::lockSprite(const ResourcePath& path)
	{
		std::scoped_lock l(mDataMutex);
		std::string spritePathId = "spr-" + path;
		auto spritePathIt = mPathsMap.find(static_cast<ResourcePath>(spritePathId));
		if (spritePathIt != mPathsMap.end())
		{
			++mResourceLocksCount[spritePathIt->second];
			return ResourceHandle(spritePathIt->second);
		}
		else
		{
			int thisHandle = createResourceLock(static_cast<ResourcePath>(spritePathId));
			ResourceHandle originalSurfaceHandle;
			auto it = mAtlasFrames.find(path);
			if (it != mAtlasFrames.end())
			{
				originalSurfaceHandle = lockResource<Graphics::Internal::Surface>(it->second.atlasPath);
				const Graphics::Internal::Surface* texture = tryGetResource<Graphics::Internal::Surface>(originalSurfaceHandle);
				mResources[thisHandle] = std::make_unique<Graphics::Sprite>(texture, it->second.quadUV);
				mResources[thisHandle]->addDependency(originalSurfaceHandle);
			}
			else
			{
				originalSurfaceHandle = lockResource<Graphics::Internal::Surface>(path);
				const Graphics::Internal::Surface* surface = tryGetResource<Graphics::Internal::Surface>(originalSurfaceHandle);
				mResources[thisHandle] = std::make_unique<Graphics::Sprite>(surface, Graphics::QuadUV());
				mResources[thisHandle]->addDependency(originalSurfaceHandle);
			}
			return ResourceHandle(thisHandle);
		}
	}

	ResourceHandle ResourceManager::lockSpriteAnimationClip(const ResourcePath& path)
	{
		std::scoped_lock l(mDataMutex);
		auto it = mPathsMap.find(path);
		if (it != mPathsMap.end())
		{
			++mResourceLocksCount[it->second];
			return ResourceHandle(it->second);
		}
		else
		{
			int thisHandle = createResourceLock(path);
			std::vector<ResourcePath> framePaths = loadSpriteAnimClipData(path);

			std::vector<ResourceHandle> frames;
			for (const auto& animFramePath : framePaths)
			{
				auto spriteHandle = lockSprite(animFramePath);
				frames.push_back(spriteHandle);
			}
			mResources[thisHandle] = std::make_unique<Graphics::SpriteAnimationClip>(std::move(frames));

			return ResourceHandle(thisHandle);
		}
	}

	ResourceHandle ResourceManager::lockAnimationGroup(const ResourcePath& path)
	{
		std::scoped_lock l(mDataMutex);
		auto it = mPathsMap.find(path);
		if (it != mPathsMap.end())
		{
			++mResourceLocksCount[it->second];
			return ResourceHandle(it->second);
		}
		else
		{
			int thisHandle = createResourceLock(path);
			AnimGroupData animGroupData = loadAnimGroupData(path);

			std::map<StringId, std::vector<ResourceHandle>> animClips;
			std::vector<ResourceHandle> clipsToRelease;
			clipsToRelease.reserve(animGroupData.clips.size());
			for (const auto& animClipPath : animGroupData.clips)
			{
				auto clipHandle = lockSpriteAnimationClip(animClipPath.second);
				animClips.emplace(animClipPath.first, tryGetResource<Graphics::SpriteAnimationClip>(clipHandle)->getSprites());
				clipsToRelease.push_back(clipHandle);
			}
			mResources[thisHandle] = std::make_unique<Graphics::AnimationGroup>(std::move(animClips), animGroupData.stateMachineID, animGroupData.defaultState);
			mResources[thisHandle]->addDependencies(std::move(clipsToRelease));

			return ResourceHandle(thisHandle);
		}
	}

	void ResourceManager::unlockResource(ResourceHandle handle)
	{
		std::scoped_lock l(mDataMutex);
		auto locksCntIt = mResourceLocksCount.find(handle.resourceIndex);
		if ALMOST_NEVER(locksCntIt == mResourceLocksCount.end())
		{
			ReportError("Unlocking non-locked resource");
			return;
		}

		if (locksCntIt->second > 1)
		{
			--(locksCntIt->second);
			return;
		}
		else
		{
			// release the resource
			auto resourceIt = mResources.find(handle.resourceIndex);
			if (resourceIt != mResources.end())
			{
				std::vector<ResourceHandle> resourcesToUnlock = resourceIt->second->getResourceDependencies();

				// unload and delete
				auto releaseFnIt = mResourceReleaseFns.find(handle.resourceIndex);
				if (releaseFnIt != mResourceReleaseFns.end())
				{
					releaseFnIt->second(resourceIt->second.get());
					mResourceReleaseFns.erase(releaseFnIt);
				}
				mResources.erase(resourceIt);

				// unlock all dependencies (do after unloading to resolve any cyclic depenencies)
				for (ResourceHandle resourceHandle : resourcesToUnlock) {
					unlockResource(resourceHandle);
				}
			}
			mResourceLocksCount.erase(handle.resourceIndex);
		}
	}

	void ResourceManager::loadAtlasesData(const ResourcePath& listPath)
	{
		std::scoped_lock l(mDataMutex);
		namespace fs = std::filesystem;
		fs::path listFsPath(static_cast<std::string>(listPath));

		try
		{
			std::ifstream listFile(listFsPath);
			nlohmann::json listJson;
			listFile >> listJson;

			const auto& atlases = listJson.at("atlases");
			for (const auto& atlasPath : atlases.items())
			{
				loadOneAtlasData(atlasPath.value());
			}
		}
		catch(const nlohmann::detail::exception& e)
		{
			LogError("Can't parse atlas list '%s': %s", listPath.c_str(), e.what());
		}
		catch(const std::exception& e)
		{
			LogError("Can't open atlas list '%s': %s", listPath.c_str(), e.what());
		}
	}

	ResourceHandle::IndexType ResourceManager::createResourceLock(const ResourcePath& path)
	{
		mPathsMap[path] = mHandleIdx;
		mPathFindMap[mHandleIdx] = path;
		mResourceLocksCount[mHandleIdx] = 1;
		return mHandleIdx++;
	}

	void ResourceManager::loadOneAtlasData(const ResourcePath& path)
	{
		namespace fs = std::filesystem;
		fs::path atlasDescPath(static_cast<std::string>(path));

		try
		{
			std::ifstream atlasDescriptionFile(atlasDescPath);
			nlohmann::json atlasJson;
			atlasDescriptionFile >> atlasJson;

			auto meta = atlasJson.at("meta");
			ResourcePath atlasPath = meta.at("image");
			auto sizeJson = meta.at("size");
			Vector2D atlasSize;
			sizeJson.at("w").get_to(atlasSize.x);
			sizeJson.at("h").get_to(atlasSize.y);

			const auto& frames = atlasJson.at("frames");
			for (const auto& frameDataJson : frames)
			{
				AtlasFrameData frameData;
				ResourcePath fileName = frameDataJson.at("filename");
				auto frame = frameDataJson.at("frame");
				frameData.atlasPath = atlasPath;
				float x, y, w, h;
				frame.at("x").get_to(x);
				frame.at("y").get_to(y);
				frame.at("w").get_to(w);
				frame.at("h").get_to(h);

				frameData.quadUV.u1 = x / atlasSize.x;
				frameData.quadUV.v1 = y / atlasSize.y;
				frameData.quadUV.u2 = (x + w) / atlasSize.x;
				frameData.quadUV.v2 = (y + h) / atlasSize.y;
				mAtlasFrames.emplace(fileName, std::move(frameData));
			}
		}
		catch(const nlohmann::detail::exception& e)
		{
			LogError("Can't parse atlas data '%s': %s", path.c_str(), e.what());
		}
		catch(const std::exception& e)
		{
			LogError("Can't open atlas data '%s': %s", path.c_str(), e.what());
		}
	}

	std::vector<ResourcePath> ResourceManager::loadSpriteAnimClipData(const ResourcePath& path)
	{
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

	ResourceManager::AnimGroupData ResourceManager::loadAnimGroupData(const ResourcePath& path)
	{
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
}
