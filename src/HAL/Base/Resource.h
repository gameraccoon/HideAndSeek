#pragma once

#include <vector>

#include <GameData/Core/ResourceHandle.h>

namespace HAL
{
	/**
	 * Base class for any resource type
	 */
	class Resource
	{
	public:
		Resource() = default;
		virtual ~Resource() = default;

		Resource(const Resource&) = delete;
		Resource& operator=(Resource&) = delete;
		Resource(Resource&&) = default;
		Resource& operator=(Resource&&) = default;

		virtual bool isValid() const = 0;

		void addDependency(ResourceHandle handle);
		void addDependencies(const std::vector<ResourceHandle>& handles);
		void addDependencies(std::vector<ResourceHandle>&& handles);
		const std::vector<ResourceHandle>& getResourceDependencies() const;
		std::vector<ResourceHandle>&& getResourceDependencies();

	private:
		std::vector<ResourceHandle> mResourceDependencies;
	};
}
