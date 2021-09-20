#include "Base/precomp.h"

#include "Base/Types/ComplexTypes/VectorUtils.h"

#include "HAL/Base/Resource.h"

namespace HAL
{
	void Resource::addDependency(ResourceHandle handle)
	{
		mResourceDependencies.push_back(handle);
	}

	void Resource::addDependencies(const std::vector<ResourceHandle>& handles)
	{
		VectorUtils::AppendToVector(mResourceDependencies, handles);
	}

	void Resource::addDependencies(std::vector<ResourceHandle>&& handles)
	{
		VectorUtils::AppendToVector(mResourceDependencies, std::move(handles));
	}

	const std::vector<ResourceHandle>& Resource::getResourceDependencies() const
	{
		return mResourceDependencies;
	}

	std::vector<ResourceHandle>&& Resource::getResourceDependencies()
	{
		return std::move(mResourceDependencies);
	}
} // namespace HAL
