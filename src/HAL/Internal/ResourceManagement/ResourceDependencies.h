#pragma once

#include <vector>
#include <unordered_map>

#include "GameData/Core/ResourceHandle.h"

namespace HAL
{
	class ResourceDependencies
	{
	public:
		// this should be followed with resource lock
		void setFirstDependOnSecond(ResourceHandle dependentResource, ResourceHandle dependency);
		void setFirstDependOnSecond(ResourceHandle dependentResource, const std::vector<ResourceHandle>& dependencies);

		const std::vector<ResourceHandle>& getDependencies(ResourceHandle resource) const;
		const std::vector<ResourceHandle>& getDependentResources(ResourceHandle resource) const;

	protected:
		std::unordered_map<ResourceHandle, std::vector<ResourceHandle>> dependencies;
		std::unordered_map<ResourceHandle, std::vector<ResourceHandle>> dependentResources;
	};

	class RuntimeDependencies : public ResourceDependencies
	{
	public:
		// returns all dependencies of the resource (to be able to unlock them)
		[[nodiscard]]
		std::vector<ResourceHandle> removeResource(ResourceHandle resource);
	};

	class LoadDependencies : public ResourceDependencies
	{
	public:
		// returns all dependent resources that don't have dependencies anymore
		std::vector<ResourceHandle> resolveDependency(ResourceHandle dependency);
	};
}
