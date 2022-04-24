#include "Base/precomp.h"

#include "GameLogic/Debug/DebugGameBehavior.h"

#include "Utils/Application/ArgumentsParser.h"

#include "HAL/Base/Engine.h"

#include "GameLogic/Game/Game.h"

void DebugGameBehavior::preInnerUpdate(Game& game)
{
	if (mForcedInputData.has_value())
	{
		game.mInputData = *mForcedInputData;
	}
}

void DebugGameBehavior::postInnerUpdate(Game& game)
{
	if (mFramesBeforeShutdown >= 0)
	{
		--mFramesBeforeShutdown;
		if (mFramesBeforeShutdown <= 0)
		{
			game.quitGame();
		}
	}
}

void DebugGameBehavior::processArguments(const ArgumentsParser& arguments)
{
	if (arguments.hasArgument("disable-input"))
	{
		mForcedInputData = InputData();
		Input::ControllerState& mouseState = mForcedInputData->controllerStates[Input::ControllerType::Mouse];
		mouseState.updateAxis(0, 0.5f);
		mouseState.updateAxis(1, 0.5f);
	}

	if (arguments.hasArgument("time-limit"))
	{
		mFramesBeforeShutdown = std::atoi(arguments.getArgumentValue("time-limit").c_str());
	}
}
