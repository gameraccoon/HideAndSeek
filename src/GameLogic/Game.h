#pragma once

#include <thread>

#include <raccoon-ecs/systems_manager.h>

#include "GameData/World.h"
#include "GameData/GameData.h"
#include "GameData/Serialization/Json/JsonComponentSerializer.h"

#include "Utils/Application/ArgumentsParser.h"
#include "Utils/Multithreading/ThreadPool.h"

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
	Game(int width, int height);

	void start(const ArgumentsParser& arguments, int workerThreadsCount);
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
	RaccoonEcs::SystemsManager& getSystemsManager() { return mSystemsManager; }
	InputData& getInputData() { return mInputData; }
	const TimeData& getTime() const { return mTime; }
	RaccoonEcs::ThreadPool& getThreadPool() { return mThreadPool; }
	GameData& getGameData() { return mGameData; }
	Json::ComponentSerializationHolder& getComponentSerializers() { return mComponentSerializers; }

private:
	void onGameShutdown();
	void workingThreadSaveProfileData();

private:
	ComponentFactory mComponentFactory;
	RaccoonEcs::IncrementalEntityGenerator mEntityGenerator;
	World mWorld{mComponentFactory, mEntityGenerator};
	GameData mGameData{mComponentFactory};
	WorldHolder mWorldHolder{&mWorld, mGameData};

	InputData mInputData;

	RaccoonEcs::ThreadPool mThreadPool;
	RaccoonEcs::SystemsManager mSystemsManager;
	Json::ComponentSerializationHolder mComponentSerializers;
	TimeData mTime;
	RenderThreadManager mRenderThread;
	const int MainThreadId = 0;
	int mWorkerThreadsCount = 3;
	int mRenderThreadId = mWorkerThreadsCount + 1;

#ifdef ENABLE_SCOPED_PROFILER
	std::string mScopedProfileOutputPath = "./scoped_profile.json";
	std::string mFrameDurationsOutputPath = "./frame_times.csv";
	std::vector<double> mFrameDurations;
	std::vector<std::pair<size_t, ScopedProfilerThreadData::Records>> mScopedProfileRecords;
	std::mutex mScopedProfileRecordsMutex;
#endif // ENABLE_SCOPED_PROFILER

#ifdef IMGUI_ENABLED
	ImguiDebugData mImguiDebugData{mWorldHolder, mTime, mComponentFactory, {}};
#endif // IMGUI_ENABLED

	DebugGameBehavior mDebugBehavior;
	friend DebugGameBehavior;
};
