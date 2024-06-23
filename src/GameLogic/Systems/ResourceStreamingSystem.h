#pragma once

#include <raccoon-ecs/utils/system.h>

class ResourceManager;
class WorldHolder;

/**
 * System that loads and distributes resources
 */
class ResourceStreamingSystem final : public RaccoonEcs::System
{
public:
	explicit ResourceStreamingSystem(
		WorldHolder& worldHolder,
		ResourceManager& resourceManager) noexcept;

	void update() override;

private:
	WorldHolder& mWorldHolder;
	ResourceManager& mResourceManager;
};
