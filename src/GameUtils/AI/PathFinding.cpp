#include "EngineCommon/precomp.h"

#include "GameUtils/AI/PathFinding.h"

#include <array>
#include <unordered_map>

#include "GameData/AI/NavMesh.h"

#include "GameUtils/Geometry/Collide.h"

namespace PathFinding
{
	static constexpr size_t INVALID_POLYGON = std::numeric_limits<size_t>::max();

	bool IsPointInsideConvexHull(const Vector2D point, const std::vector<Vector2D>& hull)
	{
		const size_t hullSize = hull.size();
		FOR_EACH_BORDER(hullSize, {
			if (Collide::SignedArea(point, hull[i], hull[j]) < 0.0f)
			{
				return false;
			}
		})

		return true;
	}

	static size_t FindPolygonForPoint(const Vector2D point, const NavMesh& navMesh)
	{
		const Vector2D cellPointFloat = (point - navMesh.geometry.navMeshStart) / navMesh.spatialHash.cellSize;
		const IntVector2D cellPoint(static_cast<int>(cellPointFloat.x), static_cast<int>(cellPointFloat.y));
		if (cellPoint.x < 0 || cellPoint.x >= navMesh.spatialHash.hashSize.x
			|| cellPoint.y < 0 || cellPoint.y >= navMesh.spatialHash.hashSize.y)
		{
			return INVALID_POLYGON;
		}

		const std::vector<size_t>& polygons = navMesh.spatialHash.polygonsHash[cellPoint.x + cellPoint.y * navMesh.spatialHash.hashSize.x];
		std::vector<Vector2D> polygonPoints(navMesh.geometry.verticesPerPoly);
		for (const size_t polygon : polygons)
		{
			const size_t polyShift = polygon * navMesh.geometry.verticesPerPoly;
			for (size_t i = 0; i < navMesh.geometry.verticesPerPoly; ++i)
			{
				polygonPoints[i] = navMesh.geometry.vertices[navMesh.geometry.indexes[polyShift + i]];
			}

			if (IsPointInsideConvexHull(point, polygonPoints))
			{
				return polygon;
			}
		}

		// try to search closest polygon
		size_t closestPolygon = INVALID_POLYGON;
		float minDistanceSq = std::numeric_limits<float>::max();
		for (int dx = -1; dx <= 1; ++dx)
		{
			const int pointX = cellPoint.x + dx;
			for (int dy = -1; dy <= 1; ++dy)
			{
				const int pointY = cellPoint.y + dy;
				if (pointX >= 0 && pointX < navMesh.spatialHash.hashSize.x
					&& pointY >= 0 && pointY < navMesh.spatialHash.hashSize.y)
				{
					const std::vector<size_t>& cellPolygons = navMesh.spatialHash.polygonsHash[cellPoint.x + cellPoint.y * navMesh.spatialHash.hashSize.x];
					for (const size_t polygon : cellPolygons)
					{
						const size_t polyShift = polygon * navMesh.geometry.verticesPerPoly;
						for (size_t i = 0; i < navMesh.geometry.verticesPerPoly; ++i)
						{
							polygonPoints[i] = navMesh.geometry.vertices[navMesh.geometry.indexes[polyShift + i]];
						}

						const float distSq = Collide::FindDistanceToConvexHullSq(polygonPoints, point);
						if (distSq < minDistanceSq)
						{
							minDistanceSq = distSq;
							closestPolygon = polygon;
						}
					}
				}
			}
		}

		return closestPolygon;
	}

	struct LineSegmentToNeighborIntersection
	{
		NavMesh::InnerLinks::LinkData link;
		Vector2D intersectionPoint;
	};

	static_assert(std::is_trivially_copyable<LineSegmentToNeighborIntersection>(), "LineSegmentToNeighborIntersection should be trivially copyable");
	static_assert(std::is_trivially_constructible<LineSegmentToNeighborIntersection>(), "LineSegmentToNeighborIntersection should be trivially constructible");

	static LineSegmentToNeighborIntersection FindLineSegmentToNeighborIntersection(const NavMesh& navMesh, const Vector2D start, const Vector2D finish, const size_t polygon, const size_t ignoredNeighbor)
	{
		LineSegmentToNeighborIntersection result{};
		result.link.neighbor = INVALID_POLYGON;
		float bestIntersectionQDistance = 0.0f;

		for (const NavMesh::InnerLinks::LinkData& link : navMesh.links.links[polygon])
		{
			if (link.neighbor != ignoredNeighbor)
			{
				Vector2D vert1 = navMesh.geometry.vertices[link.borderPoint1];
				Vector2D vert2 = navMesh.geometry.vertices[link.borderPoint2];

				const bool areIntersect = Collide::AreLinesIntersect(start, finish, vert1, vert2);

				if (areIntersect)
				{
					Vector2D intersectionPoint = Collide::GetPointIntersect2Lines(start, finish, vert1, vert2);
					const float intersectionQDistance = (intersectionPoint - finish).qSize();
					if (result.link.neighbor == INVALID_POLYGON
						// choose the variant closer to the finish point
						|| intersectionQDistance < bestIntersectionQDistance)
					{
						result.link = link;
						result.intersectionPoint = intersectionPoint;
						bestIntersectionQDistance = intersectionQDistance;
					}
				}
			}
		}

		return result;
	}

	struct PointScores
	{
		float g;
		float h;
		float f;
	};

	struct PathPoint
	{
		size_t polygon;
		Vector2D pos;
		PointScores scores;
		size_t previous;
	};

	static_assert(std::is_trivially_copyable<PathPoint>(), "PathPoint should be trivially copyable");
	static_assert(std::is_trivially_constructible<PathPoint>(), "PathPoint should be trivially constructible");

	using OpenListType = std::multimap<float, PathPoint>;
	using OpenMapType = std::unordered_map<size_t, OpenListType::iterator>;
	using ClosedListType = std::unordered_map<size_t, PathPoint>;

	static void AddToOpenListIfBetter(OpenListType& openList, OpenMapType& openMap, const PathPoint& point)
	{
		auto [mapIterator, isInserted] = openMap.try_emplace(point.polygon, openList.end());
		// if the node is new
		if (isInserted)
		{
			// add the new node to the openList and save its iterator to openMap
			mapIterator->second = openList.emplace(point.scores.f, point);
		}
		else
		{
			// if the node is better than the previous
			if (point.scores.f < mapIterator->second->second.scores.f)
			{
				// remove the old node
				openList.erase(mapIterator->second);
				// add the new node to the openList and save its iterator to openMap
				mapIterator->second = openList.emplace(point.scores.f, point);
			}
		}
		AssertFatal(openList.size() == openMap.size(), "openList and openMap have diverged");
	}

	static bool AddToClosedListIfBetter(ClosedListType& closedList, const PathPoint& point)
	{
		auto [mapIterator, isInserted] = closedList.try_emplace(point.polygon, point);

		if (!isInserted)
		{
			if (point.scores.f < mapIterator->second.scores.f)
			{
				mapIterator->second = point;
			}
			else
			{
				return false;
			}
		}
		return true;
	}

	static PathPoint PopBestFromOpenList(OpenListType& openList, OpenMapType& openMap)
	{
		const PathPoint result = openList.begin()->second;
		openList.erase(openList.begin());
		openMap.erase(result.polygon);
		AssertFatal(openList.size() == openMap.size(), "openList and openMap have diverged");
		return result;
	}

	static PointScores CalculatePointScores(const Vector2D pos, const float previousG, const Vector2D previousPos, const Vector2D target)
	{
		PointScores result;
		result.g = previousG + (pos - previousPos).size();
		result.h = (target - pos).size();
		result.f = result.g + result.h;
		return result;
	}

	static std::pair<size_t, size_t> FindNextBestPoints(const std::vector<std::array<Vector2D, 2>>& portals, const size_t start)
	{
		// find next points that are not equal in position to the current ones

		size_t bestLeft = start;
		for (; bestLeft < portals.size() - 1; ++bestLeft)
		{
			if (portals[bestLeft][0] != portals[bestLeft + 1][0])
			{
				++bestLeft;
				break;
			}
		}

		size_t bestRight = start;
		for (; bestRight < portals.size() - 1; ++bestRight)
		{
			if (portals[bestRight][1] != portals[bestRight + 1][1])
			{
				++bestRight;
				break;
			}
		}

		return std::make_pair(bestLeft, bestRight);
	}

	static void OptimizePath(std::vector<Vector2D>& outFinalPath, const std::vector<PathPoint>& path, const NavMesh& navMesh)
	{
		if (path.size() < 3)
		{
			for (const PathPoint& point : path)
			{
				outFinalPath.push_back(point.pos);
			}
			return;
		}

		// collect portals
		std::vector<std::array<Vector2D, 2>> portals;
		portals.reserve(path.size() - 1);
		for (size_t i = 0; i < path.size() - 1; ++i)
		{
			for (const NavMesh::InnerLinks::LinkData& link : navMesh.links.links[path[i].polygon])
			{
				if (link.neighbor == path[i + 1].polygon)
				{
					portals.push_back(std::array<Vector2D, 2>{ navMesh.geometry.vertices[link.borderPoint1], navMesh.geometry.vertices[link.borderPoint2] });
					break;
				}
			}
		}

		Vector2D funnelStart = path[0].pos;
		size_t bestLeft = 0;
		size_t bestRight = 0;
		outFinalPath.push_back(funnelStart);

		size_t i = 1;
		while (i <= portals.size())
		{
			bool updateLeft = false;
			bool updateRight = false;
			bool turnRight = false;
			bool turnLeft = false;

			// process the next portal
			if (i < portals.size())
			{
				// if the point is on the right side of the left border of the funnel
				if (i > bestLeft && Collide::SignedArea(portals[i][0], funnelStart, portals[bestLeft][0]) > 0.0f)
				{
					updateLeft = true;
					// if the point is on the right side of the right border of the funnel
					turnRight = Collide::SignedArea(portals[i][0], funnelStart, portals[bestRight][1]) > 0.0f;
				}
				else if (bestLeft == i - 1 && portals[i - 1][0] == portals[i][0])
				{
					updateLeft = true;
				}

				// if the point is on the left side of the right border of the funnel
				if (i > bestRight && Collide::SignedArea(portals[i][1], funnelStart, portals[bestRight][1]) < 0.0f)
				{
					updateRight = true;
					// if the point is on the left side of the left border of the funnel
					turnLeft = Collide::SignedArea(portals[i][1], funnelStart, portals[bestLeft][0]) < 0.0f;
				}
				else if (bestRight == i - 1 && portals[i - 1][1] == portals[i][1])
				{
					updateRight = true;
				}
			}
			else
			{
				// check that the last point is not outside the funnel
				turnRight = Collide::SignedArea(path.back().pos, funnelStart, portals[bestRight][1]) > 0.0f;
				turnLeft = Collide::SignedArea(path.back().pos, funnelStart, portals[bestLeft][0]) < 0.0f;
			}

			// if the portal will be rotated on more than 90 degrees to the funnel start
			{
				const size_t bestLeftPreview = updateLeft ? i : bestLeft;
				const size_t bestRightPreview = updateRight ? i : bestRight;
				if ((turnLeft && turnRight) || Collide::SignedArea(portals[bestRightPreview][1], funnelStart, portals[bestLeftPreview][0]) < 0.0f)
				{
					// make the closest turn
					if ((portals[bestLeftPreview][0] - funnelStart).qSize() < (portals[bestRightPreview][1] - funnelStart).qSize())
					{
						turnRight = false;
						turnLeft = true;
					}
					else
					{
						turnLeft = false;
						turnRight = true;
					}
				}
			}

			Assert(!(turnLeft && turnRight), "Only one turn is possible, two simultaneous turns detected");

			if (turnRight || turnLeft)
			{
				// restart search
				if (turnRight)
				{
					// restart from the best right point
					funnelStart = portals[bestRight][1];
					i = bestRight + 1;
				}
				else
				{
					// restart from the best left point
					funnelStart = portals[bestLeft][0];
					i = bestLeft + 1;
				}
				std::tie(bestLeft, bestRight) = FindNextBestPoints(portals, i - 1);
				outFinalPath.push_back(funnelStart);
				continue;
			}

			if (updateLeft) { bestLeft = i; }
			if (updateRight) { bestRight = i; }

			++i;
		}
		outFinalPath.push_back(path.back().pos);
	}

	void FindPath(std::vector<Vector2D>& outPath, const NavMesh& navMesh, Vector2D start, Vector2D finish)
	{
		outPath.clear();

		size_t startPolygon = FindPolygonForPoint(start, navMesh);
		size_t finishPolygon = FindPolygonForPoint(finish, navMesh);

		if (startPolygon == INVALID_POLYGON || finishPolygon == INVALID_POLYGON)
		{
			outPath.push_back(start);
			outPath.push_back(finish);
			return;
		}

		// open list sorted by the f value
		OpenListType openList;
		OpenMapType openMap;
		ClosedListType closedList;

		{
			PathPoint firstPoint;
			firstPoint.polygon = finishPolygon;
			firstPoint.previous = INVALID_POLYGON;
			// we are moving from finish to start, to then rewind the path
			firstPoint.pos = finish;
			firstPoint.scores.g = 0.0f;
			firstPoint.scores.h = (start - finish).size();
			firstPoint.scores.f = firstPoint.scores.g + firstPoint.scores.h;
			AddToOpenListIfBetter(openList, openMap, firstPoint);
		}

		// A* improved with first checks by raytracing

		unsigned int stepLimit = 100u;
		unsigned int step = 0u;
		PathPoint currentPoint;
		currentPoint.polygon = INVALID_POLYGON;
		while (!openList.empty() && step < stepLimit)
		{
			++step;

			currentPoint = PopBestFromOpenList(openList, openMap);
			bool isBetter = AddToClosedListIfBetter(closedList, currentPoint);

			if (currentPoint.polygon == startPolygon)
			{
				break;
			}

			if (!isBetter)
			{
				continue;
			}

			// find raycast neighbor (best possible neighbor from this point)
			LineSegmentToNeighborIntersection rayIntersection = FindLineSegmentToNeighborIntersection(navMesh, currentPoint.pos, start, currentPoint.polygon, currentPoint.previous);
			size_t bestNeighborId = rayIntersection.link.neighbor;

			if (bestNeighborId != INVALID_POLYGON)
			{
				PathPoint point;
				point.polygon = bestNeighborId;
				point.previous = currentPoint.polygon;
				point.pos = rayIntersection.intersectionPoint;
				point.scores = CalculatePointScores(point.pos, currentPoint.scores.g, currentPoint.pos, start);

				AddToOpenListIfBetter(openList, openMap, point);
			}

			for (const NavMesh::InnerLinks::LinkData& link : navMesh.links.links[currentPoint.polygon])
			{
				// we already added bestNeighborId node to the open list
				// also skip the previous point where we came from
				if (link.neighbor == bestNeighborId || link.neighbor == currentPoint.previous)
				{
					continue;
				}

				// select better border point
				Vector2D borderPointA = navMesh.geometry.vertices[link.borderPoint1];
				Vector2D borderPointB = navMesh.geometry.vertices[link.borderPoint2];

				PointScores scoresA = CalculatePointScores(borderPointA, currentPoint.scores.g, currentPoint.pos, start);
				PointScores scoresB = CalculatePointScores(borderPointB, currentPoint.scores.g, currentPoint.pos, start);

				PathPoint point;
				point.polygon = link.neighbor;
				point.previous = currentPoint.polygon;

				if (scoresA.f < scoresB.f)
				{
					point.pos = borderPointA;
					point.scores = scoresA;
				}
				else
				{
					point.pos = borderPointB;
					point.scores = scoresB;
				}

				AddToOpenListIfBetter(openList, openMap, point);
			}
		}

		std::vector<PathPoint> bestPath;
		{
			// add start point (it is not in the closed list)
			{
				PathPoint point;
				point.polygon = startPolygon;
				point.pos = start;
				bestPath.push_back(point);
			}

			// unwind the path
			size_t nextPolygon = currentPoint.polygon;
			while (nextPolygon != INVALID_POLYGON)
			{
				bestPath.push_back(closedList[nextPolygon]);
				nextPolygon = bestPath.back().previous;
			}
		}

		OptimizePath(outPath, bestPath, navMesh);
	}
} // namespace PathFinding
