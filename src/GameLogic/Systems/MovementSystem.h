#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

class MovementComponent;
class TransformComponent;
class SpatialTrackComponent;
class TrackedSpatialEntitiesComponent;

/**
 * System that process characters and objects movement
 */
class MovementSystem : public RaccoonEcs::System
{
public:
	MovementSystem(
		RaccoonEcs::ComponentFilter<MovementComponent, TransformComponent>&& movementFilter,
		RaccoonEcs::ComponentFilter<SpatialTrackComponent>&& spatialTrackFilter,
		RaccoonEcs::ComponentFilter<TrackedSpatialEntitiesComponent>&& trackedSpatialEntitiesFilter,
		RaccoonEcs::EntityTransferer&& entityTransferer,
		WorldHolder& worldHolder,
		const TimeData& timeData) noexcept;
	~MovementSystem() override = default;

	void update() override;
	std::string getName() const override { return "MovementSystem"; }

private:
	RaccoonEcs::ComponentFilter<MovementComponent, TransformComponent> mMovementFilter;
	RaccoonEcs::ComponentFilter<SpatialTrackComponent> mSpatialTrackFilter;
	RaccoonEcs::ComponentFilter<TrackedSpatialEntitiesComponent> mTrackedSpatialEntitiesFilter;
	RaccoonEcs::EntityTransferer mEntityTransferer;
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
};
