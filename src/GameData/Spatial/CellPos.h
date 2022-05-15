#pragma once

#include <functional>

#include <nlohmann/json_fwd.hpp>

struct CellPosDiff
{
	int x;
	int y;

	CellPosDiff(int x, int y);
};

struct CellPos
{
	int x;
	int y;

	CellPos() = default;
	constexpr CellPos(int x, int y)
		: x(x)
		, y(y)
	{
	}

	CellPosDiff operator-(const CellPos& other) const;
	CellPos operator+(const CellPosDiff& diff) const;
	std::strong_ordering operator<=>(const CellPos&) const = default;

	friend void to_json(nlohmann::json& outJson, const CellPos& pos);
	friend void from_json(const nlohmann::json& json, CellPos& outPos);
};

namespace std
{
	template<> struct hash<CellPos>
	{
		size_t operator()(const CellPos& cellPos) const noexcept
		{
			return hash<int>()(cellPos.x) ^ (hash<int>()(cellPos.y) << 1);
		}
	};
}
