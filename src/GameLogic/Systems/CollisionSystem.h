#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"

/**
 * System that resolve object collisions
 */
class CollisionSystem : public RaccoonEcs::System
{
public:
	explicit CollisionSystem(WorldHolder& worldHolder) noexcept;
	~CollisionSystem() override = default;

	void update() override;
	std::string getName() override { return "CollisionSystem"; }

private:
	WorldHolder& mWorldHolder;
};
