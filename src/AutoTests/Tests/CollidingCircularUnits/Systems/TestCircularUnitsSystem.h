#pragma once

#include <memory>
#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

class TestCircularUnitsSystem : public RaccoonEcs::System
{
public:
	TestCircularUnitsSystem(WorldHolder& worldHolder, TimeData& time) noexcept;
	~TestCircularUnitsSystem() override = default;

	void update() override;
	std::string getName() override { return "TestCircularUnitsSystem"; }

private:
	WorldHolder& mWorldHolder;
	TimeData& mTime;
};
