#include "dlgnewchip.h"

#include <QMessageBox>

#include "ui_dlgnewchip.h"

dlgNewChip::dlgNewChip(QWidget *parent) : QDialog(parent), ui(new Ui::dlgNewChip) {
	ui->setupUi(this);
	this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	this->window()->layout()->setSizeConstraint(QLayout::SetFixedSize);
}

dlgNewChip::~dlgNewChip() {
	delete ui;
}

void dlgNewChip::on_buttonBox_accepted() {
	qint32 rows = ui->boxRows->value(), columns = ui->boxColumns->value();
	if (rows <= 3 && columns <= 3) {
		QMessageBox::warning(this, tr("Invalid input"), tr("At least either 4 rows or 4 columns is required."));
	} else {
		this->accept();
		emit accepted(rows, columns);
	}
}
