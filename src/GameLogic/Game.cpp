#include "Base/precomp.h"

#include "GameLogic/Game.h"

#include "GameData/Components/StateMachineComponent.generated.h"

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

#ifdef IMGUI_ENABLED
#include "GameLogic/Systems/ImguiSystem.h"
#endif // IMGUI_ENABLED

#include "GameLogic/ComponentsRegistration.h"
#include "GameLogic/Initialization/StateMachines.h"

void Game::start(ArgumentsParser& arguments)
{
	initSystems();

	ComponentsRegistration::RegisterComponents(mComponentFactory);

	GameDataLoader::LoadWorld(mWorld, arguments.getArgumentValue("world", "test"), mComponentFactory);
	GameDataLoader::LoadGameData(mGameData, arguments.getArgumentValue("gameData", "gameData"), mComponentFactory);

	// ToDo: make an editor not to hardcode SM data
	auto [sm] = mGameData.getGameComponents().getComponents<StateMachineComponent>();
	StateMachines::RegisterStateMachines(sm);

	mProfileSystems = arguments.hasArgument("profile-systems");
	mSystemProfileOutputPath = arguments.getArgumentValue("profile-systems", mSystemProfileOutputPath);
	mSystemFrameRecords.setRecordsLimit(mProfileSystems ? 0u : 100u);

#ifdef IMGUI_ENABLED
	mImguiDebugData.systemNames = mSystemsManager.getSystemNames();
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
	mInputData.clearAfterFrame();

	mSystemFrameRecords.addFrame(mSystemsManager.getPreviousFrameTimeData());
}

void Game::initSystems()
{
	mSystemsManager.registerSystem<ControlSystem>(mWorldHolder, mInputData);
	mSystemsManager.registerSystem<AiSystem>(mWorldHolder, mTime);
	mSystemsManager.registerSystem<WeaponSystem>(mWorldHolder, mTime);
	mSystemsManager.registerSystem<CollisionSystem>(mWorldHolder);
	mSystemsManager.registerSystem<CameraSystem>(mWorldHolder, mInputData);
	mSystemsManager.registerSystem<MovementSystem>(mWorldHolder, mTime);
	mSystemsManager.registerSystem<CharacterStateSystem>(mWorldHolder, mTime);
	mSystemsManager.registerSystem<ResourceStreamingSystem>(mWorldHolder, getResourceManager());
	mSystemsManager.registerSystem<AnimationSystem>(mWorldHolder, mTime);
	mSystemsManager.registerSystem<RenderSystem>(mWorldHolder, mTime, getEngine(), getResourceManager(), mJobsWorkerManager);
	mSystemsManager.registerSystem<DebugDrawSystem>(mWorldHolder, mTime, getEngine(), getResourceManager());
#ifdef IMGUI_ENABLED
	mSystemsManager.registerSystem<ImguiSystem>(mImguiDebugData, getEngine());
#endif // IMGUI_ENABLED
}

void Game::initResources()
{
	getResourceManager().loadAtlasesData("resources/atlas/atlas-list.json");
	mSystemsManager.initResources();
}

void Game::onGameShutdown()
{
	if (mProfileSystems)
	{
		mSystemFrameRecords.printToFile(mSystemsManager.getSystemNames(), mSystemProfileOutputPath);
	}
	mSystemsManager.shutdown();
}
