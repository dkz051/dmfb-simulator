#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QSet>
#include <QUrl>
#include <QSound>
#include <QTimer>
#include <QDateTime>
#include <QMainWindow>

#include "utility.h"

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
	void onDlgConfigChipAccepted(const ChipConfig &config);

	void on_actionExit_triggered();

	void on_actionAboutDmfbSimulator_triggered();

	void loadFile(const QString &url);
	void selectFile();
	void render();
	void on_actionLoadCommandFile_triggered();

	void onRunTimeout();
	void onWashTimeout();
	void on_actionStart_triggered();

	void on_actionPause_triggered();

	void on_actionStep_triggered();

	void on_actionRevert_triggered();

	void on_actionReset_triggered();

	void playSound(qint32 sounds);
	void clearContaminants();
	void clearObstacles();
	bool wash(QVector<Position> &steps);
	void washRoute(const ChipConfig &config, QVector<Position> &steps, qint32 tx, qint32 ty, const QVector<QVector<bool>> &obstacles);
	void on_actionWash_triggered();

	void clearContamination(qint32 second);

private:
	Ui::MainWindow *ui;

	// Chip Config
	bool dataLoaded;
	ChipConfig config;

	// Sound Effects
	QSound sndMove, sndMerge, sndSplitting, sndSplit, sndError;
	SoundList sounds;

	// Error Info
	ErrorLog error;

	// Contamination
	ContaminantList contaminants;
	QVector<QVector<QSet<qint32>>> contamination;
	quint32 randSeed;

	// Run Timer
	QTimer timerRun;
	qint64 lastTime, displayTime;
	qint64 minTime, maxTime;
	QVector<Droplet> droplets;

	// Wash
	QTimer timerWash;
	QVector<QVector<bool>> obstacles;
	QVector<Position> steps;
	qint64 lastWashTime, curWashTime;
	QColor washColor;
};

#endif // MAINWINDOW_H
