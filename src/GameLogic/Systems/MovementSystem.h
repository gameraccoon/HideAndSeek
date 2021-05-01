#pragma once

#include <unordered_map>

#include "ECS/System.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

/**
 * System that process characters and objects movement
 */
class MovementSystem : public Ecs::System
{
public:
	MovementSystem(WorldHolder& worldHolder, const TimeData& timeData) noexcept;
	~MovementSystem() override = default;

	void update() override;
	std::string getName() override { return "MovementSystem"; }

private:
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
};
