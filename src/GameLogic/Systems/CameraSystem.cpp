#include "Base/precomp.h"

#include "GameLogic/Systems/CameraSystem.h"

#include "GameData/World.h"
#include "GameData/GameData.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/ImguiComponent.generated.h"
#include "GameData/Components/TrackedSpatialEntitiesComponent.generated.h"


CameraSystem::CameraSystem(
		WorldHolder& worldHolder,
		const InputData& inputData) noexcept
	: mWorldHolder(worldHolder)
	, mInputData(inputData)
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

		if (mainCamera.has_value())
		{
			auto [cameraTransform, cameraMovement] = mainCamera->first.getComponents<TransformComponent, MovementComponent>();
			if (cameraTransform == nullptr)
			{
				return;
			}

			Vector2D screenSize = mInputData.windowSize;
			Vector2D screenHalfSize = screenSize * 0.5f;
			Vector2D mouseScreenPos = mInputData.mousePos;

			const float cameraMobilityRate = 0.7f;

			auto [controledEntityTransform] = controlledEntity->first.getComponents<TransformComponent>();

			Vector2D cameraNewPos = controledEntityTransform->getLocation() + (mouseScreenPos - screenHalfSize) * cameraMobilityRate;
			Vector2D cameraMove = cameraNewPos - cameraTransform->getLocation();

			cameraMovement->setNextStep(cameraMove);

			auto [worldCachedData] = world.getWorldComponents().getComponents<WorldCachedDataComponent>();
			worldCachedData->setCameraPos(cameraNewPos);
			worldCachedData->setScreenSize(screenSize);
		}
	}
}
