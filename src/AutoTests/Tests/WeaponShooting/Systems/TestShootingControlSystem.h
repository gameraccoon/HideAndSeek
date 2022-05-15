#pragma once

#include <memory>
#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"


class TestShootingControlSystem : public RaccoonEcs::System
{
public:
	TestShootingControlSystem(
		WorldHolder& worldHolder,
		const TimeData& time
	) noexcept;

	~TestShootingControlSystem() override = default;

	void update() override;
	static std::string GetSystemId() { return "TestShootingControlSystem"; }

private:
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
};
