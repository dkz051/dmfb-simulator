#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[]) {
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QApplication app(argc, argv);

	MainWindow wnd;
	wnd.show();
	return app.exec();
}
