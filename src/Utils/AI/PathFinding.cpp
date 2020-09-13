#include "Base/precomp.h"

#include <unordered_map>
#include <list>

#include "Utils/AI/PathFinding.h"

#include "GameData/AI/NavMesh.h"
#include "GameData/Core/BoundingBox.h"

#include "Utils/Geometry/Collide.h"

namespace PathFinding
{
	static constexpr size_t InvalidPolygon = std::numeric_limits<size_t>::max();

	bool IsPointInsideConvexHull(Vector2D point, const std::vector<Vector2D>& hull)
	{
		FOR_EACH_BORDER(hull.size(),
		{
			if (Collide::SignedArea(point, hull[i], hull[j]) < 0.0f)
			{
				return false;
			}
		});

		return true;
	}

	static size_t FindPolygonForPoint(Vector2D point, const NavMesh& navMesh)
	{
		Vector2D cellPointFloat;
		cellPointFloat = (point - navMesh.geometry.navMeshStart) / navMesh.spatialHash.cellSize;
		IntVector2D cellPoint(static_cast<int>(cellPointFloat.x), static_cast<int>(cellPointFloat.y));
		if (cellPoint.x < 0 || cellPoint.x >= navMesh.spatialHash.hashSize.x
			||
			cellPoint.y < 0 || cellPoint.y >= navMesh.spatialHash.hashSize.y)
		{
			return InvalidPolygon;
		}

		const std::vector<size_t>& polygons = navMesh.spatialHash.polygonsHash[cellPoint.x + cellPoint.y * navMesh.spatialHash.hashSize.x];
		std::vector<Vector2D> polygonPoints(navMesh.geometry.vertsPerPoly);
		for (size_t polygon : polygons)
		{
			size_t polyShift = polygon * navMesh.geometry.vertsPerPoly;
			for (size_t i = 0; i < navMesh.geometry.vertsPerPoly; ++i){
				polygonPoints[i] = navMesh.geometry.vertices[navMesh.geometry.indexes[polyShift + i]];
			}

			if (IsPointInsideConvexHull(point, polygonPoints))
			{
				return polygon;
			}
		}

		return InvalidPolygon;
	}

	struct LineSegmentToNeighborIntersection
	{
		NavMesh::InnerLinks::LinkData link;
		Vector2D intersectionPoint;
	};

	static_assert(std::is_trivially_copyable<LineSegmentToNeighborIntersection>(), "LineSegmentToNeighborIntersection should be trivially copyable");
	static_assert(std::is_trivially_constructible<LineSegmentToNeighborIntersection>(), "LineSegmentToNeighborIntersection should be trivially constructible");

	static LineSegmentToNeighborIntersection FindLineSegmentToNeighborIntersection(const NavMesh& navMesh, Vector2D start, Vector2D finish, size_t polygon, size_t ignoredNeighbor)
	{
		LineSegmentToNeighborIntersection result;
		result.link.neighbor = InvalidPolygon;
		float bestIntersectionQDistance = 0.0f;

		for (const NavMesh::InnerLinks::LinkData& link : navMesh.links.links[polygon])
		{
			if (link.neighbor != ignoredNeighbor)
			{
				Vector2D vert1 = navMesh.geometry.vertices[link.borderPoint1];
				Vector2D vert2 = navMesh.geometry.vertices[link.borderPoint2];

				bool areIntersect = Collide::AreLinesIntersect(start, finish, vert1, vert2);

				if (areIntersect)
				{
					Vector2D intersectionPoint = Collide::GetPointIntersect2Lines(start, finish, vert1, vert2);
					float intersectionQDistance = (intersectionPoint - finish).qSize();
					if (result.link.neighbor == InvalidPolygon
						// choose the variant closer to the finish point
						|| intersectionQDistance < bestIntersectionQDistance
						)
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
		PathPoint result = std::move(openList.begin()->second);
		openList.erase(openList.begin());
		openMap.erase(result.polygon);
		AssertFatal(openList.size() == openMap.size(), "openList and openMap have diverged");
		return result;
	}

	static PointScores CalculatePointScores(Vector2D pos, float previousG, Vector2D previousPos, Vector2D target)
	{
		PointScores result;
		result.g = previousG + (pos - previousPos).size();
		result.h = (target - pos).size();
		result.f = result.g + result.h;
		return result;
	}

	static std::pair<size_t, size_t> FindNextBestPoints(const std::vector<std::array<Vector2D, 2>>& portals, size_t start)
	{
		// find next points that are not equal in position to the current ones

		size_t bestLeft;
		for (bestLeft = start; bestLeft < portals.size() - 1; ++bestLeft)
		{
			if (portals[bestLeft][0] != portals[bestLeft+1][0])
			{
				++bestLeft;
				break;
			}
		}

		size_t bestRight;
		for (bestRight = start; bestRight < portals.size() - 1; ++bestRight)
		{
			if (portals[bestRight][1] != portals[bestRight+1][1])
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
					portals.push_back(std::array<Vector2D, 2>{navMesh.geometry.vertices[link.borderPoint1], navMesh.geometry.vertices[link.borderPoint2]});
					break;
				}
			}
		}

		Vector2D funnelStart = path[0].pos;
		size_t bestLeft = 0;
		size_t bestRight = 0;
		outFinalPath.push_back(path[0].pos);

		size_t i = 1;
		while (i < portals.size())
		{
			// if the point on the right of the left side of the funnel
			if (i > bestLeft && Collide::SignedArea(portals[i][0], funnelStart, portals[bestLeft][0]) > 0.0f)
			{
				// if the point on the right side of the right side of the funnel
				if (Collide::SignedArea(portals[i][0], funnelStart, portals[bestRight][1]) > 0.0f)
				{
					// restart search from the best right point
					funnelStart = portals[bestRight][1];
					i = bestRight + 1;
					std::tie(bestLeft, bestRight) = FindNextBestPoints(portals, bestRight);
					outFinalPath.push_back(funnelStart);
					continue;
				}
				bestLeft = i;
			}
			else if (bestLeft == i-1 && portals[i-1][0] == portals[i][0])
			{
				bestLeft = i;
			}

			// if the point on the left of the right side of the funnel
			if (i > bestRight && portals[i-1][1] != portals[i][1] && Collide::SignedArea(portals[i][1], funnelStart, portals[bestRight][1]) < 0.0f)
			{
				// if the point on the left side of the left side of the funnel
				if (Collide::SignedArea(portals[i][1], funnelStart, portals[bestLeft][0]) < 0.0f)
				{
					// restart search from the best left point
					funnelStart = portals[bestLeft][0];
					i = bestLeft + 1;
					std::tie(bestLeft, bestRight) = FindNextBestPoints(portals, bestLeft);
					outFinalPath.push_back(funnelStart);
					continue;
				}
				bestRight = i;
			}
			else if (bestRight == i-1 && portals[i-1][1] == portals[i][1])
			{
				bestRight = i;
			}

			// if we processed the last portal
			if (i == portals.size() - 1)
			{
				// check that the last point not outside the funnel
				bool crossedRight = Collide::SignedArea(path.back().pos, funnelStart, portals[bestRight][1]) > 0.0f;
				bool crossedLeft = Collide::SignedArea(path.back().pos, funnelStart, portals[bestLeft][0]) < 0.0f;

				// if the point is outside a portal that is rotated more than 90 degrees
				if (crossedLeft && crossedRight)
				{
					if ((portals[bestLeft][0] - funnelStart).qSize() < (portals[bestLeft][1] - funnelStart).qSize())
					{
						crossedRight = false;
					}
					else
					{
						crossedLeft = false;
					}
				}

				if (crossedRight)
				{
					funnelStart = portals[bestRight][1];
					i = bestRight + 1;
					std::tie(bestLeft, bestRight) = FindNextBestPoints(portals, bestRight);
					outFinalPath.push_back(funnelStart);
					continue;
				}
				else if (crossedLeft)
				{
					funnelStart = portals[bestLeft][0];
					i = bestLeft + 1;
					std::tie(bestLeft, bestRight) = FindNextBestPoints(portals, bestLeft);
					outFinalPath.push_back(funnelStart);
					continue;
				}
			}

			++i;
		}
		outFinalPath.push_back(path.back().pos);
	}

	void FindPath(std::vector<Vector2D>& outPath, const NavMesh& navMesh, Vector2D start, Vector2D finish)
	{
		outPath.clear();

		size_t startPolygon = FindPolygonForPoint(start, navMesh);
		size_t finishPolygon = FindPolygonForPoint(finish, navMesh);

		if (startPolygon == InvalidPolygon || finishPolygon == InvalidPolygon)
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
			firstPoint.previous = InvalidPolygon;
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
		currentPoint.polygon = InvalidPolygon;
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
			size_t bestNeighborID = rayIntersection.link.neighbor;

			if (bestNeighborID != InvalidPolygon)
			{
				PathPoint point;
				point.polygon = bestNeighborID;
				point.previous = currentPoint.polygon;
				point.pos = rayIntersection.intersectionPoint;
				point.scores = CalculatePointScores(point.pos, currentPoint.scores.g, currentPoint.pos, start);

				AddToOpenListIfBetter(openList, openMap, point);
			}

			for (const NavMesh::InnerLinks::LinkData& link : navMesh.links.links[currentPoint.polygon])
			{
				// we already added bestNeighborID node to the open list
				// also skip the previous point where we came from
				if (link.neighbor == bestNeighborID || link.neighbor == currentPoint.previous)
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
				bestPath.push_back(std::move(point));
			}

			// unwind the path
			size_t nextPolygon = currentPoint.polygon;
			while (nextPolygon != InvalidPolygon)
			{
				bestPath.push_back(closedList[nextPolygon]);
				nextPolygon = bestPath.back().previous;
			}
		}

		OptimizePath(outPath, bestPath, navMesh);
	}
} // namespace PathFinding
