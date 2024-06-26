#pragma once

#include <vector>

#include "EngineCommon/Types/TemplateAliases.h"

#include "GameData/Spatial/CellPos.h"

class WorldCell;
class CollisionComponent;
class TransformComponent;

namespace LightBlockingGeometry
{
	using CollisionGeometry = TupleVector<std::reference_wrapper<WorldCell>, const CollisionComponent*, const TransformComponent*>;
	using CalculatedGeometry = std::unordered_map<CellPos, std::vector<SimpleBorder>>;

	void CalculateLightGeometry(CalculatedGeometry& outGeometry, const CollisionGeometry& collisionGeometry);
}
