#pragma once

#include <gtest/gtest.h>

#include "Base/Debug/Assert.h"
#include "ECS/ErrorHandling.h"

inline void EnableFailOnAssert() noexcept
{
#ifdef DEBUG_CHECKS
	gGlobalAssertHandler = [](){ GTEST_FAIL(); };
	gGlobalFatalAssertHandler = [](){ GTEST_FAIL(); };
	gGlobalAllowAssertLogs = true;
#endif // DEBUG_CHECKS

#ifdef ECS_DEBUG_CHECKS_ENABLED
	Ecs::gErrorHandler = [](const std::string& error) { ReportFatalError(error); };
#endif // ECS_DEBUG_CHECKS_ENABLED
}

inline void DisableFailOnAssert() noexcept
{
#ifdef DEBUG_CHECKS
	gGlobalAssertHandler = [](){};
	gGlobalFatalAssertHandler = [](){};
	gGlobalAllowAssertLogs = false;
#endif // DEBUG_CHECKS
}
