#pragma once

#include <memory>

#include "ECS/System.h"

#include "HAL/Base/ResourceManager.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

/**
 * System that loads and distributes resources
 */
class ResourceStreamingSystem : public Ecs::System
{
public:
	ResourceStreamingSystem(WorldHolder& worldHolder, HAL::ResourceManager& resourceManager) noexcept;
	~ResourceStreamingSystem() override = default;

	void update() override;
	std::string getName() override { return "ResourceStreamingSystem"; }

private:
	WorldHolder& mWorldHolder;
	HAL::ResourceManager& mResourceManager;
};
