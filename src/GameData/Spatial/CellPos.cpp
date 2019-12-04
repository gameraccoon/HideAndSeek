#include "GameData/Spatial/CellPos.h"

#include <nlohmann/json.hpp>

CellPos::CellPos(int x, int y)
	: x(x)
	, y(y)
{
}

CellPos CellPos::operator-(const CellPos& other) const
{
	return CellPos(x - other.x, y - other.y);
}

bool CellPos::operator==(const CellPos& other) const
{
	return x == other.x && y == other.y;
}

bool CellPos::operator!=(const CellPos& other) const
{
	return !(*this == other);
}

void to_json(nlohmann::json& outJson, const CellPos& pos)
{
	outJson = nlohmann::json{
		{"x", pos.x},
		{"y", pos.y}
	};
}

void from_json(const nlohmann::json& json, CellPos& outPos)
{
	json.at("x").get_to(outPos.x);
	json.at("y").get_to(outPos.y);
}
