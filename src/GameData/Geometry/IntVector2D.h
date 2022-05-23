#pragma once

#include <math.h>
#include <nlohmann/json_fwd.hpp>

struct IntVector2D
{
	int x;
	int y;

	// leaves inner data uninitialized
	IntVector2D() = default;
	// can be created from initializer list
	constexpr IntVector2D(int x, int y) : x(x), y(y) {}

	[[nodiscard]] bool isZeroLength() const { return x == 0 && y == 0; }

	[[nodiscard]] bool operator==(IntVector2D other) const { return x == other.x && y == other.y; }

	[[nodiscard]] IntVector2D operator*(int scalar) const { return IntVector2D(x * scalar, y * scalar); }
	[[nodiscard]] IntVector2D operator/(int scalar) const { return IntVector2D(x / scalar, y / scalar); }
};

namespace std
{
	template <>
	struct hash<IntVector2D>
	{
		std::size_t operator()(IntVector2D k) const
		{
			return hash<int>()(k.x) ^ std::rotl(hash<int>()(k.y), 7);
		}
	};
}
