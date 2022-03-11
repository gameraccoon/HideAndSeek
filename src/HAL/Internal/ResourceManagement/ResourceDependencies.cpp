#include "Base/precomp.h"

#include "HAL/Internal/ResourceManagement/ResourceDependencies.h"

#include <ranges>
#include <algorithm>

namespace HAL
{
	void ResourceDependencies::setFirstDependOnSecond(ResourceHandle dependentResource, ResourceHandle dependency)
	{
		dependencies[dependentResource].push_back(dependency);
		dependentResources[dependency].push_back(dependentResource);
	}

	void ResourceDependencies::setFirstDependOnSecond(ResourceHandle dependentResource, const std::vector<ResourceHandle>& dependencies)
	{
		for (ResourceHandle dependency : dependencies)
		{
			setFirstDependOnSecond(dependentResource, dependency);
		}
	}

	const std::vector<ResourceHandle>& ResourceDependencies::getDependencies(ResourceHandle resource) const
	{
		static const std::vector<ResourceHandle> emptyVector;
		if (auto it = dependencies.find(resource); it != dependencies.end())
		{
			return it->second;
		}
		else
		{
			return emptyVector;
		}
	}

	const std::vector<ResourceHandle>& ResourceDependencies::getDependentResources(ResourceHandle resource) const
	{
		static const std::vector<ResourceHandle> emptyVector;
		if (auto it = dependentResources.find(resource); it != dependentResources.end())
		{
			return it->second;
		}
		else
		{
			return emptyVector;
		}
	}

	std::vector<ResourceHandle> RuntimeDependencies::removeResource(ResourceHandle resource)
	{
		std::vector<ResourceHandle> result;
		if (auto it = dependencies.find(resource); it != dependencies.end())
		{
			result = std::move(it->second);
			dependencies.erase(it);
		}

		if (auto it = dependentResources.find(resource); it != dependentResources.end())
		{
			Assert(it->second.empty(), "We removing a resource that have dependent resources: %d", resource);
			dependentResources.erase(it);
		}
		return result;
	}

	std::vector<ResourceHandle> LoadDependencies::resolveDependency(ResourceHandle dependency)
	{
		std::vector<ResourceHandle> result;

		// move the dependent resources to "result" and remove the record
		if (auto it = dependentResources.find(dependency); it != dependentResources.end())
		{
			result = std::move(it->second);
			dependentResources.erase(it);
		}

		// check that our dependencies are empty and remove the record
		if (auto it = dependencies.find(dependency); it != dependencies.end())
		{
			AssertFatal(it->second.empty(), "We resolving dependency that have unresolved dependencies itself: %d", dependency);
			dependencies.erase(it);
		}

		// resolve dependencies for the dependent resources
		// and filter "result" to keep only fully resolved ones
		for (int i = 0; i < static_cast<int>(result.size()); ++i)
		{
			auto it = dependencies.find(result[i]);
			if (it != dependencies.end())
			{
				auto removedRange = std::ranges::remove_if(it->second, [dependency](ResourceHandle handle){
					return handle == dependency;
				});
				Assert(!removedRange.empty(), "We've got a dependent resource missing info about its dependency");

				it->second.erase(removedRange.begin(), removedRange.end());

				// not fully resolved yet, remove from "result"
				if (!it->second.empty())
				{
					result.erase(result.begin() + i);
					--i;
				}
			}
			else
			{
				ReportError("We tried to resolve a dependency, but it's already resolved");
			}
		}

		return result;
	}
}
