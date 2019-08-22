#include "frmconfigchip.h"
#include "ui_frmconfigchip.h"

#include "ui.h"

#include <QMessageBox>
#include <QPaintEvent>
#include <QMouseEvent>

frmConfigChip::frmConfigChip(QWidget *parent) : QMainWindow(parent), ui(new Ui::frmConfigChip) {
	ui->setupUi(this);
	this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);

	config.init();

	timer = new QTimer;
	timer->setInterval(25);
	connect(timer, SIGNAL(timeout()), this, SLOT(timerRefresh()));
	timer->start();

	ui->pWidget->installEventFilter(this);
}

void frmConfigChip::setDimensions(qint32 rows, qint32 columns) {
	this->rows = rows;
	this->columns = columns;
	config.init(rows, columns);
}

frmConfigChip::~frmConfigChip() {
	delete ui;
}

void frmConfigChip::on_optInput_clicked() {
	currentType = portType::input;
}

void frmConfigChip::on_optOutput_clicked() {
	currentType = portType::output;
}

void frmConfigChip::on_optWash_clicked() {
	currentType = portType::wash;
}

void frmConfigChip::on_optWaste_clicked() {
	currentType = portType::waste;
}

void frmConfigChip::on_optNone_clicked() {
	currentType = portType::none;
}

void frmConfigChip::on_buttonBox_accepted() {
	qint32 count[5] = {0, 0, 0, 0, 0};
	for (qint32 i = 0; i < config.rows; ++i) {
		++count[config.L[i]];
		++count[config.R[i]];
	}
	for (qint32 j = 0; j < config.columns; ++j) {
		++count[config.B[j]];
		++count[config.T[j]];
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
		emit accepted(config);
	}
}

bool frmConfigChip::eventFilter(QObject *o, QEvent *e) {
	if (o == ui->pWidget) {
		QWidget *p = static_cast<QWidget *>(o);
		if (e->type() == QEvent::Paint) {
			QPainter painter(p);
			painter.eraseRect(painter.window());

			qreal W = p->width(), H = p->height();

			renderPortConfigMask(config, W, H, &painter);
			renderPortType(config, W, H, &painter);
			renderGrid(config, W, H, &painter);
			return true;
		} else if (e->type() == QEvent::MouseButtonPress) {
			QMouseEvent *ev = static_cast<QMouseEvent *>(e);

			qreal W = p->width(), H = p->height();
			qint32 R = config.rows, C = config.columns;

			qreal grid = getGridSize(W, H, R, C);

			qint32 X = int(floor((ev->x() - (W - grid * C) / 2.0) / grid));
			qint32 Y = int(floor((ev->y() - (H - grid * R) / 2.0) / grid));

			if (Y >= 0 && Y < R) {
				if (X >= -2 && X <= 0 && Y + 1 < R) { // left
					config.L[Y] = currentType;
				} else if (X >= C - 1 && X <= C + 1 && Y > 0) { // right
					config.R[Y] = currentType;
				}
			}
			if (X >= 0 && X < C) {
				if (Y >= -2 && Y <= 0 && X > 0) { // top
					config.T[X] = currentType;
				} else if (Y >= R - 1 && Y <= R + 1 && X + 1 < C) { // bottom
					config.B[X] = currentType;
				}
			}
			if (Y >= -2 && Y <= 0 && X == 0) { // top-left
				config.L[0] = currentType;
			} else if (X >= C - 1 && X <= C + 1 && Y == 0) { // top-right
				config.T[C - 1] = currentType;
			} else if (Y >= R - 1 && Y <= R + 1 && X == C - 1) { // bottom-right
				config.R[R - 1] = currentType;
			} else if (X >= -2 && X <= 0 && Y == R - 1) { // bottom-left
				config.B[0] = currentType;
			}

			p->update();
			return true;
		}
	}
	return false;
}
