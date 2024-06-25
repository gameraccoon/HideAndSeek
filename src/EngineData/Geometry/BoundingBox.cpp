#include "EngineCommon/precomp.h"

#include "EngineData/Geometry/BoundingBox.h"

#include <nlohmann/json.hpp>

BoundingBox BoundingBox::operator+(const Vector2D shift) const noexcept
{
	return BoundingBox(minX + shift.x, minY + shift.y, maxX + shift.x, maxY + shift.y);
}

void to_json(nlohmann::json& outJson, const BoundingBox& bb)
{
	outJson = nlohmann::json{
		{"maxX", bb.maxX},
		{"maxY", bb.maxY},
		{"minX", bb.minX},
		{"minY", bb.minY}
	};
}

void from_json(const nlohmann::json& json, BoundingBox& outBb)
{
	json.at("maxX").get_to(outBb.maxX);
	json.at("maxY").get_to(outBb.maxY);
	json.at("minX").get_to(outBb.minX);
	json.at("minY").get_to(outBb.minY);
}

static_assert(std::is_trivially_copyable<BoundingBox>(), "BoundingBox should be trivially copyable");
static_assert(std::is_trivially_destructible<BoundingBox>(), "BoundingBox should be trivially destructible");
