#include "Base/precomp.h"

#include "Utils/ResourceManagement/ResourceStorageData.h"

namespace ResourceLoading
{
	ResourceHandle ResourceStorage::createResourceLock(const std::string& id)
	{
		ResourceHandle currentHandle(handleIdx);
		idsMap[id] = currentHandle;
		idFindMap[currentHandle] = id;
		resourceLocksCount[currentHandle] = 1;
		++handleIdx;
		return currentHandle;
	}
}
