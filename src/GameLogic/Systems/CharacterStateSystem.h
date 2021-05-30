#pragma once

#include <memory>

#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

/**
 * System that ensures correct character state
 */
class CharacterStateSystem : public RaccoonEcs::System
{
public:
	CharacterStateSystem(WorldHolder& worldHolder, const TimeData& timeData) noexcept;
	~CharacterStateSystem() override = default;

	void update() override;
	std::string getName() const override { return "CharacterStateSystem"; }

private:
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
};
