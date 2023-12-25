#pragma once

#include <raccoon-ecs/utils/system.h>

class WorldHolder;

/**
 * System that handles camera position
 */
class CameraSystem : public RaccoonEcs::System
{
public:
	CameraSystem(WorldHolder& worldHolder) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
};
