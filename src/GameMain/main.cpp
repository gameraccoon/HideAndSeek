#include "Base/precomp.h"

#include <ctime>

#include <raccoon-ecs/error_handling.h>

#include "Base/Random/Random.h"

#include "Utils/Application/ArgumentsParser.h"

#include "HAL/Base/Engine.h"

#include "GameLogic/Game/HapGame.h"
#include "GameLogic/Game/ApplicationData.h"

#include "GameMain/ConsoleCommands.h"

int main(int argc, char** argv)
{
	Random::gGlobalGenerator = Random::GlobalGeneratorType(std::random_device()());

#ifdef RACCOON_ECS_DEBUG_CHECKS_ENABLED
	RaccoonEcs::gErrorHandler = [](const std::string& error) { ReportError(error); };
#endif // RACCOON_ECS_DEBUG_CHECKS_ENABLED

	ArgumentsParser arguments(argc, argv);

	if (ConsoleCommands::TryExecuteQuickConsoleCommands(arguments))
	{
		return true;
	}

	ApplicationData applicationData(arguments.getIntArgumentValue("threads-count").getValueOr(ApplicationData::DefaultWorkerThreadCount));
	HAL::Engine engine(800, 600);

	// switch render context to render thread
	engine.releaseRenderContext();
	applicationData.renderThread.startThread(applicationData.resourceManager, engine, [&engine]{ engine.acquireRenderContext(); });

	HapGame game(&engine, applicationData.resourceManager, applicationData.threadPool);
	game.preStart(arguments, RenderAccessorGameRef(applicationData.renderThread.getAccessor(), 0));
	game.initResources();
	engine.start();	// this call waits until the game is being shut down
	game.onGameShutdown();

	applicationData.shutdownThreads(); // this call waits for the threads to be joined

	applicationData.writeProfilingData(); // this call waits for the data to be written to the files

	return 0;
}
