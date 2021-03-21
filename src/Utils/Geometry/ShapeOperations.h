#pragma once

#include <vector>
#include <limits>

#include "GameData/Core/Vector2D.h"
#include "GameData/Core/Border.h"
#include "GameData/Core/BoundingBox.h"

namespace ShapeOperations
{
	using Shape = std::vector<SimpleBorder>;

	Shape GetUnion(const Shape& shape1, const Shape& shape2);
	// join all borders that make a straight line
	void OptimizeShape(std::vector<SimpleBorder>& inOutShape);

	struct MergedGeometry
	{
		MergedGeometry(const std::vector<Border>& inBorders, Vector2D location);
		MergedGeometry(const std::vector<SimpleBorder>& simpleBorders);

		Shape borders;
		BoundingBox aabb
		{
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::lowest(),
			std::numeric_limits<float>::lowest()
		};
	};

	std::vector<SimpleBorder> ConvertBordersToSimpleBorders(const std::vector<Border>& borders, Vector2D location);

	bool AreShapesIntersect(const MergedGeometry& firstGeometry, const MergedGeometry& secondGeometry);

	void MergeGeometry(std::vector<MergedGeometry>& inOutGeometry);

	void SplitIntoConvexShapes(const Shape& geometry, std::vector<Shape>& newShapes);

	// returns start indexes of the figure and each hole in the resulting shape
	std::vector<size_t> SortBorders(Shape& inOutShape);

	void ExtendInPlace(Shape& inOutShape, float radius);
}
