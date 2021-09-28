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
		void setFirstDependOnSecond(ResourceHandle dependentResource, std::vector<ResourceHandle>&& dependencies);

		const std::vector<ResourceHandle>& getDependencies(ResourceHandle resource) const;
		const std::vector<ResourceHandle>& getDependentResources(ResourceHandle resource) const;

		// returns all dependencies of the resource (need to unlock them)
		[[nodiscard]]
		std::vector<ResourceHandle> removeResource(ResourceHandle resource);

	private:
		std::unordered_map<ResourceHandle, std::vector<ResourceHandle>> dependencies;
		std::unordered_map<ResourceHandle, std::vector<ResourceHandle>> dependentResources;
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
		std::unordered_map<ResourceHandle, std::unique_ptr<Resource>> resources;
		std::unordered_map<ResourceHandle, int> resourceLocksCount;
		std::unordered_map<ResourcePath, ResourceHandle> pathsMap;
		std::map<ResourceHandle, ResourcePath> pathFindMap;
		std::unordered_map<ResourcePath, AtlasFrameData> atlasFrames;
		ResourceHandle::IndexType handleIdx = 0;
	};

	// data for loading and resolving dependencies
	class ResourceLoading
	{
	public:
		struct ResourceLoadingData
		{
			ResourceLoadingData(
				ResourceHandle handle,
				std::unique_ptr<Resource>&& resource
			)
				: handle(handle)
				, resource(std::move(resource))
			{}

			ResourceHandle handle;
			std::unique_ptr<Resource> resource;
		};

	public:

		std::vector<ResourceLoadingData> resourcesWaitingInit;
		std::vector<ResourceLoadingData> resourcesWaitingDeinit;
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

		ResourceHandle lockSprite(const ResourcePath& path);
		ResourceHandle lockSpriteAnimationClip(const ResourcePath& path);
		ResourceHandle lockAnimationGroup(const ResourcePath& path);

		template<typename T, typename... Args>
		[[nodiscard]] ResourceHandle lockResource(Args&&... args)
		{
			std::scoped_lock l(mDataMutex);
			std::string id = T::GetUniqueId(args...);
			auto it = mStorage.pathsMap.find(static_cast<ResourcePath>(id));
			if (it != mStorage.pathsMap.end())
			{
				++mStorage.resourceLocksCount[it->second];
				return ResourceHandle(it->second);
			}
			else
			{
				ResourceHandle thisHandle = mStorage.createResourceLock(static_cast<ResourcePath>(id));
				startResourceLoading(thisHandle, [args...]{ return std::make_unique<T>(std::move(args)...); });

				return thisHandle;
			}
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

		void RunRenderThreadTasks();

	private:
		struct AnimGroupData
		{
			std::map<StringId, ResourcePath> clips;
			StringId stateMachineID;
			StringId defaultState;
		};

		using ReleaseFn = std::function<void(Resource*)>;
		using ResourceLoadFn = std::function<std::unique_ptr<Resource>()>;

	private:
		ResourceHandle lockSurface(const ResourcePath& path);

		void startResourceLoading(ResourceHandle handle, ResourceLoadFn&& resourceLoadFn);
		void loadOneAtlasData(const ResourcePath& path);
		std::vector<ResourcePath> loadSpriteAnimClipData(const ResourcePath& path);
		AnimGroupData loadAnimGroupData(const ResourcePath& path);

	private:
		ResourceStorage mStorage;
		ResourceLoading mLoading;
		ResourceDependencies mDependencies;

		std::recursive_mutex mDataMutex;
	};
}
