#include "mainwindow.h"

#include <QApplication>
#include <raccoon-ecs/error_handling.h>

#include "Base/Debug/Assert.h"
#include "Base/Random/Random.h"

int main(int argc, char *argv[])
{
	Random::gGlobalGenerator = std::mt19937(time(nullptr));

#ifdef ECS_DEBUG_CHECKS_ENABLED
	RaccoonEcs::gErrorHandler = [](const std::string& error) { ReportFatalError(error); };
#endif // ECS_DEBUG_CHECKS_ENABLED

	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	return a.exec();
}
