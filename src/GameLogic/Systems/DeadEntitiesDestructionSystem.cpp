#include "Base/precomp.h"

#include "GameLogic/Systems/DeadEntitiesDestructionSystem.h"

#include "Base/Types/TemplateAliases.h"

#include "GameData/Components/DeathComponent.generated.h"

#include "GameData/World.h"
#include "GameData/GameData.h"

DeadEntitiesDestructionSystem::DeadEntitiesDestructionSystem(
		RaccoonEcs::ComponentFilter<const DeathComponent>&& deathFilter,
		RaccoonEcs::EntityRemover&& entityRemover,
		WorldHolder& worldHolder) noexcept
	: mDeathFilter(std::move(deathFilter))
	, mEntityRemover(std::move(entityRemover))
	, mWorldHolder(worldHolder)
{
}

void DeadEntitiesDestructionSystem::update()
{
	World& world = mWorldHolder.getWorld();

	TupleVector<WorldCell*, Entity, const DeathComponent*> components;
	world.getSpatialData().getAllCellManagers().getSpatialComponentsWithEntitiesN(mDeathFilter, components);

	for (const auto& componentTuple : components)
	{
		std::get<0>(componentTuple)->getEntityManager().removeEntity(std::get<1>(componentTuple));
	}
}
