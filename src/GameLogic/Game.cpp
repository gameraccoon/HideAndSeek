#include "Base/precomp.h"

#include "GameLogic/Game.h"

#include "Base/Types/TemplateHelpers.h"

#include "GameData/Components/StateMachineComponent.generated.h"
#include "GameData/Components/RenderAccessorComponent.generated.h"
#include "GameData/Components/WorldCachedDataComponent.generated.h"

#include "HAL/Base/Engine.h"

#include "Utils/World/GameDataLoader.h"

#ifdef IMGUI_ENABLED
#include "GameLogic/Systems/ImguiSystem.h"
#endif // IMGUI_ENABLED

#ifdef ENABLE_SCOPED_PROFILER
#include "Utils/Profiling/ProfileDataWriter.h"
#endif // ENABLE_SCOPED_PROFILER

#include "GameLogic/Initialization/StateMachines.h"

Game::Game(int width, int height)
	: HAL::GameBase(width, height)
	, mThreadPool(0, [this]{ workingThreadSaveProfileData(); })
{
}

void Game::start([[maybe_unused]] const ArgumentsParser& arguments, int workerThreadsCount)
{
	SCOPED_PROFILER("Game::start");
	mDebugBehavior.processArguments(arguments);

	mThreadPool.spawnThreads(workerThreadsCount);

	mWorkerThreadsCount = workerThreadsCount;
	mRenderThreadId = mWorkerThreadsCount + 1;

	auto* sm = mGameData.getGameComponents().getOrAddComponent<StateMachineComponent>();
	// ToDo: make an editor not to hardcode SM data
	StateMachines::RegisterStateMachines(sm);

	mWorld.getWorldComponents().addComponent<WorldCachedDataComponent>();

	RenderAccessorComponent* renderAccessor = mGameData.getGameComponents().getOrAddComponent<RenderAccessorComponent>();
	renderAccessor->setAccessor(&mRenderThread.getAccessor());

	getEngine().releaseRenderContext();
	mRenderThread.startThread(getResourceManager(), getEngine(), [&engine = getEngine()]{ engine.acquireRenderContext(); });

#ifdef ENABLE_SCOPED_PROFILER
	mScopedProfileOutputPath = arguments.getArgumentValue("profile-systems", mScopedProfileOutputPath);
#endif // ENABLE_SCOPED_PROFILER

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

void Game::dynamicTimePreFrameUpdate(float /*dt*/)
{
	SCOPED_PROFILER("Game::dynamicTimePreFrameUpdate");
#ifdef ENABLE_SCOPED_PROFILER
	mFrameBeginTime = std::chrono::system_clock::now();
#endif // ENABLE_SCOPED_PROFILER

	std::unique_ptr<RenderData> renderCommands = std::make_unique<RenderData>();
	TemplateHelpers::EmplaceVariant<SwapBuffersCommand>(renderCommands->layers);
	mRenderThread.getAccessor().submitData(std::move(renderCommands));

	mInputData.windowSize = getEngine().getWindowSize();
	mInputData.mousePos = getEngine().getMousePos();

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
	std::chrono::time_point<std::chrono::system_clock> frameEndTime = std::chrono::system_clock::now();
	mFrameDurations.push_back(std::chrono::duration<double, std::micro>(frameEndTime - mFrameBeginTime).count());
#endif // ENABLE_SCOPED_PROFILER
}

void Game::initResources()
{
	SCOPED_PROFILER("Game::initResources");
	getResourceManager().loadAtlasesData("resources/atlas/atlas-list.json");
	mPreFrameSystemsManager.initResources();
	mGameLogicSystemsManager.initResources();
	mPostFrameSystemsManager.initResources();
}

void Game::onGameShutdown()
{
	// run this before dumping profile information to avoid data races
	mRenderThread.shutdownThread();
	mThreadPool.shutdown();

#ifdef ENABLE_SCOPED_PROFILER
	{
		ProfileDataWriter::ProfileData data;
		{
			data.scopedProfilerDatas.emplace_back();
			ProfileDataWriter::ScopedProfilerData& renderScopedProfilerData = data.scopedProfilerDatas.back();
			renderScopedProfilerData.records = mRenderThread.getAccessor().consumeScopedProfilerRecordsUnsafe();
			renderScopedProfilerData.threadId = mRenderThreadId;
		}
		{
			data.scopedProfilerDatas.emplace_back();
			ProfileDataWriter::ScopedProfilerData& mainScopedProfilerData = data.scopedProfilerDatas.back();
			mainScopedProfilerData.records = gtlScopedProfilerData.getAllRecords();
			mainScopedProfilerData.threadId = MainThreadId;
		}

		for (auto&& [threadId, records] : mScopedProfileRecords)
		{
			data.scopedProfilerDatas.emplace_back(threadId, std::move(records));
		}

		data.threadNames.resize(mRenderThreadId + 1);
		data.threadNames[MainThreadId] = "Main Thread";
		data.threadNames[mRenderThreadId] = "Render Thread";
		for (int i = 0; i < mWorkerThreadsCount; ++i)
		{
			// zero is reserved for main thread
			data.threadNames[1 + i] = std::string("Worker Thread #") + std::to_string(i+1);
		}

		ProfileDataWriter::PrintScopedProfileToFile(mScopedProfileOutputPath, data);

		ProfileDataWriter::PrintFrameDurationStatsToFile(mFrameDurationsOutputPath, mFrameDurations);
	}
#endif // ENABLE_SCOPED_PROFILER
	mPreFrameSystemsManager.shutdown();
	mGameLogicSystemsManager.shutdown();
	mPostFrameSystemsManager.shutdown();
}

void Game::workingThreadSaveProfileData()
{
#ifdef ENABLE_SCOPED_PROFILER
	std::lock_guard l(mScopedProfileRecordsMutex);
	mScopedProfileRecords.emplace_back(RaccoonEcs::ThreadPool::GetThisThreadId(), gtlScopedProfilerData.getAllRecords());
#endif // ENABLE_SCOPED_PROFILER
}
