#include "Base/precomp.h"

#include "GameLogic/Systems/MovementSystem.h"

#include <sdl/SDL_keycode.h>

#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/TrackedSpatialEntitiesComponent.generated.h"
#include "GameData/Components/SpatialTrackComponent.generated.h"

#include "GameData/World.h"


MovementSystem::MovementSystem(
		RaccoonEcs::ComponentFilter<MovementComponent, TransformComponent>&& movementFilter,
		RaccoonEcs::ComponentFilter<SpatialTrackComponent>&& spatialTrackFilter,
		RaccoonEcs::ComponentFilter<TrackedSpatialEntitiesComponent>&& trackedSpatialEntitiesFilter,
		RaccoonEcs::EntityTransferer&& entityTransferer,
		WorldHolder& worldHolder,
		const TimeData& timeData) noexcept
	: mMovementFilter(std::move(movementFilter))
	, mSpatialTrackFilter(std::move(spatialTrackFilter))
	, mTrackedSpatialEntitiesFilter(std::move(trackedSpatialEntitiesFilter))
	, mEntityTransferer(std::move(entityTransferer))
	, mWorldHolder(worldHolder)
	, mTime(timeData)
{
}

void MovementSystem::update()
{
	World& world = mWorldHolder.getWorld();
	const GameplayTimestamp timestampNow = mTime.currentTimestamp;

	struct CellScheduledTransfers
	{
		CellPos cellTo;
		AsyncEntityView entityView;

		MAYBE_UNUSED CellScheduledTransfers(CellPos to, AsyncEntityView entity)
			: cellTo(to)
			, entityView(entity)
		{
		}
	};

	std::vector<CellScheduledTransfers> transfers;

	world.getSpatialData().getAllCellManagers().forEachSpatialComponentSetWithEntity(
		mMovementFilter,
		[timestampNow, &transfers](WorldCell* cell, Entity entity, MovementComponent* movement, TransformComponent* transform)
	{
		AsyncEntityView entityView{ entity, cell->getEntityManager() };

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
		if (auto [spatialTracked] = transfer.entityView.getComponents(mSpatialTrackFilter); spatialTracked != nullptr)
		{
			StringId spatialTrackId = spatialTracked->getId();
			auto [trackedComponents] = mTrackedSpatialEntitiesFilter.getComponents(world.getWorldComponents());
			auto it = trackedComponents->getEntitiesRef().find(spatialTrackId);
			if (it != trackedComponents->getEntitiesRef().end())
			{
				it->second.cell = transfer.cellTo;
			}
		}
		AsyncEntityManager& oldEntityManager = transfer.entityView.getManager();
		AsyncEntityManager& newEntityManager = world.getSpatialData().getOrCreateCell(transfer.cellTo).getEntityManager();
		// the component pointer get invalidated from this line
		mEntityTransferer.transferEntity(oldEntityManager, newEntityManager, transfer.entityView.getEntity());
	}
}
