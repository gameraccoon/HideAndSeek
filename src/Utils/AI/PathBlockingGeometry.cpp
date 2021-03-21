#include "Base/precomp.h"

#include "Utils/AI/PathBlockingGeometry.h"

#include "GameData/Core/Hull.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"

#include "Utils/Geometry/ShapeOperations.h"

namespace PathBlockingGeometry
{
	static const float GEOMETRY_EXTENT = 10.0f;

	void CalculatePathBlockingGeometry(std::vector<std::vector<Vector2D>>& outGeometry, const TupleVector<CollisionComponent*, TransformComponent*>& components)
	{
		// extend all geometry by defined radius
		std::vector<ShapeOperations::MergedGeometry> mergedGeometry;
		for (const auto& [collision, transform] : components)
		{
			const Hull& hull = collision->getGeometry();
			if (hull.type == HullType::Angular)
			{
				std::vector<SimpleBorder> borders = ShapeOperations::ConvertBordersToSimpleBorders(hull.borders, transform->getLocation());
				ShapeOperations::ExtendInPlace(borders, GEOMETRY_EXTENT);
				mergedGeometry.push_back(std::move(borders));
			}
		}

		// merge intersecting shapes
		ShapeOperations::MergeGeometry(mergedGeometry);

		//std::vector<ShapeOperations::Shape> splitGeometry;
		//splitGeometry.reserve(mergedGeometry.size() * 2);
		std::for_each(
			mergedGeometry.begin(),
			mergedGeometry.end(),
			[](ShapeOperations::MergedGeometry& geometry)
			{
				ShapeOperations::OptimizeShape(geometry.borders);
				ShapeOperations::SortBorders(geometry.borders);
				//ShapeOperations::SplitIntoConvexShapes(geometry.borders, splitGeometry);
			}
		);

		// gather the results as set of points
		outGeometry.clear();
		for (ShapeOperations::MergedGeometry& geometry : mergedGeometry)
		{
			const size_t pointsSize = geometry.borders.size();

			outGeometry.emplace_back();
			std::vector<Vector2D>& polygon = outGeometry.back();
			polygon.resize(pointsSize);

			for (size_t i = 0; i < pointsSize; ++i)
			{
				// path blocking geometry has the opposite winding order
				const Vector2D point = geometry.borders[pointsSize - 1 - i].a;
				polygon[i] = point;
			}
		}
	}
} // namespace PathBlockingGeometry
