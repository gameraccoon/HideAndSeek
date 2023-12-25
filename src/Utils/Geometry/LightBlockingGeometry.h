#pragma once

#include <vector>
#include <raccoon-ecs/entity.h>

#include "Base/Types/TemplateAliases.h"

#include "GameData/Geometry/Hull.h"

#include "GameData/Geometry/Vector2D.h"
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
