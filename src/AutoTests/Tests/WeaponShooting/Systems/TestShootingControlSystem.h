#pragma once

#include <memory>
#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

class TrackedSpatialEntitiesComponent;

class TestShootingControlSystem : public RaccoonEcs::System
{
public:
	TestShootingControlSystem(
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>&& trackedFilter,
		WorldHolder& worldHolder,
		TimeData& time
	) noexcept;

	~TestShootingControlSystem() override = default;

	void update() override;
	std::string getName() const override { return "TestShootingControlSystem"; }

private:
	RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent> mTrackedFilter;
	WorldHolder& mWorldHolder;
	TimeData& mTime;
};
