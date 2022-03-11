#pragma once

#include <map>
#include <unordered_map>

#include "Base/Types/String/Path.h"

#include "GameData/Core/ResourceHandle.h"

#include "HAL/Base/Resource.h"
#include "HAL/Base/Types.h"

namespace HAL
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
