#include "Base/precomp.h"

#include "GameLogic/Systems/ControlSystem.h"

#include <sdl/SDL_keycode.h>
#include <sdl/SDL_mouse.h>

#include "GameData/World.h"
#include "GameData/Components/CharacterStateComponent.generated.h"
#include "GameData/Components/GameplayInputComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"

#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/InputData.h"


ControlSystem::ControlSystem(WorldHolder& worldHolder) noexcept
	: mWorldHolder(worldHolder)
{
}

void ControlSystem::update()
{
	SCOPED_PROFILER("ControlSystem::update");

	World& world = mWorldHolder.getWorld();

	std::optional<std::pair<EntityView, CellPos>> controlledEntity = world.getTrackedSpatialEntity(STR_TO_ID("ControlledEntity"));

	if (!controlledEntity.has_value())
	{
		return;
	}

	GameplayInputComponent* gameplayInput = world.getWorldComponents().getOrAddComponent<GameplayInputComponent>();
	const GameplayInput::FrameState& inputState = gameplayInput->getCurrentFrameState();

	const bool isSprintPressed = inputState.isKeyActive(GameplayInput::InputKey::Sprint);
	const bool isShootPressed = inputState.isKeyActive(GameplayInput::InputKey::Shoot);

	const Vector2D movementDirection(inputState.getAxisValue(GameplayInput::InputAxis::MoveHorizontal), inputState.getAxisValue(GameplayInput::InputAxis::MoveVertical));
	const Vector2D aimDirection(inputState.getAxisValue(GameplayInput::InputAxis::AimHorizontal), inputState.getAxisValue(GameplayInput::InputAxis::AimVertical));

	if (auto [characterState] = controlledEntity->first.getComponents<CharacterStateComponent>(); characterState != nullptr)
	{
		characterState->getBlackboardRef().setValue<bool>(CharacterStateBlackboardKeys::TryingToMove, !movementDirection.isZeroLength());
		characterState->getBlackboardRef().setValue<bool>(CharacterStateBlackboardKeys::ReadyToRun, isSprintPressed);
		characterState->getBlackboardRef().setValue<bool>(CharacterStateBlackboardKeys::TryingToShoot, isShootPressed);
	}

	auto [transform, movement] = controlledEntity->first.getComponents<const TransformComponent, MovementComponent>();
	movement->setMoveDirection(movementDirection);

	std::optional<std::pair<EntityView, CellPos>> mainCamera = world.getTrackedSpatialEntity(STR_TO_ID("CameraEntity"));

	if (mainCamera.has_value())
	{
		auto [cameraTransform] = mainCamera->first.getComponents<TransformComponent>();
		if (cameraTransform == nullptr)
		{
			return;
		}

		movement->setSightDirection(aimDirection);
	}
}

