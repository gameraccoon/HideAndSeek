#pragma once

#include "EngineData/Geometry/Vector2D.h"

class CollisionComponent;
class BoundingBox;

namespace Collide
{
	inline float SignedArea(const Vector2D a, const Vector2D b, const Vector2D c)
	{
		return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
	}

	bool DoCollide(const CollisionComponent* collisionA, const Vector2D& locationA,
		const CollisionComponent* collisionB, const Vector2D& locationB, Vector2D& outResist);

	void UpdateBoundingBox(CollisionComponent* collision);

	bool AreAABBsIntersect(const BoundingBox& boxA, const BoundingBox& boxB);
	bool AreAABBsIntersectInclusive(const BoundingBox& boxA, const BoundingBox& boxB);
	bool AreLinesIntersect(const Vector2D& a1, const Vector2D& a2, const Vector2D& b1, const Vector2D& b2);
	bool AreLinesParallel(const Vector2D& a1, const Vector2D& a2, const Vector2D& b1, const Vector2D& b2);
	bool IsLineIntersectAABB(const BoundingBox& box, const Vector2D& start, const Vector2D& finish);
	Vector2D GetPointIntersect2Lines(const Vector2D& a1, const Vector2D& a2, const Vector2D& b1, const Vector2D& b2);
	float DistanceToLineSegmentSq(Vector2D lineA, Vector2D lineB, Vector2D point);
	float FindDistanceToConvexHullSq(const std::vector<Vector2D>& hull, Vector2D point);
}

// macro that calls CODE for each border, where i and j are indexes of the vertices of this border
#define FOR_EACH_BORDER(size, CODE) { \
	do { \
		for (size_t i = 0; i + 1 < (size); ++i) \
		{ \
			size_t j = i + 1; \
			CODE \
		} \
		{ \
			size_t i = (size) - 1; \
			size_t j = 0; \
			CODE \
		} \
	} while(false); \
}
