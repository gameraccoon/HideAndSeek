#include "Base/precomp.h"

#include "GameLogic/Game/HapGame.h"

#include "Base/Types/TemplateHelpers.h"

#include "GameData/ComponentRegistration/ComponentFactoryRegistration.h"
#include "GameData/ComponentRegistration/ComponentJsonSerializerRegistration.h"

#include "GameData/ComponentRegistration/ComponentFactoryRegistration.h"
#include "GameData/ComponentRegistration/ComponentJsonSerializerRegistration.h"
#include "GameData/Components/StateMachineComponent.generated.h"
#include "GameData/Components/RenderAccessorComponent.generated.h"

#include "HAL/Base/Engine.h"

#include "Utils/World/GameDataLoader.h"
#include "Utils/Application/ArgumentsParser.h"

#include "GameLogic/Systems/AiSystem.h"
#include "GameLogic/Systems/AnimationSystem.h"
#include "GameLogic/Systems/CameraSystem.h"
#include "GameLogic/Systems/CharacterStateSystem.h"
#include "GameLogic/Systems/CollisionSystem.h"
#include "GameLogic/Systems/ControlSystem.h"
#include "GameLogic/Systems/DeadEntitiesDestructionSystem.h"
#include "GameLogic/Systems/DebugDrawSystem.h"
#include "GameLogic/Systems/InputSystem.h"
#include "GameLogic/Systems/MovementSystem.h"
#include "GameLogic/Systems/RenderSystem.h"
#include "GameLogic/Systems/ResourceStreamingSystem.h"
#include "GameLogic/Systems/WeaponSystem.h"

#ifdef IMGUI_ENABLED
#include "GameLogic/Systems/ImguiSystem.h"
#endif // IMGUI_ENABLED

#include "GameLogic/Initialization/StateMachines.h"

void HapGame::preStart(ArgumentsParser& arguments, const RenderAccessorGameRef& renderAccessor)
{
	SCOPED_PROFILER("HapGame::start");

	ComponentsRegistration::RegisterComponents(getComponentFactory());
	ComponentsRegistration::RegisterJsonSerializers(getComponentSerializers());

	initSystems();

	GameDataLoader::LoadWorld(getWorldHolder().getWorld(), arguments.getArgumentValue("world", "test"), getComponentSerializers());
	GameDataLoader::LoadGameData(getGameData(), arguments.getArgumentValue("gameData", "gameData"), getComponentSerializers());

	RenderAccessorComponent* renderAccessorComponent = getGameData().getGameComponents().getOrAddComponent<RenderAccessorComponent>();
	renderAccessorComponent->setAccessor(renderAccessor);

	Game::preStart(arguments);
}

void HapGame::initSystems()
{
	SCOPED_PROFILER("HapGame::initSystems");

	AssertFatal(getEngine(), "HapGame was created without Engine, we are going to crash");

	getPreFrameSystemsManager().registerSystem<InputSystem>(getWorldHolder(), getInputData());
	getGameLogicSystemsManager().registerSystem<ControlSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<AiSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<WeaponSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<DeadEntitiesDestructionSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<CollisionSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<CameraSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<MovementSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<CharacterStateSystem>(getWorldHolder());
	getGameLogicSystemsManager().registerSystem<AnimationSystem>(getWorldHolder());
	getPostFrameSystemsManager().registerSystem<ResourceStreamingSystem>(getWorldHolder(), getResourceManager());
	getPostFrameSystemsManager().registerSystem<RenderSystem>(getWorldHolder(), getResourceManager(), getThreadPool());
	getPostFrameSystemsManager().registerSystem<DebugDrawSystem>(getWorldHolder(), getResourceManager());

#ifdef IMGUI_ENABLED
	getPostFrameSystemsManager().registerSystem<ImguiSystem>(mImguiDebugData, *getEngine());
#endif // IMGUI_ENABLED
}

void HapGame::initResources()
{
	SCOPED_PROFILER("HapGame::initResources");
	getResourceManager().loadAtlasesData("resources/atlas/atlas-list.json");
	Game::initResources();
}
