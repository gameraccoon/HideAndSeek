#include "Base/precomp.h"

#include <algorithm>
#include <ranges>

#include "Utils/AI/PathBlockingGeometry.h"

#include "GameData/Geometry/Hull.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"

#include "Utils/Geometry/ShapeOperations.h"

namespace PathBlockingGeometry
{
	static const float GEOMETRY_EXTENT = 17.0f;

	void CalculatePathBlockingGeometry(std::vector<std::vector<Vector2D>>& outGeometry, const TupleVector<const CollisionComponent*, const TransformComponent*>& components)
	{
		// extend all geometry by defined radius
		std::vector<ShapeOperations::MergedGeometry> mergedGeometry;
		for (const auto& [collision, transform] : components)
		{
			const Hull& hull = collision->getGeometry();
			if (hull.type == HullType::Angular)
			{
				std::vector<SimpleBorder> borders;
				ShapeOperations::ExtendGeometry(borders, hull.points, GEOMETRY_EXTENT);
				std::ranges::for_each(borders, [location = transform->getLocation()](SimpleBorder& border)
				{
					border.a += location;
					border.b += location;
				});

				mergedGeometry.emplace_back(std::move(borders));
			}
		}

		// merge intersecting shapes
		ShapeOperations::MergeGeometry(mergedGeometry);

		outGeometry.clear();
		outGeometry.reserve(mergedGeometry.size());
		std::ranges::for_each(mergedGeometry,
			[&outGeometry](ShapeOperations::MergedGeometry& geometry)
			{
				ShapeOperations::SortBorders(geometry.borders);
				outGeometry.emplace_back(geometry.borders.size());
				std::vector<Vector2D>& newGeometry = outGeometry.back();
				for (size_t i = 0; i < geometry.borders.size(); ++i)
				{
					newGeometry[geometry.borders.size() - i - 1] = geometry.borders[i].a;
				}
			}
		);
	}
} // namespace PathBlockingGeometry
