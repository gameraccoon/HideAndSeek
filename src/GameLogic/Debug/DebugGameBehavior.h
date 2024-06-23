#pragma once

#include "GameLogic/Debug/DebugRecordedInput.h"

class Game;
class ArgumentsParser;

class DebugGameBehavior
{
public:
	explicit DebugGameBehavior(int instanceIndex);

	void preInnerUpdate(Game& game);
	void postInnerUpdate(Game& game);
	void processArguments(const ArgumentsParser& arguments);

private:
	DebugRecordedInput mDebugRecordedInput;

	int mFramesBeforeShutdown = -1;
};
