#include "mainwindow.h"
#include <QApplication>

#include "Base/Random/Random.h"

int main(int argc, char *argv[])
{
	Random::gGlobalGenerator = std::mt19937(time(nullptr));

	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	return a.exec();
}
