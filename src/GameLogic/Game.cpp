#include "Base/precomp.h"

#include "GameLogic/Game.h"

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

Game::Game(int width, int height)
	: HAL::GameBase(width, height)
	, mThreadPool(0, [this]{ workingThreadSaveProfileData(); })
{
}

void Game::start([[maybe_unused]] const ArgumentsParser& arguments, int workerThreadsCount, SystemsInitFunction&& initFn)
{
	SCOPED_PROFILER("Game::start");
	ComponentsRegistration::RegisterComponents(mComponentFactory);
	ComponentsRegistration::RegisterJsonSerializers(mComponentSerializers);

	mThreadPool.spawnThreads(workerThreadsCount);

	mWorkerThreadsCount = workerThreadsCount;
	mRenderThreadId = mWorkerThreadsCount + 1;

	mSystemsManager.init(
		0, // don't spawn additional threads since we already preallocated some
		[this, initFn](const RaccoonEcs::InnerDataAccessor& dataAccessor)
		{
			if (initFn)
			{
				initFn(dataAccessor);
			}

			auto* sm = mGameData.getGameComponents().getOrAddComponent<StateMachineComponent>();
			// ToDo: make an editor not to hardcode SM data
			StateMachines::RegisterStateMachines(sm);

			mWorld.getWorldComponents().addComponent<WorldCachedDataComponent>();

			RenderAccessorComponent* renderAccessor = mGameData.getGameComponents().getOrAddComponent<RenderAccessorComponent>();
			renderAccessor->setAccessor(&mRenderThread.getAccessor());
		}
	);

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

void Game::update(float dt)
{
	SCOPED_PROFILER("Game::update");
#ifdef SCOPED_PROFILER
	std::chrono::time_point<std::chrono::system_clock> frameBeginTime = std::chrono::system_clock::now();
#endif

	preInnderUpdate();
	innerUpdate(dt);
	postInnerUpdate();

#ifdef SCOPED_PROFILER
	std::chrono::time_point<std::chrono::system_clock> frameEndTime = std::chrono::system_clock::now();
	mFrameDurations.push_back((frameEndTime - frameBeginTime).count());
#endif
}

void Game::preInnderUpdate()
{
	SCOPED_PROFILER("Game::preInnderUpdate");
	std::unique_ptr<RenderData> renderCommands = std::make_unique<RenderData>();
	TemplateHelpers::EmplaceVariant<SwapBuffersCommand>(renderCommands->layers);
	mRenderThread.getAccessor().submitData(std::move(renderCommands));

	mInputData.windowSize = getEngine().getWindowSize();
	mInputData.mousePos = getEngine().getMousePos();
}

void Game::innerUpdate(float dt)
{
	SCOPED_PROFILER("Game::innerUpdate");
	mTime.update(dt);
	mSystemsManager.update();
}

void Game::postInnerUpdate()
{
	SCOPED_PROFILER("Game::postInnerUpdate");
	// test code
	//mRenderThread.testRunMainThread(*mGameData.getGameComponents().getOrAddComponent<RenderAccessorComponent>()->getAccessor(), getResourceManager(), getEngine());

	mInputData.clearAfterFrame();
}

void Game::initResources()
{
	SCOPED_PROFILER("Game::initResources");
	getResourceManager().loadAtlasesData("resources/atlas/atlas-list.json");
	mSystemsManager.initResources();
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
	mSystemsManager.shutdown();
}

void Game::workingThreadSaveProfileData()
{
#ifdef ENABLE_SCOPED_PROFILER
	std::lock_guard l(mScopedProfileRecordsMutex);
	mScopedProfileRecords.emplace_back(RaccoonEcs::ThreadPool::GetThisThreadId(), gtlScopedProfilerData.getAllRecords());
#endif // ENABLE_SCOPED_PROFILER
}
