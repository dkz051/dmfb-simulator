#include <QDateTime>
#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[]) {
	srand(quint32(QDateTime::currentMSecsSinceEpoch()));

	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication a(argc, argv);
	MainWindow *w = new MainWindow;
	w->show();

	int retVal = a.exec();
	delete w;
	return retVal;
}
