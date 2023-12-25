#pragma once

#include <raccoon-ecs/utils/system.h>

class WorldHolder;

/**
 * System that updates animations
 */
class AnimationSystem : public RaccoonEcs::System
{
public:
	AnimationSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
