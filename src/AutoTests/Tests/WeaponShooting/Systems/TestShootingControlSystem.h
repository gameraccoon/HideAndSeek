#pragma once

#include <memory>
#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

class TestShootingControlSystem : public RaccoonEcs::System
{
public:
	TestShootingControlSystem(WorldHolder& worldHolder, TimeData& time) noexcept;
	~TestShootingControlSystem() override = default;

	void update() override;
	std::string getName() override { return "TestShootingControlSystem"; }

private:
	WorldHolder& mWorldHolder;
	TimeData& mTime;
};
