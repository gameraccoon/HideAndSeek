#include "Base/precomp.h"

#include "GameLogic/Systems/MovementSystem.h"

#include <sdl/SDL_keycode.h>

#include "GameData/World.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/TrackedSpatialEntitiesComponent.generated.h"
#include "GameData/Components/SpatialTrackComponent.generated.h"


MovementSystem::MovementSystem(WorldHolder& worldHolder, const TimeData& timeData) noexcept
	: mWorldHolder(worldHolder)
	, mTime(timeData)
{
}

void MovementSystem::update()
{
	SCOPED_PROFILER("MovementSystem::update");
	World& world = mWorldHolder.getWorld();
	const GameplayTimestamp timestampNow = mTime.currentTimestamp;

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

	world.getSpatialData().getAllCellManagers().forEachSpatialComponentSetWithEntity<MovementComponent, TransformComponent>(
		[timestampNow, &transfers](WorldCell* cell, Entity entity, MovementComponent* movement, TransformComponent* transform)
	{
		EntityView entityView{ entity, cell->getEntityManager() };

		if (!movement->getNextStep().isZeroLength())
		{
			Vector2D pos = transform->getLocation() + movement->getNextStep();
			transform->setLocation(pos);

			CellPos cellPos = cell->getPos();
			bool isCellChanged = SpatialWorldData::TransformCellForPos(cellPos, pos);
			if (isCellChanged)
			{
				transfers.emplace_back(cellPos, entityView);
			}
			transform->setUpdateTimestamp(timestampNow);
			movement->setNextStep(ZERO_VECTOR);
		}

		if (transform->getRotation() != movement->getSightDirection().rotation())
		{
			transform->setRotation(movement->getSightDirection().rotation());
			transform->setUpdateTimestamp(timestampNow);
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
