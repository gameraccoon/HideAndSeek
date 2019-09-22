#pragma once

#include "ECS/SystemsManager.h"
#include "ECS/ComponentFactory.h"

#include "GameData/World.h"
#include "GameData/GameData.h"

#include "Utils/Application/ArgumentsParser.h"
#include "Utils/Profiling/SystemFrameRecords.h"

#include "HAL/GameBase.h"
#include "HAL/KeyStatesMap.h"

#include "GameLogic/SharedManagers/TimeData.h"
#include "GameLogic/SharedManagers/WorldHolder.h"

class Game : public HAL::GameBase
{
public:
	using HAL::GameBase::GameBase;

	void start(ArgumentsParser& arguments);
	void update(float dt) override;
	void setKeyboardKeyState(int key, bool isPressed) override;
	void setMouseKeyState(int key, bool isPressed) override;
	void initResources() override;

private:
	void initSystems();
	void onGameShutdown();

private:
	WorldHolder mWorldHolder;
	World mWorld;
	GameData mGameData;

	HAL::KeyStatesMap mKeyStates;
	SystemsManager mSystemsManager;
	ComponentFactory mComponentFactory;
	TimeData mTime;

	bool mProfileSystems = false;
	SystemFrameRecords mSystemFrameRecords;
	std::string mSystemProfileOutputPath = "systemProfile.csv";
};
