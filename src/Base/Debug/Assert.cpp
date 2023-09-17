#include "Base/precomp.h"

#include <exception>

#include <nemequ/debug-trap.h>

#include "Base/Debug/Assert.h"

AssertHandlerFn gGlobalAssertHandler = []{ psnip_trap(); };

AssertHandlerFn gGlobalFatalAssertHandler = []{ std::terminate(); };

bool gGlobalAllowAssertLogs = true;
