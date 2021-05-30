#include "Base/precomp.h"

#include "GameLogic/Systems/CameraSystem.h"

#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/ImguiComponent.generated.h"
#include "GameData/World.h"
#include "GameData/GameData.h"


CameraSystem::CameraSystem(
		RaccoonEcs::ComponentFilter<const TransformComponent, MovementComponent>&& cameraMoveFilter,
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>&& trackedFilter,
		RaccoonEcs::ComponentFilter<const TransformComponent>&& transformFilter,
		RaccoonEcs::ComponentFilter<const ImguiComponent>&& imguiFilter,
		RaccoonEcs::ComponentAdder<WorldCachedDataComponent>&& worldDataAdder,
		WorldHolder& worldHolder,
		const InputData& inputData) noexcept
	: mCameraMoveFilter(std::move(cameraMoveFilter))
	, mTrackedFilter(std::move(trackedFilter))
	, mTransformFilter(std::move(transformFilter))
	, mImguiFilter(std::move(imguiFilter))
	, mWorldDataAdder(std::move(worldDataAdder))
	, mWorldHolder(worldHolder)
	, mInputData(inputData)
{
}

void CameraSystem::update()
{
	GameData& gameData = mWorldHolder.getGameData();

	auto [imgui] = gameData.getGameComponents().getComponents<ImguiComponent>();
	if (imgui && imgui->getIsImguiVisible())
	{
		return;
	}

	World& world = mWorldHolder.getWorld();

	std::optional<std::pair<EntityView, CellPos>> controlledEntity = world.getTrackedSpatialEntity(mTrackedFilter, STR_TO_ID("ControlledEntity"));

	if (controlledEntity.has_value())
	{
		std::optional<std::pair<EntityView, CellPos>> mainCamera = world.getTrackedSpatialEntity(mTrackedFilter, STR_TO_ID("CameraEntity"));

		if (mainCamera.has_value())
		{
			auto [cameraTransform, cameraMovement] = mCameraMoveFilter.getComponents(mainCamera->first);
			if (cameraTransform == nullptr)
			{
				return;
			}

			Vector2D screenSize = mInputData.windowSize;
			Vector2D screenHalfSize = screenSize * 0.5f;
			Vector2D mouseScreenPos = mInputData.mousePos;

			const float cameraMobilityRate = 0.7f;

			auto [controledEntityTransform] = mTransformFilter.getComponents(controlledEntity->first);

			Vector2D cameraNewPos = controledEntityTransform->getLocation() + (mouseScreenPos - screenHalfSize) * cameraMobilityRate;
			Vector2D cameraMove = cameraNewPos - cameraTransform->getLocation();

			cameraMovement->setNextStep(cameraMove);

			WorldCachedDataComponent* worldCachedData = mWorldDataAdder.getOrAddComponent(world.getWorldComponents());
			worldCachedData->setCameraPos(cameraNewPos);
			worldCachedData->setScreenSize(screenSize);
		}
	}
}
