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
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);
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
}

void MainWindow::on_dlgConfigChip_accepted(const chipConfig &config)
{
	ui->actionLoadCommandFile->setEnabled(true);
	ui->pWidget->config = config;
	selectFile();
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
	if (e->mimeData()->hasFormat("text/uri-list")) {
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

	if (!::loadFile(url, ui->pWidget->config, errorMsg, drops, totalTime)) {
		QMessageBox::warning(this, tr("Error loading command file"), errorMsg);
		return;
	}

	for (qint32 i = 0; i < drops.size(); ++i) {
		ui->txtContent->append(QString("Drop %1:").arg(i));
		for (qint32 j = 0; j < drops[i].route.size(); ++j) {
			ui->txtContent->append(QString("Time %1, x = %2, y = %3").arg(drops[i].route[j].t).arg(drops[i].route[j].x).arg(drops[i].route[j].y));
		}
		ui->txtContent->append("------------");
	}

	ui->actionStart->setEnabled(true);
	ui->actionStep->setEnabled(true);
}
