#pragma once

#include <raccoon-ecs/utils/system.h>

class WorldHolder;

/**
 * System that resolve object collisions
 */
class CollisionSystem final : public RaccoonEcs::System
{
public:
	explicit CollisionSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
