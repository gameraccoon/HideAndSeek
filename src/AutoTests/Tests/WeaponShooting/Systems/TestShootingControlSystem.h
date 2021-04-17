#pragma once

#include <memory>
#include "ECS/System.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

class TestShootingControlSystem : public System
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
