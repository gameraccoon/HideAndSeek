#include "EngineCommon/precomp.h"

#include <exception>

#include <nemequ/debug-trap.h>

#include "EngineCommon/Debug/Assert.h"

AssertHandlerFn gGlobalAssertHandler = []{ psnip_trap(); };

AssertHandlerFn gGlobalFatalAssertHandler = []{ std::terminate(); };

bool gGlobalAllowAssertLogs = true;
