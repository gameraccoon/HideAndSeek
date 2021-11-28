#include "Base/precomp.h"

#include "GameLogic/Debug/DebugGameBehavior.h"

#include "HAL/Base/Engine.h"

#include "GameLogic/Game.h"

void DebugGameBehavior::preInnerUpdate(Game& game)
{
	if (mForcedInputData.has_value())
	{
		game.mInputData = *mForcedInputData;
		game.mInputData.windowSize = game.getEngine().getWindowSize();
		game.mInputData.mousePos = Vector2D::HadamardProduct(game.mInputData.mousePos, game.mInputData.windowSize);
	}
}

void DebugGameBehavior::postInnerUpdate(Game& game)
{
	if (mFramesBeforeShutdown >= 0)
	{
		--mFramesBeforeShutdown;
		if (mFramesBeforeShutdown <= 0)
		{
			game.getEngine().quit();
		}
	}
}

void DebugGameBehavior::processArguments(const ArgumentsParser& arguments)
{
	if (arguments.hasArgument("disable-input"))
	{
		mForcedInputData = InputData();
		mForcedInputData->mousePos = Vector2D(0.5f, 0.5f);
	}

	if (arguments.hasArgument("time-limit"))
	{
		mFramesBeforeShutdown = std::atoi(arguments.getArgumentValue("time-limit").c_str());
	}
}
