#pragma once

#include <raccoon-ecs/utils/systems_manager.h>

#include "GameData/GameData.h"
#include "GameData/Serialization/Json/JsonComponentSerializer.h"
#include "GameData/World.h"

#include "HAL/GameBase.h"
#include "HAL/InputControllersData.h"

#include "GameLogic/Debug/DebugGameBehavior.h"
#include "GameLogic/SharedManagers/WorldHolder.h"

class ArgumentsParser;
class ThreadPool;

class Game : public HAL::GameBase
{
public:
	Game(HAL::Engine* engine, ResourceManager& resourceManager, ThreadPool& threadPool);

	void preStart(const ArgumentsParser& arguments);
	void onGameShutdown();

	void notPausablePreFrameUpdate(float dt) final;
	void dynamicTimePreFrameUpdate(float dt, int plannedFixedTimeUpdates) final;
	void fixedTimeUpdate(float dt) override;
	void dynamicTimePostFrameUpdate(float dt, int processedFixedTimeUpdates) final;
	void notPausableRenderUpdate(float frameAlpha) final;
	void initResources() override;

	std::chrono::duration<s64, std::micro> getFrameLengthCorrection() const final { return std::chrono::duration<s64, std::micro>(0); }

protected:
	ComponentFactory& getComponentFactory() { return mComponentFactory; }
	WorldHolder& getWorldHolder() { return mWorldHolder; }
	RaccoonEcs::SystemsManager& getNotPausablePreFrameSystemsManager() { return mNotPausablePreFrameSystemsManager; }
	RaccoonEcs::SystemsManager& getPreFrameSystemsManager() { return mPreFrameSystemsManager; }
	RaccoonEcs::SystemsManager& getGameLogicSystemsManager() { return mGameLogicSystemsManager; }
	RaccoonEcs::SystemsManager& getPostFrameSystemsManager() { return mPostFrameSystemsManager; }
	RaccoonEcs::SystemsManager& getNotPausableRenderSystemsManager() { return mNotPausableRenderSystemsManager; }
	HAL::InputControllersData& getInputData() { return mInputControllersData; }
	ThreadPool& getThreadPool() { return mThreadPool; }
	GameData& getGameData() { return mGameData; }
	Json::ComponentSerializationHolder& getComponentSerializers() { return mComponentSerializers; }

private:
	ComponentFactory mComponentFactory;
	World mWorld{ mComponentFactory };
	GameData mGameData{ mComponentFactory };
	WorldHolder mWorldHolder{ &mWorld, mGameData };

	HAL::InputControllersData mInputControllersData;

	ThreadPool& mThreadPool;
	RaccoonEcs::SystemsManager mNotPausablePreFrameSystemsManager;
	RaccoonEcs::SystemsManager mPreFrameSystemsManager;
	RaccoonEcs::SystemsManager mGameLogicSystemsManager;
	RaccoonEcs::SystemsManager mPostFrameSystemsManager;
	RaccoonEcs::SystemsManager mNotPausableRenderSystemsManager;
	Json::ComponentSerializationHolder mComponentSerializers;

#ifdef ENABLE_SCOPED_PROFILER
	std::string mFrameDurationsOutputPath = "./frame_times.csv";
	std::vector<double> mFrameDurations;
	std::chrono::time_point<std::chrono::steady_clock> mFrameBeginTime;
#endif // ENABLE_SCOPED_PROFILER

	DebugGameBehavior mDebugBehavior;
	friend DebugGameBehavior;
};
