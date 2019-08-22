#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dlgnewchip.h"
#include "frmconfigchip.h"
#include "dlgabout.h"
#include "commandset.h"
#include "ui.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QFile>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QTextStream>

#include <string>

static const qreal acceleration = 3.0;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), timer(this), dataLoaded(false) {
	ui->setupUi(this);
	timer.setInterval(25);
	connect(&timer, SIGNAL(timeout()), this, SLOT(on_timer_timeout()));
	ui->picDisplay->installEventFilter(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_actionNewChip_triggered() {
	dlgNewChip wndNewChip(this);
	connect(&wndNewChip, SIGNAL(accepted(qint32, qint32)), this, SLOT(on_dlgNewChip_accepted(qint32, qint32)));
	wndNewChip.exec();
}

void MainWindow::on_dlgNewChip_accepted(qint32 rows, qint32 columns) {
	frmConfigChip *wndConfigChip = new frmConfigChip(this);
	connect(wndConfigChip, SIGNAL(accepted(const chipConfig &)), this, SLOT(on_dlgConfigChip_accepted(const chipConfig &)));
	wndConfigChip->setDimensions(rows, columns);
	wndConfigChip->show();

	dataLoaded = false;
}

void MainWindow::on_dlgConfigChip_accepted(const chipConfig &config) {
	displayTime = 0;
	this->config = config;
	ui->actionLoadCommandFile->setEnabled(true);
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
	QString errorMsg;

	if (!::loadFile(url, config, errorMsg, droplets, minTime, maxTime)) {
		QMessageBox::warning(this, tr("Error loading command file"), errorMsg);
		return;
	}

	ui->actionStart->setEnabled(true);
	ui->actionStep->setEnabled(true);

	displayTime = minTime;
	dataLoaded = true;

	this->update();
}

void MainWindow::render() {
	this->update();
}

void MainWindow::on_timer_timeout() {
	qint64 thisTime = QDateTime::currentMSecsSinceEpoch();
	displayTime += (thisTime - lastTime) * acceleration;
	lastTime = thisTime;

	if (displayTime > maxTime) {
		displayTime = maxTime;
		timer.stop();
		ui->actionStart->setEnabled(true);
		ui->actionPause->setEnabled(false);
		ui->actionStep->setEnabled(true);
		ui->actionRevert->setEnabled(true);
		ui->actionReset->setEnabled(true);
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
	//displayTime = (displayTime / 1000) * 1000; // truncate to seconds

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
