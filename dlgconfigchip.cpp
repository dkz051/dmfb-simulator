#include "dlgconfigchip.h"
#include "ui_dlgconfigchip.h"

dlgConfigChip::dlgConfigChip(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::dlgConfigChip)
{
	ui->setupUi(this);
}

dlgConfigChip::~dlgConfigChip()
{
	delete ui;
}
