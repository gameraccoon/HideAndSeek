#include "mainwindow.h"
#include <QApplication>

#include "Base/Debug/Assert.h"
#include "Base/Random/Random.h"
#include "ECS/ErrorHandling.h"

int main(int argc, char *argv[])
{
	Random::gGlobalGenerator = std::mt19937(time(nullptr));

#ifdef ECS_DEBUG_CHECKS_ENABLED
	Ecs::gErrorHandler = [](const std::string& error) { ReportFatalError(error); };
#endif // ECS_DEBUG_CHECKS_ENABLED

	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	return a.exec();
}
