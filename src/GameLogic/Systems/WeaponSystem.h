#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"

/**
 * System that handles weapon manipulation
 */
class WeaponSystem : public RaccoonEcs::System
{
public:
	WeaponSystem(WorldHolder& worldHolder) noexcept;

	void update() override;
	static std::string GetSystemId() { return "WeaponSystem"; }

private:
	WorldHolder& mWorldHolder;
};
