#include "Base/precomp.h"

#include "GameData/Spatial/CellPos.h"

#include <nlohmann/json.hpp>

CellPosDiff::CellPosDiff(int x, int y)
	: x(x)
	, y(y)
{
}

CellPosDiff CellPos::operator-(const CellPos& other) const
{
	return CellPosDiff(x - other.x, y - other.y);
}

CellPos CellPos::operator+(const CellPosDiff& diff) const
{
	return CellPos(x + diff.x, y + diff.y);
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
