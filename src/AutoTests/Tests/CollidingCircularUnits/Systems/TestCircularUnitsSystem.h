#pragma once

#include <memory>
#include "ECS/System.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

class TestCircularUnitsSystem : public Ecs::System
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
