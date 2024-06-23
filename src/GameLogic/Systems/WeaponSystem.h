#pragma once

#include <raccoon-ecs/utils/system.h>

class WorldHolder;

/**
 * System that handles weapon manipulation
 */
class WeaponSystem final : public RaccoonEcs::System
{
public:
	explicit WeaponSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
