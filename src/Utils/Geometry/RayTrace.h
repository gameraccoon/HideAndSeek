#pragma once

#include "GameData/Geometry/Vector2D.h"
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

	bool FastTrace(World& world, const Vector2D& startPoint, const Vector2D& endPoint);
	TraceResult Trace(World& world, Vector2D startPoint, Vector2D endPoint);
}
