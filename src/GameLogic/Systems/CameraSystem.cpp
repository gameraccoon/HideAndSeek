#include "EngineCommon/precomp.h"

#include "GameLogic/Systems/CameraSystem.h"

#include "GameData/Components/GameplayInputComponent.generated.h"
#include "GameData/Components/ImguiComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/TrackedSpatialEntitiesComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"
#include "GameData/GameData.h"
#include "GameData/World.h"

#include "GameLogic/SharedManagers/WorldHolder.h"


CameraSystem::CameraSystem(WorldHolder& worldHolder) noexcept
	: mWorldHolder(worldHolder)
{
}

void CameraSystem::update()
{
	SCOPED_PROFILER("CameraSystem::update");
	GameData& gameData = mWorldHolder.getGameData();

	auto [imgui] = gameData.getGameComponents().getComponents<ImguiComponent>();
	if (imgui && imgui->getIsImguiVisible())
	{
		return;
	}

	World& world = mWorldHolder.getWorld();

	std::optional<std::pair<EntityView, CellPos>> controlledEntity = world.getTrackedSpatialEntity(STR_TO_ID("ControlledEntity"));

	if (controlledEntity.has_value())
	{
		std::optional<std::pair<EntityView, CellPos>> mainCamera = world.getTrackedSpatialEntity(STR_TO_ID("CameraEntity"));

		WorldCachedDataComponent* worldCachedData = world.getWorldComponents().getOrAddComponent<WorldCachedDataComponent>();
		if (mainCamera.has_value())
		{
			auto [cameraTransform, cameraMovement] = mainCamera->first.getComponents<TransformComponent, MovementComponent>();
			if (cameraTransform == nullptr)
			{
				return;
			}

			const GameplayInputComponent* gameplayInput = world.getWorldComponents().getOrAddComponent<GameplayInputComponent>();
			const GameplayInput::FrameState& inputState = gameplayInput->getCurrentFrameState();
			const Vector2D aimDirection{inputState.getAxisValue(GameplayInput::InputAxis::AimHorizontal), inputState.getAxisValue(GameplayInput::InputAxis::AimVertical)};

			constexpr float cameraMobilityRate = 100.0f;

			auto [controledEntityTransform] = controlledEntity->first.getComponents<TransformComponent>();

			Vector2D cameraNewPos = controledEntityTransform->getLocation() + aimDirection * cameraMobilityRate;
			Vector2D cameraMove = cameraNewPos - cameraTransform->getLocation();

			cameraMovement->setNextStep(cameraMove);

			worldCachedData->setCameraPos(cameraNewPos);
		}
	}
}
