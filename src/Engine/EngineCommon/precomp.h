#pragma once

#include <cstdio>
#include <fstream>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>

#include <fnv1a/hash_fnv1a_constexpr.h>
#include <nlohmann/json_fwd.hpp>

#include "EngineCommon/CompilerHelpers.h"
#include "EngineCommon/Debug/MemoryLeakDetection.h"
#include "EngineCommon/Types/String/StringId.h"
#include "EngineCommon/Types/String/StringHelpers.h"
#include "EngineCommon/Debug/Log.h"
#include "EngineCommon/Debug/Assert.h"
#include "EngineCommon/Profile/ScopedProfiler.h"
