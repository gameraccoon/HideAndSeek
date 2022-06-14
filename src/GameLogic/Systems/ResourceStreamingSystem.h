#pragma once

#include <memory>

#include <raccoon-ecs/system.h>

class ResourceManager;
class WorldHolder;

/**
 * System that loads and distributes resources
 */
class ResourceStreamingSystem : public RaccoonEcs::System
{
public:
	ResourceStreamingSystem(
		WorldHolder& worldHolder,
		ResourceManager& resourceManager) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
	ResourceManager& mResourceManager;
};
