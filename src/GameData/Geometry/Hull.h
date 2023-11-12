#pragma once

#include <vector>

#include <nlohmann/json_fwd.hpp>

#include "GameData/Geometry/Vector2D.h"
#include "GameData/Geometry/Border.h"
#include "GameData/Enums/HullType.generated.h"

class Hull
{
public:
	Hull() = default;

	/** Get the radius */
	[[nodiscard]] float getRadius() const { return mRadius; }
	/** Get pre-calculated squared radius */
	[[nodiscard]] float getQRadius() const { return mQRadius; }
	/** Set the radius */
	void setRadius(float newRadius) noexcept;
	/** Calc borders from points */
	void generateBorders() noexcept;

	[[nodiscard]] bool operator==(const Hull& other) const noexcept;
	[[nodiscard]] bool operator!=(const Hull& other) const noexcept;

	friend void to_json(nlohmann::json& outJson, const Hull& hull);
	friend void from_json(const nlohmann::json& json, Hull& hull);

public:
	/** Corners of a hull's borders */
	std::vector<Vector2D> points;
	/** Borders of hull (calculates from Points) */
	std::vector<Border> borders;
	HullType type = HullType::Angular;

private:
	/** Squared radius */
	float mQRadius = 0.0f;
	/** Radius for the circular hull */
	float mRadius = 0.0f;
};
