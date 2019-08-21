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
	QFile file(url);

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		QMessageBox::warning(this, tr("Error"), tr("Cannot open the command file specified."));
		return;
	}

//	QByteArray data = file.readAll();
//	ui->txtContent->setText(QString::fromLocal8Bit(data));
	QTextStream stream(&file);

	cmdCount = 0;

	while (!stream.atEnd()) {
		std::string line = stream.readLine().toStdString();

		++cmdCount;

		char buf[16];

		qint32 t, x1, y1, x2, y2;
		sscanf(line.c_str(), "%s", buf);

		switch (buf[1]) {
			case 'n': { // input
				sscanf(line.c_str(), "%s%d,%d,%d;", buf, &t, &x1, &y1);
				commands[t].push_back(command(commandType::Input, t, x1, y1, 0, 0));
				break;
			}
			case 'u': { // output
				sscanf(line.c_str(), "%s%d,%d,%d;", buf, &t, &x1, &y1);
				commands[t].push_back(command(commandType::Output, t, x1, y1, 0, 0));
				break;
			}
			case 'o': { // move
				sscanf(line.c_str(), "%s%d,%d,%d,%d,%d;", buf, &t, &x1, &y1, &x2, &y2);
				commands[t].push_back(command(commandType::Move, t, x1, y1, x2, y2));
				break;
			}
			case 'e': { // merge
				sscanf(line.c_str(), "%s%d,%d,%d,%d,%d;", buf, &t, &x1, &y1, &x2, &y2);
				commands[t].push_back(command(commandType::Merge, t, x1, y1, x2, y2));
				break;
			}
			case 'p': { // split
				sscanf(line.c_str(), "%s%d,%d,%d,%d,%d;", buf, &t, &x1, &y1, &x2, &y2);
				commands[t].push_back(command(commandType::Split, t, x1, y1, x2, y2));
				break;
			}
			case 'i': { // mix
				// TODO
				break;
			}
			default: { // invalid command
				QMessageBox::warning(this, tr("Invalid command file"), tr(QString("Unrecognized command on line %1.").arg(cmdCount).toStdString().c_str()));
				return;
			}
		}
	}

	if (cmdCount == 0) { // no command read
		QMessageBox::warning(this, tr("Invalid command file"), tr("Empty file, nothing loaded."));
	}

	this->resetId();
	this->totalTime = commands.lastKey();

	ui->actionStart->setEnabled(true);
	ui->actionStep->setEnabled(true);
}

void MainWindow::resetId()
{
	idTotal = 0;
}
