#pragma once

#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "Base/Debug/ConcurrentAccessDetector.h"
#include "Base/Profile/ScopedProfiler.h"
#include "Base/Types/String/ResourcePath.h"

#include "GameData/Resources/Resource.h"
#include "GameData/Resources/ResourceHandle.h"

#include "HAL/Base/Types.h"

#include "Utils/ResourceManagement/ResourceDependencies.h"
#include "Utils/ResourceManagement/ResourceLoadingData.h"
#include "Utils/ResourceManagement/ResourceStorageData.h"

struct ResourceDependencyType
{
	// not using enum class to avoid static-casting
	enum Type : unsigned char
	{
		None = 0,
		// if set, the next step of resource loading will wait until the dependent resource is loaded
		Load = 1 << 0,
		// if set, the resource won't be unloaded automatically until the dependent resource is unloaded
		Unload = 1 << 1,
		LoadAndUnload = Load | Unload
	};
};

/**
 * Class that manages resources such as textures
 */
class ResourceManager
{
public:
	explicit ResourceManager() noexcept;
	explicit ResourceManager(const std::filesystem::path& resourcesDirectoryPath) noexcept;

	~ResourceManager();

	ResourceManager(const ResourceManager&) = delete;
	ResourceManager& operator=(const ResourceManager&) = delete;
	ResourceManager(ResourceManager&&) = delete;
	ResourceManager& operator=(ResourceManager&&) = delete;

	AbsoluteResourcePath getAbsoluteResourcePath(const RelativeResourcePath& relativePath) const;
	AbsoluteResourcePath getAbsoluteResourcePath(RelativeResourcePath&& relativePath) const;

	template<typename T, typename... Args>
	[[nodiscard]] ResourceHandle lockResource(Args&&... args)
	{
		return lockResourceFromThread<T>(Resource::Thread::Any, std::forward<Args>(args)...);
	}

	template<typename T, typename... Args>
	[[nodiscard]] ResourceHandle lockResourceFromThread(Resource::Thread currentThread, Args&&... args)
	{
		SCOPED_PROFILER("ResourceManager::lockResourceFromThread");
		std::scoped_lock l(mDataMutex);
		DETECT_CONCURRENT_ACCESS(gResourceManagerAccessDetector);
		std::string id = T::GetUniqueId(args...);

		auto it = mStorage.idsMap.find(id);
		if (it != mStorage.idsMap.end())
		{
			++mStorage.resourceLocksCount[it->second];
			return ResourceHandle(it->second);
		}
		else
		{
			ResourceHandle newHandle = mStorage.createResourceLock(id);

			if constexpr (sizeof...(Args) == 1)
			{
				startResourceLoading(std::make_unique<ResourceLoading::ResourceLoad::LoadingData>(
					newHandle,
					T::GetInitSteps(),
					// if it's one argument, pass it normally to create UniqueAny
					UniqueAny::Create<Args...>(std::forward<Args>(args)...)
				), currentThread);
			}
			else
			{
				startResourceLoading(std::make_unique<ResourceLoading::ResourceLoad::LoadingData>(
					newHandle,
					T::GetInitSteps(),
					// if there are multiple arguments, pack them as tuple
					UniqueAny::Create<std::tuple<Args...>>(std::forward<Args>(args)...)
				), currentThread);
			}

			return newHandle;
		}
	}

	template<typename T>
	[[nodiscard]] const T* tryGetResource(ResourceHandle handle)
	{
		std::scoped_lock l(mDataMutex);
		DETECT_CONCURRENT_ACCESS(gResourceManagerAccessDetector);
		auto it = mStorage.resources.find(handle);
		return it == mStorage.resources.end() ? nullptr : static_cast<T*>(it->second.get());
	}

	void unlockResource(ResourceHandle handle);

	void loadAtlasesData(const RelativeResourcePath& listPath);

	void runThreadTasks(Resource::Thread currentThread);

	void setFirstResourceDependOnSecond(ResourceHandle dependentResource, ResourceHandle dependency, ResourceDependencyType::Type type = ResourceDependencyType::LoadAndUnload);

	// for now atlases are always loaded and don't require dependency management
	// if that ever changes, manage atlases as any other resources and remove this method
	const std::unordered_map<RelativeResourcePath, ResourceLoading::ResourceStorage::AtlasFrameData>& getAtlasFrames() const;

	void startLoadingThread(std::function<void()>&& threadFinalizerFn = nullptr);
	// this call will wait for the thread to join
	void stopLoadingThread();

private:
	void startResourceLoading(ResourceLoading::ResourceLoad::LoadingDataPtr&& loadingGata, Resource::Thread currentThread);
	void loadOneAtlasData(const AbsoluteResourcePath& path);
	void finalizeResourceLoading(ResourceHandle handle, Resource::Ptr&& resource);

private:
	ResourceLoading::ResourceStorage mStorage;
	ResourceLoading::ResourceLoad mLoading;
	ResourceLoading::RuntimeDependencies mDependencies;

	std::thread mLoadingThread;
	bool mShouldStopLoadingThread = false;
	std::condition_variable_any mNotifyLoadingThread;

	std::recursive_mutex mDataMutex;

	std::filesystem::path mResourcesDirectoryPath;

#ifdef CONCURRENT_ACCESS_DETECTION
	static ConcurrentAccessDetector gResourceManagerAccessDetector;
#endif // CONCURRENT_ACCESS_DETECTION
};
