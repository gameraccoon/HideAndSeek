#pragma once

#include "ECS/EntityManager.h"
#include "ECS/ComponentSetHolder.h"

#include "GameData/Spatial/SpatialEntityManager.h"
#include "GameData/Spatial/WorldCell.h"
#include "GameData/Spatial/CellPos.h"
#include "GameData/Core/Vector2D.h"

class ComponentSerializersHolder;

class SpatialWorldData
{
public:
	/**
	 * @brief getCellsAround returns cells that are inside or toucing a rect that have center in
	 * `centerPosition` and size of `rect`
	 * @param centerPosition center of the rectangle
	 * @param rect sizes of the rectangle
	 * @return pointers to loaded cells meeting the criteria
	 */
	std::vector<WorldCell*> getCellsAround(const Vector2D& centerPosition, const Vector2D& rect);
	SpatialEntityManager getCellManagersAround(const Vector2D& centerPosition, const Vector2D& rect);
	WorldCell* getCell(const CellPos& pos);
	WorldCell& getOrCreateCell(const CellPos& pos);

	std::unordered_map<CellPos, WorldCell>& getAllCells() { return mCells; }
	SpatialEntityManager getAllCellManagers();

	// returns true if cell changed
	static bool TransformCellPos(CellPos& inOutCellPos, Vector2D& inOutPos);
	static CellPos GetCellForPos(const Vector2D& pos);
	// returns true if cell changed
	static bool TransformCellForPos(CellPos& inOutCellPos, const Vector2D& pos);

	static Vector2D GetRelativeLocation(const CellPos& baseCell, const CellPos& targetCell, const Vector2D& targetPos);

	// to convert old data
	static std::pair<CellPos, Vector2D> TransformCellFromOldSize(const Vector2D& pos, const CellPos& oldPos, const Vector2D& oldSize);

	static Vector2D GetCellRealDistance(const CellPosDiff& cellDiff);

	nlohmann::json toJson(const ComponentSerializersHolder& componentSerializers) const;
	void fromJson(const nlohmann::json& json, const ComponentSerializersHolder& componentSerializers);

public:
	static constexpr int CellSizeInt = 200;
	static constexpr float CellSize = static_cast<float>(CellSizeInt);
	static constexpr Vector2D CellSizeVector{CellSize, CellSize};
	static constexpr float MaxObjectSize = CellSize * 0.5f;

private:
	CellPos mBaseCell{0, 0};
	std::unordered_map<CellPos, WorldCell> mCells;
};
