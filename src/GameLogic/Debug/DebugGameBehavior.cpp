#include "Base/precomp.h"

#include "GameLogic/Debug/DebugGameBehavior.h"

#include "Utils/Application/ArgumentsParser.h"

#include "HAL/Base/Engine.h"

#include "GameLogic/Game/Game.h"

void DebugGameBehavior::preInnerUpdate(Game& game)
{
	if (mForcedInputData.has_value())
	{
		game.mInputControllersData = *mForcedInputData;
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
		mForcedInputData = HAL::InputControllersData();
	}

	if (arguments.hasArgument("time-limit"))
	{
		mFramesBeforeShutdown = arguments.getIntArgumentValue("time-limit").getValueOr(-1);
	}
}
