#pragma once

#include "GameLogic/Debug/DebugRecordedInput.h"

class Game;
class ArgumentsParser;

class DebugGameBehavior
{
public:
	DebugGameBehavior(int instanceIndex);

	void preInnerUpdate(Game& game);
	void postInnerUpdate(Game& game);
	void processArguments(const ArgumentsParser& arguments);

private:
	DebugRecordedInput mDebugRecordedInput;

	int mFramesBeforeShutdown = -1;
};
