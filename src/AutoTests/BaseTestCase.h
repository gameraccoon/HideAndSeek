#pragma once

#include "GameLogic/Game/Game.h"

#include "AutoTests/TestChecklist.h"

class RenderAccessor;
class ArgumentParser;

class BaseTestCase : public Game
{
public:
	using Game::Game;

	TestChecklist start(const ArgumentsParser& arguments, RenderAccessor& renderAccessor);
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
