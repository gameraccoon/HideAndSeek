#include "Base/precomp.h"

#include "GameLogic/Game/Game.h"

#include "Base/Types/TemplateHelpers.h"

#include "GameData/Components/RenderAccessorComponent.generated.h"
#include "GameData/Components/StateMachineComponent.generated.h"
#include "GameData/Components/TimeComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"

#include "Utils/Application/ArgumentsParser.h"
#include "Utils/Multithreading/ThreadPool.h"
#include "Utils/Profiling/ProfileDataWriter.h"
#include "Utils/ResourceManagement/ResourceManager.h"

#include "HAL/Base/Engine.h"

#include "GameLogic/Initialization/StateMachines.h"
#include "GameLogic/Render/RenderAccessor.h"

Game::Game(HAL::Engine* engine, ResourceManager& resourceManager, ThreadPool& threadPool)
	: HAL::GameBase(engine, resourceManager)
	, mThreadPool(threadPool)
	, mDebugBehavior(0)
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
	mWorld.getWorldComponents().addComponent<TimeComponent>();
}

void Game::notPausablePreFrameUpdate(float dt)
{
	SCOPED_PROFILER("Game::notPausablePreFrameUpdate");

#ifdef ENABLE_SCOPED_PROFILER
	mFrameBeginTime = std::chrono::steady_clock::now();
#endif // ENABLE_SCOPED_PROFILER

	if (HAL::Engine* engine = getEngine())
	{
		mWorld.getWorldComponents().getOrAddComponent<WorldCachedDataComponent>()->setScreenSize(engine->getWindowSize());
	}

	auto [time] = mWorld.getWorldComponents().getComponents<TimeComponent>();
	time->getValueRef().lastUpdateDt = dt;

	mNotPausablePreFrameSystemsManager.update();
}

void Game::dynamicTimePreFrameUpdate(float dt, int plannedFixedTimeUpdates)
{
	SCOPED_PROFILER("Game::dynamicTimePreFrameUpdate");

	auto [time] = mWorld.getWorldComponents().getComponents<TimeComponent>();
	time->getValueRef().lastUpdateDt = dt;
	time->getValueRef().countFixedTimeUpdatesThisFrame = plannedFixedTimeUpdates;

	mDebugBehavior.preInnerUpdate(*this);

	mPreFrameSystemsManager.update();
}

void Game::fixedTimeUpdate(float dt)
{
	SCOPED_PROFILER("Game::fixedTimeUpdate");

	auto [time] = mWorld.getWorldComponents().getComponents<TimeComponent>();
	time->getValueRef().fixedUpdate(dt);
	mGameLogicSystemsManager.update();
	mInputControllersData.resetLastFrameStates();
}

void Game::dynamicTimePostFrameUpdate(float dt, int processedFixedTimeUpdates)
{
	SCOPED_PROFILER("Game::dynamicTimePostFrameUpdate");

	auto [time] = mWorld.getWorldComponents().getComponents<TimeComponent>();
	time->getValueRef().lastUpdateDt = dt;
	time->getValueRef().countFixedTimeUpdatesThisFrame = processedFixedTimeUpdates;

	mPostFrameSystemsManager.update();

	mDebugBehavior.postInnerUpdate(*this);
}

void Game::notPausablePostFrameUpdate(float dt) {
	SCOPED_PROFILER("Game::notPausablePostFrameUpdate");

	auto [time] = mWorld.getWorldComponents().getComponents<TimeComponent>();
	time->getValueRef().lastUpdateDt = dt;

	mNotPausablePostFrameSystemsManager.update();

	// this additional reset is needed for debug input in case we are paused
	mInputControllersData.resetLastFrameStates();

	// test code
	//mRenderThread.testRunMainThread(*mGameData.getGameComponents().getOrAddComponent<RenderAccessorComponent>()->getAccessor(), getResourceManager(), getEngine());

	{
		auto [renderAccessorComponent] = getGameData().getGameComponents().getComponents<RenderAccessorComponent>();
		if (renderAccessorComponent != nullptr && renderAccessorComponent->getAccessor().has_value())
		{
			std::unique_ptr<RenderData> renderCommands = std::make_unique<RenderData>();
			TemplateHelpers::EmplaceVariant<FinalizeFrameCommand>(renderCommands->layers);
			renderAccessorComponent->getAccessorRef()->submitData(std::move(renderCommands));
		}
	}

#ifdef ENABLE_SCOPED_PROFILER
	std::chrono::time_point<std::chrono::steady_clock> frameEndTime = std::chrono::steady_clock::now();
	mFrameDurations.push_back(std::chrono::duration<double, std::micro>(frameEndTime - mFrameBeginTime).count());
#endif // ENABLE_SCOPED_PROFILER
}

void Game::initResources()
{
	SCOPED_PROFILER("Game::initResources");
	mNotPausablePreFrameSystemsManager.initResources();
	mPreFrameSystemsManager.initResources();
	mGameLogicSystemsManager.initResources();
	mPostFrameSystemsManager.initResources();
	mNotPausablePostFrameSystemsManager.initResources();
}

void Game::onGameShutdown()
{
#ifdef ENABLE_SCOPED_PROFILER
	ProfileDataWriter::PrintFrameDurationStatsToFile(mFrameDurationsOutputPath, mFrameDurations);
#endif // ENABLE_SCOPED_PROFILER

	mNotPausablePreFrameSystemsManager.shutdown();
	mPreFrameSystemsManager.shutdown();
	mGameLogicSystemsManager.shutdown();
	mPostFrameSystemsManager.shutdown();
	mNotPausablePostFrameSystemsManager.shutdown();
}
