#pragma once

#ifdef ECS_DEBUG_CHECKS_ENABLED

#include <functional>
#include <string>

namespace Ecs
{
	inline std::function<void(const std::string)> gErrorHandler = [](const std::string&){};
} // namespace Ecs

#endif // ECS_DEBUG_CHECKS_ENABLED
