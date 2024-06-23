#pragma once

#include <nlohmann/json_fwd.hpp>

#include "GameData/Geometry/Vector2D.h"
/**
 * Axis-aligned bounding box (AABB)
 */
class BoundingBox
{
public:
	BoundingBox() = default;

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
	float minX = 0.0f;
	/** Y-pos of top border */
	float minY = 0.0f;
	/** X-pos of right border */
	float maxX = 0.0f;
	/** Y-pos of bottom border */
	float maxY = 0.0f;

	[[nodiscard]] BoundingBox operator+(Vector2D shift) const noexcept;

	friend void to_json(nlohmann::json& outJson, const BoundingBox& bb);
	friend void from_json(const nlohmann::json& json, BoundingBox& outBb);
};
