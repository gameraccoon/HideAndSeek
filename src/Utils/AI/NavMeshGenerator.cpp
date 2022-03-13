#include "Base/precomp.h"

#include "Utils/AI/NavMeshGenerator.h"

#include <cmath>
#include <unordered_map>
#include <algorithm>

#include "polypartition.h"

#include "Base/Types/TemplateAliases.h"

#include "GameData/Geometry/BoundingBox.h"

#include "Utils/Geometry/Collide.h"

namespace NavMeshGenerator
{
	static IntVector2D IntVecFromTPPLPoint(TPPLPoint point)
	{
		return IntVector2D(static_cast<int>(std::round(point.x)), static_cast<int>(std::round(point.y)));
	}

#ifdef DEBUG_CHECKS
	static Vector2D VecFromTPPLPoint(TPPLPoint point)
	{
		return Vector2D(point.x, point.y);
	}
#endif // DEBUG_CHECKS

	void GenerateNavMeshGeometry(NavMesh::Geometry& outGeometry, const std::vector<std::vector<Vector2D>>& pathBlockingGeometry, Vector2D start, Vector2D size)
	{
		outGeometry.vertices.clear();
		outGeometry.indexes.clear();
		// only triangles are supported now
		outGeometry.verticesPerPoly = 3;
		outGeometry.isCalculated = false;

		IntVector2D halfSize(static_cast<int>(size.x * 0.5f), static_cast<int>(size.y * 0.5f));
		IntVector2D centerPos(static_cast<int>(start.x) + halfSize.x, static_cast<int>(start.y) + halfSize.y);

		std::vector<TPPLPoly> polygons;
		std::vector<TPPLPoly> resultPolygons;

		TPPLPoly borderPolygon;
		borderPolygon.Init(4);
		borderPolygon[0].x = static_cast<tppl_float>(centerPos.x - halfSize.x);
		borderPolygon[0].y = static_cast<tppl_float>(centerPos.y - halfSize.y);
		borderPolygon[1].x = static_cast<tppl_float>(centerPos.x + halfSize.x);
		borderPolygon[1].y = static_cast<tppl_float>(centerPos.y - halfSize.y);
		borderPolygon[2].x = static_cast<tppl_float>(centerPos.x + halfSize.x);
		borderPolygon[2].y = static_cast<tppl_float>(centerPos.y + halfSize.y);
		borderPolygon[3].x = static_cast<tppl_float>(centerPos.x - halfSize.x);
		borderPolygon[3].y = static_cast<tppl_float>(centerPos.y + halfSize.y);
		polygons.push_back(borderPolygon);

		for (const auto& polygon : pathBlockingGeometry)
		{
			TPPLPoly ttplPolygon;
			const size_t pointsCount = polygon.size();

			ttplPolygon.Init(pointsCount);

			for (size_t i = 0; i < pointsCount; ++i)
			{
				ttplPolygon[i].x = polygon[i].x;
				ttplPolygon[i].y = polygon[i].y;
			}
			ttplPolygon.SetHole(true);
			polygons.push_back(std::move(ttplPolygon));
		}

		TPPLPartition pp{};
		pp.Triangulate_EC(&polygons, &resultPolygons);

		std::unordered_map<IntVector2D, size_t> verticesMap;
		for (const TPPLPoly& polygon : resultPolygons)
		{
			verticesMap.emplace(IntVecFromTPPLPoint(polygon[0]), static_cast<size_t>(0u));
			verticesMap.emplace(IntVecFromTPPLPoint(polygon[1]), static_cast<size_t>(0u));
			verticesMap.emplace(IntVecFromTPPLPoint(polygon[2]), static_cast<size_t>(0u));
		}

		outGeometry.vertices.resize(verticesMap.size());
		size_t idx = 0;
		for (auto& it : verticesMap)
		{
			outGeometry.vertices[idx].x = static_cast<float>(it.first.x);
			outGeometry.vertices[idx].y = static_cast<float>(it.first.y);
			it.second = idx++;
		}

#ifdef DEBUG_CHECKS
		bool order = false;
		bool inited = false;
#endif // DEBUG_CHECKS

		outGeometry.indexes.reserve(resultPolygons.size() * outGeometry.verticesPerPoly);
		for (const TPPLPoly& polygon : resultPolygons)
		{
#ifdef DEBUG_CHECKS
			bool newOrder = (Collide::SignedArea(VecFromTPPLPoint(polygon[0]), VecFromTPPLPoint(polygon[1]), VecFromTPPLPoint(polygon[2])) >= 0);
			if (inited && newOrder != order)
			{
				LogError("winding order changed");
			}
			inited = true;
			order = newOrder;
#endif // DEBUG_CHECKS

			outGeometry.indexes.push_back(verticesMap.find(IntVecFromTPPLPoint(polygon[0]))->second);
			outGeometry.indexes.push_back(verticesMap.find(IntVecFromTPPLPoint(polygon[1]))->second);
			outGeometry.indexes.push_back(verticesMap.find(IntVecFromTPPLPoint(polygon[2]))->second);
		}

		outGeometry.polygonsCount = resultPolygons.size();
		outGeometry.navMeshStart = start;
		outGeometry.navMeshSize = size;
		outGeometry.isCalculated = true;
	}

	struct SortedBorderPair
	{
		bool operator==(const SortedBorderPair& other) const { return first == other.first && second == other.second; }
		size_t first;
		size_t second;
		bool isSwapped;
	};

	struct BorderPairHash
	{
		std::size_t operator() (const SortedBorderPair& p) const
		{
			return std::hash<size_t>()(p.first) ^ std::rotl(std::hash<size_t>()(p.second), 7);
		}
	};

	static SortedBorderPair MakeSortedPair(size_t first, size_t second)
	{
		return (first < second) ? SortedBorderPair{first, second, false} : SortedBorderPair{second, first, true};
	}

	void LinkNavMesh(NavMesh::InnerLinks& outLinks, const NavMesh::Geometry& geometry)
	{
		Assert(geometry.isCalculated, "Geometry should be calculated before calculating links");

		// form a dictionary with borders as keys and polygon indexes as values
		std::unordered_map<SortedBorderPair, std::vector<size_t>, BorderPairHash> polysByBorders;
		for (size_t p = 0, pSize = geometry.polygonsCount; p < pSize; ++p)
		{
			size_t pShift = p * geometry.verticesPerPoly;
			FOR_EACH_BORDER(geometry.verticesPerPoly,
			{
				size_t indexA = geometry.indexes[pShift + i];
				size_t indexB = geometry.indexes[pShift + j];
				SortedBorderPair sortedPair = MakeSortedPair(indexA, indexB);
				std::vector<size_t>& vector = polysByBorders[sortedPair];
				// the first link has points in correct winding order, and the second has them reversed
				if (sortedPair.isSwapped)
				{
					vector.push_back(p);
				}
				else
				{
					vector.insert(vector.begin(), p);
				}
			});
		}

		// connect polygons that have a shared border
		outLinks.links.clear();
		outLinks.links.resize(geometry.polygonsCount);
		for (auto& pair : polysByBorders)
		{
			Assert(pair.second.size() <= 2, "There are more than 2 triangles that share the same border. That should not happen.");
			if (pair.second.size() == 2)
			{
				size_t firstBorder = pair.first.first;
				size_t secondBorder = pair.first.second;
				outLinks.links[pair.second[0]].emplace_back(pair.second[1], firstBorder, secondBorder);
				outLinks.links[pair.second[1]].emplace_back(pair.second[0], secondBorder, firstBorder);
			}
		}

#ifdef DEBUG_CHECKS
		for (size_t p = 0; p < geometry.polygonsCount; ++p)
		{
			size_t pShift = p * geometry.verticesPerPoly;
			for (const NavMesh::InnerLinks::LinkData& link : outLinks.links[p])
			{
				bool found = false;
				FOR_EACH_BORDER(geometry.verticesPerPoly,
				{
					size_t index1 = pShift + i;
					size_t index2 = pShift + j;
					if (geometry.indexes[index1] == link.borderPoint1 && geometry.indexes[index2] == link.borderPoint2)
					{
						found = true;
					}
				});
				Assert(found, "Border from a link not found (probably incorrect winding order)");
			}
		}
#endif // DEBUG_CHECKS

		outLinks.isCalculated = true;
	}

	static void UpdateAABB(BoundingBox& aabb, Vector2D vertex)
	{
		if (vertex.x < aabb.minX)
		{
			aabb.minX = vertex.x;
		}
		if (vertex.y < aabb.minY)
		{
			aabb.minY = vertex.y;
		}
		if (vertex.x > aabb.maxX)
		{
			aabb.maxX = vertex.x;
		}
		if (vertex.y > aabb.maxY)
		{
			aabb.maxY = vertex.y;
		}
	}

	static BoundingBox GetAABB(const NavMesh::Geometry& geometry, size_t polygonIdx)
	{
		BoundingBox Result
		(
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::min(),
			std::numeric_limits<float>::min()
		);

		for (size_t indexIdx = 0, indexSize = geometry.verticesPerPoly; indexIdx < indexSize; ++indexIdx)
		{
			UpdateAABB(Result, geometry.vertices[geometry.indexes[polygonIdx * geometry.verticesPerPoly + indexIdx]]);
		}

		return Result;
	}

	static bool DoesConvexPolygonIntersectCell(std::vector<Vector2D>& inOutPolygon, size_t polygonIdx, size_t cellX, size_t cellY, const NavMesh::Geometry& geometry, float cellSize)
	{
		for (size_t i = 0; i < geometry.verticesPerPoly; ++i)
		{
			inOutPolygon[i] = geometry.vertices[geometry.indexes[polygonIdx * geometry.verticesPerPoly + i]];
		}

		BoundingBox aabb
		(
			geometry.navMeshStart.x + static_cast<float>(cellX) * cellSize,
			geometry.navMeshStart.y + static_cast<float>(cellY) * cellSize,
			geometry.navMeshStart.x + static_cast<float>(cellX + 1) * cellSize,
			geometry.navMeshStart.y + static_cast<float>(cellY + 1) * cellSize
		);

		for (size_t i = 0; i < geometry.verticesPerPoly - 1; ++i)
		{
			if (inOutPolygon[i].x >= aabb.minX && inOutPolygon[i].x <= aabb.maxX
				&& inOutPolygon[i].y >= aabb.minY && inOutPolygon[i].y <= aabb.maxY)
			{
				return true;
			}

			if (Collide::IsLineIntersectAABB(aabb, inOutPolygon[i], inOutPolygon[i + 1]))
			{
				return true;
			}
		}

		size_t lastIndex = geometry.verticesPerPoly - 1;

		if (inOutPolygon[lastIndex].x >= aabb.minX && inOutPolygon[lastIndex].x <= aabb.maxX
			&& inOutPolygon[lastIndex].y >= aabb.minY && inOutPolygon[lastIndex].y <= aabb.maxY)
		{
			return true;
		}

		if (Collide::IsLineIntersectAABB(aabb, inOutPolygon[lastIndex], inOutPolygon[0]))
		{
			return true;
		}

		return false;
	}

	void BuildSpatialHash(NavMesh::SpatialHash& outSpatialHash, const NavMesh::Geometry& geometry, HashGenerationType generationType)
	{
		Assert(geometry.isCalculated, "Geometry should be calculated before calculating spatial hash");

		Vector2D cellsCountFloat = Vector2D::HadamardProduct(geometry.navMeshSize, Vector2D(1.0f / outSpatialHash.cellSize, 1.0f / outSpatialHash.cellSize));
		outSpatialHash.hashSize = IntVector2D(static_cast<int>(std::ceil(cellsCountFloat.x)), static_cast<int>(std::ceil(cellsCountFloat.y)));

		outSpatialHash.polygonsHash.resize(outSpatialHash.hashSize.x * outSpatialHash.hashSize.y);
		std::for_each(
			outSpatialHash.polygonsHash.begin(),
			outSpatialHash.polygonsHash.end(),
			[](std::vector<size_t>& vector){ vector.clear(); }
		);

		std::vector<Vector2D> reusablePolygon(geometry.verticesPerPoly);

		for (size_t polygonIdx = 0; polygonIdx < geometry.polygonsCount; ++polygonIdx)
		{
			BoundingBox aabb = GetAABB(geometry, polygonIdx);
			size_t leftCellIdx = static_cast<size_t>((aabb.minX - geometry.navMeshStart.x) / outSpatialHash.cellSize);
			size_t topCellIdx = static_cast<size_t>((aabb.minY - geometry.navMeshStart.y) / outSpatialHash.cellSize);
			size_t rightCellIdx = std::min(static_cast<size_t>(aabb.maxX - geometry.navMeshStart.x / outSpatialHash.cellSize), static_cast<size_t>(outSpatialHash.hashSize.x - 1));
			size_t bottomCellIdx = std::min(static_cast<size_t>(aabb.maxY - geometry.navMeshStart.y / outSpatialHash.cellSize), static_cast<size_t>(outSpatialHash.hashSize.y - 1));

			for (size_t y = topCellIdx; y <= bottomCellIdx; ++y)
			{
				size_t yShift = y * outSpatialHash.hashSize.x;
				for (size_t x = leftCellIdx; x <= rightCellIdx; ++x)
				{
					if (generationType == HashGenerationType::Fast
						|| DoesConvexPolygonIntersectCell(reusablePolygon, polygonIdx, x, y, geometry, outSpatialHash.cellSize))
					{
						outSpatialHash.polygonsHash[yShift + x].push_back(polygonIdx);
					}
				}
			}
		}

		outSpatialHash.isCalculated = true;
	}

} // namespace NavMeshGenerator
