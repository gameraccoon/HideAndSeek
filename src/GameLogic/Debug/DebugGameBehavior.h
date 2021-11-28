#pragma once

#include <optional>

#include "GameLogic/SharedManagers/InputData.h"

class Game;
class ArgumentsParser;

class DebugGameBehavior
{
public:
	void preInnerUpdate(Game& game);
	void postInnerUpdate(Game& game);
	void processArguments(const ArgumentsParser& arguments);

private:
	std::optional<InputData> mForcedInputData;
	int mFramesBeforeShutdown = -1;
};
