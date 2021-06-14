#pragma once

#include <memory>
#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameData/Components/NavMeshComponent.generated.h"
#include "GameData/Components/AiControllerComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

class TestCircularUnitsSystem : public RaccoonEcs::System
{
public:
	TestCircularUnitsSystem(
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>&& trackedFilter,
		RaccoonEcs::ComponentFilter<const TransformComponent>&& transformFilter,
		RaccoonEcs::ComponentFilter<const AiControllerComponent, const TransformComponent, MovementComponent>&& aiMovementFilter,
		WorldHolder& worldHolder,
		TimeData& time
	) noexcept;
	~TestCircularUnitsSystem() override = default;

	void update() override;
	std::string getName() const override { return "TestCircularUnitsSystem"; }

private:
	RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent> mTrackedFilter;
	RaccoonEcs::ComponentFilter<const TransformComponent> mTransformFilter;
	RaccoonEcs::ComponentFilter<const AiControllerComponent, const TransformComponent, MovementComponent> mAiMovementFilter;
	WorldHolder& mWorldHolder;
	TimeData& mTime;
};
