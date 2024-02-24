#pragma once

#include "GameData/Render/RenderAccessorGameRef.h"

#include "GameLogic/Game/Game.h"

#include "AutoTests/TestCheckList.h"

class RenderAccessor;
class ArgumentsParser;

class BaseTestCase : public Game
{
public:
	using Game::Game;

	TestChecklist start(const ArgumentsParser& arguments, RenderAccessorGameRef renderAccessor);
	void fixedTimeUpdate(float dt) final;

	bool shouldPauseGame() const override { return false; }
	bool shouldQuitGame() const override { return mShouldQuit; }
	void quitGame() override { mShouldQuit = true; }

protected:
	virtual void initTestCase(const ArgumentsParser& arguments) = 0;
	virtual void finalizeTestCase();

protected:
	TestChecklist mTestChecklist;
	int mTicksToFinish = 100;

private:
	int mTicksCount = 0;
	bool mOneFrame = false;
	bool mShouldQuit = false;
};
