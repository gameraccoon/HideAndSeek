#pragma once

#include <gtest/gtest.h>

inline void EnableFailOnAssert() noexcept
{
#ifdef DEBUG_CHECKS
	gGlobalAssertHandler = [](){ GTEST_FAIL(); };
	gGlobalFatalAssertHandler = [](){ GTEST_FAIL(); };
	gGlobalAllowAssertLogs = true;
#endif // DEBUG_CHECKS
}

inline void DisableFailOnAssert() noexcept
{
#ifdef DEBUG_CHECKS
	gGlobalAssertHandler = [](){};
	gGlobalFatalAssertHandler = [](){};
	gGlobalAllowAssertLogs = false;
#endif // DEBUG_CHECKS
}
