#include "Base/precomp.h"

#include "GameLogic/HapGame.h"

#include <raccoon-ecs/async_operations.h>

#include "Base/Types/TemplateHelpers.h"

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

#ifdef ENABLE_SCOPED_PROFILER
#include "Utils/Profiling/ProfileDataWriter.h"
#endif // ENABLE_SCOPED_PROFILER

#include "GameLogic/Initialization/StateMachines.h"

HapGame::HapGame(int width, int height)
	: Game(width, height)
{
}

void HapGame::start(ArgumentsParser& arguments)
{
	SCOPED_PROFILER("HapGame::start");

	initSystems();

	const int workerThreadCount = 3;

	Game::start(
		arguments,
		workerThreadCount,
		[this, &arguments](const RaccoonEcs::InnerDataAccessor& dataAccessor)
		{
			GameDataLoader::LoadWorld(getWorldHolder().getWorld(), dataAccessor, arguments.getArgumentValue("world", "test"), getComponentSerializers());
			GameDataLoader::LoadGameData(getGameData(), arguments.getArgumentValue("gameData", "gameData"), getComponentSerializers());
		}
	);
}

void HapGame::setKeyboardKeyState(int key, bool isPressed)
{
	getInputData().keyboardKeyStates.updateState(key, isPressed);
}

void HapGame::setMouseKeyState(int key, bool isPressed)
{
	getInputData().mouseKeyStates.updateState(key, isPressed);
}

void HapGame::initSystems()
{
	SCOPED_PROFILER("Game::initSystems");
	getSystemsManager().registerSystem<ControlSystem,
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>,
		RaccoonEcs::ComponentFilter<CharacterStateComponent>,
		RaccoonEcs::ComponentFilter<ImguiComponent>,
		RaccoonEcs::ComponentFilter<RenderModeComponent>,
		RaccoonEcs::ComponentFilter<const TransformComponent, MovementComponent>,
		RaccoonEcs::ComponentFilter<const TransformComponent>>(
		RaccoonEcs::SystemDependencies(),
		getWorldHolder(),
		getInputData()
	);

	getSystemsManager().registerSystem<AiSystem,
		RaccoonEcs::ComponentAdder<NavMeshComponent>,
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent>,
		RaccoonEcs::ComponentFilter<const TransformComponent>,
		RaccoonEcs::ComponentFilter<AiControllerComponent, const TransformComponent, MovementComponent, CharacterStateComponent>,
		RaccoonEcs::ComponentFilter<DebugDrawComponent>,
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>,
		RaccoonEcs::ComponentFilter<const PathBlockingGeometryComponent>>(
		RaccoonEcs::SystemDependencies(),
		getWorldHolder(),
		getTime()
	);

	getSystemsManager().registerSystem<WeaponSystem,
		RaccoonEcs::ComponentFilter<WeaponComponent, CharacterStateComponent>,
		RaccoonEcs::ComponentFilter<const TransformComponent>,
		RaccoonEcs::ComponentFilter<HealthComponent>,
		RaccoonEcs::ComponentAdder<DeathComponent>,
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent>>(
		RaccoonEcs::SystemDependencies().goesAfter<ControlSystem, AiSystem>(),
		getWorldHolder(),
		getTime()
	);

	getSystemsManager().registerSystem<DeadEntitiesDestructionSystem,
		RaccoonEcs::ComponentFilter<const DeathComponent>,
		RaccoonEcs::EntityRemover>(
		RaccoonEcs::SystemDependencies().goesAfter<WeaponSystem>(),
		getWorldHolder()
	);

	getSystemsManager().registerSystem<CollisionSystem,
		RaccoonEcs::ComponentFilter<CollisionComponent, const TransformComponent>,
		RaccoonEcs::ComponentFilter<MovementComponent>,
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent, MovementComponent>>(
		RaccoonEcs::SystemDependencies().goesAfter<ControlSystem, AiSystem>(),
		getWorldHolder()
	);

	getSystemsManager().registerSystem<CameraSystem,
		RaccoonEcs::ComponentFilter<const TransformComponent, MovementComponent>,
		RaccoonEcs::ComponentFilter<const TrackedSpatialEntitiesComponent>,
		RaccoonEcs::ComponentFilter<const TransformComponent>,
		RaccoonEcs::ComponentFilter<const ImguiComponent>,
		RaccoonEcs::ComponentFilter<WorldCachedDataComponent>>(
		RaccoonEcs::SystemDependencies().goesAfter<ControlSystem>(),
		getWorldHolder(),
		getInputData()
	);

	getSystemsManager().registerSystem<MovementSystem,
		RaccoonEcs::ComponentFilter<MovementComponent, TransformComponent>,
		RaccoonEcs::ComponentFilter<SpatialTrackComponent>,
		RaccoonEcs::ComponentFilter<TrackedSpatialEntitiesComponent>,
		RaccoonEcs::EntityTransferer>(
		RaccoonEcs::SystemDependencies().goesAfter<CollisionSystem>(),
		getWorldHolder(),
		getTime()
	);

	getSystemsManager().registerSystem<CharacterStateSystem,
		RaccoonEcs::ComponentFilter<const StateMachineComponent>,
		RaccoonEcs::ComponentFilter<CharacterStateComponent>,
		RaccoonEcs::ComponentFilter<const CharacterStateComponent, MovementComponent>,
		RaccoonEcs::ComponentFilter<const CharacterStateComponent, const MovementComponent, AnimationGroupsComponent>>(
		RaccoonEcs::SystemDependencies().goesAfter<ControlSystem, AiSystem>(),
		getWorldHolder(),
		getTime()
	);

	getSystemsManager().registerSystem<ResourceStreamingSystem,
		RaccoonEcs::ComponentFilter<WorldCachedDataComponent>,
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
		RaccoonEcs::SystemDependencies().goesBefore<RenderSystem>(),
		getWorldHolder(),
		getResourceManager()
	);

	getSystemsManager().registerSystem<AnimationSystem,
		RaccoonEcs::ComponentFilter<AnimationGroupsComponent, AnimationClipsComponent>,
		RaccoonEcs::ComponentFilter<AnimationClipsComponent, SpriteRenderComponent>,
		RaccoonEcs::ComponentFilter<const StateMachineComponent>,
		RaccoonEcs::ComponentFilter<const WorldCachedDataComponent>>(
		RaccoonEcs::SystemDependencies().goesAfter<CharacterStateSystem>(),
		getWorldHolder(),
		getTime()
	);

	getSystemsManager().registerSystem<RenderSystem,
		RaccoonEcs::ComponentFilter<const WorldCachedDataComponent>,
		RaccoonEcs::ComponentFilter<const RenderModeComponent>,
		RaccoonEcs::ComponentFilter<BackgroundTextureComponent>, // hey, why isn't it const?
		RaccoonEcs::ComponentFilter<const LightBlockingGeometryComponent>,
		RaccoonEcs::ComponentFilter<const SpriteRenderComponent, const TransformComponent>,
		RaccoonEcs::ComponentFilter<LightComponent, const TransformComponent>,
		RaccoonEcs::ComponentFilter<RenderAccessorComponent>>(
		RaccoonEcs::SystemDependencies().goesAfter<AnimationSystem>(),
		getWorldHolder(),
		getTime(),
		getResourceManager(),
		getThreadPool()
	);

	getSystemsManager().registerSystem<DebugDrawSystem,
		RaccoonEcs::ComponentFilter<const WorldCachedDataComponent>,
		RaccoonEcs::ComponentFilter<const RenderModeComponent>,
		RaccoonEcs::ComponentFilter<const CollisionComponent, const TransformComponent>,
		RaccoonEcs::ComponentFilter<const NavMeshComponent>,
		RaccoonEcs::ComponentFilter<const AiControllerComponent>,
		RaccoonEcs::ComponentFilter<const DebugDrawComponent>,
		RaccoonEcs::ComponentFilter<const CharacterStateComponent, class TransformComponent>,
		RaccoonEcs::ComponentFilter<RenderAccessorComponent>>(
		RaccoonEcs::SystemDependencies().goesAfter<RenderSystem>(),
		getWorldHolder(),
		getTime(),
		getResourceManager()
	);

#ifdef IMGUI_ENABLED
	getSystemsManager().registerSystem<ImguiSystem,
		RaccoonEcs::ComponentAdder<ImguiComponent>,
		RaccoonEcs::ComponentFilter<RenderAccessorComponent>,
		RaccoonEcs::InnerDataAccessor>(
		RaccoonEcs::SystemDependencies().goesAfter<DebugDrawSystem>(),
		mImguiDebugData,
		getEngine()
	);
#endif // IMGUI_ENABLED
}

void HapGame::initResources()
{
	SCOPED_PROFILER("Game::initResources");
	getResourceManager().loadAtlasesData("resources/atlas/atlas-list.json");
	getSystemsManager().initResources();
}
