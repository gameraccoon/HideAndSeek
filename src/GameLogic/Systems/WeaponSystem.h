#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"


/**
 * System that handles weapon manipulation
 */
class WeaponSystem : public RaccoonEcs::System
{
public:
	WeaponSystem(WorldHolder& worldHolder, const TimeData& timeData) noexcept;

	void update() override;
	std::string getName() override { return "WeaponSystem"; }

private:
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
};
