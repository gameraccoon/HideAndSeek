#pragma once

#include <raccoon-ecs/async_operations.h>

#include "GameData/Core/Vector2D.h"
#include "GameData/Spatial/SpatialEntity.h"

class World;
struct Vector2D;
class BoundingBox;

namespace RayTrace
{
	struct TraceResult {
		bool hasHit = false;
		SpatialEntity hitEntity;
		Vector2D hitPoint;
		Vector2D hitBorderNormal;
	};

	using CollisionsFilter = const RaccoonEcs::ComponentFilter<const class CollisionComponent, const class TransformComponent>;

	bool FastTrace(World& world, CollisionsFilter& collisionsFilter, const Vector2D& startPoint, const Vector2D& endPoint);
	TraceResult Trace(World& world, CollisionsFilter& collisionsFilter, Vector2D startPoint, Vector2D endPoint);
}
