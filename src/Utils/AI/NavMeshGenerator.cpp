#include "Base/precomp.h"

#include "Utils/AI/NavMeshGenerator.h"

#include <cmath>
#include <unordered_map>

#include <polypartition.h>

#include "Base/Types/TemplateAliases.h"

#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Core/BoundingBox.h"

#include "Utils/Geometry/Collide.h"

namespace NavMeshGenerator
{
	static IntVector2D IntVecFromTPPLPoint(TPPLPoint point)
	{
		return IntVector2D(static_cast<int>(std::round(point.x)), static_cast<int>(std::round(point.y)));
	}

	static Vector2D VecFromTPPLPoint(TPPLPoint point)
	{
		return Vector2D(point.x, point.y);
	}

	void GenerateNavMeshGeometry(NavMesh::Geometry& outGeometry, const TupleVector<CollisionComponent*, TransformComponent*>& collidableComponents, Vector2D start, Vector2D size)
	{
		outGeometry.vertices.clear();
		outGeometry.indexes.clear();
		// only triangles are supported now
		outGeometry.vertsPerPoly = 3;
		outGeometry.isCalculated = false;

		IntVector2D halfSize(static_cast<int>(size.x * 0.5f), static_cast<int>(size.y * 0.5f));
		IntVector2D centerPos(static_cast<int>(start.x) + halfSize.x, static_cast<int>(start.y) + halfSize.y);

		std::vector<TPPLPoly> polygons;
		std::vector<TPPLPoly> resultPolygons;

		TPPLPoly borderPolygon;
		borderPolygon.Init(4);
		borderPolygon[0].x = centerPos.x - halfSize.x;
		borderPolygon[0].y = centerPos.y - halfSize.y;
		borderPolygon[1].x = centerPos.x + halfSize.x;
		borderPolygon[1].y = centerPos.y - halfSize.y;
		borderPolygon[2].x = centerPos.x + halfSize.x;
		borderPolygon[2].y = centerPos.y + halfSize.y;
		borderPolygon[3].x = centerPos.x - halfSize.x;
		borderPolygon[3].y = centerPos.y + halfSize.y;
		polygons.push_back(borderPolygon);

		for (auto [collision, transform] : collidableComponents)
		{
			Vector2D location = transform->getLocation();
			const Hull& geometry = collision->getGeometry();
			if (geometry.type == HullType::Angular)
			{
				TPPLPoly polygon;
				polygon.Init(geometry.points.size());

				size_t pointsSize = geometry.points.size();
				for (size_t i = 0; i < pointsSize; ++i)
				{
					const Vector2D& point = geometry.points[pointsSize - 1 - i];
					polygon[i].x = point.x + location.x;
					polygon[i].y = point.y + location.y;
				}
				polygon.SetHole(true);
				polygons.push_back(polygon);
			}
		}

		TPPLPartition pp;
		pp.Triangulate_MONO(&polygons, &resultPolygons);

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

		outGeometry.indexes.reserve(resultPolygons.size() * outGeometry.vertsPerPoly);
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
			return std::hash<size_t>()(p.first) ^ (std::hash<size_t>()(p.second) << 1);
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
			size_t pShift = p * geometry.vertsPerPoly;
			FOR_EACH_BORDER(geometry.vertsPerPoly,
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
				outLinks.links[pair.second[0]].emplace_back(pair.second[1], std::make_pair(firstBorder, secondBorder));
				outLinks.links[pair.second[1]].emplace_back(pair.second[0], std::make_pair(secondBorder, firstBorder));
			}
		}

#ifdef DEBUG_CHECKS
		for (size_t p = 0; p < geometry.polygonsCount; ++p)
		{
			size_t pShift = p * geometry.vertsPerPoly;
			for (const NavMesh::InnerLinks::LinkData& link : outLinks.links[p])
			{
				bool found = false;
				FOR_EACH_BORDER(geometry.vertsPerPoly,
				{
					size_t index1 = pShift + i;
					size_t index2 = pShift + j;
					if (geometry.indexes[index1] == link.border.first && geometry.indexes[index2] == link.border.second)
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
		BoundingBox result
		(
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::min(),
			std::numeric_limits<float>::min()
		);

		for (size_t indexIdx = 0, indexSize = geometry.vertsPerPoly; indexIdx < indexSize; ++indexIdx)
		{
			UpdateAABB(result, geometry.vertices[geometry.indexes[polygonIdx * geometry.vertsPerPoly + indexIdx]]);
		}

		return result;
	}

	static bool DoesConvexPolygonIntersectCell(size_t polygonIdx, size_t cellX, size_t cellY, const NavMesh::Geometry& geometry, float cellSize)
	{
		std::vector<Vector2D> polygon(geometry.vertsPerPoly);
		for (size_t i = 0; i < geometry.vertsPerPoly; ++i)
		{
			polygon[i] = geometry.vertices[geometry.indexes[polygonIdx * geometry.vertsPerPoly + i]];
		}

		BoundingBox aabb
		(
			geometry.navMeshStart.x + cellX * cellSize,
			geometry.navMeshStart.y + cellY * cellSize,
			geometry.navMeshStart.x + (cellX + 1) * cellSize,
			geometry.navMeshStart.y + (cellY + 1) * cellSize
		);

		for (size_t i = 0; i < geometry.vertsPerPoly - 1; ++i)
		{
			if (polygon[i].x >= aabb.minX && polygon[i].x <= aabb.maxX
				&& polygon[i].y >= aabb.minY && polygon[i].y <= aabb.maxY)
			{
				return true;
			}

			if (Collide::IsLineIntersectAABB(aabb, polygon[i], polygon[i + 1]))
			{
				return true;
			}
		}

		size_t lastIndex = geometry.vertsPerPoly - 1;

		if (polygon[lastIndex].x >= aabb.minX && polygon[lastIndex].x <= aabb.maxX
			&& polygon[lastIndex].y >= aabb.minY && polygon[lastIndex].y <= aabb.maxY)
		{
			return true;
		}

		if (Collide::IsLineIntersectAABB(aabb, polygon[lastIndex], polygon[0]))
		{
			return true;
		}

		return false;
	}

	void BuildSpatialHash(NavMesh::SpatialHash& outSpatialHash, const NavMesh::Geometry& geometry)
	{
		Assert(geometry.isCalculated, "Geometry should be calculated before calculating spatial hash");

		outSpatialHash.polygonsHash.clear();

		Vector2D cellsCountFloat = Vector2D::HadamardProduct(geometry.navMeshSize, Vector2D(1.0f / outSpatialHash.cellSize, 1.0f / outSpatialHash.cellSize));
		outSpatialHash.hashSize = IntVector2D(std::ceil(cellsCountFloat.x), std::ceil(cellsCountFloat.y));

		outSpatialHash.polygonsHash.resize(outSpatialHash.hashSize.x * outSpatialHash.hashSize.y);

		for (size_t polygonIdx = 0; polygonIdx < geometry.polygonsCount; ++polygonIdx)
		{
			BoundingBox aabb = GetAABB(geometry, polygonIdx);
			size_t leftCellIdx = static_cast<size_t>(aabb.minX - geometry.navMeshStart.x) / outSpatialHash.cellSize;
			size_t topCellIdx = static_cast<size_t>(aabb.minY - geometry.navMeshStart.y) / outSpatialHash.cellSize;
			size_t rightCellIdx = std::min(static_cast<size_t>(aabb.maxX - geometry.navMeshStart.x / outSpatialHash.cellSize), static_cast<size_t>(outSpatialHash.hashSize.x - 1));
			size_t bottomCellIdx = std::min(static_cast<size_t>(aabb.maxY - geometry.navMeshStart.y / outSpatialHash.cellSize), static_cast<size_t>(outSpatialHash.hashSize.y - 1));

			for (size_t y = topCellIdx; y <= bottomCellIdx; ++y)
			{
				size_t yShift = y * outSpatialHash.hashSize.x;
				for (size_t x = leftCellIdx; x <= rightCellIdx; ++x)
				{
					if (DoesConvexPolygonIntersectCell(polygonIdx, x, y, geometry, outSpatialHash.cellSize))
					{
						outSpatialHash.polygonsHash[yShift + x].push_back(polygonIdx);
					}
				}
			}
		}

		outSpatialHash.isCalculated = true;
	}

} // namespace NavMeshGenerator
