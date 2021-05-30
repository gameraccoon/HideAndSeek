#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/InputData.h"

class TransformComponent;
class MovementComponent;
class TrackedSpatialEntitiesComponent;
class ImguiComponent;
class WorldCachedDataComponent;

/**
 * System that handles camera position
 */
class CameraSystem : public RaccoonEcs::System
{
public:
	CameraSystem(
		RaccoonEcs::ComponentFilter<const TransformComponent, MovementComponent>&& cameraMoveFilter,
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>&& trackedFilter,
		RaccoonEcs::ComponentFilter<const TransformComponent>&& transformFilter,
		RaccoonEcs::ComponentFilter<const ImguiComponent>&& imguiFilter,
		RaccoonEcs::ComponentAdder<WorldCachedDataComponent>&& worldDataAdder,
		WorldHolder& worldHolder,
		const InputData& inputData) noexcept;
	~CameraSystem() override = default;

	void update() override;
	std::string getName() const override { return "CameraSystem"; }

private:
	RaccoonEcs::ComponentFilter<const TransformComponent, MovementComponent> mCameraMoveFilter;
	RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent> mTrackedFilter;
	RaccoonEcs::ComponentFilter<const TransformComponent> mTransformFilter;
	RaccoonEcs::ComponentFilter<const ImguiComponent> mImguiFilter;
	RaccoonEcs::ComponentAdder<WorldCachedDataComponent> mWorldDataAdder;
	WorldHolder& mWorldHolder;
	const InputData& mInputData;
};
