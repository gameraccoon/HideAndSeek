#pragma once

#include <memory>
#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"


class TestShootingControlSystem : public RaccoonEcs::System
{
public:
	TestShootingControlSystem(WorldHolder& worldHolder) noexcept;

	~TestShootingControlSystem() override = default;

	void update() override;
	static std::string GetSystemId() { return "TestShootingControlSystem"; }

private:
	WorldHolder& mWorldHolder;
};
