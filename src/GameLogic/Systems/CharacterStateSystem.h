#pragma once

#include <memory>

#include <raccoon-ecs/system.h>
#include <raccoon-ecs/async_operations.h>

#include "GameData/Components/CharacterStateComponent.generated.h"
#include "GameData/Components/StateMachineComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/AnimationGroupsComponent.generated.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/TimeData.h"

/**
 * System that ensures correct character state
 */
class CharacterStateSystem : public RaccoonEcs::System
{
public:
	CharacterStateSystem(
		RaccoonEcs::ComponentFilter<const StateMachineComponent>&& stateMachineFilter,
		RaccoonEcs::ComponentFilter<CharacterStateComponent>&& characterStateFilter,
		RaccoonEcs::ComponentFilter<const CharacterStateComponent, MovementComponent>&& characterMovementFilter,
		RaccoonEcs::ComponentFilter<const CharacterStateComponent, const MovementComponent, AnimationGroupsComponent>&& characterMovementAnimationFilter,
		WorldHolder& worldHolder,
		const TimeData& timeData) noexcept;
	~CharacterStateSystem() override = default;

	void update() override;
	static std::string GetSystemId() { return "CharacterStateSystem"; }

private:
	RaccoonEcs::ComponentFilter<const StateMachineComponent> mStateMachineFilter;
	RaccoonEcs::ComponentFilter<CharacterStateComponent> mCharacterStateFilter;
	RaccoonEcs::ComponentFilter<const CharacterStateComponent, MovementComponent> mCharacterMovementFilter;
	RaccoonEcs::ComponentFilter<const CharacterStateComponent, const MovementComponent, AnimationGroupsComponent> mCharacterMovementAnimationFilter;
	WorldHolder& mWorldHolder;
	const TimeData& mTime;
};
