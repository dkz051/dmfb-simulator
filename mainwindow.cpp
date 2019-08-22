#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dlgnewchip.h"
#include "frmconfigchip.h"
#include "dlgabout.h"
#include "commandset.h"

#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QFile>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QTextStream>

#include <string>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    timer(this)
{
	ui->setupUi(this);
	timer.setInterval(25);
	connect(&timer, SIGNAL(timeout()), this, SLOT(on_timer_timeout()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionNewChip_triggered()
{
	dlgNewChip wndNewChip(this);
	connect(&wndNewChip, SIGNAL(accepted(qint32, qint32)), this, SLOT(on_dlgNewChip_accepted(qint32, qint32)));
	wndNewChip.exec();
}

void MainWindow::on_dlgNewChip_accepted(qint32 rows, qint32 columns)
{
	frmConfigChip *wndConfigChip = new frmConfigChip(this);
	connect(wndConfigChip, SIGNAL(accepted(const chipConfig &)), this, SLOT(on_dlgConfigChip_accepted(const chipConfig &)));
	wndConfigChip->setDimensions(rows, columns);
	wndConfigChip->show();
	ui->pWidget->dataLoaded = false;
}

void MainWindow::on_dlgConfigChip_accepted(const chipConfig &config)
{
	displayTime = ui->pWidget->displayTime = 0;
	ui->actionLoadCommandFile->setEnabled(true);
	ui->pWidget->config = config;
//	selectFile();
}

void MainWindow::on_actionExit_triggered()
{
	this->close();
}

void MainWindow::on_actionAboutDmfbSimulator_triggered()
{
	dlgAbout wndAbout(this);
	wndAbout.exec();
}

void MainWindow::on_actionLoadCommandFile_triggered()
{
	selectFile();
}

void MainWindow::selectFile()
{
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

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
	if (ui->actionLoadCommandFile->isEnabled() && e->mimeData()->hasFormat("text/uri-list")) {
		e->acceptProposedAction();
	}
}

void MainWindow::dropEvent(QDropEvent *e)
{
	QList<QUrl> urls = e->mimeData()->urls();
	if (urls.empty()) return;
	loadFile(urls.first().toLocalFile());
}

void MainWindow::loadFile(const QString &url)
{
	QString errorMsg;

	if (!::loadFile(url, ui->pWidget->config, errorMsg, ui->pWidget->droplets, ui->pWidget->minTime, ui->pWidget->maxTime)) {
		QMessageBox::warning(this, tr("Error loading command file"), errorMsg);
		return;
	}

	ui->actionStart->setEnabled(true);
	ui->actionStep->setEnabled(true);

	displayTime = ui->pWidget->displayTime = ui->pWidget->minTime;
	ui->pWidget->dataLoaded = true;

	ui->pWidget->update();
}

void MainWindow::render()
{
	ui->pWidget->displayTime = displayTime;
	ui->pWidget->update();
}

void MainWindow::on_timer_timeout()
{
	qint64 thisTime = QDateTime::currentMSecsSinceEpoch();
	displayTime += (thisTime - lastTime) * acceleration;
	lastTime = thisTime;

	if (displayTime > ui->pWidget->maxTime) {
		displayTime = ui->pWidget->maxTime;
		timer.stop();
		ui->actionStart->setEnabled(true);
		ui->actionPause->setEnabled(false);
		ui->actionStep->setEnabled(true);
		ui->actionRevert->setEnabled(true);
		ui->actionReset->setEnabled(true);
	}

	render();
}

void MainWindow::on_actionStart_triggered()
{
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

void MainWindow::on_actionPause_triggered()
{
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

void MainWindow::on_actionStep_triggered()
{
	displayTime = qint32(floor(displayTime / 1000.0) + 1.0) * 1000;
	displayTime = std::min(displayTime, ui->pWidget->maxTime);
	render();
}

void MainWindow::on_actionRevert_triggered()
{
	displayTime = qint32(ceil(displayTime / 1000.0) - 1.0) * 1000;
	displayTime = std::max(displayTime, ui->pWidget->minTime);
	render();
}

void MainWindow::on_actionReset_triggered()
{
	displayTime = ui->pWidget->minTime;
	render();
}
