#pragma once

#include <raccoon-ecs/utils/system.h>

class WorldHolder;

class TestCircularUnitsSystem final : public RaccoonEcs::System
{
public:
	explicit TestCircularUnitsSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
