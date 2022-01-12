#include "Base/precomp.h"

#include "GameLogic/HapGame.h"

#include "Base/Types/TemplateHelpers.h"

#include "GameData/ComponentRegistration/ComponentFactoryRegistration.h"
#include "GameData/ComponentRegistration/ComponentJsonSerializerRegistration.h"

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

	ComponentsRegistration::RegisterComponents(getComponentFactory());
	ComponentsRegistration::RegisterJsonSerializers(getComponentSerializers());

	initSystems();

	const int workerThreadCount = 3;

	GameDataLoader::LoadWorld(getWorldHolder().getWorld(), arguments.getArgumentValue("world", "test"), getComponentSerializers());
	GameDataLoader::LoadGameData(getGameData(), arguments.getArgumentValue("gameData", "gameData"), getComponentSerializers());

	Game::start(arguments, workerThreadCount);
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
	getSystemsManager().registerSystem<ControlSystem>(getWorldHolder(), getInputData());
	getSystemsManager().registerSystem<AiSystem>(getWorldHolder(), getTime());
	getSystemsManager().registerSystem<WeaponSystem>(getWorldHolder(), getTime());
	getSystemsManager().registerSystem<DeadEntitiesDestructionSystem>(getWorldHolder());
	getSystemsManager().registerSystem<CollisionSystem>(getWorldHolder());
	getSystemsManager().registerSystem<CameraSystem>(getWorldHolder(), getInputData());
	getSystemsManager().registerSystem<MovementSystem>(getWorldHolder(), getTime());
	getSystemsManager().registerSystem<CharacterStateSystem>(getWorldHolder(), getTime());
	getSystemsManager().registerSystem<ResourceStreamingSystem>(getWorldHolder(), getResourceManager());
	getSystemsManager().registerSystem<AnimationSystem>(getWorldHolder(), getTime());
	getSystemsManager().registerSystem<RenderSystem>(getWorldHolder(), getTime(), getResourceManager(), getThreadPool());
	getSystemsManager().registerSystem<DebugDrawSystem>(getWorldHolder(), getTime(), getResourceManager());

#ifdef IMGUI_ENABLED
	getSystemsManager().registerSystem<ImguiSystem>(mImguiDebugData, getEngine());
#endif // IMGUI_ENABLED
}

void HapGame::initResources()
{
	SCOPED_PROFILER("Game::initResources");
	getResourceManager().loadAtlasesData("resources/atlas/atlas-list.json");
	getSystemsManager().initResources();
}
