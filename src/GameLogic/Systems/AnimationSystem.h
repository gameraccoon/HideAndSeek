#pragma once

#include <raccoon-ecs/utils/system.h>

class WorldHolder;

/**
 * System that updates animations
 */
class AnimationSystem final : public RaccoonEcs::System
{
public:
	explicit AnimationSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
