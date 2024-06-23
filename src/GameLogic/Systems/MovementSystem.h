#pragma once

#include <raccoon-ecs/utils/system.h>

class WorldHolder;

/**
 * System that process characters and objects movement
 */
class MovementSystem final : public RaccoonEcs::System
{
public:
	explicit MovementSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
