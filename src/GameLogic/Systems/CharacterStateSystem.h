#pragma once

#include <memory>

#include "ECS/System.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

/**
 * System that ensures correct character state
 */
class CharacterStateSystem : public Ecs::System
{
public:
	CharacterStateSystem(WorldHolder& worldHolder, const TimeData& timeData) noexcept;
	~CharacterStateSystem() override = default;

	void update() override;
	std::string getName() override { return "CharacterStateSystem"; }

private:
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
};
