#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dlgnewchip.h"
#include "frmconfigchip.h"
#include "dlgabout.h"

#include <QMessageBox>

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
	QMessageBox::information(this, tr("Confirmed"), "Chip configuration finished");
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
