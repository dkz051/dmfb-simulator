#include "frmconfigchip.h"
#include "ui_frmconfigchip.h"

#include "ui.h"

#include <QMessageBox>

frmConfigChip::frmConfigChip(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmConfigChip)
{
	ui->setupUi(this);
	this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

	ui->pWidget->config.init();

	timer = new QTimer;
	timer->setInterval(25);
	connect(timer, SIGNAL(timeout()), this, SLOT(timerRefresh()));
	timer->start();
}

void frmConfigChip::setDimensions(qint32 rows, qint32 columns)
{
	this->rows = rows;
	this->columns = columns;
	ui->pWidget->config.init(rows, columns);
}

frmConfigChip::~frmConfigChip()
{
	delete ui;
}

void frmConfigChip::timerRefresh()
{
//	ui->pWidget->update();
}

void frmConfigChip::on_optInput_clicked()
{
	ui->pWidget->currentType = portType::input;
}

void frmConfigChip::on_optOutput_clicked()
{
	ui->pWidget->currentType = portType::output;
}

void frmConfigChip::on_optWash_clicked()
{
	ui->pWidget->currentType = portType::wash;
}

void frmConfigChip::on_optWaste_clicked()
{
	ui->pWidget->currentType = portType::waste;
}

void frmConfigChip::on_optNone_clicked()
{
	ui->pWidget->currentType = portType::none;
}

void frmConfigChip::on_buttonBox_accepted()
{
	qint32 count[5] = {0, 0, 0, 0, 0};
	for (qint32 i = 0; i < ui->pWidget->config.rows; ++i) {
		++count[ui->pWidget->config.L[i]];
		++count[ui->pWidget->config.R[i]];
	}
	for (qint32 j = 0; j < ui->pWidget->config.columns; ++j) {
		++count[ui->pWidget->config.B[j]];
		++count[ui->pWidget->config.T[j]];
	}
	if (count[portType::input] == 0) {
		QMessageBox::warning(this, tr("Warning"), tr("Please specify at least one input port."));
	} else if (count[portType::output] != 1) {
		QMessageBox::warning(this, tr("Warning"), tr("Please specify exactly one output port."));
	} else if (count[portType::wash] == 0 && count[portType::waste] != 0) {
		QMessageBox::warning(this, tr("Warning"), tr("Waste port is not allowed without wash input port."));
	} else if (count[portType::wash] != 0 && count[portType::waste] != 1) {
		QMessageBox::warning(this, tr("Warning"), tr("Please specify exactly one waste port with wash input port."));
	} else {
		this->close();
		emit accepted(ui->pWidget->config);
	}
}
