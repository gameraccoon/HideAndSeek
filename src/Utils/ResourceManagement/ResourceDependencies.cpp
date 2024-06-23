#include "Base/precomp.h"

#include "Utils/ResourceManagement/ResourceDependencies.h"

#include <ranges>
#include <algorithm>

namespace ResourceLoading
{
	void ResourceDependencies::setFirstDependOnSecond(const ResourceHandle dependentResource, const ResourceHandle dependency)
	{
		mDependencies[dependentResource].push_back(dependency);
		mDependentResources[dependency].push_back(dependentResource);
	}

	void ResourceDependencies::setFirstDependOnSecond(const ResourceHandle dependentResource, const std::vector<ResourceHandle>& dependencies)
	{
		for (const ResourceHandle dependency : dependencies)
		{
			setFirstDependOnSecond(dependentResource, dependency);
		}
	}

	const std::vector<ResourceHandle>& ResourceDependencies::getDependencies(const ResourceHandle resource) const
	{
		static const std::vector<ResourceHandle> emptyVector;
		if (const auto it = mDependencies.find(resource); it != mDependencies.end())
		{
			return it->second;
		}

		return emptyVector;
	}

	const std::vector<ResourceHandle>& ResourceDependencies::getDependentResources(const ResourceHandle resource) const
	{
		static const std::vector<ResourceHandle> emptyVector;
		if (const auto it = mDependentResources.find(resource); it != mDependentResources.end())
		{
			return it->second;
		}

		return emptyVector;
	}

	std::vector<ResourceHandle> RuntimeDependencies::removeResource(const ResourceHandle resource)
	{
		std::vector<ResourceHandle> result;
		if (const auto it = mDependencies.find(resource); it != mDependencies.end())
		{
			result = std::move(it->second);
			mDependencies.erase(it);
		}

		if (const auto it = mDependentResources.find(resource); it != mDependentResources.end())
		{
			Assert(it->second.empty(), "We removing a resource that have dependent resources: %d", resource);
			mDependentResources.erase(it);
		}
		return result;
	}

	std::vector<ResourceHandle> LoadDependencies::resolveDependency(ResourceHandle dependency)
	{
		std::vector<ResourceHandle> result;

		// move the dependent resources to "result" and remove the record
		if (const auto it = mDependentResources.find(dependency); it != mDependentResources.end())
		{
			result = std::move(it->second);
			mDependentResources.erase(it);
		}

		// check that our dependencies are empty and remove the record
		if (const auto it = mDependencies.find(dependency); it != mDependencies.end())
		{
			AssertFatal(it->second.empty(), "We resolving dependency that have unresolved dependencies itself: %d", dependency);
			mDependencies.erase(it);
		}

		// resolve dependencies for the dependent resources
		// and filter "result" to keep only fully resolved ones
		for (int i = 0; i < static_cast<int>(result.size()); ++i)
		{
			auto it = mDependencies.find(result[i]);
			if (it != mDependencies.end())
			{
				auto removedRange = std::ranges::remove_if(it->second, [dependency](const ResourceHandle handle){
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
