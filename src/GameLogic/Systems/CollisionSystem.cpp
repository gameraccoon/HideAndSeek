#include "EngineCommon/precomp.h"

#include "GameLogic/Systems/CollisionSystem.h"

#include "EngineCommon/Types/TemplateAliases.h"

#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/World.h"

#include "GameUtils/Geometry/Collide.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

CollisionSystem::CollisionSystem(WorldHolder& worldHolder) noexcept
	: mWorldHolder(worldHolder)
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
		for (auto& [cellPos, cell] : allCellsMap)
		{
			cell.getEntityManager().getComponentsWithEntities<CollisionComponent, const TransformComponent>(collidableComponentGroups[i].components);
			collidableComponentGroups[i].cell = &cell;
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
	world.getSpatialData().getAllCellManagers().forEachComponentSet<const CollisionComponent, const TransformComponent, MovementComponent>(
		[&collidableComponentGroups](const CollisionComponent* collisionComponent, const TransformComponent* transformComponent, MovementComponent* movementComponent) {
			Vector2D resist = ZERO_VECTOR;
			for (auto& pair : collidableComponentGroups)
			{
				for (auto [entity, collision, transform] : pair.components)
				{
					if (collision != collisionComponent)
					{
						const bool doCollide = Collide::DoCollide(collisionComponent, transformComponent->getLocation() + movementComponent->getNextStep(), collision, transform->getLocation(), resist);

						if (doCollide)
						{
							auto [movement] = pair.cell->getEntityManager().getEntityComponents<MovementComponent>(entity);
							if (movement)
							{
								movementComponent->setNextStep(movementComponent->getNextStep() + resist / 2);
								movement->setNextStep(movement->getNextStep() - resist / 2);
							}
							else
							{
								movementComponent->setNextStep(movementComponent->getNextStep() + resist);
							}
						}
					}
				}
			}
		}
	);
}
