#pragma once

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

		[[nodiscard]] virtual bool isValid() const = 0;
	};
}
