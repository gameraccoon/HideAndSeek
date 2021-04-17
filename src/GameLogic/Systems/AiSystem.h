#pragma once

#include <memory>

#include "ECS/System.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

/**
 * System that calculates AI
 */
class AiSystem : public System
{
public:
	AiSystem(WorldHolder& worldHolder, const TimeData& timeData) noexcept;
	~AiSystem() override = default;

	void update() override;
	std::string getName() override { return "AiSystem"; }

private:
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
};
