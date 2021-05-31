#include "Base/precomp.h"

#include "AutoTests/Tests/WeaponShooting/TestCase.h"

#include <memory>

#include "HAL/Base/Engine.h"

#include "GameData/Spatial/SpatialWorldData.h"

#include "GameData/Components/SpriteCreatorComponent.generated.h"
#include "GameData/Components/CollisionComponent.generated.h"
#include "GameData/Components/TransformComponent.generated.h"
#include "GameData/Components/StateMachineComponent.generated.h"
#include "GameData/Components/MovementComponent.generated.h"
#include "GameData/Components/WeaponComponent.generated.h"
#include "GameData/Components/CharacterStateComponent.generated.h"

#include "GameLogic/Systems/RenderSystem.h"
#include "GameLogic/Systems/CollisionSystem.h"
#include "GameLogic/Systems/ResourceStreamingSystem.h"
#include "GameLogic/Systems/MovementSystem.h"
#include "GameLogic/Systems/CharacterStateSystem.h"
#include "GameLogic/Systems/CameraSystem.h"
#include "GameLogic/Systems/WeaponSystem.h"
#include "GameLogic/Systems/DeadEntitiesDestructionSystem.h"

#include "GameLogic/Initialization/StateMachines.h"

#include "AutoTests/Tests/WeaponShooting/Systems/TestShootingControlSystem.h"
#include "AutoTests/Tests/WeaponShooting/Systems/TestSpawnShootableUnitsSystem.h"
#include "AutoTests/Tests/WeaponShooting/Systems/TestDestroyedEntitiesRegistrationSystem.h"

void WeaponShootingTestCase::initTestCase(const ArgumentsParser& /*arguments*/)
{
	getResourceManager().loadAtlasesData("resources/atlas/atlas-list.json");

	mTestChecklist.checks.emplace("destroyedEntities", std::make_unique<DestroyedEntitiesTestCheck>(100));
	DestroyedEntitiesTestCheck& destroyedEntitiesTestCheck = *static_cast<DestroyedEntitiesTestCheck*>(mTestChecklist.checks["destroyedEntities"].get());

	mSystemsManager.registerSystem<TestSpawnShootableUnitsSystem>(mWorldHolder);

	mSystemsManager.registerSystem<TestShootingControlSystem>(
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>(),
		mWorldHolder,
		mTime
	);

	mSystemsManager.registerSystem<WeaponSystem>(
		RaccoonEcs::ComponentFilter<WeaponComponent, CharacterStateComponent>(),
		RaccoonEcs::ComponentFilter<const TransformComponent>(),
		RaccoonEcs::ComponentFilter<HealthComponent>(),
		RaccoonEcs::ComponentAdder<DeathComponent>(),
		mWorldHolder,
		mTime
	);

	mSystemsManager.registerSystem<TestDestroyedEntitiesRegistrationSystem>(
		RaccoonEcs::ComponentFilter<const DeathComponent>(),
		mWorldHolder,
		destroyedEntitiesTestCheck
	);

	mSystemsManager.registerSystem<DeadEntitiesDestructionSystem>(
		RaccoonEcs::ComponentFilter<const DeathComponent>(),
		RaccoonEcs::EntityRemover(),
		mWorldHolder
	);

	mSystemsManager.registerSystem<CollisionSystem>(
		RaccoonEcs::ComponentFilter<CollisionComponent, const TransformComponent>(),
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
		mInputData);

	mSystemsManager.registerSystem<MovementSystem>(
		RaccoonEcs::ComponentFilter<MovementComponent, TransformComponent>(),
		RaccoonEcs::ComponentFilter<SpatialTrackComponent>(),
		RaccoonEcs::ComponentFilter<TrackedSpatialEntitiesComponent>(),
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
		mWorldHolder,
		getResourceManager()
	);

	mSystemsManager.registerSystem<RenderSystem>(
		RaccoonEcs::ComponentFilter<const WorldCachedDataComponent>(),
		RaccoonEcs::ComponentFilter<const RenderModeComponent>(),
		RaccoonEcs::ComponentFilter<BackgroundTextureComponent>(),
		RaccoonEcs::ComponentFilter<const LightBlockingGeometryComponent>(),
		RaccoonEcs::ComponentFilter<const RenderComponent, const TransformComponent>(),
		mWorldHolder,
		mTime,
		getEngine(),
		getResourceManager(),
		mWorkerManager
	);

	Vector2D playerPos{ZERO_VECTOR};
	EntityView playerEntity = mWorld.createTrackedSpatialEntity(
		RaccoonEcs::ComponentAdder<class TrackedSpatialEntitiesComponent>(),
		RaccoonEcs::ComponentAdder<class SpatialTrackComponent>(),
		RaccoonEcs::EntityAdder(),
		STR_TO_ID("ControlledEntity"), SpatialWorldData::GetCellForPos(playerPos)
	);

	{
		TransformComponent* transform = playerEntity.addComponent<TransformComponent>();
		transform->setLocation(playerPos);
	}
	{
		SpriteCreatorComponent* sprite = playerEntity.addComponent<SpriteCreatorComponent>();
		SpriteDescription spriteDesc;
		spriteDesc.params.size = Vector2D(30.0f, 30.0f);
		spriteDesc.path = "resources/textures/hero.png";
		sprite->getDescriptionsRef().emplace_back(std::move(spriteDesc));
	}
	{
		CollisionComponent* collision = playerEntity.addComponent<CollisionComponent>();
		Hull& hull = collision->getGeometryRef();
		hull.type = HullType::Circular;
		hull.setRadius(15.0f);
	}
	playerEntity.addComponent<MovementComponent>();
	{
		WeaponComponent* weapon = playerEntity.addComponent<WeaponComponent>();
		weapon->setShotDistance(1000.0f);
		weapon->setDamageValue(70.0f);
		weapon->setShotPeriod(0.0001f);
	}
	playerEntity.addComponent<CharacterStateComponent>();

	Vector2D cameraPos{ZERO_VECTOR};
	EntityView camera = mWorld.createTrackedSpatialEntity(
		RaccoonEcs::ComponentAdder<class TrackedSpatialEntitiesComponent>(),
		RaccoonEcs::ComponentAdder<class SpatialTrackComponent>(),
		RaccoonEcs::EntityAdder(),
		STR_TO_ID("CameraEntity"), SpatialWorldData::GetCellForPos(cameraPos)
	);

	{
		TransformComponent* transform = camera.addComponent<TransformComponent>();
		transform->setLocation(cameraPos);
	}
	camera.addComponent<MovementComponent>();

	{
		StateMachineComponent* stateMachine = mGameData.getGameComponents().addComponent<StateMachineComponent>();
		StateMachines::RegisterStateMachines(stateMachine);
	}

	mInputData.windowSize = getEngine().getWindowSize();
	mInputData.mousePos = mInputData.windowSize * 0.5f;

	mTicksToFinish = 300;
}
