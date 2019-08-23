#include "mainwindow.h"

#include <string>

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QFile>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QTextStream>

#include "dlgabout.h"
#include "dlgnewchip.h"
#include "frmconfigchip.h"

#include "ui_mainwindow.h"

#include "ui.h"
#include "commandset.h"

static const qreal acceleration = 1.0;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), timer(this), dataLoaded(false), sndMove("qrc:/sounds/move.wav"), sndMerge("qrc:/sounds/merge.wav"), sndSplitting("qrc:/sounds/splitting.wav"), sndSplit("qrc:/sounds/split.wav"), sndError("qrc:/sounds/error.wav") {
	ui->setupUi(this);
	timer.setInterval(25);
	connect(&timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
	ui->picDisplay->installEventFilter(this);

	sndMove.setLoops(1);
	sndMerge.setLoops(1);
	sndSplit.setLoops(1);
	sndSplitting.setLoops(1);
	sndError.setLoops(1);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_actionNewChip_triggered() {
	dlgNewChip wndNewChip(this);
	connect(&wndNewChip, SIGNAL(accepted(qint32, qint32)), this, SLOT(onDlgNewChipAccepted(qint32, qint32)));
	wndNewChip.exec();
}

void MainWindow::onDlgNewChipAccepted(qint32 rows, qint32 columns) {
	ui->actionLoadCommandFile->setEnabled(false);
	ui->actionStart->setEnabled(false);
	ui->actionPause->setEnabled(false);
	ui->actionStep->setEnabled(false);
	ui->actionRevert->setEnabled(false);
	ui->actionReset->setEnabled(false);

	dataLoaded = false;

	frmConfigChip *wndConfigChip = new frmConfigChip(this);
	connect(wndConfigChip, SIGNAL(accepted(const chipConfig &)), this, SLOT(onDlgConfigChipAccepted(const chipConfig &)));
	wndConfigChip->setDimensions(rows, columns);
	wndConfigChip->show();
}

void MainWindow::onDlgConfigChipAccepted(const chipConfig &config) {
	displayTime = 0;
	this->config = config;
	ui->actionLoadCommandFile->setEnabled(true);

	selectFile();
}

void MainWindow::on_actionExit_triggered() {
	this->close();
}

void MainWindow::on_actionAboutDmfbSimulator_triggered() {
	dlgAbout wndAbout(this);
	wndAbout.exec();
}

void MainWindow::on_actionLoadCommandFile_triggered() {
	selectFile();
}

void MainWindow::selectFile() {
	QFileDialog *fileDlg = new QFileDialog(this);
	fileDlg->setWindowTitle(tr("Open Command File"));
	fileDlg->setDirectory(".");
	fileDlg->setNameFilter("All Files (*.*)");
	fileDlg->setFileMode(QFileDialog::ExistingFile);
	if (fileDlg->exec()) {
		QStringList fileName = fileDlg->selectedFiles();
		if (fileName.empty()) return;
		loadFile(fileName[0]);
	}
	this->setFocus();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
	if (ui->actionLoadCommandFile->isEnabled() && e->mimeData()->hasFormat("text/uri-list")) {
		e->acceptProposedAction();
	}
}

void MainWindow::dropEvent(QDropEvent *e) {
	QList<QUrl> urls = e->mimeData()->urls();
	if (urls.empty()) return;
	loadFile(urls.first().toLocalFile());
}

void MainWindow::loadFile(const QString &url) {
	::loadFile(url, config, droplets, minTime, maxTime, sounds, errors);
	maxTime = (maxTime / 1000) * 1000; // Truncate to seconds (in case any command failed)

	ui->actionStart->setEnabled(true);
	ui->actionStep->setEnabled(true);

	displayTime = minTime;
	dataLoaded = true;

	this->update();
}

void MainWindow::render() {
	this->update();
}

void MainWindow::onTimeout() {
	qint64 thisTime = QDateTime::currentMSecsSinceEpoch();
	qint64 lastDisplay = displayTime;

	displayTime += (thisTime - lastTime) * acceleration;
	lastTime = thisTime;

	auto iter = sounds.lowerBound(lastDisplay / 1000.0);
	auto jter = sounds.lowerBound(displayTime / 1000.0);

	if (iter != jter) {
		playSound(iter.value());
	}

	auto kter = std::lower_bound(errors.begin(), errors.end(), lastDisplay / 1000.0, [](errorLog e, qreal t) -> bool { return e.t < t; });
	auto lter = std::lower_bound(errors.begin(), errors.end(), displayTime / 1000.0, [](errorLog e, qreal t) -> bool { return e.t < t; });

	if (kter != lter) {
		on_actionPause_triggered();
		sndError.play();
		QMessageBox::warning(this, tr("Error"), kter->msg);
	}

	if (displayTime > maxTime) {
		displayTime = maxTime;
		on_actionPause_triggered();
	}

	render();
}

void MainWindow::on_actionStart_triggered() {
	timer.start();
	lastTime = QDateTime::currentMSecsSinceEpoch();

	ui->actionStart->setEnabled(false);
	ui->actionPause->setEnabled(true);
	ui->actionStep->setEnabled(false);
	ui->actionRevert->setEnabled(false);
	ui->actionReset->setEnabled(false);

	ui->actionNewChip->setEnabled(false);
	ui->actionLoadCommandFile->setEnabled(false);
}

void MainWindow::on_actionPause_triggered() {
	timer.stop();
	//displayTime = ceil(displayTime / 1000.0) * 1000; // set to the next second

	ui->actionStart->setEnabled(true);
	ui->actionPause->setEnabled(false);
	ui->actionStep->setEnabled(true);
	ui->actionRevert->setEnabled(true);
	ui->actionReset->setEnabled(true);

	ui->actionNewChip->setEnabled(true);
	ui->actionLoadCommandFile->setEnabled(true);

	render();
}

void MainWindow::on_actionStep_triggered() {
	displayTime = qint32(floor(displayTime / 1000.0) + 1.0) * 1000;
	displayTime = std::min(displayTime, maxTime);
	render();
}

void MainWindow::on_actionRevert_triggered() {
	displayTime = qint32(ceil(displayTime / 1000.0) - 1.0) * 1000;
	displayTime = std::max(displayTime, minTime);
	render();
}

void MainWindow::on_actionReset_triggered() {
	displayTime = minTime;
	ui->actionNewChip->setEnabled(true);
	ui->actionLoadCommandFile->setEnabled(true);
	render();
}

bool MainWindow::eventFilter(QObject *o, QEvent *e) {
	if (o == ui->picDisplay) {
		QWidget *p = static_cast<QWidget *>(o);
		if (e->type() == QEvent::Paint) {
			QPainter painter(p);
			painter.eraseRect(painter.window());
			painter.setRenderHints(QPainter::Antialiasing);

			qreal W = p->width(), H = p->height();

			renderPortType(config, W, H, &painter);
			renderGridAxisNumber(config, W, H, &painter);
			if (dataLoaded) {
				renderTime(config, displayTime / 1000.0, maxTime / 1000.0, W, H, &painter);
				renderDroplets(config, droplets, displayTime / 1000.0, W, H, &painter);
			}
			renderGrid(config, W, H, &painter);
			return true;
		} else if (e->type() == QEvent::MouseButtonPress) {
			// TODO: Toggle obstacle (of wash droplets) status when clicked
		}
	}
	return false;
}

void MainWindow::playSound(qint32 sounds) {
	sndMove.stop();
	sndMerge.stop();
	sndSplit.stop();
	sndSplitting.stop();
	if (sounds & sndFxMove) {
		sndMove.play();
	}
	if (sounds & sndFxMerge) {
		sndMerge.play();
	}
	if (sounds & sndFxSplit) {
		sndSplit.play();
	}
	if (sounds & sndFxSplitting) {
		sndSplitting.play();
	}
}
