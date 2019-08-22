#include "dlgabout.h"
#include "ui_dlgabout.h"

dlgAbout::dlgAbout(QWidget *parent) : QDialog(parent), ui(new Ui::dlgAbout) {
	ui->setupUi(this);
	this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	this->window()->layout()->setSizeConstraint(QLayout::SetFixedSize);
}

dlgAbout::~dlgAbout() {
	delete ui;
}
