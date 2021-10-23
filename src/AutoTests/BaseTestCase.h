#pragma once

#include <raccoon-ecs/async_systems_manager.h>

#include "GameData/EcsDefinitions.h"
#include "GameData/World.h"
#include "GameData/GameData.h"

#include "Utils/Application/ArgumentsParser.h"
#include "Utils/Profiling/SystemFrameRecords.h"
#include "Utils/Jobs/WorkerManager.h"

#include "HAL/GameBase.h"

#include "GameLogic/SharedManagers/TimeData.h"
#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/InputData.h"
#include "GameLogic/Render/RenderThreadManager.h"

#include "AutoTests/TestChecklist.h"

class BaseTestCase : public HAL::GameBase
{
public:
	BaseTestCase(int width, int height);

	TestChecklist start(const ArgumentsParser& arguments);
	void update(float dt) final;
	void initResources() override;
	void setKeyboardKeyState(int, bool) override {}
	void setMouseKeyState(int, bool) override {}

protected:
	virtual void initTestCase(const ArgumentsParser& arguments) = 0;
	virtual void finalizeTestCase();

protected:
	ComponentFactory mComponentFactory;
	RaccoonEcs::EntityGenerator mEntityGenerator;
	World mWorld{mComponentFactory, mEntityGenerator};
	GameData mGameData{mComponentFactory};
	WorldHolder mWorldHolder{&mWorld, mGameData};
	RaccoonEcs::AsyncSystemsManager<StringId> mSystemsManager;
	Jobs::WorkerManager mWorkerManager{1};
	TimeData mTime;
	InputData mInputData;
	RenderThreadManager mRenderThread;

	TestChecklist mTestChecklist;

	int mTicksToFinish = 100;

private:
	int mTicksCount = 0;
	bool mOneFrame = false;

	bool mProfileSystems = false;
	SystemFrameRecords mSystemFrameRecords;
	std::string mSystemProfileOutputPath = "systemProfile.csv";
};
