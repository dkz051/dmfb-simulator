#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>
#include <QTimer>
#include <QDateTime>

#include "chipconfig.h"
#include "commandset.h"

static const qreal acceleration = 2.0;

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

	void loadFile(const QString &url);
	void selectFile();
	void render();
	void on_actionLoadCommandFile_triggered();

	void on_timer_timeout();
	void on_actionStart_triggered();

	void on_actionPause_triggered();

	void on_actionStep_triggered();

	void on_actionRevert_triggered();

	void on_actionReset_triggered();

private:
	Ui::MainWindow *ui;

	QTimer timer;
	qint64 lastTime, displayTime;
};

#endif // MAINWINDOW_H
