#pragma once

#include <raccoon-ecs/system.h>

class WorldHolder;

/**
 * System that resolve object collisions
 */
class CollisionSystem : public RaccoonEcs::System
{
public:
	CollisionSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
