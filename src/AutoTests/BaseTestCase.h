#pragma once

#include <raccoon-ecs/systems_manager.h>

#include "GameData/EcsDefinitions.h"
#include "GameData/World.h"
#include "GameData/GameData.h"

#include "Utils/Application/ArgumentsParser.h"

#include "HAL/GameBase.h"

#include "GameLogic/Game.h"
#include "GameLogic/SharedManagers/TimeData.h"
#include "GameLogic/SharedManagers/WorldHolder.h"
#include "GameLogic/SharedManagers/InputData.h"
#include "GameLogic/Render/RenderThreadManager.h"

#include "AutoTests/TestChecklist.h"

class BaseTestCase : public Game
{
public:
	BaseTestCase(int width, int height);

	TestChecklist start(const ArgumentsParser& arguments);
	void fixedTimeUpdate(float dt) final;
	void setKeyboardKeyState(int, bool) override {}
	void setMouseKeyState(int, bool) override {}

protected:
	virtual void initTestCase(const ArgumentsParser& arguments) = 0;
	virtual void finalizeTestCase();

protected:
	TestChecklist mTestChecklist;
	int mTicksToFinish = 100;

private:
	int mTicksCount = 0;
	bool mOneFrame = false;
};
