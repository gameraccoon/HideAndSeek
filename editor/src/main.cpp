#include "mainwindow.h"

#include <QApplication>
#include <raccoon-ecs/error_handling.h>

#include "EngineCommon/Debug/Assert.h"
#include "EngineCommon/Random/Random.h"

int main(int argc, char *argv[])
{
	INITIALIZE_STRING_IDS();
	Random::gGlobalGenerator = std::mt19937(time(nullptr));

#ifdef RACCOON_ECS_DEBUG_CHECKS_ENABLED
	RaccoonEcs::gErrorHandler = [](const std::string& error) { ReportFatalError(error); };
#endif // RACCOON_ECS_DEBUG_CHECKS_ENABLED

	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	return a.exec();
}
