#include "Base/precomp.h"

#include "GameLogic/Systems/DeadEntitiesDestructionSystem.h"

#include "Base/Types/TemplateAliases.h"

#include "GameData/Components/DeathComponent.generated.h"

#include "GameData/World.h"
#include "GameData/GameData.h"

DeadEntitiesDestructionSystem::DeadEntitiesDestructionSystem(WorldHolder& worldHolder) noexcept
	: mWorldHolder(worldHolder)
{
}

void DeadEntitiesDestructionSystem::update()
{
	World& world = mWorldHolder.getWorld();

	TupleVector<WorldCell*, Entity, DeathComponent*> components;
	world.getSpatialData().getAllCellManagers().getSpatialComponentsWithEntities<DeathComponent>(components);

	for (auto& componentTuple : components)
	{
		std::get<0>(componentTuple)->getEntityManager().removeEntity(std::get<1>(componentTuple));
	}
}
