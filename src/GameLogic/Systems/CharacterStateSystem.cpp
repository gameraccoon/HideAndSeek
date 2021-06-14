#include "Base/precomp.h"

#include "GameLogic/Systems/CharacterStateSystem.h"

#include "GameData/Core/Rotator.h"

#include "GameData/Enums/MoveDirection4.generated.h"

#include "GameData/World.h"
#include "GameData/GameData.h"


CharacterStateSystem::CharacterStateSystem(
		RaccoonEcs::ComponentFilter<const StateMachineComponent>&& stateMachineFilter,
		RaccoonEcs::ComponentFilter<CharacterStateComponent>&& characterStateFilter,
		RaccoonEcs::ComponentFilter<const CharacterStateComponent, MovementComponent>&& characterMovementFilter,
		RaccoonEcs::ComponentFilter<const CharacterStateComponent, const MovementComponent, AnimationGroupsComponent>&& characterMovementAnimationFilter,
		WorldHolder& worldHolder,
		const TimeData& timeData) noexcept
	: mStateMachineFilter(std::move(stateMachineFilter))
	, mCharacterStateFilter(std::move(characterStateFilter))
	, mCharacterMovementFilter(std::move(characterMovementFilter))
	, mCharacterMovementAnimationFilter(std::move(characterMovementAnimationFilter))
	, mWorldHolder(worldHolder)
	, mTime(timeData)
{
}

static bool CanMove(CharacterState /*state*/)
{
	return true;
}

static bool IsRunning(CharacterState state)
{
	return state == CharacterState::Run;
}

void CharacterStateSystem::update()
{
	World& world = mWorldHolder.getWorld();
	GameData& gameData = mWorldHolder.getGameData();
	float dt = mTime.dt;

	auto [stateMachine] = mStateMachineFilter.getComponents(gameData.getGameComponents());

	if (stateMachine)
	{
		auto allCellManagers = world.getSpatialData().getAllCellManagers();
		// update states
		allCellManagers.forEachComponentSet(
			mCharacterStateFilter,
			[stateMachine, dt](CharacterStateComponent* characterState)
		{
			// calculate state
			CharacterState state = stateMachine->getCharacterSM().getNextState(characterState->getBlackboard(), characterState->getState());
			characterState->setState(state);
		});

		// update movements
		allCellManagers.forEachComponentSet(
			mCharacterMovementFilter,
			[stateMachine, dt](const CharacterStateComponent* characterState, MovementComponent* movement)
		{
			CharacterState state = characterState->getState();
			// allow movement
			if (!CanMove(state))
			{
				movement->setMoveDirection(ZERO_VECTOR);
				movement->setSightDirection(ZERO_VECTOR);
				movement->setNextStep(ZERO_VECTOR);
			}
			movement->setSpeed(IsRunning(state) ? movement->getOriginalSpeed() * 2.0f : movement->getOriginalSpeed());
			movement->setNextStep(movement->getMoveDirection().unit() * movement->getSpeed() * dt);
		});

		// update animation
		allCellManagers.forEachComponentSet(
			mCharacterMovementAnimationFilter,
			[stateMachine, dt](const CharacterStateComponent* characterState, const MovementComponent* movement, AnimationGroupsComponent* animationGroups)
		{
			CharacterState state = characterState->getState();

			auto& animBlackboard = animationGroups->getBlackboardRef();
			animBlackboard.setValue<StringId>(STR_TO_ID("charState"), enum_to_string(state));

			animBlackboard.setValue<bool>(enum_to_string(CharacterStateBlackboardKeys::TryingToMove), characterState->getBlackboard().getValue<bool>(CharacterStateBlackboardKeys::TryingToMove, false));
			animBlackboard.setValue<bool>(enum_to_string(CharacterStateBlackboardKeys::ReadyToRun), characterState->getBlackboard().getValue<bool>(CharacterStateBlackboardKeys::ReadyToRun, false));

			Vector2D moveDirection = movement->getMoveDirection();
			if (!moveDirection.isZeroLength())
			{
				float relativeRotation = (moveDirection.rotation() - movement->getSightDirection().rotation()).getValue();

				if (relativeRotation < PI * 0.25f && relativeRotation > -PI * 0.25f)
				{
					animBlackboard.setValue<MoveDirection4>(STR_TO_ID("moveDir"), MoveDirection4::Front);
				}
				else if (relativeRotation > PI * 0.75f || relativeRotation < -PI * 0.75f)
				{
					animBlackboard.setValue<MoveDirection4>(STR_TO_ID("moveDir"), MoveDirection4::Back);
				}
				else if (relativeRotation < 0.0f)
				{
					animBlackboard.setValue<MoveDirection4>(STR_TO_ID("moveDir"), MoveDirection4::Left);
				}
				else
				{
					animBlackboard.setValue<MoveDirection4>(STR_TO_ID("moveDir"), MoveDirection4::Right);
				}
			}
		});
	}
}
