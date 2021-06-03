#pragma once

#include <GameData/Core/Vector2D.h>
#include <Base/Types/TemplateAliases.h>

class TransformComponent;
class CollisionComponent;

namespace PathBlockingGeometry
{
	void CalculatePathBlockingGeometry(std::vector<std::vector<Vector2D>>& outGeometry, const TupleVector<const CollisionComponent*, const TransformComponent*>& components);
}
