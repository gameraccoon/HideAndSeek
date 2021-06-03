#include "Base/precomp.h"

#include "AutoTests/Tests/CollidingCircularUnits/TestCase.h"

#include <memory>

#include "HAL/Base/Engine.h"

#include "GameData/Spatial/SpatialWorldData.h"

#include "GameData/Components/SpriteCreatorComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/StateMachineComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"

#include "GameLogic/Systems/RenderSystem.h"
#include "GameLogic/Systems/CollisionSystem.h"
#include "GameLogic/Systems/ResourceStreamingSystem.h"
#include "GameLogic/Systems/MovementSystem.h"
#include "GameLogic/Systems/CharacterStateSystem.h"
#include "GameLogic/Systems/CameraSystem.h"

#include "AutoTests/Tests/CollidingCircularUnits/Systems/TestCircularUnitsSystem.h"
#include "AutoTests/Tests/CollidingCircularUnits/Systems/TestUnitsCountControlSystem.h"

void CollidingCircularUnitsTestCase::initTestCase(const ArgumentsParser& /*arguments*/)
{
	getResourceManager().loadAtlasesData("resources/atlas/atlas-list.json");

	mSystemsManager.registerSystem<TestUnitsCountControlSystem>(
		RaccoonEcs::EntityAdder(),
		RaccoonEcs::ComponentAdder<TransformComponent>(),
		RaccoonEcs::ComponentAdder<MovementComponent>(),
		RaccoonEcs::ComponentAdder<SpriteCreatorComponent>(),
		RaccoonEcs::ComponentAdder<CollisionComponent>(),
		RaccoonEcs::ComponentAdder<AiControllerComponent>(),
		RaccoonEcs::ComponentAdder<CharacterStateComponent>(),
		mWorldHolder
	);

	mSystemsManager.registerSystem<TestCircularUnitsSystem>(
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>(),
		RaccoonEcs::ComponentFilter<const TransformComponent>(),
		RaccoonEcs::ComponentFilter<const AiControllerComponent, const TransformComponent, MovementComponent>(),
		mWorldHolder,
		mTime
	);

	mSystemsManager.registerSystem<CollisionSystem>(
		RaccoonEcs::ComponentFilter<CollisionComponent, const TransformComponent>(),
		RaccoonEcs::ComponentFilter<MovementComponent>(),
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent, MovementComponent>(),
		mWorldHolder
	);

	mSystemsManager.registerSystem<CameraSystem>(
		RaccoonEcs::ComponentFilter<const TransformComponent, MovementComponent>(),
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>(),
		RaccoonEcs::ComponentFilter<const TransformComponent>(),
		RaccoonEcs::ComponentFilter<const ImguiComponent>(),
		RaccoonEcs::ComponentAdder<WorldCachedDataComponent>(),
		mWorldHolder,
		mInputData
	);

	mSystemsManager.registerSystem<MovementSystem>(
		RaccoonEcs::ComponentFilter<MovementComponent, TransformComponent>(),
		RaccoonEcs::ComponentFilter<SpatialTrackComponent>(),
		RaccoonEcs::ComponentFilter<TrackedSpatialEntitiesComponent>(),
		RaccoonEcs::EntityTransferer(),
		mWorldHolder,
		mTime
	);

	mSystemsManager.registerSystem<CharacterStateSystem>(
		RaccoonEcs::ComponentFilter<const StateMachineComponent>(),
		RaccoonEcs::ComponentFilter<CharacterStateComponent>(),
		RaccoonEcs::ComponentFilter<const CharacterStateComponent, MovementComponent>(),
		RaccoonEcs::ComponentFilter<const CharacterStateComponent, const MovementComponent, AnimationGroupsComponent>(),
		mWorldHolder,
		mTime
	);

	mSystemsManager.registerSystem<ResourceStreamingSystem>(
		RaccoonEcs::ComponentAdder<WorldCachedDataComponent>(),
		RaccoonEcs::ComponentRemover<SpriteCreatorComponent>(),
		RaccoonEcs::ComponentFilter<SpriteCreatorComponent>(),
		RaccoonEcs::ComponentAdder<RenderComponent>(),
		RaccoonEcs::ComponentAdder<AnimationClipsComponent>(),
		RaccoonEcs::ComponentRemover<AnimationClipCreatorComponent>(),
		RaccoonEcs::ComponentFilter<AnimationClipCreatorComponent>(),
		RaccoonEcs::ComponentAdder<AnimationGroupsComponent>(),
		RaccoonEcs::ComponentRemover<AnimationGroupCreatorComponent>(),
		RaccoonEcs::ComponentFilter<AnimationGroupCreatorComponent>(),
		RaccoonEcs::ScheduledActionsExecutor(),
		mWorldHolder,
		getResourceManager()
	);

	mSystemsManager.registerSystem<RenderSystem>(
		RaccoonEcs::ComponentFilter<const WorldCachedDataComponent>(),
		RaccoonEcs::ComponentFilter<const RenderModeComponent>(),
		RaccoonEcs::ComponentFilter<BackgroundTextureComponent>(),
		RaccoonEcs::ComponentFilter<const LightBlockingGeometryComponent>(),
		RaccoonEcs::ComponentFilter<const RenderComponent, const TransformComponent>(),
		RaccoonEcs::ComponentFilter<LightComponent, const TransformComponent>(),
		mWorldHolder,
		mTime,
		getEngine(),
		getResourceManager(),
		mWorkerManager
	);

	Vector2D playerPos{ZERO_VECTOR};
	AsyncEntityView playerEntity = mWorld.createTrackedSpatialEntity(
		RaccoonEcs::ComponentAdder<class TrackedSpatialEntitiesComponent>(),
		RaccoonEcs::ComponentAdder<class SpatialTrackComponent>(),
		RaccoonEcs::EntityAdder(),
		STR_TO_ID("ControlledEntity"),
		SpatialWorldData::GetCellForPos(playerPos)
	);

	{
		TransformComponent* transform = playerEntity.addComponent(RaccoonEcs::ComponentAdder<TransformComponent>());
		transform->setLocation(playerPos);
	}
	{
		SpriteCreatorComponent* sprite = playerEntity.addComponent(RaccoonEcs::ComponentAdder<SpriteCreatorComponent>());
		SpriteDescription spriteDesc;
		spriteDesc.params.size = Vector2D(30.0f, 30.0f);
		spriteDesc.path = "resources/textures/hero.png";
		sprite->getDescriptionsRef().emplace_back(std::move(spriteDesc));
	}
	{
		CollisionComponent* collision = playerEntity.addComponent(RaccoonEcs::ComponentAdder<CollisionComponent>());
		Hull& hull = collision->getGeometryRef();
		hull.type = HullType::Circular;
		hull.setRadius(15.0f);
	}
	playerEntity.addComponent(RaccoonEcs::ComponentAdder<MovementComponent>());

	Vector2D cameraPos{ZERO_VECTOR};
	AsyncEntityView camera = mWorld.createTrackedSpatialEntity(
		RaccoonEcs::ComponentAdder<class TrackedSpatialEntitiesComponent>(),
		RaccoonEcs::ComponentAdder<class SpatialTrackComponent>(),
		RaccoonEcs::EntityAdder(),
		STR_TO_ID("CameraEntity"), SpatialWorldData::GetCellForPos(cameraPos)
	);

	{
		TransformComponent* transform = camera.addComponent(RaccoonEcs::ComponentAdder<TransformComponent>());
		transform->setLocation(cameraPos);
	}
	camera.addComponent(RaccoonEcs::ComponentAdder<MovementComponent>());

	RaccoonEcs::ComponentAdder<StateMachineComponent>().addComponent(mGameData.getGameComponents());

	mInputData.windowSize = getEngine().getWindowSize();
	mInputData.mousePos = mInputData.windowSize * 0.5f;
}
