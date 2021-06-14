#pragma once

#include <memory>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameData/Components/NavMeshComponent.generated.h"
#include "GameData/Components/AiControllerComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/CharacterStateComponent.generated.h"
#include "GameData/Components/PathBlockingGeometryComponent.generated.h"
#include "GameData/Components/DebugDrawComponent.generated.h"
#include "GameData/Components/TrackedSpatialEntitiesComponent.generated.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

/**
 * System that calculates AI
 */
class AiSystem : public RaccoonEcs::System
{
	using NavDataReader = RaccoonEcs::ComponentFilter<
		AiControllerComponent,
		const TransformComponent,
		MovementComponent,
		CharacterStateComponent
	>;

public:
	AiSystem(
		RaccoonEcs::ComponentAdder<NavMeshComponent>&& navMeshDataFilter,
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent>&& collisionDataFilter,
		RaccoonEcs::ComponentFilter<const TransformComponent>&& transformFilter,
		NavDataReader&& navDataFilter,
		RaccoonEcs::ComponentFilter<DebugDrawComponent>&& debugDrawFilter,
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>&& trackedFilter,
		RaccoonEcs::ComponentFilter<const PathBlockingGeometryComponent>&& pathBlockingGeometryFilter,
		WorldHolder& worldHolder,
		const TimeData& timeData
	) noexcept;

	~AiSystem() override = default;

	void update() override;
	std::string getName() const override { return "AiSystem"; }

private:
	RaccoonEcs::ComponentAdder<NavMeshComponent> mNavMeshDataFilter;
	RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent>&& mCollisionDataFilter;
	RaccoonEcs::ComponentFilter<const TransformComponent> mTransformFilter;
	NavDataReader mNavDataFilter;
	RaccoonEcs::ComponentFilter<DebugDrawComponent> mDebugDrawFilter;
	RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent> mTrackedFilter;
	RaccoonEcs::ComponentFilter<const PathBlockingGeometryComponent> mPathBlockingGeometryFilter;
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
};
