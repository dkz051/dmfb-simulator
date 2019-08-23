#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QUrl>
#include <QTimer>
#include <QDateTime>
#include <QMainWindow>
#include <QSoundEffect>

#include "chipconfig.h"
#include "commandset.h"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
	void dragEnterEvent(QDragEnterEvent *e);
	void dropEvent(QDropEvent *e);

	bool eventFilter(QObject *o, QEvent *e);

private slots:
	void on_actionNewChip_triggered();

	void onDlgNewChipAccepted(qint32 rows, qint32 columns);
	void onDlgConfigChipAccepted(const chipConfig &config);

	void on_actionExit_triggered();

	void on_actionAboutDmfbSimulator_triggered();

	void loadFile(const QString &url);
	void selectFile();
	void render();
	void on_actionLoadCommandFile_triggered();

	void onTimeout();
	void on_actionStart_triggered();

	void on_actionPause_triggered();

	void on_actionStep_triggered();

	void on_actionRevert_triggered();

	void on_actionReset_triggered();

private:
	Ui::MainWindow *ui;

	QTimer timer;
	qint64 lastTime, displayTime;

	qint64 minTime, maxTime;
	QVector<droplet> droplets;
	bool dataLoaded;
	chipConfig config;

	QSoundEffect sndMove, sndMerge, sndSplitting, sndSplit;
	soundList sounds;
};

#endif // MAINWINDOW_H
