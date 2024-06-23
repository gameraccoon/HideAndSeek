#pragma once

#include <raccoon-ecs/utils/system.h>

class WorldHolder;

/**
 * System that ensures correct character state
 */
class CharacterStateSystem final : public RaccoonEcs::System
{
public:
	explicit CharacterStateSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
