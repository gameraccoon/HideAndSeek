#pragma once

#include <raccoon-ecs/utils/system.h>

class WorldHolder;

/**
 * System that destructs enemies marked with DeathComponent
 */
class DeadEntitiesDestructionSystem : public RaccoonEcs::System
{
public:
	explicit DeadEntitiesDestructionSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
