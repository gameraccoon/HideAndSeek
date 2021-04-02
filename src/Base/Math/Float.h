#pragma once

#include <cmath>
#include "Base/CompilerHelpers.h"

// delete this code when C++20 is fully supported
#if __cplusplus <= 201703L && USED_COMPILER != COMPILER_MSVC
namespace std
{
	inline float lerp(float a, float b, float t)
	{
		return a + t * (b - a);
	}
}
#endif

namespace Math
{
	constexpr float DEFAULT_EPSILON = 0.001f;

	bool AreEqualWithEpsilon(float a, float b, float epsilon = DEFAULT_EPSILON);
	bool IsGreaterWithEpsilon(float a, float b, float epsilon = DEFAULT_EPSILON);
	bool IsGreaterOrEqualWithEpsilon(float a, float b, float epsilon = DEFAULT_EPSILON);
	bool IsLessWithEpsilon(float a, float b, float epsilon = DEFAULT_EPSILON);
	bool IsLessOrEqualWithEpsilon(float a, float b, float epsilon = DEFAULT_EPSILON);

	// special case of AreEqualWithEpsilon with a = v and b = 0
	bool IsNearZero(float v, float epsilon = DEFAULT_EPSILON);
}
