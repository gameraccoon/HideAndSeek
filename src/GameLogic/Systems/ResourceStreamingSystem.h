#pragma once

#include <memory>

#include <raccoon-ecs/system.h>

#include "HAL/Base/ResourceManager.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

/**
 * System that loads and distributes resources
 */
class ResourceStreamingSystem : public RaccoonEcs::System
{
public:
	ResourceStreamingSystem(
		WorldHolder& worldHolder,
		HAL::ResourceManager& resourceManager) noexcept;
	~ResourceStreamingSystem() override = default;

	void update() override;
	static std::string GetSystemId() { return "ResourceStreamingSystem"; }

private:
	WorldHolder& mWorldHolder;
	HAL::ResourceManager& mResourceManager;
};
