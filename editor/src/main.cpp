#include <QApplication>

#include "mainwindow.h"
#include <raccoon-ecs/error_handling.h>

#include "EngineCommon/Debug/Assert.h"
#include "EngineCommon/Random/Random.h"

int main(int argc, char* argv[])
{
	INITIALIZE_STRING_IDS();
	Random::gGlobalGenerator = Random::GlobalGeneratorType(std::random_device()());

#ifdef RACCOON_ECS_DEBUG_CHECKS_ENABLED
	RaccoonEcs::gErrorHandler = [](const std::string& error) { ReportFatalError(error); };
#endif // RACCOON_ECS_DEBUG_CHECKS_ENABLED

	QApplication app(argc, argv);
	MainWindow window;
	window.show();

	return app.exec();
}
