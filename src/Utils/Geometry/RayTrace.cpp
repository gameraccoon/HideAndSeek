#include "Base/precomp.h"

#include "Base/Types/TemplateAliases.h"

#include "Utils/Geometry/RayTrace.h"

#include <algorithm>

#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"

#include "GameData/Geometry/Vector2D.h"
#include "GameData/Geometry/Rotator.h"
#include "GameData/Geometry/BoundingBox.h"
#include "GameData/World.h"

#include "Utils/Geometry/Collide.h"

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
					Vector2D d = endPoint - startPoint;
					Vector2D f = startPoint - transform->getLocation();
					float r = hull.getRadius();

					float a = Vector2D::DotProduct(d, d);
					float b = 2.0f * Vector2D::DotProduct(f, d);
					float c = Vector2D::DotProduct(f, f) - r * r;

					float discriminant = b * b - 4 * a * c;
					if (discriminant >= 0)
					{
						discriminant = sqrt(discriminant);

						float t1 = (-b - discriminant) / (2.0f * a);

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
			[&result, &minRayQLength, startPoint, endPoint](WorldCell& cell, Entity entity, const CollisionComponent* collision, const TransformComponent* transform)
		{
			Vector2D transformedStartPoint = startPoint - transform->getLocation();
			Vector2D transformedEndPoint = endPoint - transform->getLocation();

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

						Vector2D hitLocation = Collide::GetPointIntersect2Lines(border.getA(), border.getB(),
							transformedStartPoint, transformedEndPoint);

						float rayQLength = (transformedStartPoint - hitLocation).qSize();

						// if currentActor closer than the previous one
						if (rayQLength < minRayQLength)
						{
							minRayQLength = rayQLength;
							result.hasHit = true;
							result.hitEntity.entity = entity;
							result.hitEntity.cell = cell.getPos();
							result.hitPoint = transform->getLocation() + hitLocation;
							result.hitBorderNormal = border.getNormal();
						}
					}
				}
				else
				{
					Vector2D d = endPoint - startPoint;
					Vector2D f = startPoint - transform->getLocation();
					float r = hull.getRadius();

					float a = Vector2D::DotProduct(d, d);
					float b = 2.0f * Vector2D::DotProduct(f, d);
					float c = Vector2D::DotProduct(f, f) - r * r;

					float discriminant = b * b - 4 * a * c;
					if (discriminant >= 0)
					{
						discriminant = sqrt(discriminant);

						float t1 = (-b - discriminant) / (2.0f * a);

						if (t1 >= 0 && t1 <= 1)
						{
							float rayLength = d.size() * t1;
							float rayQLength = rayLength * rayLength;
							if (rayQLength < minRayQLength)
							{
								minRayQLength = rayQLength;
								result.hasHit = true;
								result.hitEntity.entity = entity;
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
