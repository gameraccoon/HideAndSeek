#include "Base/precomp.h"

#include "HAL/Internal/ResourceManagement/ResourceStorageData.h"

namespace HAL
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
