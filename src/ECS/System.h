#pragma once

#include <string>

namespace Ecs
{
	/**
	 * The base class for game Systems
	 *
	 * Abstract
	 */
	class System
	{
	public:
		virtual ~System() = default;

		virtual void update() = 0;
		virtual void initResources() {}
		virtual void shutdown() {}

		// for debug purposes
		virtual std::string getName() = 0;
	};

} // namespace Ecs
