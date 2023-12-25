#include "Base/precomp.h"

#include "GameLogic/Systems/MovementSystem.h"

#include <SDL_keycode.h>

#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/SpatialTrackComponent.generated.h"
#include "GameData/Components/TimeComponent.generated.h"
#include "GameData/Components/TrackedSpatialEntitiesComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/World.h"

#include "GameLogic/SharedManagers/WorldHolder.h"


MovementSystem::MovementSystem(WorldHolder& worldHolder) noexcept
	: mWorldHolder(worldHolder)
{
}

void MovementSystem::update()
{
	SCOPED_PROFILER("MovementSystem::update");
	World& world = mWorldHolder.getWorld();

	const auto [time] = world.getWorldComponents().getComponents<const TimeComponent>();
	const GameplayTimestamp timestampNow = time->getValue().lastFixedUpdateTimestamp;

	struct CellScheduledTransfers
	{
		CellPos cellTo;
		EntityView entityView;

		MAYBE_UNUSED CellScheduledTransfers(CellPos to, EntityView entity)
			: cellTo(to)
			, entityView(entity)
		{
		}
	};

	std::vector<CellScheduledTransfers> transfers;

	world.getSpatialData().getAllCellManagers().forEachComponentSetWithEntityAndExtraData<MovementComponent, TransformComponent>(
		[timestampNow, &transfers](WorldCell& cell, Entity entity, MovementComponent* movement, TransformComponent* transform)
	{
		EntityView entityView{ entity, cell.getEntityManager() };

		if (!movement->getNextStep().isZeroLength())
		{
			Vector2D pos = transform->getLocation() + movement->getNextStep();
			transform->setLocation(pos);

			CellPos cellPos = cell.getPos();
			bool isCellChanged = SpatialWorldData::TransformCellForPos(cellPos, pos);
			if (isCellChanged)
			{
				transfers.emplace_back(cellPos, entityView);
			}
			movement->setUpdateTimestamp(timestampNow);
			movement->setNextStep(ZERO_VECTOR);
		}

		if (transform->getRotation() != movement->getSightDirection().rotation())
		{
			transform->setRotation(movement->getSightDirection().rotation());
			movement->setUpdateTimestamp(timestampNow);
		}
	});

	for (auto& transfer : transfers)
	{
		if (auto [spatialTracked] = transfer.entityView.getComponents<SpatialTrackComponent>(); spatialTracked != nullptr)
		{
			StringId spatialTrackId = spatialTracked->getId();
			auto [trackedComponents] = world.getWorldComponents().getComponents<TrackedSpatialEntitiesComponent>();
			auto it = trackedComponents->getEntitiesRef().find(spatialTrackId);
			if (it != trackedComponents->getEntitiesRef().end())
			{
				it->second.cell = transfer.cellTo;
			}
		}
		EntityManager& oldEntityManager = transfer.entityView.getManager();
		EntityManager& newEntityManager = world.getSpatialData().getOrCreateCell(transfer.cellTo).getEntityManager();
		// the component pointer get invalidated from this line
		oldEntityManager.transferEntityTo(newEntityManager, transfer.entityView.getEntity());
	}
}
