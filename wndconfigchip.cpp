#include "wndconfigchip.h"
#include "ui_wndconfigchip.h"

wndConfigChip::wndConfigChip(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::wndConfigChip)
{
	ui->setupUi(this);
}

wndConfigChip::~wndConfigChip()
{
	delete ui;
}
