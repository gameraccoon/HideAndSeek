#include "Base/precomp.h"

#include "Utils/ResourceManagement/ResourceStorageData.h"

namespace ResourceLoading
{
	ResourceHandle ResourceStorage::createResourceLock(const ResourcePath& path)
	{
		ResourceHandle currentHandle(handleIdx);
		pathsMap[path] = currentHandle;
		pathFindMap[currentHandle] = path;
		resourceLocksCount[currentHandle] = 1;
		++handleIdx;
		return currentHandle;
	}
}
