#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[]) {
	srand(unsigned(time(nullptr)));

	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
	MainWindow *w = new MainWindow;
	w->show();

	int retVal = a.exec();
	delete w;
	return retVal;
}
