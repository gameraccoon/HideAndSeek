#pragma once

#include <raccoon-ecs/system.h>

class WorldHolder;

/**
 * System that ensures correct character state
 */
class CharacterStateSystem : public RaccoonEcs::System
{
public:
	CharacterStateSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
