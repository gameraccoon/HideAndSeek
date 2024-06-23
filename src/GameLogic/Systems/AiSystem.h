#pragma once

#include <raccoon-ecs/utils/system.h>

class WorldHolder;

/**
 * System that calculates AI
 */
class AiSystem final : public RaccoonEcs::System
{
public:
	explicit AiSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
