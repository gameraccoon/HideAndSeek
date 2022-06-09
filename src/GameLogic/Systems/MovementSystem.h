#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"

/**
 * System that process characters and objects movement
 */
class MovementSystem : public RaccoonEcs::System
{
public:
	MovementSystem(WorldHolder& worldHolder) noexcept;
	~MovementSystem() override = default;

	void update() override;
	static std::string GetSystemId() { return "MovementSystem"; }

private:
	WorldHolder& mWorldHolder;
};
