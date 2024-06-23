#pragma once

#include "HAL/Base/Types.h"

#include "GameData/Geometry/Vector2D.h"

namespace Graphics
{
	inline Vector2D QuadLerp(const QuadUV& uv, const Vector2D& v)
	{
		return Vector2D(std::lerp(uv.u1, uv.u2, v.x), std::lerp(uv.v1, uv.v2, v.y));
	}

	inline Vector2D QuadLerp(const QuadUV& uv, const float x, const float y)
	{
		return Vector2D(std::lerp(uv.u1, uv.u2, x), std::lerp(uv.v1, uv.v2, y));
	}
}
