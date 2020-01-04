#pragma once

#include <map>
#include <unordered_map>
#include <functional>

#include "Base/Debug/Assert.h"
#include "Base/Types/String/StringID.h"
#include "Base/Types/String/Path.h"

#include "GameData/Core/ResourceHandle.h"

#include "HAL/Base/Resource.h"
#include "HAL/Base/Types.h"
#include "HAL/EngineFwd.h"

namespace HAL
{
	/**
	 * Class that manages resources such as textures
	 */
	class ResourceManager
	{
	public:
		explicit ResourceManager(Engine& engine);

		~ResourceManager() = default;

		ResourceManager(const ResourceManager&) = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;
		ResourceManager(ResourceManager&&) = delete;
		ResourceManager& operator=(ResourceManager&&) = delete;

		ResourceHandle lockFont(const ResourcePath& path, int fontSize);
		ResourceHandle lockTexture(const ResourcePath& path);
		ResourceHandle lockSprite(const ResourcePath& path);
		ResourceHandle lockSpriteAnimationClip(const ResourcePath& path);
		ResourceHandle lockAnimationGroup(const ResourcePath& path);
		ResourceHandle lockSound(const ResourcePath& path);
		ResourceHandle lockMusic(const ResourcePath& path);

		template<typename T>
		[[nodiscard]] const T& getResource(ResourceHandle handle)
		{
			auto it = mResources.find(handle.ResourceIndex);
			AssertFatal(it != mResources.end(), "Trying to access non loaded resource");
			return static_cast<T&>(*(it->second.get()));
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
			std::map<StringID, ResourcePath> clips;
			StringID stateMachineID;
			StringID defaultState;
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

		Engine& mEngine;

		int mHandleIdx = 0;
	};
}
