#include "Base/precomp.h"

#include "HAL/Base/ResourceManager.h"

#include <fstream>
#include <vector>
#include <filesystem>
#include <string>

#include <nlohmann/json.hpp>

#include "HAL/Audio/Music.h"
#include "HAL/Audio/Sound.h"
#include "HAL/Graphics/Font.h"
#include "HAL/Graphics/Sprite.h"
#include "HAL/Graphics/SpriteAnimationClip.h"
#include "HAL/Graphics/AnimationGroup.h"
#include "HAL/Internal/SdlSurface.h"

namespace HAL
{
	void ResourceDependencies::setFirstDependOnSecond(ResourceHandle dependentResource, ResourceHandle dependency)
	{
		dependencies[dependentResource].push_back(dependency);
		dependentResources[dependency].push_back(dependentResource);
	}

	void ResourceDependencies::setFirstDependOnSecond(ResourceHandle dependentResource, std::vector<ResourceHandle>&& dependencies)
	{
		for (ResourceHandle dependency : dependencies)
		{
			setFirstDependOnSecond(dependentResource, dependency);
		}
	}

	const std::vector<ResourceHandle>& ResourceDependencies::getDependencies(ResourceHandle resource) const
	{
		static const std::vector<ResourceHandle> emptyVector;
		if (auto it = dependencies.find(resource); it != dependencies.end())
		{
			return it->second;
		}
		else
		{
			return emptyVector;
		}
	}

	const std::vector<ResourceHandle>& ResourceDependencies::getDependentResources(ResourceHandle resource) const
	{
		static const std::vector<ResourceHandle> emptyVector;
		if (auto it = dependentResources.find(resource); it != dependentResources.end())
		{
			return it->second;
		}
		else
		{
			return emptyVector;
		}
	}

	std::vector<ResourceHandle> ResourceDependencies::removeResource(ResourceHandle resource)
	{
		std::vector<ResourceHandle> result;
		if (auto it = dependencies.find(resource); it != dependencies.end())
		{
			result = std::move(it->second);
			dependencies.erase(it);
		}

		if (auto it = dependentResources.find(resource); it != dependentResources.end())
		{
			Assert(it->second.empty(), "We removing a resource that have dependent resources: %d", resource);
			dependentResources.erase(it);
		}
		return result;
	}

	ResourceHandle ResourceStorage::createResourceLock(const ResourcePath& path)
	{
		ResourceHandle currentHandle(handleIdx);
		pathsMap[path] = currentHandle;
		pathFindMap[currentHandle] = path;
		resourceLocksCount[currentHandle] = 1;
		++handleIdx;
		return currentHandle;
	}

	ResourceManager::ResourceManager() noexcept = default;

	ResourceHandle ResourceManager::lockSprite(const ResourcePath& path)
	{
		std::scoped_lock l(mDataMutex);
		std::string spritePathId = "spr-" + path;
		auto spritePathIt = mStorage.pathsMap.find(static_cast<ResourcePath>(spritePathId));
		if (spritePathIt != mStorage.pathsMap.end())
		{
			++mStorage.resourceLocksCount[spritePathIt->second];
			return ResourceHandle(spritePathIt->second);
		}
		else
		{
			ResourceHandle thisHandle = mStorage.createResourceLock(static_cast<ResourcePath>(spritePathId));
			ResourceHandle originalSurfaceHandle;
			auto it = mStorage.atlasFrames.find(path);
			if (it != mStorage.atlasFrames.end())
			{
				originalSurfaceHandle = lockResource<Graphics::Internal::Surface>(it->second.atlasPath);
				const Graphics::Internal::Surface* texture = tryGetResource<Graphics::Internal::Surface>(originalSurfaceHandle);
				mStorage.resources[thisHandle] = std::make_unique<Graphics::Sprite>(texture, it->second.quadUV);
				mDependencies.setFirstDependOnSecond(thisHandle, originalSurfaceHandle);
			}
			else
			{
				originalSurfaceHandle = lockResource<Graphics::Internal::Surface>(path);
				const Graphics::Internal::Surface* surface = tryGetResource<Graphics::Internal::Surface>(originalSurfaceHandle);
				mStorage.resources[thisHandle] = std::make_unique<Graphics::Sprite>(surface, Graphics::QuadUV());
				mDependencies.setFirstDependOnSecond(thisHandle, originalSurfaceHandle);
			}
			return thisHandle;
		}
	}

	ResourceHandle ResourceManager::lockSpriteAnimationClip(const ResourcePath& path)
	{
		std::scoped_lock l(mDataMutex);
		auto it = mStorage.pathsMap.find(path);
		if (it != mStorage.pathsMap.end())
		{
			++mStorage.resourceLocksCount[it->second];
			return ResourceHandle(it->second);
		}
		else
		{
			ResourceHandle thisHandle = mStorage.createResourceLock(path);
			std::vector<ResourcePath> framePaths = loadSpriteAnimClipData(path);

			std::vector<ResourceHandle> frames;
			for (const auto& animFramePath : framePaths)
			{
				auto spriteHandle = lockSprite(animFramePath);
				frames.push_back(spriteHandle);
			}
			mStorage.resources[thisHandle] = std::make_unique<Graphics::SpriteAnimationClip>(std::move(frames));

			return thisHandle;
		}
	}

	ResourceHandle ResourceManager::lockAnimationGroup(const ResourcePath& path)
	{
		std::scoped_lock l(mDataMutex);
		auto it = mStorage.pathsMap.find(path);
		if (it != mStorage.pathsMap.end())
		{
			++mStorage.resourceLocksCount[it->second];
			return ResourceHandle(it->second);
		}
		else
		{
			ResourceHandle thisHandle = mStorage.createResourceLock(path);
			AnimGroupData animGroupData = loadAnimGroupData(path);

			std::map<StringId, std::vector<ResourceHandle>> animClips;
			std::vector<ResourceHandle> dependencies;
			dependencies.reserve(animGroupData.clips.size());
			for (const auto& animClipPath : animGroupData.clips)
			{
				auto clipHandle = lockSpriteAnimationClip(animClipPath.second);
				animClips.emplace(animClipPath.first, tryGetResource<Graphics::SpriteAnimationClip>(clipHandle)->getSprites());
				dependencies.push_back(clipHandle);
			}
			mStorage.resources[thisHandle] = std::make_unique<Graphics::AnimationGroup>(std::move(animClips), animGroupData.stateMachineID, animGroupData.defaultState);
			mDependencies.setFirstDependOnSecond(thisHandle, std::move(dependencies));

			return thisHandle;
		}
	}

	void ResourceManager::unlockResource(ResourceHandle handle)
	{
		std::scoped_lock l(mDataMutex);
		auto locksCntIt = mStorage.resourceLocksCount.find(handle);
		if ALMOST_NEVER(locksCntIt == mStorage.resourceLocksCount.end())
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
			auto resourceIt = mStorage.resources.find(handle);
			if (resourceIt != mStorage.resources.end())
			{
				// unload and delete
				if (const HAL::Resource::SpecialThreadInit* specInit = resourceIt->second->getSpecialThreadInitialization())
				{
					if (specInit->steps[0].deinit != nullptr)
					{
						mLoading.resourcesWaitingDeinit.emplace_back(handle, std::move(resourceIt->second));
					}
				}

				mStorage.resources.erase(resourceIt);

				// unlock all dependencies (do after unloading to resolve any cyclic depenencies)
				std::vector<ResourceHandle> resourcesToUnlock = mDependencies.removeResource(handle);
				for (ResourceHandle resourceHandle : resourcesToUnlock) {
					unlockResource(ResourceHandle(resourceHandle));
				}
			}
			mStorage.resourceLocksCount.erase(handle);
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

	void ResourceManager::RunRenderThreadTasks()
	{
		std::unique_lock l(mDataMutex);
		for (int i = 0; i < static_cast<int>(mLoading.resourcesWaitingInit.size()); ++i)
		{
			auto&& [handle, resource] = mLoading.resourcesWaitingInit[static_cast<size_t>(i)];
			if (const HAL::Resource::SpecialThreadInit* specInit = resource->getSpecialThreadInitialization())
			{
				if (specInit->steps[0].thread == HAL::Resource::SpecialThreadInit::Thread::Render)
				{
					specInit->steps[0].init(resource.get());
					mStorage.resources[handle] = std::move(resource);
					mLoading.resourcesWaitingInit.erase(mLoading.resourcesWaitingInit.begin() + i);
					--i;
				}
			}
		}

		for (int i = 0; i < static_cast<int>(mLoading.resourcesWaitingDeinit.size()); ++i)
		{
			auto&& [handle, resource] = mLoading.resourcesWaitingDeinit[static_cast<size_t>(i)];
			if (const HAL::Resource::SpecialThreadInit* specInit = resource->getSpecialThreadInitialization())
			{
				if (specInit->steps[0].thread == HAL::Resource::SpecialThreadInit::Thread::Render)
				{
					specInit->steps[0].init(resource.get());
					// resource unloading happens here
					mLoading.resourcesWaitingDeinit.erase(mLoading.resourcesWaitingDeinit.begin() + i);
					--i;
				}
			}
		}
	}

	void ResourceManager::startResourceLoading(ResourceHandle handle, ResourceLoadFn&& resourceLoadFn)
	{
		auto deletionIt = std::find_if(
			mLoading.resourcesWaitingDeinit.begin(),
			mLoading.resourcesWaitingDeinit.end(),
			[handle](const ResourceLoading::ResourceLoadingData& resourceLoadData)
			{
				return resourceLoadData.handle == handle;
			}
		);

		// revive a resource that we were about to delete
		if (deletionIt != mLoading.resourcesWaitingDeinit.end())
		{
			mStorage.resources[handle] = std::move(deletionIt->resource);
			mLoading.resourcesWaitingDeinit.erase(deletionIt);
			return;
		}

		std::unique_ptr<Resource> resource = resourceLoadFn();
		if (const Resource::SpecialThreadInit* specInit = resource->getSpecialThreadInitialization())
		{
			if (specInit->steps[0].init != nullptr)
			{
				switch (specInit->steps[0].thread)
				{
				case Resource::SpecialThreadInit::Thread::Render:
					mLoading.resourcesWaitingInit.emplace_back(handle, std::move(resource));
					break;
				default:
					ReportError("Unknown resource init thread");
				}
			}
		}
		else
		{
			mStorage.resources[handle] = std::move(resource);
		}
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
				ResourceStorage::AtlasFrameData frameData;
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
				mStorage.atlasFrames.emplace(fileName, std::move(frameData));
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
