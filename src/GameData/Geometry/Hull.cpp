#include "Base/precomp.h"

#include "GameData/Geometry/Hull.h"

#include <nlohmann/json.hpp>

void Hull::generateBorders() noexcept
{
	borders.clear();
	for (size_t i = 0, count = points.size(); i < count; ++i)
	{
		borders.insert(borders.end(), Border(points[i], points[(i + 1) % count]));
	}
}

bool Hull::operator==(const Hull& other) const noexcept
{
	return type == other.type &&
		((type == HullType::Angular && points == other.points)
		||
		(type == HullType::Circular && mRadius == other.mRadius));
}

bool Hull::operator!=(const Hull& other) const noexcept
{
	return !(*this == other);
}

void Hull::setRadius(const float newRadius) noexcept
{
	mRadius = newRadius;
	mQRadius = newRadius * newRadius;
}

void to_json(nlohmann::json& outJson, const Hull& hull)
{
	outJson = nlohmann::json{
		{"points", hull.points},
		{"radius", hull.mRadius},
		{"type", hull.type}
	};
}

void from_json(const nlohmann::json& json, Hull& hull)
{
	json.at("points").get_to(hull.points);
	hull.setRadius(json.at("radius").get<float>());
	json.at("type").get_to(hull.type);
	hull.generateBorders();
}
