#pragma once

#include <map>
#include <unordered_map>
#include <functional>
#include <mutex>

#include "Base/Types/String/Path.h"

#include "Base/Debug/ConcurrentAccessDetector.h"

#include "GameData/Core/ResourceHandle.h"

#include "HAL/Base/Resource.h"
#include "HAL/Base/Types.h"
#include "HAL/Internal/ResourceManagement/ResourceDependencies.h"
#include "HAL/Internal/ResourceManagement/ResourceStorageData.h"
#include "HAL/Internal/ResourceManagement/ResourceLoadingData.h"

namespace HAL
{
#ifdef CONCURRENT_ACCESS_DETECTION
	extern ConcurrentAccessDetector gResourceManagerAccessDetector;
#endif // CONCURRENT_ACCESS_DETECTION

	/**
	 * Class that manages resources such as textures
	 */
	class ResourceManager
	{
	public:
		explicit ResourceManager() noexcept;

		~ResourceManager() = default;

		ResourceManager(const ResourceManager&) = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager(ResourceManager&&) = delete;
		ResourceManager& operator=(ResourceManager&&) = delete;

		ResourceHandle lockSprite(const ResourcePath& path, Resource::Thread currentThread = Resource::Thread::Any);
		ResourceHandle lockSpriteAnimationClip(const ResourcePath& path, Resource::Thread currentThread = Resource::Thread::Any);
		ResourceHandle lockAnimationGroup(const ResourcePath& path, Resource::Thread currentThread = Resource::Thread::Any);

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

			auto it = mStorage.pathsMap.find(static_cast<ResourcePath>(id));
			if (it != mStorage.pathsMap.end())
			{
				++mStorage.resourceLocksCount[it->second];
				return ResourceHandle(it->second);
			}
			else
			{
				ResourceHandle newHandle = mStorage.createResourceLock(static_cast<ResourcePath>(id));

				if constexpr (sizeof...(Args) == 1)
				{
					startResourceLoading(std::make_unique<ResourceLoading::LoadingData>(
						newHandle,
						T::GetInitSteps(),
						// if it's one argument, pass it normally to create UniqueAny
						UniqueAny::Create<Args...>(std::forward<Args>(args)...)
					), currentThread);
				}
				else
				{
					startResourceLoading(std::make_unique<ResourceLoading::LoadingData>(
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

		void lockResource(ResourceHandle handle);
		void unlockResource(ResourceHandle handle);

		void loadAtlasesData(const ResourcePath& listPath);

		void runThreadTasks(Resource::Thread currentThread);

		void setFirstResourceDependOnSecond(ResourceHandle dependentResource, ResourceHandle dependency);

		// for now atlasses are always loaded and don't require dependecy management
		// if that ever changes, manage atlasses as any other resources and remove this method
		const std::unordered_map<ResourcePath, ResourceStorage::AtlasFrameData>& getAtlasFrames() const;

	private:
		ResourceHandle lockSurface(const ResourcePath& path);

		void startResourceLoading(ResourceLoading::LoadingDataPtr&& loadingGata, Resource::Thread currentThread);
		void loadOneAtlasData(const ResourcePath& path);
		void finalizeResourceLoading(ResourceHandle handle, Resource::Ptr&& resource);

	private:
		ResourceStorage mStorage;
		ResourceLoading mLoading;
		RuntimeDependencies mDependencies;

		std::recursive_mutex mDataMutex;
	};
}
