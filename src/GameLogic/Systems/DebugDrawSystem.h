#pragma once

#include <raccoon-ecs/utils/system.h>

#include "EngineUtils/ResourceManagement/ResourceManager.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

/**
 * System that handles rendering of world objects
 */
class DebugDrawSystem final : public RaccoonEcs::System
{
public:
	explicit DebugDrawSystem(
		WorldHolder& worldHolder,
		ResourceManager& resourceManager
	) noexcept;

	void update() override;
	void init() override;

private:
	WorldHolder& mWorldHolder;
	ResourceManager& mResourceManager;

	ResourceHandle mCollisionSpriteHandle;
	ResourceHandle mNavmeshSpriteHandle;
	ResourceHandle mFontHandle;
	ResourceHandle mPointTextureHandle;
	ResourceHandle mLineTextureHandle;
};
