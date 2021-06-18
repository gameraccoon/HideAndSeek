#pragma once

#include <unordered_map>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/RenderModeComponent.generated.h"
#include "GameData/Components/ImguiComponent.generated.h"
#include "GameData/Components/CharacterStateComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/InputData.h"

/**
 * System that handles movement controls
 */
class ControlSystem : public RaccoonEcs::System
{
public:
	ControlSystem(
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>&& trackedFilter,
		RaccoonEcs::ComponentFilter<CharacterStateComponent>&& characterStateFilter,
		RaccoonEcs::ComponentFilter<ImguiComponent>&& imguiFilter,
		RaccoonEcs::ComponentFilter<RenderModeComponent>&& renderModeFilter,
		RaccoonEcs::ComponentFilter<const TransformComponent, MovementComponent>&& moveFilter,
		RaccoonEcs::ComponentFilter<const TransformComponent>&& transformFilter,
		WorldHolder& worldHolder,
		const InputData& inputData
	) noexcept;
	~ControlSystem() override = default;

	void update() override;
	static std::string GetSystemId() { return "ControlSystem"; }

private:
	void processPlayerInput();

private:
	RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent> mTrackedFilter;
	RaccoonEcs::ComponentFilter<CharacterStateComponent> mCharacterStateFilter;
	RaccoonEcs::ComponentFilter<ImguiComponent> mImguiFilter;
	RaccoonEcs::ComponentFilter<RenderModeComponent> mRenderModeFilter;
	RaccoonEcs::ComponentFilter<const TransformComponent, MovementComponent> mMoveFilter;
	RaccoonEcs::ComponentFilter<const TransformComponent> mTransformFilter;
	WorldHolder& mWorldHolder;
	const InputData& mInputData;
};
