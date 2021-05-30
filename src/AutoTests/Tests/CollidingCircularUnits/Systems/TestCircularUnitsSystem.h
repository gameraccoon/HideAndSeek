#pragma once

#include <memory>
#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

class TrackedSpatialEntitiesComponent;

class TestCircularUnitsSystem : public RaccoonEcs::System
{
public:
	TestCircularUnitsSystem(
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>&& trackedFilter,
		WorldHolder& worldHolder,
		TimeData& time
	) noexcept;
	~TestCircularUnitsSystem() override = default;

	void update() override;
	std::string getName() const override { return "TestCircularUnitsSystem"; }

private:
	RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent> mTrackedFilter;
	WorldHolder& mWorldHolder;
	TimeData& mTime;
};
