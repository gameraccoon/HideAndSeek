#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

/**
 * System that process characters and objects movement
 */
class MovementSystem : public RaccoonEcs::System
{
public:
	MovementSystem(WorldHolder& worldHolder, const TimeData& timeData) noexcept;
	~MovementSystem() override = default;

	void update() override;
	std::string getName() const override { return "MovementSystem"; }

private:
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
};
