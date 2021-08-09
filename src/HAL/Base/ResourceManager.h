#pragma once

#include <map>
#include <unordered_map>
#include <functional>

#include "Base/Types/String/Path.h"
#include "Base/Debug/ConcurrentAccessDetector.h"

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

		ResourceHandle lockFont(const ResourcePath& path, int fontSize);
		ResourceHandle lockSprite(const ResourcePath& path);
		ResourceHandle lockSpriteAnimationClip(const ResourcePath& path);
		ResourceHandle lockAnimationGroup(const ResourcePath& path);
		ResourceHandle lockSound(const ResourcePath& path);
		ResourceHandle lockMusic(const ResourcePath& path);
	private:

		ResourceHandle lockSurface(const ResourcePath& path);
	public:

		template<typename T>
		[[nodiscard]] const T& getResource(ResourceHandle handle)
		{
			DETECT_CONCURRENT_ACCESS(mConcurrencyDetector);
			auto it = mResources.find(handle.resourceIndex);
			AssertFatal(it != mResources.end(), "Trying to access non loaded resource");
			return static_cast<T&>(*it->second);
		}

		template<typename T>
		[[nodiscard]] const T* tryGetResource(ResourceHandle handle)
		{
			DETECT_CONCURRENT_ACCESS(mConcurrencyDetector);
			auto it = mResources.find(handle.resourceIndex);
			return it == mResources.end() ? nullptr : static_cast<T*>(it->second.get());
		}

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
		int createResourceLock(const ResourcePath& path);

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

		int mHandleIdx = 0;

		ConcurrentAccessDetector mConcurrencyDetector;
	};
}
