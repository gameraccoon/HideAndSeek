#include "EngineCommon/precomp.h"

#include "GameUtils/Geometry/LightBlockingGeometry.h"

#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"

#include "GameData/Spatial/SpatialWorldData.h"

#include "GameUtils/Geometry/ShapeOperations.h"

namespace LightBlockingGeometry
{
	static bool isPointInsideAABB(const BoundingBox& box, const Vector2D a)
	{
		return box.minX <= a.x && a.x < box.maxX
			&& box.minY <= a.y && a.y < box.maxY;
	}

	using CellGeometry = std::vector<ShapeOperations::MergedGeometry>;

	static Vector2D GetLeftmostPoint(const SimpleBorder& border)
	{
		if (border.a.x < border.b.x)
		{
			return border.a;
		}
		else if (border.b.x > border.a.x)
		{
			return border.b;
		}
		// if a.x and b.x are equal, compare by y axis
		else if (border.a.y < border.b.y)
		{
			return border.a;
		}
		else
		{
			return border.b;
		}
	}

	static void FillGeometryInCell(std::vector<SimpleBorder>& inOutResult, const CellGeometry& cellGeometry, const BoundingBox& cellAABB)
	{
		for (const ShapeOperations::MergedGeometry& geometry : cellGeometry)
		{
			for (const SimpleBorder& border : geometry.borders)
			{
				if (isPointInsideAABB(cellAABB, border.a) && isPointInsideAABB(cellAABB, border.b))
				{
					inOutResult.push_back(border);
				}
				else
				{
					const Vector2D leftmostPoint = GetLeftmostPoint(border);
					// only add to the cell where our left-most point
					if (isPointInsideAABB(cellAABB, leftmostPoint))
					{
						inOutResult.push_back(border);
					}
				}
			}
		}
	}

	void CalculateLightGeometry(CalculatedGeometry& outGeometry, const CollisionGeometry& collisionGeometry)
	{
		std::unordered_map<CellPos, CellGeometry> cellGeometry;
		for (const auto& [cell, collision, transform] : collisionGeometry)
		{
			CellGeometry& currentCellGeometry = cellGeometry[cell.get().getPos()];

			const Vector2D location = transform->getLocation();
			const Hull& hull = collision->getGeometry();

			currentCellGeometry.emplace_back(hull.borders, location);
			const ShapeOperations::MergedGeometry& newGeometry = currentCellGeometry.back();

			const CellPos currentCellPos = cell.get().getPos();

			const BoundingBox cellAABB = SpatialWorldData::GetCellAABB(cell.get().getPos());

			const bool intersectsMinX = newGeometry.aabb.minX < cellAABB.minX;
			const bool intersectsMaxX = newGeometry.aabb.maxX >= cellAABB.maxX;
			const bool intersectsMinY = newGeometry.aabb.minY < cellAABB.minY;
			const bool intersectsMaxY = newGeometry.aabb.maxY >= cellAABB.maxY;

			if (intersectsMinX)
			{
				cellGeometry[CellPos(currentCellPos.x - 1, currentCellPos.y)].push_back(newGeometry);
			}
			if (intersectsMaxX)
			{
				cellGeometry[CellPos(currentCellPos.x + 1, currentCellPos.y)].push_back(newGeometry);
			}
			if (intersectsMinY)
			{
				cellGeometry[CellPos(currentCellPos.x, currentCellPos.y - 1)].push_back(newGeometry);
			}
			if (intersectsMaxY)
			{
				cellGeometry[CellPos(currentCellPos.x, currentCellPos.y + 1)].push_back(newGeometry);
			}
			if (intersectsMinX && intersectsMinY)
			{
				cellGeometry[CellPos(currentCellPos.x - 1, currentCellPos.y - 1)].push_back(newGeometry);
			}
			if (intersectsMinX && intersectsMaxY)
			{
				cellGeometry[CellPos(currentCellPos.x - 1, currentCellPos.y + 1)].push_back(newGeometry);
			}
			if (intersectsMaxX && intersectsMinY)
			{
				cellGeometry[CellPos(currentCellPos.x + 1, currentCellPos.y - 1)].push_back(newGeometry);
			}
			if (intersectsMaxX && intersectsMaxY)
			{
				cellGeometry[CellPos(currentCellPos.x + 1, currentCellPos.y + 1)].push_back(newGeometry);
			}
		}

		// collect intersections inside cells
		for (auto& [pos, oneCellGeometry] : cellGeometry)
		{
			const BoundingBox cellAABB = SpatialWorldData::GetCellAABB(pos);
			ShapeOperations::MergeGeometry(oneCellGeometry);
			FillGeometryInCell(outGeometry[pos], oneCellGeometry, cellAABB);
		}
	}
}
