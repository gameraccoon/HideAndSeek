#pragma once

#include <vector>

#include "GameData/Geometry/Vector2D.h"
#include "GameData/Geometry/Border.h"

class LightBlockingGeometryComponent;

/**
 * Class that calculates visibility polygons
 *
 * Don't try to use a single instance in multiple threads, instead use one instance per thread
 */
class VisibilityPolygonCalculator
{
public:
	struct AngledBorder
	{
		AngledBorder(const Vector2D& posA, const Vector2D& posB)
			: coords(posA, posB)
			, angleA(posA.rotation().getValue())
			, angleB(posB.rotation().getValue())
		{}

		SimpleBorder coords;
		float angleA;
		float angleB;
	};

public:
	VisibilityPolygonCalculator() = default;

	// returns reference to the calculated polygon
	void calculateVisibilityPolygon(std::vector<Vector2D>& outVisibilityPolygon, const std::vector<const LightBlockingGeometryComponent*>& components, Vector2D sourcePos, Vector2D polygonMaxSize);

	// copying objects of this class has a little sense and usually indicates a poor use
	VisibilityPolygonCalculator(VisibilityPolygonCalculator const&) = delete;
	VisibilityPolygonCalculator& operator=(VisibilityPolygonCalculator const&) = delete;

private:
	// caches that can be reused
	struct Caches
	{
		std::vector<AngledBorder> bordersToTrace;
	};

private:
	[[nodiscard]] std::tuple<size_t, Vector2D> getNextClosestBorderFromCandidates(const std::vector<size_t>& potentialContinuations, const AngledBorder& closestBorder, float maxExtent) const;

private:
	Caches mCaches;
};
