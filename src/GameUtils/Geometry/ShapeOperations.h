#pragma once

#include <limits>
#include <vector>

#include "EngineData/Geometry/Border.h"
#include "EngineData/Geometry/BoundingBox.h"
#include "EngineData/Geometry/Vector2D.h"

namespace ShapeOperations
{
	using Shape = std::vector<SimpleBorder>;

	Shape GetUnion(const Shape& shape1, const Shape& shape2);
	// join all borders that make a straight line
	void OptimizeShape(std::vector<SimpleBorder>& inOutShape);

	struct MergedGeometry
	{
		explicit MergedGeometry(const std::vector<Border>& inBorders, Vector2D location);
		explicit MergedGeometry(const std::vector<SimpleBorder>& simpleBorders);
		explicit MergedGeometry(std::vector<SimpleBorder>&& simpleBorders);

		void updateAABB();

		Shape borders;
		BoundingBox aabb{
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::lowest(),
			std::numeric_limits<float>::lowest()
		};
	};

	bool AreShapesIntersect(const MergedGeometry& firstGeometry, const MergedGeometry& secondGeometry);

	void MergeGeometry(std::vector<MergedGeometry>& inOutGeometry);

	void SplitIntoConvexShapes(std::vector<std::vector<Vector2D>>& inOutShapes, Shape geometry);

	// returns start indexes of the figure and each hole in the resulting shape
	std::vector<size_t> SortBorders(Shape& inOutShape);

	void ExtendGeometry(Shape& outResultingShape, const std::vector<Vector2D>& geometry, float radius);
} // namespace ShapeOperations
