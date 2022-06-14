#pragma once

#include <memory>
#include <raccoon-ecs/system.h>

class WorldHolder;

class TestCircularUnitsSystem : public RaccoonEcs::System
{
public:
	TestCircularUnitsSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
