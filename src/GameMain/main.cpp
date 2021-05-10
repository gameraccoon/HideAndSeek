#include "Base/precomp.h"

#include <ctime>

#include "Base/Random/Random.h"

#include <raccoon-ecs/error_handling.h>

#include "GameLogic/Game.h"

int main(int argc, char** argv)
{
	Random::gGlobalGenerator = Random::GlobalGeneratorType(static_cast<unsigned int>(time(nullptr)));

#ifdef ECS_DEBUG_CHECKS_ENABLED
	RaccoonEcs::gErrorHandler = [](const std::string& error) { ReportError(error); };
#endif // ECS_DEBUG_CHECKS_ENABLED

	ArgumentsParser arguments(argc, argv);

	Game game(800, 600);
	game.start(arguments);

	return 0;
}
