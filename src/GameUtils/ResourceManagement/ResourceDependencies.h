#pragma once

#include <unordered_map>
#include <vector>

#include "GameData/Resources/ResourceHandle.h"

namespace ResourceLoading
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
		std::unordered_map<ResourceHandle, std::vector<ResourceHandle>> mDependencies;
		std::unordered_map<ResourceHandle, std::vector<ResourceHandle>> mDependentResources;
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
} // namespace ResourceLoading
