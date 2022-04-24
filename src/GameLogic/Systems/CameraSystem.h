#pragma once

#include <string>
#include <unordered_map>

#include <raccoon-ecs/system.h>

class WorldHolder;

/**
 * System that handles camera position
 */
class CameraSystem : public RaccoonEcs::System
{
public:
	CameraSystem(WorldHolder& worldHolder) noexcept;

	void update() override;
	static std::string GetSystemId() { return "CameraSystem"; }

private:
	WorldHolder& mWorldHolder;
};
