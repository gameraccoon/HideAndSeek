#include "Base/precomp.h"

#include "GameLogic/Game.h"

#include <raccoon-ecs/async_operations.h>

#include "GameData/ComponentRegistration/ComponentFactoryRegistration.h"
#include "GameData/ComponentRegistration/ComponentJsonSerializerRegistration.h"

#include "GameData/Components/StateMachineComponent.generated.h"
#include "GameData/Components/RenderAccessorComponent.generated.h"

#include "HAL/Base/Engine.h"

#include "Utils/World/GameDataLoader.h"

#include "GameLogic/Systems/RenderSystem.h"
#include "GameLogic/Systems/ControlSystem.h"
#include "GameLogic/Systems/CollisionSystem.h"
#include "GameLogic/Systems/ResourceStreamingSystem.h"
#include "GameLogic/Systems/AiSystem.h"
#include "GameLogic/Systems/CharacterStateSystem.h"
#include "GameLogic/Systems/DebugDrawSystem.h"
#include "GameLogic/Systems/MovementSystem.h"
#include "GameLogic/Systems/AnimationSystem.h"
#include "GameLogic/Systems/CameraSystem.h"
#include "GameLogic/Systems/WeaponSystem.h"
#include "GameLogic/Systems/DeadEntitiesDestructionSystem.h"

#ifdef IMGUI_ENABLED
#include "GameLogic/Systems/ImguiSystem.h"
#endif // IMGUI_ENABLED

#include "GameLogic/Initialization/StateMachines.h"

Game::Game(int width, int height)
	: HAL::GameBase(width, height)
{
}

void Game::start(ArgumentsParser& arguments)
{
	ComponentsRegistration::RegisterComponents(mComponentFactory);
	ComponentsRegistration::RegisterJsonSerializers(mComponentSerializers);

	initSystems();

	mSystemsManager.init(
		//3, // threads count available for systems manager
		[this, &arguments](const RaccoonEcs::InnerDataAccessor& dataAccessor)
		{
			GameDataLoader::LoadWorld(mWorld, dataAccessor, arguments.getArgumentValue("world", "test"), mComponentSerializers);
			GameDataLoader::LoadGameData(mGameData, arguments.getArgumentValue("gameData", "gameData"), mComponentSerializers);

			auto [sm] = mGameData.getGameComponents().getComponents<StateMachineComponent>();
			// ToDo: make an editor not to hardcode SM data
			StateMachines::RegisterStateMachines(sm);

			RenderAccessorComponent* renderAccessor = mGameData.getGameComponents().getOrAddComponent<RenderAccessorComponent>();
			renderAccessor->setAccessor(&mRenderThread.getAccessor());
		}
	);

	//getEngine().releaseRenderContext();
	mRenderThread.startThread(getResourceManager(), [&engine = getEngine()]{ engine.acquireRenderContext(); });

#ifdef PROFILE_SYSTEMS
	mProfileSystems = arguments.hasArgument("profile-systems");
	mSystemProfileOutputPath = arguments.getArgumentValue("profile-systems", mSystemProfileOutputPath);
	mSystemFrameRecords.setRecordsLimit(mProfileSystems ? 0u : 100u);
#endif // PROFILE_SYSTEMS

#ifdef IMGUI_ENABLED
	//mImguiDebugData.systemNames = mSystemsManager.getSystemNames();
#endif // IMGUI_ENABLED

	// start the main loop
	getEngine().start(this);

	onGameShutdown();
}

void Game::setKeyboardKeyState(int key, bool isPressed)
{
	mInputData.keyboardKeyStates.updateState(key, isPressed);
}

void Game::setMouseKeyState(int key, bool isPressed)
{
	mInputData.mouseKeyStates.updateState(key, isPressed);
}

void Game::update(float dt)
{
	mInputData.windowSize = getEngine().getWindowSize();
	mInputData.mousePos = getEngine().getMousePos();

	mTime.update(dt);
	mSystemsManager.update();

	// test code
	mRenderThread.testRunMainThread(*mGameData.getGameComponents().getOrAddComponent<RenderAccessorComponent>()->getAccessor(), getResourceManager());

	mInputData.clearAfterFrame();

#ifdef PROFILE_SYSTEMS
	mSystemFrameRecords.addFrame(mSystemsManager.getPreviousFrameTimeData());
#endif
}

void Game::initSystems()
{
	mSystemsManager.registerSystem<ControlSystem,
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>,
		RaccoonEcs::ComponentFilter<CharacterStateComponent>,
		RaccoonEcs::ComponentFilter<ImguiComponent>,
		RaccoonEcs::ComponentFilter<RenderModeComponent>,
		RaccoonEcs::ComponentFilter<const TransformComponent, MovementComponent>,
		RaccoonEcs::ComponentFilter<const TransformComponent>>(
		RaccoonEcs::SystemDependencies(),
		mWorldHolder,
		mInputData
	);

	mSystemsManager.registerSystem<AiSystem,
		RaccoonEcs::ComponentAdder<NavMeshComponent>,
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent>,
		RaccoonEcs::ComponentFilter<const TransformComponent>,
		RaccoonEcs::ComponentFilter<AiControllerComponent, const TransformComponent, MovementComponent, CharacterStateComponent>,
		RaccoonEcs::ComponentFilter<DebugDrawComponent>,
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>,
		RaccoonEcs::ComponentFilter<const PathBlockingGeometryComponent>>(
		RaccoonEcs::SystemDependencies(),
		mWorldHolder,
		mTime
	);

	mSystemsManager.registerSystem<WeaponSystem,
		RaccoonEcs::ComponentFilter<WeaponComponent, CharacterStateComponent>,
		RaccoonEcs::ComponentFilter<const TransformComponent>,
		RaccoonEcs::ComponentFilter<HealthComponent>,
		RaccoonEcs::ComponentAdder<DeathComponent>,
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent>>(
		RaccoonEcs::SystemDependencies().dependsOn<ControlSystem>().dependsOn<AiSystem>(),
		mWorldHolder,
		mTime
	);

	mSystemsManager.registerSystem<DeadEntitiesDestructionSystem,
		RaccoonEcs::ComponentFilter<const DeathComponent>,
		RaccoonEcs::EntityRemover>(
		RaccoonEcs::SystemDependencies().dependsOn<WeaponSystem>(),
		mWorldHolder
	);

	mSystemsManager.registerSystem<CollisionSystem,
		RaccoonEcs::ComponentFilter<CollisionComponent, const TransformComponent>,
		RaccoonEcs::ComponentFilter<MovementComponent>,
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent, MovementComponent>>(
		RaccoonEcs::SystemDependencies().dependsOn<ControlSystem>().dependsOn<AiSystem>(),
		mWorldHolder
	);

	mSystemsManager.registerSystem<CameraSystem,
		RaccoonEcs::ComponentFilter<const TransformComponent, MovementComponent>,
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>,
		RaccoonEcs::ComponentFilter<const TransformComponent>,
		RaccoonEcs::ComponentFilter<const ImguiComponent>,
		RaccoonEcs::ComponentAdder<WorldCachedDataComponent>>(
		RaccoonEcs::SystemDependencies().dependsOn<ControlSystem>(),
		mWorldHolder,
		mInputData
	);

	mSystemsManager.registerSystem<MovementSystem,
		RaccoonEcs::ComponentFilter<MovementComponent, TransformComponent>,
		RaccoonEcs::ComponentFilter<SpatialTrackComponent>,
		RaccoonEcs::ComponentFilter<TrackedSpatialEntitiesComponent>,
		RaccoonEcs::EntityTransferer>(
		RaccoonEcs::SystemDependencies().dependsOn<CollisionSystem>(),
		mWorldHolder,
		mTime
	);

	mSystemsManager.registerSystem<CharacterStateSystem,
		RaccoonEcs::ComponentFilter<const StateMachineComponent>,
		RaccoonEcs::ComponentFilter<CharacterStateComponent>,
		RaccoonEcs::ComponentFilter<const CharacterStateComponent, MovementComponent>,
		RaccoonEcs::ComponentFilter<const CharacterStateComponent, const MovementComponent, AnimationGroupsComponent>>(
		RaccoonEcs::SystemDependencies().dependsOn<ControlSystem>().dependsOn<AiSystem>(),
		mWorldHolder,
		mTime
	);

	mSystemsManager.registerSystem<ResourceStreamingSystem,
		RaccoonEcs::ComponentAdder<WorldCachedDataComponent>,
		RaccoonEcs::ComponentRemover<SpriteCreatorComponent>,
		RaccoonEcs::ComponentFilter<SpriteCreatorComponent>,
		RaccoonEcs::ComponentAdder<SpriteRenderComponent>,
		RaccoonEcs::ComponentAdder<AnimationClipsComponent>,
		RaccoonEcs::ComponentRemover<AnimationClipCreatorComponent>,
		RaccoonEcs::ComponentFilter<AnimationClipCreatorComponent>,
		RaccoonEcs::ComponentAdder<AnimationGroupsComponent>,
		RaccoonEcs::ComponentRemover<AnimationGroupCreatorComponent>,
		RaccoonEcs::ComponentFilter<AnimationGroupCreatorComponent>,
		RaccoonEcs::ScheduledActionsExecutor>(
		RaccoonEcs::SystemDependencies(),
		mWorldHolder,
		getResourceManager()
	);

	mSystemsManager.registerSystem<AnimationSystem,
		RaccoonEcs::ComponentFilter<AnimationGroupsComponent, AnimationClipsComponent>,
		RaccoonEcs::ComponentFilter<AnimationClipsComponent, SpriteRenderComponent>,
		RaccoonEcs::ComponentFilter<const StateMachineComponent>,
		RaccoonEcs::ComponentFilter<const WorldCachedDataComponent>>(
		RaccoonEcs::SystemDependencies().dependsOn<CharacterStateSystem>(),
		mWorldHolder,
		mTime
	);

	mSystemsManager.registerSystem<RenderSystem,
		RaccoonEcs::ComponentFilter<const WorldCachedDataComponent>,
		RaccoonEcs::ComponentFilter<const RenderModeComponent>,
		RaccoonEcs::ComponentFilter<BackgroundTextureComponent>, // hey, why isn't it const?
		RaccoonEcs::ComponentFilter<const LightBlockingGeometryComponent>,
		RaccoonEcs::ComponentFilter<const SpriteRenderComponent, const TransformComponent>,
		RaccoonEcs::ComponentFilter<LightComponent, const TransformComponent>,
		RaccoonEcs::ComponentFilter<RenderAccessorComponent>>(
		RaccoonEcs::SystemDependencies().dependsOn<AnimationSystem>(),
		mWorldHolder,
		mTime,
		getResourceManager(),
		mJobsWorkerManager
	);

	mSystemsManager.registerSystem<DebugDrawSystem,
		RaccoonEcs::ComponentFilter<const WorldCachedDataComponent>,
		RaccoonEcs::ComponentFilter<const RenderModeComponent>,
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent>,
		RaccoonEcs::ComponentFilter<const NavMeshComponent>,
		RaccoonEcs::ComponentFilter<const AiControllerComponent>,
		RaccoonEcs::ComponentFilter<const DebugDrawComponent>,
		RaccoonEcs::ComponentFilter<const CharacterStateComponent, class TransformComponent>,
		RaccoonEcs::ComponentFilter<RenderAccessorComponent>>(
		RaccoonEcs::SystemDependencies(),
		mWorldHolder,
		mTime,
		getResourceManager()
	);

#ifdef IMGUI_ENABLED
	mSystemsManager.registerSystem<ImguiSystem,
		RaccoonEcs::ComponentAdder<ImguiComponent>,
		RaccoonEcs::ComponentFilter<RenderAccessorComponent>,
		RaccoonEcs::InnerDataAccessor>(
		RaccoonEcs::SystemDependencies(),
		mImguiDebugData,
		getEngine()
	);
#endif // IMGUI_ENABLED
}

void Game::initResources()
{
	getResourceManager().loadAtlasesData("resources/atlas/atlas-list.json");
	mSystemsManager.initResources();
}

void Game::onGameShutdown()
{
#ifdef PROFILE_SYSTEMS
	if (mProfileSystems)
	{
		mSystemFrameRecords.printToFile(mSystemsManager.getSystemNames(), mSystemProfileOutputPath);
	}
#endif // PROFILE_SYSTEMS
	mSystemsManager.shutdown();
}
