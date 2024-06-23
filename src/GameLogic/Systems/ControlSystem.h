#pragma once

#include <raccoon-ecs/utils/system.h>

class WorldHolder;

/**
 * System that processes gameplay controls
 */
class ControlSystem final : public RaccoonEcs::System
{
public:
	explicit ControlSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
