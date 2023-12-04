#include "Base/precomp.h"

#include "GameLogic/Debug/DebugGameBehavior.h"

#include "Utils/Application/ArgumentsParser.h"

#include "GameLogic/Game/Game.h"

DebugGameBehavior::DebugGameBehavior(int instanceIndex)
	: mDebugRecordedInput(instanceIndex)
{
}

void DebugGameBehavior::preInnerUpdate(Game& game)
{
	const bool shouldQuit = mDebugRecordedInput.processFrameInput(game.mInputControllersData);
	if (shouldQuit)
	{
		game.quitGame();
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
	mDebugRecordedInput.processArguments(arguments);

	if (arguments.hasArgument("time-limit"))
	{
		mFramesBeforeShutdown = arguments.getIntArgumentValue("time-limit").getValueOr(-1);
	}
}
