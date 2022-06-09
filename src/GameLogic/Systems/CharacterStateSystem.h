#pragma once

#include <memory>

#include <raccoon-ecs/system.h>

#include "GameLogic/SharedManagers/WorldHolder.h"

/**
 * System that ensures correct character state
 */
class CharacterStateSystem : public RaccoonEcs::System
{
public:
	CharacterStateSystem(WorldHolder& worldHolder) noexcept;
	~CharacterStateSystem() override = default;

	void update() override;
	static std::string GetSystemId() { return "CharacterStateSystem"; }

private:
	WorldHolder& mWorldHolder;
};
