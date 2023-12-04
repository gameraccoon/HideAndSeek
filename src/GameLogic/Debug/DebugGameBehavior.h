#pragma once

#include <optional>

#include "GameData/Input/GameplayInputConstants.h"

#include "HAL/InputControllersData.h"

class Game;
class ArgumentsParser;

class DebugGameBehavior
{
public:
	void preInnerUpdate(Game& game);
	void postInnerUpdate(Game& game);
	void processArguments(const ArgumentsParser& arguments);

private:
	std::optional<HAL::InputControllersData> mForcedInputData;
	int mFramesBeforeShutdown = -1;
};
