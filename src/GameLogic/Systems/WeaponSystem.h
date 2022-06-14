#pragma once

#include <raccoon-ecs/system.h>

class WorldHolder;

/**
 * System that handles weapon manipulation
 */
class WeaponSystem : public RaccoonEcs::System
{
public:
	WeaponSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
