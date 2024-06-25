#pragma once

#include "EngineCommon/Types/TemplateAliases.h"

#include "EngineData/Geometry/Vector2D.h"

class TransformComponent;
class CollisionComponent;

namespace PathBlockingGeometry
{
	void CalculatePathBlockingGeometry(std::vector<std::vector<Vector2D>>& outGeometry, const TupleVector<const CollisionComponent*, const TransformComponent*>& components);
}
