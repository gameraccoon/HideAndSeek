#include "Base/precomp.h"

#include "Base/Types/TemplateAliases.h"

#include "GameLogic/Systems/CollisionSystem.h"

#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/World.h"

#include "Utils/Geometry/Collide.h"


CollisionSystem::CollisionSystem(WorldHolder& worldHolder) noexcept
	: mWorldHolder(worldHolder)
{
}

void CollisionSystem::update()
{
	struct SpatialComponents
	{
		WorldCell* cell;
		TupleVector<Entity, CollisionComponent*, TransformComponent*> components;
	};

	World& world = mWorldHolder.getWorld();

	auto& allCellsMap = world.getSpatialData().getAllCells();
	std::vector<SpatialComponents> collidableComponentGroups(allCellsMap.size());
	size_t i = 0;
	for (auto& pair : allCellsMap)
	{
		pair.second.getEntityManager().getComponentsWithEntities<CollisionComponent, TransformComponent>(collidableComponentGroups[i].components);
		collidableComponentGroups[i].cell = &pair.second;
		++i;
	}

	for (auto& pair : collidableComponentGroups)
	{
		for (auto [entity, collision, transform] : pair.components)
		{
			Collide::UpdateBoundingBox(collision);
		}
	}

	world.getSpatialData().getAllCellManagers().forEachComponentSet<CollisionComponent, TransformComponent, MovementComponent>([&collidableComponentGroups](CollisionComponent* collisionComponent, TransformComponent* transformComponent, MovementComponent* movementComponent)
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
						auto [movement] = pair.cell->getEntityManager().getEntityComponents<MovementComponent>(entity);
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
