#pragma once

#include <memory>
#include <vector>

#include <raccoon-ecs/system.h>

#include "HAL/EngineFwd.h"

#include "Utils/ResourceManagement/ResourceManager.h"

#include "GameLogic/SharedManagers/WorldHolder.h"

/**
 * System that handles rendering of world objects
 */
class DebugDrawSystem : public RaccoonEcs::System
{
public:
	using KeyStatesMap = std::unordered_map<int, bool>;

public:
	DebugDrawSystem(
		WorldHolder& worldHolder,
		ResourceManager& resourceManager) noexcept;
	~DebugDrawSystem() override = default;

	void update() override;
	void init() override;
	static std::string GetSystemId() { return "DebugDrawSystem"; }

private:
	WorldHolder& mWorldHolder;
	ResourceManager& mResourceManager;

	ResourceHandle mCollisionSpriteHandle;
	ResourceHandle mNavmeshSpriteHandle;
	ResourceHandle mFontHandle;
	ResourceHandle mPointTextureHandle;
	ResourceHandle mLineTextureHandle;
};
