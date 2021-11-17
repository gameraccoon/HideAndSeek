#pragma once

#include <map>
#include <unordered_map>
#include <functional>
#include <mutex>

#include "Base/Types/String/Path.h"

#include "GameData/Core/ResourceHandle.h"

#include "HAL/Base/Resource.h"
#include "HAL/Base/Types.h"

namespace HAL
{
	class ResourceDependencies
	{
	public:
		// this should be followed with resource lock
		void setFirstDependOnSecond(ResourceHandle dependentResource, ResourceHandle dependency);
		void setFirstDependOnSecond(ResourceHandle dependentResource, const std::vector<ResourceHandle>& dependencies);

		const std::vector<ResourceHandle>& getDependencies(ResourceHandle resource) const;
		const std::vector<ResourceHandle>& getDependentResources(ResourceHandle resource) const;

	protected:
		std::unordered_map<ResourceHandle, std::vector<ResourceHandle>> dependencies;
		std::unordered_map<ResourceHandle, std::vector<ResourceHandle>> dependentResources;
	};

	class RuntimeDependencies : public ResourceDependencies
	{
	public:
		// returns all dependencies of the resource (need to unlock them)
		[[nodiscard]]
		std::vector<ResourceHandle> removeResource(ResourceHandle resource);
	};

	class LoadDependencies : public ResourceDependencies
	{
	public:
		// returns all dependent resources that don't have dependencies anymore
		std::vector<ResourceHandle> resolveDependency(ResourceHandle dependency);
	};

	// storage for loaded and ready resources
	class ResourceStorage
	{
	public:
		struct AtlasFrameData
		{
			ResourcePath atlasPath;
			Graphics::QuadUV quadUV;
		};

	public:
		ResourceHandle createResourceLock(const ResourcePath& path);

	public:
		std::unordered_map<ResourceHandle, Resource::Ptr> resources;
		std::unordered_map<ResourceHandle, int> resourceLocksCount;
		std::unordered_map<ResourcePath, ResourceHandle> pathsMap;
		std::map<ResourceHandle, ResourcePath> pathFindMap;
		std::unordered_map<ResourcePath, AtlasFrameData> atlasFrames;
		ResourceHandle::IndexType handleIdx = 0;
	};

	// data for loading and resolving dependencies on load
	class ResourceLoading
	{
	public:
		struct LoadingData
		{
			using ResourceFactoryFn = std::function<Resource::Ptr()>;

			LoadingData(
				ResourceHandle handle,
				Resource::InitSteps&& loadingSteps,
				ResourceFactoryFn&& factoryFn,
				Resource::Thread factoryThread
			)
				: handle(handle)
				, stepsLeft(std::move(loadingSteps))
				, factoryFn(std::move(factoryFn))
				, factoryThread(factoryThread)
			{}

			ResourceHandle handle;
			Resource::Ptr resource;
			Resource::InitSteps stepsLeft;
			ResourceFactoryFn factoryFn;
			Resource::Thread factoryThread;
		};

		struct UnloadingData
		{
			UnloadingData(
				ResourceHandle handle,
				Resource::Ptr&& resource,
				Resource::DeinitSteps&& unloadingSteps
			)
				: handle(handle)
				, resource(std::move(resource))
				, stepsLeft(std::move(unloadingSteps))
			{}

			ResourceHandle handle;
			Resource::Ptr resource;
			Resource::DeinitSteps stepsLeft;
		};

		using LoadingDataPtr = std::unique_ptr<LoadingData>;
		using UnloadingDataPtr = std::unique_ptr<UnloadingData>;

	public:
		std::vector<LoadingDataPtr> resourcesWaitingInit;
		std::unordered_map<ResourceHandle, LoadingDataPtr> resourcesWaitingDependencies;
		std::vector<UnloadingDataPtr> resourcesWaitingDeinit;
		LoadDependencies loadDependencies;
	};

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
			std::string id = T::GetUniqueId(args...);
			return lockCustomResource<T>(
				static_cast<ResourcePath>(id),
				[](ResourceManager& resourceManager, ResourceHandle handle, Resource::Thread currentThread, Args&&... args){
					resourceManager.startResourceLoading(std::make_unique<ResourceLoading::LoadingData>(
						handle,
						T::GetInitSteps(),
						[args...]{ return std::make_unique<T>(std::move(args)...); },
						T::GetCreateThread()
					),
					currentThread);
				},
				currentThread,
				std::forward<Args>(args)...
			);
		}

		template<typename T>
		[[nodiscard]] const T* tryGetResource(ResourceHandle handle)
		{
			std::scoped_lock l(mDataMutex);
			auto it = mStorage.resources.find(handle);
			return it == mStorage.resources.end() ? nullptr : static_cast<T*>(it->second.get());
		}

		void lockResource(ResourceHandle handle);
		void unlockResource(ResourceHandle handle);

		void loadAtlasesData(const ResourcePath& listPath);

		void runThreadTasks(Resource::Thread currentThread);

	private:
		struct AnimGroupData
		{
			std::map<StringId, ResourcePath> clips;
			StringId stateMachineID;
			StringId defaultState;
		};

		using ReleaseFn = std::function<void(Resource*)>;

	private:
		ResourceHandle lockSurface(const ResourcePath& path);

		void startResourceLoading(ResourceLoading::LoadingDataPtr&& loadingGata, Resource::Thread currentThread);
		void loadOneAtlasData(const ResourcePath& path);
		std::vector<ResourcePath> loadSpriteAnimClipData(const ResourcePath& path);
		AnimGroupData loadAnimGroupData(const ResourcePath& path);
		void createLoadDependency(ResourceHandle dependency, ResourceLoading::LoadingDataPtr&& loadingData);
		void finalizeResourceLoading(ResourceHandle handle, Resource::Ptr&& resource, Resource::Thread currentThread);
		static void StartSpriteLoading(ResourceManager& resourceManager, ResourceHandle handle, Resource::Thread currentThread, const ResourcePath& path);

		template<typename T, typename Func, typename... Args>
		[[nodiscard]] ResourceHandle lockCustomResource(const ResourcePath& path, Func loadFn, Resource::Thread currentThread, Args&&... args)
		{
			SCOPED_PROFILER("ResourceManager::lockCustomResource");
			auto it = mStorage.pathsMap.find(path);
			if (it != mStorage.pathsMap.end())
			{
				++mStorage.resourceLocksCount[it->second];
				return ResourceHandle(it->second);
			}
			else
			{
				ResourceHandle handle = mStorage.createResourceLock(path);
				loadFn(*this, handle, currentThread, std::forward<Args>(args)...);
				return handle;
			}
		}

	private:
		ResourceStorage mStorage;
		ResourceLoading mLoading;
		RuntimeDependencies mDependencies;

		std::recursive_mutex mDataMutex;
	};
}
