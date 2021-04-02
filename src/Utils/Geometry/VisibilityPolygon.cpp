#include "Base/precomp.h"

#include "Utils/Geometry/VisibilityPolygon.h"

#include <algorithm>
#include <ranges>

#include "Base/Math/Float.h"

#include "GameData/Components/LightBlockingGeometryComponent.generated.h"
#include "GameData/Spatial/SpatialWorldData.h"

#include "Utils/Geometry/Collide.h"

static float CalcClockwiseDirection(const Vector2D& a, const Vector2D& b)
{
	return a.x * (b.y - a.y) - a.y * (b.x - a.x);
}

static bool LessPointAngle(const VisibilityPolygonCalculator::AngledBorder& a, const VisibilityPolygonCalculator::AngledBorder& b)
{
	return a.angleA < b.angleA;
}

static bool NormalizePoint(Vector2D& point, const Vector2D& pair, float left, float right, float top, float bottom)
{
	if (point.x < left)
	{
		point = pair + abs((left - pair.x)/(point.x - pair.x)) * (point-pair);
		return true;
	}
	else if (point.x > right)
	{
		point = pair + abs((pair.x - right)/(pair.x - point.x)) * (point-pair);
		return true;
	}

	if (point.y < top)
	{
		point = pair + abs((top - pair.y)/(point.y - pair.y)) * (point-pair);
		return true;
	}
	else if (point.y > bottom)
	{
		point = pair + abs((pair.y - bottom)/(pair.y - point.y)) * (point-pair);
		return true;
	}
	return false;
}

static void FilterOutPotentialContinuations(std::vector<size_t>& outPotentialContinuations, std::vector<VisibilityPolygonCalculator::AngledBorder>& boardersToTrace, float maxAngle)
{
	// filter out the continuations that are not longer stick out of the end of the closest border
	outPotentialContinuations.erase(
		std::remove_if(
			outPotentialContinuations.begin(),
			outPotentialContinuations.end(),
			[&boardersToTrace, maxAngle](size_t idx)
			{
				return (Rotator::NormalizeRawAngle(maxAngle - boardersToTrace[idx].angleB) >= 0.0f);
			}
		),
		outPotentialContinuations.end()
	);
}

std::tuple<size_t, Vector2D> VisibilityPolygonCalculator::GetNextClosestBorderFromCondidates(const std::vector<size_t>& potentialContinuations, const AngledBorder& closestBorder, float maxExtent) const
{
	float closestBorderQDist = std::numeric_limits<float>::max();
	Vector2D closestIntersectionPoint;
	size_t closestBorderIdx;
	Vector2D raytraceCoordinate = closestBorder.coords.b.unit() * (maxExtent + 10.0f);
	for (size_t idx : potentialContinuations)
	{
		const AngledBorder& potentialContinuation = mCaches.bordersToTrace[idx];
		Vector2D intersectionPoint = Collide::GetPointIntersect2Lines(potentialContinuation.coords.a, potentialContinuation.coords.b, ZERO_VECTOR, raytraceCoordinate);
		float intersectionQDist = intersectionPoint.qSize();
		if (intersectionQDist < closestBorderQDist)
		{
			closestBorderQDist = intersectionQDist;
			closestBorderIdx = idx;
			closestIntersectionPoint = intersectionPoint;
		}
	}
	return std::make_tuple(closestBorderIdx, closestIntersectionPoint);
}

void VisibilityPolygonCalculator::calculateVisibilityPolygon(std::vector<Vector2D>& outVisibilityPolygon, const std::vector<LightBlockingGeometryComponent*>& components, Vector2D sourcePos, Vector2D polygonMaxSize)
{
	outVisibilityPolygon.clear();

	mCaches.bordersToTrace.clear();
	mCaches.bordersToTrace.reserve(components.size() * 4 + 4);

	const float maxExtent = std::max(polygonMaxSize.x, polygonMaxSize.y);

	// populate the borders of visible area
	const float left = -polygonMaxSize.x * 0.5f;
	const float right = polygonMaxSize.x * 0.5f;
	const float top = -polygonMaxSize.y * 0.5f;
	const float bottom = polygonMaxSize.y * 0.5f;
	const Vector2D leftTop(left, top);
	const Vector2D rightTop(right, top);
	const Vector2D rightBottom(right, bottom);
	const Vector2D leftBottom(left, bottom);
	const Vector2D startTrace(left * 1.1f, 0.0f);
	mCaches.bordersToTrace.emplace_back(leftTop, rightTop);
	mCaches.bordersToTrace.emplace_back(rightTop, rightBottom);
	mCaches.bordersToTrace.emplace_back(rightBottom, leftBottom);
	mCaches.bordersToTrace.emplace_back(leftBottom, leftTop);

	// find borders that facing the light source and their points inside the limits
	for (const LightBlockingGeometryComponent* lightBlockingGeometry : components)
	{
		for (SimpleBorder border : lightBlockingGeometry->getBorders())
		{
			border.a -= sourcePos;
			border.b -= sourcePos;

			// keep only borders inside the draw area and facing the light source
			bool isVisible = (
				(border.a.x > left && border.a.x < right && border.a.y > top && border.a.y < bottom)
				|| (border.b.x > left && border.b.x < right && border.b.y > top && border.b.y < bottom)
			) && CalcClockwiseDirection(border.a, border.b) < 0.0f;

			if (isVisible)
			{
				NormalizePoint(border.a, border.b, left, right, top, bottom);
				NormalizePoint(border.b, border.a, left, right, top, bottom);

				// revert border points so they go clockwise
				mCaches.bordersToTrace.emplace_back(border.b, border.a);
			}
		}
	}

	if (mCaches.bordersToTrace.empty())
	{
		return;
	}

	// sort the borders so we can iterate over them in clockwise order
	std::ranges::sort(mCaches.bordersToTrace, LessPointAngle);

	size_t closestBorderIdx = 0;
	std::vector<size_t> potentialContinuations;

	// preparation loop
	{
		float closestBorderQDist = std::numeric_limits<float>::max();
		for (size_t i = 1; i < mCaches.bordersToTrace.size(); ++i)
		{
			const AngledBorder& item = mCaches.bordersToTrace[i];
			const AngledBorder& previousItem = mCaches.bordersToTrace[i - 1];

			size_t processedItemIdx = i;

			// fix a corner case with two or more points have the exact same angle
			// the problem is present only if the angles are exactly equal (not nearly equal)
			if (item.angleA == previousItem.angleA)
			{
				// if the next border is closer than the previous one, swap them
				if (item.coords.a.qSize() < previousItem.coords.a.qSize())
				{
					std::iter_swap(mCaches.bordersToTrace.begin() + static_cast<int>(i), mCaches.bordersToTrace.begin() + static_cast<int>(i - 1));
					processedItemIdx = i - 1;
				}
			}

			// find the closest border for the zero angle and potential next borders
			const AngledBorder& processedItem = mCaches.bordersToTrace[processedItemIdx];
			if (Collide::AreLinesIntersect(processedItem.coords.a, processedItem.coords.b, ZERO_VECTOR, startTrace))
			{
				float intersectionQDist = Collide::GetPointIntersect2Lines(processedItem.coords.a, processedItem.coords.b, ZERO_VECTOR, startTrace).qSize();
				if (intersectionQDist < closestBorderQDist)
				{
					closestBorderQDist = intersectionQDist;
					closestBorderIdx = processedItemIdx;
				}
				potentialContinuations.push_back(processedItemIdx);
			}
		}
	}

	FilterOutPotentialContinuations(potentialContinuations, mCaches.bordersToTrace, mCaches.bordersToTrace[closestBorderIdx].angleB);

	// calculate visibility polygon
	for (size_t i = 0, iSize = mCaches.bordersToTrace.size(); i < iSize; ++i)
	{
		AngledBorder& border = mCaches.bordersToTrace[i];
		AngledBorder& closestBorder = mCaches.bordersToTrace[closestBorderIdx];

		// if this border is outside of the angles of the closest border
		if (Rotator::NormalizeRawAngle(closestBorder.angleB - border.angleA) <= 0.0f)
		{
			// if this border is a continuation of the closest border
			if (closestBorder.coords.b.isNearlyEqualTo(border.coords.a))
			{
				outVisibilityPolygon.push_back(border.coords.a);
				closestBorderIdx = i;
			}
			else
			{
				outVisibilityPolygon.push_back(closestBorder.coords.b);
				if (!potentialContinuations.empty())
				{
					// find the best candidate to cast the shadow to
					Vector2D closestIntersectionPoint;
					std::tie(closestBorderIdx, closestIntersectionPoint) = GetNextClosestBorderFromCondidates(potentialContinuations, closestBorder, maxExtent);

					outVisibilityPolygon.push_back(closestIntersectionPoint);
				}
				else
				{
					// special case when the new border has the same begin angle as the old one
					// but they end/begin are not on the same point
					// use this border as continuation
					closestBorderIdx = i;
					outVisibilityPolygon.push_back(border.coords.a);
				}
			}

			FilterOutPotentialContinuations(potentialContinuations, mCaches.bordersToTrace, mCaches.bordersToTrace[closestBorderIdx].angleB);

			// process this point again if we jumped to some other border
			if (closestBorderIdx != i)
			{
				--i;
			}
			continue;
		}

		// if the first point is behind the closest border
		if (Collide::SignedArea(closestBorder.coords.a, closestBorder.coords.b, border.coords.a) < 0.0f)
		{
			// if its end sticks out of the end of the closest border
			if (Rotator::NormalizeRawAngle(closestBorder.angleB - border.angleB) < 0.0f)
			{
				potentialContinuations.push_back(i);
			}
			continue;
		}

		// if the first point is closer than the closest border
		// calculate the intersection point to cast the shadow
		Vector2D intersectPoint = Collide::GetPointIntersect2Lines(closestBorder.coords.a, closestBorder.coords.b, ZERO_VECTOR, border.coords.a);

		if (!intersectPoint.isNearlyEqualTo(border.coords.a))
		{
			outVisibilityPolygon.push_back(intersectPoint);
		}
		outVisibilityPolygon.push_back(border.coords.a);

		// the old "closest" border can still stick out of our new "closest" border
		// so keep it in the list (will be filtered later if its end is not really sticking out)
		potentialContinuations.push_back(closestBorderIdx);

		closestBorderIdx = i;

		FilterOutPotentialContinuations(potentialContinuations, mCaches.bordersToTrace, mCaches.bordersToTrace[closestBorderIdx].angleB);
	}

	// process the potential continuations that have left
	Vector2D closestIntersectionPoint;
	while (!potentialContinuations.empty())
	{
		// if this border goes to the beginning of the, it means we're done with the polygon
		if (Rotator::NormalizeRawAngle(Rotator::MinValue - mCaches.bordersToTrace[closestBorderIdx].angleB) < 0.0f)
		{
			break;
		}

		AngledBorder& closestBorder = mCaches.bordersToTrace[closestBorderIdx];
		outVisibilityPolygon.push_back(closestBorder.coords.b);
		std::tie(closestBorderIdx, closestIntersectionPoint) = GetNextClosestBorderFromCondidates(potentialContinuations, closestBorder, maxExtent);
		outVisibilityPolygon.push_back(closestIntersectionPoint);
		FilterOutPotentialContinuations(potentialContinuations, mCaches.bordersToTrace, mCaches.bordersToTrace[closestBorderIdx].angleB);
	}
}
