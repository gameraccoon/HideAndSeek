#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
	std::srand(static_cast<unsigned int>(time(nullptr)));

	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	return a.exec();
}
