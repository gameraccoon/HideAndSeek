#pragma once

#include "GameData/Geometry/Vector2D.h"
/**
 * Axis-aligned bounding box (AABB)
 */
class BoundingBox
{
public:
	constexpr BoundingBox(float minX, float minY, float maxX, float maxY)
		: minX(minX)
		, minY(minY)
		, maxX(maxX)
		, maxY(maxY)
	{
	}

	constexpr BoundingBox(Vector2D minPoint, Vector2D maxPoint)
		: minX(minPoint.x)
		, minY(minPoint.y)
		, maxX(maxPoint.x)
		, maxY(maxPoint.y)
	{
	}

	/** X-pos of left border */
	float minX;
	/** Y-pos of top border */
	float minY;
	/** X-pos of right border */
	float maxX;
	/** Y-pos of bottom border */
	float maxY;

	[[nodiscard]] BoundingBox operator+(Vector2D shift) const noexcept;
};
