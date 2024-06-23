#pragma once

#include <cstdio>
#include <fstream>
#include <functional>
#include <string>
#include <type_traits>
#include <unordered_map>

#include <fnv1a/hash_fnv1a_constexpr.h>
#include <nlohmann/json_fwd.hpp>

#include "Base/CompilerHelpers.h"
#include "Base/Debug/MemoryLeakDetection.h"
#include "Base/Types/String/StringId.h"
#include "Base/Types/String/StringHelpers.h"
#include "Base/Debug/Log.h"
#include "Base/Debug/Assert.h"
#include "Base/Profile/ScopedProfiler.h"
