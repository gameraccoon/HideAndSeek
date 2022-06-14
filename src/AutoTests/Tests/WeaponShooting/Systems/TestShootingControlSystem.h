#pragma once

#include <raccoon-ecs/system.h>

class WorldHolder;

class TestShootingControlSystem : public RaccoonEcs::System
{
public:
	TestShootingControlSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
