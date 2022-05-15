#pragma once

#include <cstdint>

#include "GameData/Geometry/Vector2D.h"

namespace Graphics
{
	struct QuadUV
	{
		float u1 = 0.0f;
		float v1 = 0.0f;
		float u2 = 1.0f;
		float v2 = 1.0f;

		QuadUV() = default;

		QuadUV(float u1, float v1, float u2, float v2)
			: u1(u1)
			, v1(v1)
			, u2(u2)
			, v2(v2)
		{}

		QuadUV(Vector2D start, Vector2D end)
			: u1(start.x)
			, v1(start.y)
			, u2(end.x)
			, v2(end.y)
		{}
	};

	struct Color
	{
		std::uint8_t r;
		std::uint8_t g;
		std::uint8_t b;
		std::uint8_t a;
	};

	struct DrawPoint
	{
		Vector2D spacePoint;
		Vector2D texturePoint;
	};
}
