#include "Base/precomp.h"

#include "GameLogic/Game/Game.h"

#include "Base/Types/TemplateHelpers.h"

#include "GameData/Components/StateMachineComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"
#include "GameData/Components/RenderAccessorComponent.generated.h"
#include "GameLogic/Render/RenderAccessor.h"

#include "Utils/Application/ArgumentsParser.h"
#include "Utils/Multithreading/ThreadPool.h"

#ifdef ENABLE_SCOPED_PROFILER
#include "Utils/Profiling/ProfileDataWriter.h"
#endif // ENABLE_SCOPED_PROFILER

#include "HAL/Base/Engine.h"

#include "GameLogic/Initialization/StateMachines.h"

Game::Game(HAL::Engine* engine, ResourceManager& resourceManager, ThreadPool& threadPool)
	: HAL::GameBase(engine, resourceManager)
	, mThreadPool(threadPool)
{
}

void Game::preStart(const ArgumentsParser& arguments)
{
	SCOPED_PROFILER("Game::start");
	mDebugBehavior.processArguments(arguments);

	auto* sm = mGameData.getGameComponents().getOrAddComponent<StateMachineComponent>();
	// ToDo: make an editor not to hardcode SM data
	StateMachines::RegisterStateMachines(sm);

	mWorld.getWorldComponents().addComponent<WorldCachedDataComponent>();
}

void Game::start()
{
	// start the main loop
	if (HAL::Engine* engine = getEngine())
	{
		engine->start(this);
	}
	else
	{
		ReportFatalError("Trying to start() game with no Engine");
	}
}

void Game::setKeyboardKeyState(int key, bool isPressed)
{
	mInputData.keyboardKeyStates.updateState(key, isPressed);
}

void Game::setMouseKeyState(int key, bool isPressed)
{
	mInputData.mouseKeyStates.updateState(key, isPressed);
}

void Game::dynamicTimePreFrameUpdate(float /*dt*/)
{
	SCOPED_PROFILER("Game::dynamicTimePreFrameUpdate");
#ifdef ENABLE_SCOPED_PROFILER
	mFrameBeginTime = std::chrono::steady_clock::now();
#endif // ENABLE_SCOPED_PROFILER

	RenderAccessorComponent* renderAccessorComponent = getGameData().getGameComponents().getOrAddComponent<RenderAccessorComponent>();
	if (RenderAccessor* renderAccessor = renderAccessorComponent->getAccessor())
	{
		std::unique_ptr<RenderData> renderCommands = std::make_unique<RenderData>();
		TemplateHelpers::EmplaceVariant<SwapBuffersCommand>(renderCommands->layers);
		renderAccessor->submitData(std::move(renderCommands));
	}

	if (HAL::Engine* engine = getEngine())
	{
		mInputData.windowSize = engine->getWindowSize();
		mInputData.mousePos = engine->getMousePos();
	}

	mDebugBehavior.preInnerUpdate(*this);

	mPreFrameSystemsManager.update();
}

void Game::fixedTimeUpdate(float dt)
{
	SCOPED_PROFILER("Game::fixedTimeUpdate");

	mTime.update(dt);
	mGameLogicSystemsManager.update();
}

void Game::dynamicTimePostFrameUpdate(float /*dt*/)
{
	SCOPED_PROFILER("Game::dynamicTimePostFrameUpdate");

	mPostFrameSystemsManager.update();

	// test code
	//mRenderThread.testRunMainThread(*mGameData.getGameComponents().getOrAddComponent<RenderAccessorComponent>()->getAccessor(), getResourceManager(), getEngine());

	mInputData.clearAfterFrame();

	mDebugBehavior.postInnerUpdate(*this);

#ifdef ENABLE_SCOPED_PROFILER
	std::chrono::time_point<std::chrono::steady_clock> frameEndTime = std::chrono::steady_clock::now();
	mFrameDurations.push_back(std::chrono::duration<double, std::micro>(frameEndTime - mFrameBeginTime).count());
#endif // ENABLE_SCOPED_PROFILER
}

void Game::initResources()
{
	SCOPED_PROFILER("Game::initResources");
	mPreFrameSystemsManager.initResources();
	mGameLogicSystemsManager.initResources();
	mPostFrameSystemsManager.initResources();
}

void Game::onGameShutdown()
{
	ProfileDataWriter::PrintFrameDurationStatsToFile(mFrameDurationsOutputPath, mFrameDurations);

	mPreFrameSystemsManager.shutdown();
	mGameLogicSystemsManager.shutdown();
	mPostFrameSystemsManager.shutdown();
}
