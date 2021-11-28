#pragma once

#include <thread>

#include <raccoon-ecs/async_systems_manager.h>

#include "GameData/World.h"
#include "GameData/GameData.h"
#include "GameData/Serialization/Json/JsonComponentSerializer.h"

#include "Utils/Application/ArgumentsParser.h"

#include "HAL/GameBase.h"

#include "GameLogic/Debug/DebugGameBehavior.h"
#include "GameLogic/SharedManagers/TimeData.h"
#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/InputData.h"
#include "GameLogic/Render/RenderThreadManager.h"

#ifdef IMGUI_ENABLED
#include "GameLogic/Imgui/ImguiDebugData.h"
#endif

#ifdef ENABLE_SCOPED_PROFILER
#include <mutex>
#endif

class Game : public HAL::GameBase
{
public:
	using SystemsInitFunction = std::function<void(const RaccoonEcs::InnerDataAccessor& dataAccessor)>;

public:
	Game(int width, int height);

	void start(const ArgumentsParser& arguments, int workerThreadsCount, SystemsInitFunction&& initFn);
	void update(float dt) final;
	void preInnderUpdate();
	virtual void innerUpdate(float dt);
	void postInnerUpdate();
	void setKeyboardKeyState(int key, bool isPressed) override;
	void setMouseKeyState(int key, bool isPressed) override;
	void initResources() override;

protected:
	ComponentFactory& getComponentFactory() { return mComponentFactory; }
	WorldHolder& getWorldHolder() { return mWorldHolder; }
	RaccoonEcs::AsyncSystemsManager<StringId>& getSystemsManager() { return mSystemsManager; }
	InputData& getInputData() { return mInputData; }
	const TimeData& getTime() const { return mTime; }
	RaccoonEcs::ThreadPool& getThreadPool() { return mThreadPool; }
	GameData& getGameData() { return mGameData; }
	const Json::ComponentSerializationHolder& getComponentSerializers() const { return mComponentSerializers; }

private:
	void onGameShutdown();
	void workingThreadSaveProfileData();

private:
	ComponentFactory mComponentFactory;
	RaccoonEcs::EntityGenerator mEntityGenerator;
	World mWorld{mComponentFactory, mEntityGenerator};
	GameData mGameData{mComponentFactory};
	WorldHolder mWorldHolder{&mWorld, mGameData};

	InputData mInputData;

	RaccoonEcs::ThreadPool mThreadPool;
	RaccoonEcs::AsyncSystemsManager<StringId> mSystemsManager{mThreadPool};
	Json::ComponentSerializationHolder mComponentSerializers;
	TimeData mTime;
	RenderThreadManager mRenderThread;
	const int MainThreadId = 0;
	int mWorkerThreadsCount = 3;
	int mRenderThreadId = mWorkerThreadsCount + 1;

#ifdef ENABLE_SCOPED_PROFILER
	std::string mScopedProfileOutputPath = "scoped_profile.json";
	std::string mFrameDurationsOutputPath = "frame_times.csv";
	std::vector<long> mFrameDurations;
	std::vector<std::pair<size_t, ScopedProfilerThreadData::Records>> mScopedProfileRecords;
	std::mutex mScopedProfileRecordsMutex;
#endif // ENABLE_SCOPED_PROFILER

#ifdef IMGUI_ENABLED
	ImguiDebugData mImguiDebugData{mWorldHolder, mTime, mComponentFactory, {}};
#endif // IMGUI_ENABLED

	DebugGameBehavior mDebugBehavior;
	friend DebugGameBehavior;
};
