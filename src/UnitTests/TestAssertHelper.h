#pragma once

#include <gtest/gtest.h>
#include <raccoon-ecs/error_handling.h>

#include "Base/Debug/Assert.h"

inline void EnableFailOnAssert() noexcept
{
#ifdef DEBUG_CHECKS
	gGlobalAssertHandler = [](){
		GTEST_FAIL();
	};
	gGlobalFatalAssertHandler = [](){
		GTEST_FAIL();
	};
	gGlobalAllowAssertLogs = true;
#endif // DEBUG_CHECKS

#ifdef RACCOON_ECS_DEBUG_CHECKS_ENABLED
	RaccoonEcs::gErrorHandler = [](const std::string& error){
		ReportFatalError(error);
	};
#endif // RACCOON_ECS_DEBUG_CHECKS_ENABLED
}

inline void DisableFailOnAssert() noexcept
{
#ifdef DEBUG_CHECKS
	gGlobalAssertHandler = [](){};
	gGlobalFatalAssertHandler = [](){};
	gGlobalAllowAssertLogs = false;
#endif // DEBUG_CHECKS
}
