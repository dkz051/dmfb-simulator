#include "paintwidget.h"

#include "ui.h"

#include <cmath>
#include <QDebug>

void previewWidget::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.eraseRect(painter.window());

	qreal W = this->width(), H = this->height();

	renderPortType(config, W, H, &painter);
	renderGrid(config, W, H, &painter);
	renderPortConfigMask(config, W, H, &painter);
}

void previewWidget::mousePressEvent(QMouseEvent *e)
{
	qreal W = this->width(), H = this->height();
	qint32 R = config.rows, C = config.columns;

	qreal grid = getGridSize(W, H, R, C);

	qint32 X = int(floor((e->x() - (W - grid * C) / 2.0) / grid));
	qint32 Y = int(floor((e->y() - (H - grid * R) / 2.0) / grid));

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

	this->update();
}

displayWidget::displayWidget(QWidget *parent) : paintWidget(parent), dataLoaded(false) {}

void displayWidget::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.eraseRect(painter.window());
	painter.setRenderHints(QPainter::Antialiasing);

	qreal W = this->width(), H = this->height();

	renderPortType(config, W, H, &painter);
	if (dataLoaded) {
		renderTime(config, displayTime / 1000.0, maxTime / 1000.0, W, H, &painter);
		renderDroplets(config, droplets, displayTime / 1000.0, W, H, &painter);
	}
	renderGrid(config, W, H, &painter);
}

void displayWidget::mousePressEvent(QMouseEvent *)
{
	// TODO: Toggle obstacle (of wash droplets) status when clicked

/*	qreal W = this->width(), H = this->height();
	qint32 R = config.rows, C = config.columns;

	qreal grid = getGridSize(W, H, R, C);

	qint32 X = int(floor((e->x() - (W - grid * C) / 2.0) / grid));
	qint32 Y = int(floor((e->y() - (H - grid * R) / 2.0) / grid));

	if (Y >= 0 && Y < R) {
		if ((X == -1 || X == -2) && Y + 1 < R) { // left
			config.L[Y] = currentType;
		} else if ((X == C || X == C + 1) && Y > 0) { // right
			config.R[Y] = currentType;
		}
	} else if (X >= 0 && X < C) {
		if ((Y == -1 || Y == -2) && X > 0) { // top
			config.T[X] = currentType;
		} else if ((Y == R || Y == R + 1) && X + 1 < C) { // bottom
			config.B[X] = currentType;
		}
	}

	this->update();*/
}
