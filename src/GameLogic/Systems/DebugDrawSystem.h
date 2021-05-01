#pragma once

#include <memory>
#include <vector>

#include "ECS/System.h"

#include "HAL/Base/ResourceManager.h"
#include "HAL/EngineFwd.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

class TransformComponent;
class CollisionComponent;
struct Vector2D;

/**
 * System that handles rendering of world objects
 */
class DebugDrawSystem : public Ecs::System
{
public:
	using KeyStatesMap = std::unordered_map<int, bool>;

public:
	DebugDrawSystem(WorldHolder& worldHolder, const TimeData& timeData, HAL::Engine& engine, HAL::ResourceManager& resourceManager) noexcept;
	~DebugDrawSystem() override = default;

	void update() override;
	void initResources() override;
	std::string getName() override { return "DebugDrawSystem"; }

private:
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
	HAL::Engine& mEngine;
	HAL::ResourceManager& mResourceManager;

	ResourceHandle mCollisionSpriteHandle;
	ResourceHandle mNavmeshSpriteHandle;
	ResourceHandle mFontHandle;
	ResourceHandle mPointTextureHandle;
	ResourceHandle mLineTextureHandle;
};
