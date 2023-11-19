#include "Base/precomp.h"

#include "Utils/ResourceManagement/ResourceManager.h"

#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include <ranges>

#include <nlohmann/json.hpp>

#ifdef CONCURRENT_ACCESS_DETECTION
ConcurrentAccessDetector ResourceManager::gResourceManagerAccessDetector;
#endif // CONCURRENT_ACCESS_DETECTION

ResourceManager::ResourceManager() noexcept = default;

ResourceManager::~ResourceManager()
{
	stopLoadingThread();
}

void ResourceManager::unlockResource(ResourceHandle handle)
{
	std::scoped_lock l(mDataMutex);
	DETECT_CONCURRENT_ACCESS(gResourceManagerAccessDetector);
	auto locksCntIt = mStorage.resourceLocksCount.find(handle);
	if (locksCntIt == mStorage.resourceLocksCount.end()) [[unlikely]]
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
					std::make_unique<ResourceLoading::ResourceLoad::UnloadingData>(
						handle,
						UniqueAny::Create<Resource::Ptr>(std::move(resourceIt->second)),
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
	DETECT_CONCURRENT_ACCESS(gResourceManagerAccessDetector);
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
	std::unique_lock lock(mDataMutex);
	bool shouldWakeUpLoadingThread = false;
	for (auto resourceIt = mLoading.resourcesWaitingInit.begin(); resourceIt != mLoading.resourcesWaitingInit.end();)
	{
		bool skipIncrement = false;
		ResourceLoading::ResourceLoad::LoadingDataPtr& loadingData = *resourceIt;
		while (!loadingData->stepsLeft.empty())
		{
			Resource::InitStep& step = loadingData->stepsLeft.front();
			if (step.thread == currentThread || step.thread == Resource::Thread::Any)
			{
				// don't allow any other thread to take the step
				step.thread = currentThread;

				lock.unlock();
				loadingData->resourceData = step.init(std::move(loadingData->resourceData), *this, loadingData->handle);
				lock.lock();

				loadingData->stepsLeft.pop_front();

				// found a dependecy that need to resolve first
				auto it = mLoading.resourcesWaitingDependencies.find(loadingData->handle);
				if (it != mLoading.resourcesWaitingDependencies.end())
				{
					it->second = std::move(loadingData);
					// this invalidates "step" variable
					resourceIt = mLoading.resourcesWaitingInit.erase(resourceIt);
					skipIncrement = true; // need to skip because erase already points to the next element
					break;
				}

				// done all steps
				if (loadingData->stepsLeft.empty())
				{
					Resource::Ptr* resultDataPtr = loadingData->resourceData.cast<Resource::Ptr>();
					finalizeResourceLoading(loadingData->handle, (resultDataPtr ? std::move(*resultDataPtr) : Resource::Ptr{}));
					// this invalidates "step" variable
					resourceIt = mLoading.resourcesWaitingInit.erase(resourceIt);
					skipIncrement = true; // need to skip because erase already points to the next element
					break;
				}
			}
			else
			{
				if (step.thread == Resource::Thread::Loading)
				{
					shouldWakeUpLoadingThread = true;
				}
				break;
			}
		}

		if (!skipIncrement)
		{
			++resourceIt;
		}
	}

	for (auto resourceIt = mLoading.resourcesWaitingDeinit.begin(); resourceIt != mLoading.resourcesWaitingDeinit.end(); ++resourceIt)
	{
		auto&& [handle, resourceData, steps] = **resourceIt;
		while (!steps.empty())
		{
			Resource::DeinitStep& step = steps.front();
			if (step.thread == currentThread || step.thread == Resource::Thread::Any)
			{
				resourceData = step.deinit(std::move(resourceData), *this, handle);
				steps.pop_front();

				// done all steps
				if (steps.empty())
				{
					resourceIt = mLoading.resourcesWaitingDeinit.erase(resourceIt);
					--resourceIt;
					break;
				}
			}
			else
			{
				if (step.thread == Resource::Thread::Loading)
				{
					shouldWakeUpLoadingThread = true;
				}
				break;
			}
		}
	}

	lock.unlock();

	if (shouldWakeUpLoadingThread)
	{
		mNotifyLoadingThread.notify_one();
	}
}

const std::unordered_map<ResourcePath, ResourceLoading::ResourceStorage::AtlasFrameData>& ResourceManager::getAtlasFrames() const
{
	DETECT_CONCURRENT_ACCESS(gResourceManagerAccessDetector);
	return mStorage.atlasFrames;
}

void ResourceManager::startLoadingThread(std::function<void()>&& threadFinalizerFn)
{
	AssertFatal(!mLoadingThread.joinable(), "Tried to start already started thread");
	mLoadingThread = std::thread([this, finalizeFn = std::move(threadFinalizerFn)]{
		while (true)
		{
			{
				std::unique_lock lock(mDataMutex);
				mNotifyLoadingThread.wait(lock, [this]{ return mShouldStopLoadingThread || !mLoading.resourcesWaitingInit.empty(); });

				if (mShouldStopLoadingThread)
				{
					break;
				}
			}

			runThreadTasks(Resource::Thread::Loading);
		}

		if (finalizeFn)
		{
			finalizeFn();
		}
	});
}

void ResourceManager::stopLoadingThread()
{
	if (mLoadingThread.joinable())
	{
		{
			std::scoped_lock l(mDataMutex);
			mShouldStopLoadingThread = true;
		}

		mNotifyLoadingThread.notify_one();

		mLoadingThread.join();

		// if for some reason we want to restart the tread in this class
		mShouldStopLoadingThread = false;
	}
}

void ResourceManager::startResourceLoading(ResourceLoading::ResourceLoad::LoadingDataPtr&& loadingData, Resource::Thread currentThread)
{
	SCOPED_PROFILER("ResourceManager::startResourceLoading");
	std::unique_lock lock(mDataMutex);
	auto deletionIt = std::ranges::find_if(
		mLoading.resourcesWaitingDeinit,
		[handle = loadingData->handle](const ResourceLoading::ResourceLoad::UnloadingDataPtr& resourceUnloadData)
		{
			return resourceUnloadData->handle == handle;
		}
	);

	// revive a resource that we were about to delete
	if (deletionIt != mLoading.resourcesWaitingDeinit.end())
	{
		Resource::Ptr* resourcePtr = (*deletionIt)->resourceData.cast<Resource::Ptr>();
		finalizeResourceLoading(loadingData->handle, (resourcePtr ? std::move(*resourcePtr) : Resource::Ptr{}));
		mLoading.resourcesWaitingDeinit.erase(deletionIt);
		ReportError("This code is incomplete, the resource can be half-unloaded");
		return;
	}

	bool hasUnresolvedDeps = false;
	bool shouldWakeUpLoadingThread = false;
	while (!loadingData->stepsLeft.empty())
	{
		Resource::InitStep& step = loadingData->stepsLeft.front();
		if (step.thread == currentThread || step.thread == Resource::Thread::Any)
		{
			// don't allow any other thread to take the step
			step.thread = currentThread;

			lock.unlock();
			loadingData->resourceData = step.init(std::move(loadingData->resourceData), *this, loadingData->handle);
			lock.lock();

			loadingData->stepsLeft.pop_front();

			auto it = mLoading.resourcesWaitingDependencies.find(loadingData->handle);
			if (it != mLoading.resourcesWaitingDependencies.end())
			{
				it->second = std::move(loadingData);
				hasUnresolvedDeps = true;
				break;
			}
		}
		else
		{
			if (step.thread == Resource::Thread::Loading)
			{
				shouldWakeUpLoadingThread = true;
			}
			break;
		}
	}

	if (!hasUnresolvedDeps)
	{
		if (!loadingData->stepsLeft.empty())
		{
			mLoading.resourcesWaitingInit.push_back(std::move(loadingData));
		}
		else
		{
			Resource::Ptr* resourcePtr = loadingData->resourceData.cast<Resource::Ptr>();
			finalizeResourceLoading(loadingData->handle, (resourcePtr ? std::move(*resourcePtr) : Resource::Ptr{}));
		}
	}

	lock.unlock();

	if (shouldWakeUpLoadingThread)
	{
		mNotifyLoadingThread.notify_one();
	}
}

void ResourceManager::loadOneAtlasData(const ResourcePath& path)
{
	DETECT_CONCURRENT_ACCESS(gResourceManagerAccessDetector);
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
			ResourceLoading::ResourceStorage::AtlasFrameData frameData;
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

void ResourceManager::setFirstResourceDependOnSecond(ResourceHandle dependentResource, ResourceHandle dependency, ResourceDependencyType::Type type)
{
	DETECT_CONCURRENT_ACCESS(gResourceManagerAccessDetector);
	if (type & ResourceDependencyType::Unload)
	{
		mDependencies.setFirstDependOnSecond(dependentResource, dependency);
	}

	if (type & ResourceDependencyType::Load)
	{
		// add dependency only if the resource is not loaded yet
		if (auto it = mStorage.resources.find(dependency); it == mStorage.resources.end() || it->second == nullptr)
		{
			mLoading.loadDependencies.setFirstDependOnSecond(dependentResource, dependency);
			mLoading.resourcesWaitingDependencies.emplace(dependentResource, ResourceLoading::ResourceLoad::LoadingDataPtr{});
		}
	}
}

void ResourceManager::finalizeResourceLoading(ResourceHandle handle, Resource::Ptr&& resource)
{
	DETECT_CONCURRENT_ACCESS(gResourceManagerAccessDetector);
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

		// continue loading a resource that was waiting for this dependency
		if (it->second->stepsLeft.empty())
		{
			Resource::Ptr* resultDataPtr = it->second->resourceData.cast<Resource::Ptr>();
			finalizeResourceLoading(readyResourceHandle, (resultDataPtr ? std::move(*resultDataPtr) : Resource::Ptr{}));
		}
		else
		{
			mLoading.resourcesWaitingInit.push_back(std::move(it->second));
		}
		mLoading.resourcesWaitingDependencies.erase(it);
	}
}
