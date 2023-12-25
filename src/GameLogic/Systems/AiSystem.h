#pragma once

#include <raccoon-ecs/utils/system.h>

class WorldHolder;

/**
 * System that calculates AI
 */
class AiSystem : public RaccoonEcs::System
{
public:
	AiSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
