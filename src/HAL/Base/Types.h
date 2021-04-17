#pragma once

#include <cstdint>

namespace Graphics
{
	struct QuadUV
	{
		float u1 = 0.0f;
		float v1 = 0.0f;
		float u2 = 1.0f;
		float v2 = 1.0f;
	};

	struct Color
	{
		std::uint8_t r;
		std::uint8_t g;
		std::uint8_t b;
		std::uint8_t a;
	};
}
