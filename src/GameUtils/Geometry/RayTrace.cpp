#include "EngineCommon/precomp.h"

#include "EngineCommon/Types/TemplateAliases.h"

#include "GameUtils/Geometry/RayTrace.h"

#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"

#include "EngineData/Geometry/Vector2D.h"
#include "EngineData/Geometry/Rotator.h"
#include "GameData/World.h"

#include "GameUtils/Geometry/Collide.h"

namespace RayTrace
{
	bool FastTrace(World& world, const Vector2D& startPoint, const Vector2D& endPoint)
	{
		TupleVector<const CollisionComponent*, const TransformComponent*> components;
		// ToDo: choose only potentially intersected cells
		world.getSpatialData().getAllCellManagers().getComponents<const CollisionComponent, const TransformComponent>(components);

		for (auto [collision, transform] : components)
		{
			Vector2D transformedStartPoint = startPoint - transform->getLocation();
			Vector2D transformedEndPoint = endPoint - transform->getLocation();

			// if the raytrace intersects with AABB of this actor
			if (Collide::IsLineIntersectAABB(collision->getBoundingBox(), transformedStartPoint, transformedEndPoint))
			{
				const Hull& hull = collision->getGeometry();

				if (hull.type == HullType::Angular)
				{
					// for each border
					for (auto& border : hull.borders)
					{
						// if ray have opposite direction with normal
						if (std::abs((border.getNormal().rotation() - (transformedEndPoint - transformedStartPoint).rotation()).getValue()) <= PI/2)
						{
							continue;
						}

						// if the raytrace intersects with this border
						if (Collide::AreLinesIntersect(border.getA(), border.getB(), transformedStartPoint, transformedEndPoint))
						{
							return true;
						}
					}
				}
				else
				{
					const Vector2D d = endPoint - startPoint;
					const Vector2D f = startPoint - transform->getLocation();
					const float r = hull.getRadius();

					const float a = Vector2D::DotProduct(d, d);
					const float b = 2.0f * Vector2D::DotProduct(f, d);
					const float c = Vector2D::DotProduct(f, f) - r * r;

					float discriminant = b * b - 4 * a * c;
					if (discriminant >= 0)
					{
						discriminant = sqrt(discriminant);

						const float t1 = (-b - discriminant) / (2.0f * a);

						if (t1 >= 0 && t1 <= 1)
						{
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	TraceResult Trace(World& world, Vector2D startPoint, Vector2D endPoint)
	{
		TraceResult result;

		float minRayQLength = (startPoint - endPoint).qSize() + 20.0f;

		// ToDo: choose only potentially intersecting cells
		world.getSpatialData().getAllCellManagers().forEachComponentSetWithEntityAndExtraData<const CollisionComponent, const TransformComponent>(
			[&result, &minRayQLength, startPoint, endPoint](const WorldCell& cell, const EntityView entityView, const CollisionComponent* collision, const TransformComponent* transform)
		{
			const Vector2D transformedStartPoint = startPoint - transform->getLocation();
			const Vector2D transformedEndPoint = endPoint - transform->getLocation();

			// if the raytrace intersects with AABB of this entity
			if (Collide::IsLineIntersectAABB(collision->getBoundingBox(), transformedStartPoint, transformedEndPoint))
			{
				const Hull& hull = collision->getGeometry();

				if (hull.type == HullType::Angular)
				{
					// for each border
					for (auto& border : hull.borders)
					{
						// if ray have opposite direction with normal
						if (std::abs((border.getNormal().rotation() - (transformedEndPoint - transformedStartPoint).rotation()).getValue()) <= PI/2)
						{
							continue;
						}

						// if the raytrace intersects with this border
						if (!Collide::AreLinesIntersect(border.getA(), border.getB(), transformedStartPoint, transformedEndPoint))
						{
							continue;
						}

						const Vector2D hitLocation = Collide::GetPointIntersect2Lines(border.getA(), border.getB(),
							transformedStartPoint, transformedEndPoint);

						const float rayQLength = (transformedStartPoint - hitLocation).qSize();

						// if currentActor closer than the previous one
						if (rayQLength < minRayQLength)
						{
							minRayQLength = rayQLength;
							result.hasHit = true;
							result.hitEntity.entity = entityView.getEntity();
							result.hitEntity.cell = cell.getPos();
							result.hitPoint = transform->getLocation() + hitLocation;
							result.hitBorderNormal = border.getNormal();
						}
					}
				}
				else
				{
					const Vector2D d = endPoint - startPoint;
					const Vector2D f = startPoint - transform->getLocation();
					const float r = hull.getRadius();

					const float a = Vector2D::DotProduct(d, d);
					const float b = 2.0f * Vector2D::DotProduct(f, d);
					const float c = Vector2D::DotProduct(f, f) - r * r;

					float discriminant = b * b - 4 * a * c;
					if (discriminant >= 0)
					{
						discriminant = sqrt(discriminant);

						const float t1 = (-b - discriminant) / (2.0f * a);

						if (t1 >= 0 && t1 <= 1)
						{
							const float rayLength = d.size() * t1;
							const float rayQLength = rayLength * rayLength;
							if (rayQLength < minRayQLength)
							{
								minRayQLength = rayQLength;
								result.hasHit = true;
								result.hitEntity.entity = entityView.getEntity();
								result.hitEntity.cell = cell.getPos();
								result.hitPoint = startPoint + d * t1;
								result.hitBorderNormal = (result.hitPoint - transform->getLocation()).unit();
							}
						}
					}
				}
			}
		});

		return result;
	}
}
