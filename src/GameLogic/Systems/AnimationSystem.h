#pragma once

#include <memory>

#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"

/**
 * System that updates animations
 */
class AnimationSystem : public RaccoonEcs::System
{
public:
	AnimationSystem(WorldHolder& worldHolder) noexcept;
	~AnimationSystem() override = default;

	void update() override;
	static std::string GetSystemId() { return "AnimationSystem"; }

private:
	WorldHolder& mWorldHolder;
};
