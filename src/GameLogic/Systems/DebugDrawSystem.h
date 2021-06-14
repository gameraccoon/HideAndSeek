#pragma once

#include <memory>
#include <vector>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/NavMeshComponent.generated.h"
#include "GameData/Components/RenderModeComponent.generated.h"
#include "GameData/Components/AiControllerComponent.generated.h"
#include "GameData/Components/CharacterStateComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"
#include "GameData/Components/DebugDrawComponent.generated.h"

#include "HAL/Base/ResourceManager.h"
#include "HAL/EngineFwd.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

/**
 * System that handles rendering of world objects
 */
class DebugDrawSystem : public RaccoonEcs::System
{
public:
	using KeyStatesMap = std::unordered_map<int, bool>;

public:
	DebugDrawSystem(
		RaccoonEcs::ComponentFilter<const WorldCachedDataComponent>&& worldCachedDataFilter,
		RaccoonEcs::ComponentFilter<const RenderModeComponent>&& renderModeFilter,
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent>&& collisionDataFilter,
		RaccoonEcs::ComponentFilter<const NavMeshComponent>&& navMeshFilter,
		RaccoonEcs::ComponentFilter<const AiControllerComponent>&& aiControllerFilter,
		RaccoonEcs::ComponentFilter<const DebugDrawComponent>&& debugDrawFilter,
		RaccoonEcs::ComponentFilter<const CharacterStateComponent, class TransformComponent>&& characterStateFilter,
		WorldHolder& worldHolder,
		const TimeData& timeData,
		HAL::Engine& engine,
		HAL::ResourceManager& resourceManager) noexcept;
	~DebugDrawSystem() override = default;

	void update() override;
	void initResources() override;
	std::string getName() const override { return "DebugDrawSystem"; }

private:
	RaccoonEcs::ComponentFilter<const WorldCachedDataComponent> mWorldCachedDataFilter;
	RaccoonEcs::ComponentFilter<const RenderModeComponent> mRenderModeFilter;
	RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent> mCollisionDataFilter;
	RaccoonEcs::ComponentFilter<const NavMeshComponent> mNavMeshFilter;
	RaccoonEcs::ComponentFilter<const AiControllerComponent> mAiControllerFilter;
	RaccoonEcs::ComponentFilter<const DebugDrawComponent> mDebugDrawFilter;
	RaccoonEcs::ComponentFilter<const CharacterStateComponent, class TransformComponent> mCharacterStateFilter;
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
