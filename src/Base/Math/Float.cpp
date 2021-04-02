#include "Base/precomp.h"

#include "Base/Math/Float.h"

#include <math.h>
#include <memory.h>

namespace Math
{
#ifdef DEBUG_CHECKS
	static void CheckFloatsCanBeCompared(float a, float b, float epsilon)
	{
		Assert(isfinite(a) && isfinite(b), "Comparison with NaN or Inf is not allowed");
		Assert(isfinite(epsilon), "Epsilon should not be NaN or Inf");

		if ((a < 0.0f) == (b < 0.0f))
		{
			if (std::ilogb(a) >= std::ilogb(epsilon) + 27
				&&
				std::ilogb(b) >= std::ilogb(epsilon) + 27)
			{
				ReportError("Numbers are too big to be compared with this epsilon: %f %f, epsilon %f", a, b, epsilon);
			}
		}
	}
#endif // DEBUG_CHECKS

	bool AreEqualWithEpsilon(float a, float b, float epsilon)
	{
#ifdef DEBUG_CHECKS
		CheckFloatsCanBeCompared(a, b, epsilon);
#endif // DEBUG_CHECKS
		return fabs(a - b) < epsilon;
	}

	bool IsGreaterWithEpsilon(float a, float b, float epsilon)
	{
#ifdef DEBUG_CHECKS
		CheckFloatsCanBeCompared(a, b, epsilon);
#endif // DEBUG_CHECKS
		return a > b + epsilon;
	}

	bool IsGreaterOrEqualWithEpsilon(float a, float b, float epsilon)
	{
#ifdef DEBUG_CHECKS
		CheckFloatsCanBeCompared(a, b, epsilon);
#endif // DEBUG_CHECKS
		return a + epsilon >= b;
	}

	bool IsLessWithEpsilon(float a, float b, float epsilon)
	{
#ifdef DEBUG_CHECKS
		CheckFloatsCanBeCompared(a, b, epsilon);
#endif // DEBUG_CHECKS
		return a < b - epsilon;
	}

	bool IsLessOrEqualWithEpsilon(float a, float b, float epsilon)
	{
#ifdef DEBUG_CHECKS
		CheckFloatsCanBeCompared(a, b, epsilon);
#endif // DEBUG_CHECKS
		return a - epsilon <= b;
	}

	bool IsNearZero(float v, float epsilon)
	{
#ifdef DEBUG_CHECKS
		Assert(isfinite(v), "Comparison with NaN or Inf is not allowed");
#endif // DEBUG_CHECKS
		return fabs(v) < epsilon;
	}
}
