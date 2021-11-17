#include "Base/precomp.h"

#include "HAL/Base/ResourceManager.h"

#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include <ranges>

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

	void ResourceDependencies::setFirstDependOnSecond(ResourceHandle dependentResource, const std::vector<ResourceHandle>& dependencies)
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

	std::vector<ResourceHandle> RuntimeDependencies::removeResource(ResourceHandle resource)
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

	std::vector<ResourceHandle> LoadDependencies::resolveDependency(ResourceHandle dependency)
	{
		std::vector<ResourceHandle> result;

		// move the dependent resources to "result" and remove the record
		if (auto it = dependentResources.find(dependency); it != dependentResources.end())
		{
			result = std::move(it->second);
			dependentResources.erase(it);
		}

		// check that our dependencies are empty and remove the record
		if (auto it = dependencies.find(dependency); it != dependencies.end())
		{
			Assert(it->second.empty(), "We resolving dependency that have unresolved dependencies itself: %d", dependency);
			dependencies.erase(it);
		}

		// resolve dependencies for the dependent resources
		// and filter "result" to keep only fully resolved ones
		for (int i = 0; i < static_cast<int>(result.size()); ++i)
		{
			auto it = dependencies.find(result[i]);
			if (it != dependencies.end())
			{
				auto removedRange = std::ranges::remove_if(it->second, [dependency](ResourceHandle handle){
					return handle == dependency;
				});
				Assert(!removedRange.empty(), "We've got a dependent resource missing info about its dependency");

				it->second.erase(removedRange.begin(), removedRange.end());

				// not fully resolved yet, remove from "result"
				if (!it->second.empty())
				{
					result.erase(result.begin() + i);
					--i;
				}
			}
			else
			{
				ReportError("We tried to resolve a dependency, but it's already resolved");
			}
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

	ResourceHandle ResourceManager::lockSprite(const ResourcePath& path, Resource::Thread currentThread)
	{
		std::scoped_lock l(mDataMutex);
		std::string spritePathId = "spr-" + path;
		return lockCustomResource<Graphics::Sprite>(
			static_cast<ResourcePath>(spritePathId),
			&ResourceManager::StartSpriteLoading,
			currentThread,
			static_cast<ResourcePath>(path)
		);
	}

	ResourceHandle ResourceManager::lockSpriteAnimationClip(const ResourcePath& path, Resource::Thread currentThread)
	{
		std::scoped_lock l(mDataMutex);

		return lockCustomResource<Graphics::SpriteAnimationClip>(
			path,
			[](ResourceManager& resourceManager, ResourceHandle handle, Resource::Thread currentThread, const ResourcePath& path){
				std::vector<ResourcePath> framePaths = resourceManager.loadSpriteAnimClipData(path);
				std::vector<ResourceHandle> frames;
				for (const auto& animFramePath : framePaths)
				{
					ResourceHandle spriteHandle = resourceManager.lockSprite(animFramePath);
					resourceManager.mDependencies.setFirstDependOnSecond(handle, spriteHandle);
					frames.push_back(spriteHandle);
				}

				resourceManager.finalizeResourceLoading(
					handle,
					std::make_unique<Graphics::SpriteAnimationClip>(std::move(frames)),
					currentThread
				);
			},
			currentThread,
			path
		);
	}

	ResourceHandle ResourceManager::lockAnimationGroup(const ResourcePath& path, Resource::Thread currentThread)
	{
		std::scoped_lock l(mDataMutex);

		return lockCustomResource<Graphics::AnimationGroup>(
			path,
			[](ResourceManager& resourceManager, ResourceHandle handle, Resource::Thread currentThread, const ResourcePath& path){
				AnimGroupData animGroupData = resourceManager.loadAnimGroupData(path);

				std::map<StringId, std::vector<ResourceHandle>> animClips;
				for (const auto& animClipPath : animGroupData.clips)
				{
					ResourceHandle clipHandle = resourceManager.lockSpriteAnimationClip(animClipPath.second);
					resourceManager.mDependencies.setFirstDependOnSecond(handle, clipHandle);
					animClips.emplace(animClipPath.first, resourceManager.tryGetResource<Graphics::SpriteAnimationClip>(clipHandle)->getSprites());
				}

				resourceManager.finalizeResourceLoading(
					handle,
					std::make_unique<Graphics::AnimationGroup>(std::move(animClips), animGroupData.stateMachineID, animGroupData.defaultState),
					currentThread
				);
			},
			currentThread,
			path
		);
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
				Resource::DeinitSteps deinitSteps = resourceIt->second->getDeinitSteps();
				if (deinitSteps.empty())
				{
					mLoading.resourcesWaitingDeinit.push_back(
						std::make_unique<ResourceLoading::UnloadingData>(
							handle,
							std::move(resourceIt->second),
							std::move(deinitSteps)
						)
					);
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
		SCOPED_PROFILER("ResourceManager::loadAtlasesData");
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

	void ResourceManager::runThreadTasks(Resource::Thread currentThread)
	{
		SCOPED_PROFILER("ResourceManager::runThreadTasks");
		std::unique_lock l(mDataMutex);
		for (int i = 0; i < static_cast<int>(mLoading.resourcesWaitingInit.size()); ++i)
		{
			ResourceLoading::LoadingDataPtr& loadingData = mLoading.resourcesWaitingInit[static_cast<size_t>(i)];
			while (!loadingData->stepsLeft.empty())
			{
				const Resource::InitStep& step = loadingData->stepsLeft.front();
				if (step.thread == currentThread || step.thread == Resource::Thread::Any)
				{
					step.init(loadingData->resource);
					loadingData->stepsLeft.pop_front();
					// done all steps
					if (loadingData->stepsLeft.empty())
					{
						finalizeResourceLoading(loadingData->handle, std::move(loadingData->resource), currentThread);
						// this invalidates "step" variable
						mLoading.resourcesWaitingInit.erase(mLoading.resourcesWaitingInit.begin() + i);
						--i;
						break;
					}
				}
				else
				{
					break;
				}
			}
		}

		for (int i = 0; i < static_cast<int>(mLoading.resourcesWaitingDeinit.size()); ++i)
		{
			auto&& [handle, resource, steps] = *mLoading.resourcesWaitingDeinit[static_cast<size_t>(i)];
			while (!steps.empty())
			{
				const Resource::DeinitStep& step = steps.front();
				if (step.thread == currentThread || step.thread == Resource::Thread::Any)
				{
					step.deinit(resource);
					steps.pop_front();
					// done all steps
					if (steps.empty())
					{
						mLoading.resourcesWaitingDeinit.erase(mLoading.resourcesWaitingDeinit.begin() + i);
						--i;
						break;
					}
				}
			}
		}
	}

	void ResourceManager::startResourceLoading(ResourceLoading::LoadingDataPtr&& loadingData, Resource::Thread currentThread)
	{
		SCOPED_PROFILER("ResourceManager::startResourceLoading");
		auto deletionIt = std::ranges::find_if(
			mLoading.resourcesWaitingDeinit,
			[handle = loadingData->handle](const ResourceLoading::UnloadingDataPtr& resourceUnloadData)
			{
				return resourceUnloadData->handle == handle;
			}
		);

		// revive a resource that we were about to delete
		if (deletionIt != mLoading.resourcesWaitingDeinit.end())
		{
			finalizeResourceLoading(loadingData->handle, std::move((*deletionIt)->resource), currentThread);
			mLoading.resourcesWaitingDeinit.erase(deletionIt);
			ReportError("This code is incomplete, the resource can be half-unloaded");
			return;
		}

		if (loadingData->factoryFn)
		{
			if (loadingData->factoryThread == currentThread || loadingData->factoryThread == Resource::Thread::Any)
			{
				loadingData->resource = loadingData->factoryFn();
			}
			else
			{
				ReportError("Construction of a resource from a specific thread is not implemented yet. Use InitSteps instead");
			}
		}

		while (!loadingData->stepsLeft.empty())
		{
			Resource::InitStep& step = loadingData->stepsLeft.front();
			if (step.thread == currentThread || step.thread == Resource::Thread::Any)
			{
				step.init(loadingData->resource);
				loadingData->stepsLeft.pop_front();
			}
			else
			{
				break;
			}
		}

		if (!loadingData->stepsLeft.empty())
		{
			mLoading.resourcesWaitingInit.push_back(std::move(loadingData));
		}
		else
		{
			finalizeResourceLoading(loadingData->handle, std::move(loadingData->resource), currentThread);
		}
	}

	void ResourceManager::loadOneAtlasData(const ResourcePath& path)
	{
		SCOPED_PROFILER("ResourceManager::loadOneAtlasData");
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

	ResourceManager::AnimGroupData ResourceManager::loadAnimGroupData(const ResourcePath& path)
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

	void ResourceManager::createLoadDependency(ResourceHandle dependency, ResourceLoading::LoadingDataPtr&& loadingData)
	{
		ResourceHandle handle = loadingData->handle;
		mLoading.loadDependencies.setFirstDependOnSecond(handle, dependency);
		mLoading.resourcesWaitingDependencies.emplace(handle, std::move(loadingData));
	}

	void ResourceManager::finalizeResourceLoading(ResourceHandle handle, Resource::Ptr&& resource, Resource::Thread currentThread)
	{
		SCOPED_PROFILER("ResourceManager::finalizeResourceLoading");
		mStorage.resources[handle] = std::move(resource);

		const std::vector<ResourceHandle> readyToLoadResources = mLoading.loadDependencies.resolveDependency(handle);
		for (ResourceHandle readyResourceHandle : readyToLoadResources)
		{
			auto it = mLoading.resourcesWaitingDependencies.find(readyResourceHandle);
			if (it == mLoading.resourcesWaitingDependencies.end())
			{
				ReportError("Couldn't find a resource waiting dependencies");
				continue;
			}

			ResourceLoading::LoadingDataPtr loadData = std::move(it->second);
			mLoading.resourcesWaitingDependencies.erase(it);
			startResourceLoading(std::move(loadData), currentThread);
		}
	}

	void ResourceManager::StartSpriteLoading(ResourceManager& resourceManager, ResourceHandle handle, Resource::Thread currentThread, const ResourcePath& path)
	{
		SCOPED_PROFILER("ResourceManager::StartSpriteLoading");
		ResourceHandle originalSurfaceHandle;
		auto it = resourceManager.mStorage.atlasFrames.find(path);
		std::string surfacePath;
		Graphics::QuadUV spriteUV;

		if (it != resourceManager.mStorage.atlasFrames.end())
		{
			surfacePath = it->second.atlasPath;
			spriteUV = it->second.quadUV;
		}
		else
		{
			surfacePath = path;
		}
		originalSurfaceHandle = resourceManager.lockResource<Graphics::Internal::Surface>(surfacePath);
		resourceManager.mDependencies.setFirstDependOnSecond(handle, originalSurfaceHandle);

		if (const Graphics::Internal::Surface* surface = resourceManager.tryGetResource<Graphics::Internal::Surface>(originalSurfaceHandle))
		{
			resourceManager.finalizeResourceLoading(handle, std::make_unique<Graphics::Sprite>(surface, spriteUV), currentThread);
		}
		else
		{
			auto factoryFn = [&resourceManager, originalSurfaceHandle, spriteUV]{
				const Graphics::Internal::Surface* surface = resourceManager.tryGetResource<Graphics::Internal::Surface>(originalSurfaceHandle);
				AssertRelease(surface != nullptr, "The surface should be loaded before loading sprite");
				return std::make_unique<Graphics::Sprite>(surface, spriteUV);
			};

			resourceManager.createLoadDependency(
				originalSurfaceHandle,
				std::make_unique<ResourceLoading::LoadingData>(
					handle,
					Graphics::Sprite::GetInitSteps(),
					factoryFn,
					Graphics::Sprite::GetCreateThread()
				)
			);
		}
	}
}
