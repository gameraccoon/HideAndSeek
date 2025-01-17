#include "EngineCommon/precomp.h"

#include "GameLogic/Systems/DeadEntitiesDestructionSystem.h"

#include "EngineCommon/Types/TemplateAliases.h"

#include "GameData/Components/DeathComponent.generated.h"
#include "GameData/GameData.h"
#include "GameData/World.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

DeadEntitiesDestructionSystem::DeadEntitiesDestructionSystem(WorldHolder& worldHolder) noexcept
	: mWorldHolder(worldHolder)
{
}

void DeadEntitiesDestructionSystem::update()
{
	SCOPED_PROFILER("DeadEntitiesDestructionSystem::update");
	World& world = mWorldHolder.getWorld();

	TupleVector<std::reference_wrapper<WorldCell>, Entity, const DeathComponent*> components;
	world.getSpatialData().getAllCellManagers().getComponentsWithEntitiesAndExtraData<const DeathComponent>(components);

	for (const auto& componentTuple : components)
	{
		std::get<0>(componentTuple).get().getEntityManager().removeEntity(std::get<1>(componentTuple));
	}
}
