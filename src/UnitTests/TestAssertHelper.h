#pragma once

#include <gtest/gtest.h>
#include <raccoon-ecs/error_handling.h>
#include <atomic>

#include "EngineCommon/Debug/Assert.h"

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

class DisableAssertGuard
{
public:
	DisableAssertGuard()
	{
		AssertsTriggeredCount.store(0, std::memory_order_relaxed);
		AssertFatal(!IsGuardLocked.load(std::memory_order_relaxed), "DisableAssertGuard can't be created more than once at the same time");
		IsGuardLocked.store(true, std::memory_order_relaxed);
		DisableFailOnAssert();
		gGlobalAssertHandler = []() { AssertsTriggeredCount.fetch_add(1, std::memory_order_relaxed); };
	}

	~DisableAssertGuard()
	{
		EnableFailOnAssert();
		AssertFatal(IsGuardLocked.load(std::memory_order_relaxed), "Something is wrong with DisableAssertGuard lifetime");
		IsGuardLocked.store(false, std::memory_order_relaxed);
		AssertsTriggeredCount.store(0, std::memory_order_relaxed);
	}

	int getTriggeredAssertsCount() const
	{
		return AssertsTriggeredCount.load(std::memory_order_relaxed);
	}

private:
	inline static std::atomic<int> AssertsTriggeredCount = 0;
	inline static std::atomic<bool> IsGuardLocked = false;
};
