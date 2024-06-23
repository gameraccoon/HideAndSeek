#pragma once

#include <raccoon-ecs/utils/system.h>

class WorldHolder;

class TestShootingControlSystem final : public RaccoonEcs::System
{
public:
	explicit TestShootingControlSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
