#pragma once

#include <thread>

#include <raccoon-ecs/async_systems_manager.h>

#include "GameData/World.h"
#include "GameData/GameData.h"
#include "GameData/Serialization/Json/JsonComponentSerializer.h"

#include "Utils/Application/ArgumentsParser.h"
#include "Utils/Profiling/SystemFrameRecords.h"

#include "HAL/GameBase.h"

#include "GameLogic/SharedManagers/TimeData.h"
#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/InputData.h"
#include "GameLogic/Render/RenderThreadManager.h"

#ifdef IMGUI_ENABLED
#include "GameLogic/Imgui/ImguiDebugData.h"
#endif

class Game : public HAL::GameBase
{
public:
	Game(int width, int height);

	void start(ArgumentsParser& arguments);
	void update(float dt) override;
	void setKeyboardKeyState(int key, bool isPressed) override;
	void setMouseKeyState(int key, bool isPressed) override;
	void initResources() override;

private:
	void initSystems();
	void onGameShutdown();

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
	static inline const int WorkerThreadsCount = 3;
	static inline const int RenderThreadId = WorkerThreadsCount + 1;

#ifdef RACCOON_ECS_PROFILE_SYSTEMS
	bool mProfileSystems = false;
	std::string mSystemProfileOutputPath = "systemProfile.json";
#endif // RACCOON_ECS_PROFILE_SYSTEMS

#if defined(IMGUI_ENABLED) || defined(RACCOON_ECS_PROFILE_SYSTEMS)
	SystemFrameRecords mSystemFrameRecords;
#endif // IMGUI_ENABLED || RACCOON_ECS_PROFILE_SYSTEMS

#ifdef IMGUI_ENABLED
	ImguiDebugData mImguiDebugData{mWorldHolder, mTime, mSystemFrameRecords, mComponentFactory, {}};
#endif // IMGUI_ENABLED
};
