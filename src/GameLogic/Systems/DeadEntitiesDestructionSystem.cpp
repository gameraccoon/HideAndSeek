#include "Base/precomp.h"

#include "GameLogic/Systems/DeadEntitiesDestructionSystem.h"

#include "Base/Types/TemplateAliases.h"

#include "GameData/World.h"
#include "GameData/GameData.h"
#include "GameData/Components/DeathComponent.generated.h"

DeadEntitiesDestructionSystem::DeadEntitiesDestructionSystem(WorldHolder& worldHolder) noexcept
	: mWorldHolder(worldHolder)
{
}

void DeadEntitiesDestructionSystem::update()
{
	SCOPED_PROFILER("DeadEntitiesDestructionSystem::update");
	World& world = mWorldHolder.getWorld();

	TupleVector<WorldCell*, Entity, const DeathComponent*> components;
	world.getSpatialData().getAllCellManagers().getSpatialComponentsWithEntities<const DeathComponent>(components);

	for (const auto& componentTuple : components)
	{
		std::get<0>(componentTuple)->getEntityManager().removeEntity(std::get<1>(componentTuple));
	}
}
