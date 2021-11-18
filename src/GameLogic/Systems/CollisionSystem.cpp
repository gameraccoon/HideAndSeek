#include "Base/precomp.h"

#include "Base/Types/TemplateAliases.h"

#include "GameData/World.h"

#include "Utils/Geometry/Collide.h"

#include "GameLogic/Systems/CollisionSystem.h"


CollisionSystem::CollisionSystem(
		RaccoonEcs::ComponentFilter<CollisionComponent, const TransformComponent>&& collidingFilter,
		RaccoonEcs::ComponentFilter<MovementComponent>&& movementFilter,
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent, MovementComponent>&& movingCollisionsFilter,
		WorldHolder& worldHolder) noexcept
	: mCollidingFilter(std::move(collidingFilter))
	, mMovementFilter(std::move(movementFilter))
	, mMovingCollisionsFilter(std::move(movingCollisionsFilter))
	, mWorldHolder(worldHolder)
{
}

void CollisionSystem::update()
{
	SCOPED_PROFILER("CollisionSystem::update");
	struct SpatialComponents
	{
		WorldCell* cell;
		TupleVector<Entity, CollisionComponent*, const TransformComponent*> components;
	};

	World& world = mWorldHolder.getWorld();

	auto& allCellsMap = world.getSpatialData().getAllCells();
	std::vector<SpatialComponents> collidableComponentGroups(allCellsMap.size());
	{
		SCOPED_PROFILER("get components");
		size_t i = 0;
		for (auto& pair : allCellsMap)
		{
			mCollidingFilter.getComponentsWithEntities(pair.second.getEntityManager(), collidableComponentGroups[i].components);
			collidableComponentGroups[i].cell = &pair.second;
			++i;
		}
	}
	{
		SCOPED_PROFILER("update bounding boxes");
		for (auto& pair : collidableComponentGroups)
		{
			for (auto [entity, collision, transform] : pair.components)
			{
				Collide::UpdateBoundingBox(collision);
			}
		}
	}

	SCOPED_PROFILER("resolve collisions");
	world.getSpatialData().getAllCellManagers().forEachComponentSet(
			mMovingCollisionsFilter,
			[&collidableComponentGroups, this](const CollisionComponent* collisionComponent, const TransformComponent* transformComponent, MovementComponent* movementComponent)
	{
		Vector2D resist = ZERO_VECTOR;
		for (auto& pair : collidableComponentGroups)
		{
			for (auto [entity, collision, transform] : pair.components)
			{
				if (collision != collisionComponent)
				{
					bool doCollide = Collide::DoCollide(collisionComponent, transformComponent->getLocation() + movementComponent->getNextStep(), collision, transform->getLocation(), resist);

					if (doCollide)
					{
						auto [movement] = mMovementFilter.getEntityComponents(pair.cell->getEntityManager(), entity);
						if (movement)
						{
							movementComponent->setNextStep(movementComponent->getNextStep() + resist/2);
							movement->setNextStep(movement->getNextStep() - resist/2);
						}
						else
						{
							movementComponent->setNextStep(movementComponent->getNextStep() + resist);
						}
					}
				}
			}
		}
	});
}
