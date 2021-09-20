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
			std::string id = T::getUniqueId(args...);
			auto it = mPathsMap.find(static_cast<ResourcePath>(id));
			if (it != mPathsMap.end())
			{
				++mResourceLocksCount[it->second];
				return ResourceHandle(it->second);
			}
			else
			{
				int thisHandle = createResourceLock(static_cast<ResourcePath>(id));
				mResources[thisHandle] = std::make_unique<T>(std::forward<Args>(args)...);
				return ResourceHandle(thisHandle);
			}
		}

		template<typename T>
		[[nodiscard]] const T* tryGetResource(ResourceHandle handle)
		{
			std::scoped_lock l(mDataMutex);
			auto it = mResources.find(handle.resourceIndex);
			return it == mResources.end() ? nullptr : static_cast<T*>(it->second.get());
		}

		void lockResource(ResourceHandle handle);
		void unlockResource(ResourceHandle handle);

		void loadAtlasesData(const ResourcePath& listPath);

	private:
		struct AtlasFrameData
		{
			ResourcePath atlasPath;
			Graphics::QuadUV quadUV;
		};

		struct AnimGroupData
		{
			std::map<StringId, ResourcePath> clips;
			StringId stateMachineID;
			StringId defaultState;
		};

		using ReleaseFn = std::function<void(Resource*)>;

	private:

		ResourceHandle lockSurface(const ResourcePath& path);

		ResourceHandle::IndexType createResourceLock(const ResourcePath& path);

		void loadOneAtlasData(const ResourcePath& path);
		std::vector<ResourcePath> loadSpriteAnimClipData(const ResourcePath& path);
		AnimGroupData loadAnimGroupData(const ResourcePath& path);

	private:
		std::unordered_map<ResourceHandle::IndexType, std::unique_ptr<Resource>> mResources;
		std::unordered_map<ResourceHandle::IndexType, int> mResourceLocksCount;
		std::unordered_map<ResourceHandle::IndexType, ReleaseFn> mResourceReleaseFns;
		std::unordered_map<ResourcePath, ResourceHandle::IndexType> mPathsMap;
		std::map<ResourceHandle::IndexType, ResourcePath> mPathFindMap;

		std::unordered_map<ResourcePath, AtlasFrameData> mAtlasFrames;

		ResourceHandle::IndexType mHandleIdx = 0;

		std::recursive_mutex mDataMutex;
	};
}
