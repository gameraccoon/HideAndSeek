#pragma once

#include <map>
#include <unordered_map>

#include "Base/Types/String/Path.h"

#include "GameData/Resources/ResourceHandle.h"
#include "GameData/Resources/Resource.h"

#include "HAL/Base/Types.h"

namespace ResourceLoading
{
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
}
