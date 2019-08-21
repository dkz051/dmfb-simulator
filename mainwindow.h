#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>

#include "chipconfig.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
	void dragEnterEvent(QDragEnterEvent *e);
	void dropEvent(QDropEvent *e);

private slots:
	void on_actionNewChip_triggered();

	void on_dlgNewChip_accepted(qint32 rows, qint32 columns);
	void on_dlgConfigChip_accepted(const chipConfig &config);

	void on_actionExit_triggered();

	void on_actionAboutDmfbSimulator_triggered();

	void loadFile(const QUrl &url);
	void selectFile();
	void on_actionLoadCommandFile_triggered();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
