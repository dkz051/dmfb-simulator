#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dlgnewchip.h"
#include

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
	dlgNewChip wndNewChip;
	connect(&wndNewChip, SIGNAL(accepted(qint32, qint32)), this, SLOT(on_dlgNewChip_accepted(qint32, qint32)));
	connect(&wndNewChip, SIGNAL(rejected()), this, SLOT(on_dlgNewChip_rejected()));
	wndNewChip.exec();
}

void MainWindow::on_dlgNewChip_accepted(qint32 rows, qint32 columns)
{
	QString prompt = QString("Dialog returned with %1 row(s) and %2 column(s).").arg(rows).arg(columns);
	QMessageBox::information(this, tr("Confirmed"), prompt);


}

void MainWindow::on_actionExit_triggered()
{
	this->close();
}
