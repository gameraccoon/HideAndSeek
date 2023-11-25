#pragma once

#include <map>
#include <unordered_map>

#include "Base/Types/String/ResourcePath.h"

#include "GameData/Resources/Resource.h"
#include "GameData/Resources/ResourceHandle.h"

#include "HAL/Base/Types.h"

namespace ResourceLoading
{
	// storage for loaded and ready resources
	class ResourceStorage
	{
	public:
		struct AtlasFrameData
		{
			RelativeResourcePath atlasPath;
			Graphics::QuadUV quadUV;
		};

	public:
		ResourceHandle createResourceLock(const std::string& id);

	public:
		std::unordered_map<ResourceHandle, Resource::Ptr> resources;
		std::unordered_map<ResourceHandle, int> resourceLocksCount;
		std::unordered_map<std::string, ResourceHandle> idsMap;
		std::map<ResourceHandle, std::string> idFindMap;
		std::unordered_map<RelativeResourcePath, AtlasFrameData> atlasFrames;
		ResourceHandle::IndexType handleIdx = 0;
	};
}
