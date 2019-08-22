#include "ui.h"

#include <QFont>
#include <QPalette>
#include <QDebug>

static const qreal minGridSize = 4.0;

qreal getGridSize(qreal width, qreal height, qint32 rows, qint32 columns)
{
	width *= 0.8;
	height *= 0.8;
	// Determine the size of a single grid.
	// Port identifier (Input/Output/Wash/Waste) shall take the place of 2 grids.
	// Grid size is truncated to 4 logical pixels, or at least 4 pixels.
	return std::max(std::min(width / (columns + 4), height / (rows + 4)), minGridSize);
}

void renderGrid(const chipConfig &config, qreal W, qreal H, QPainter *g)
{
	if (!config.valid) return;

	qint32 R = config.rows, C = config.columns;
	qreal grid = getGridSize(W, H, R, C);
	g->save();

	g->translate((W - grid * C) / 2.0, (H - grid * R) / 2.0);

	g->setPen(QPen(Qt::black, 1.0));

	for (qint32 i = 0; i <= R; ++i) {
		g->drawLine(QPointF(0.0, i * grid), QPointF(C * grid, i * grid));
	}
	for (qint32 j = 0; j <= C; ++j) {
		g->drawLine(QPointF(j * grid, 0.0), QPointF(j * grid, R * grid));
	}

	g->restore();
}

void renderPortConfigGrid(const chipConfig &config, qreal W, qreal H, QPainter *g)
{
	if (!config.valid) return;

	qint32 R = config.rows, C = config.columns;
	qreal grid = getGridSize(W, H, R, C);
	g->save();

	g->translate((W - grid * C) / 2.0, (H - grid * R) / 2.0);

	g->setPen(QPen(Qt::black, 1.0));

	for (qint32 i = 0; i <= R; ++i) {
		if (i < R) g->drawLine(QPointF(0.0, i * grid), QPointF(-2.0 * grid, i * grid));
		if (i > 0) g->drawLine(QPointF(C, i * grid), QPointF((C + 2.0) * grid, i * grid));
	}
	for (qint32 j = 0; j <= C; ++j) {
		if (j > 0) g->drawLine(QPointF(j * grid, 0.0), QPointF(j * grid, -2.0 * grid));
		if (j < C) g->drawLine(QPointF(j * grid, R * grid), QPointF(j * grid, (R + 2.0) * grid));
	}

	g->drawLine(QPointF(grid, -2.0 * grid), QPointF(C * grid, -2.0 * grid));
	g->drawLine(QPointF(0.0, (R + 2.0) * grid), QPointF((C - 1.0) * grid, (R + 2.0) * grid));

	g->drawLine(QPointF(-2.0 * grid, 0.0), QPointF(-2.0 * grid, (R - 1.0) * grid));
	g->drawLine(QPointF((C + 2.0) * grid, grid), QPointF((C + 2.0) * grid, R * grid));

	g->restore();
}

void renderPort(qreal grid, qreal X, qreal Y, qreal W, qreal H, portType T, QPainter *g)
{
	QColor color, forecolor;
	QString str;

	switch (T) {
		case portType::input: {
			color = Qt::yellow;
			forecolor = Qt::black;
			str = "Input";
			break;
		}
		case portType::output: {
			color = Qt::cyan;
			forecolor = Qt::black;
			str = "Output";
			break;
		}
		case portType::wash: {
			color = Qt::green;
			forecolor = Qt::black;
			str = "Wash";
			break;
		}
		case portType::waste: {
			color = Qt::red;
			forecolor = Qt::white;
			str = "Waste";
			break;
		}
		case portType::none: {
			return;
		}
	}

	g->setBrush(color);
	g->drawRect(QRectF(X * grid, Y * grid, W * grid, H * grid));

	QFont font("Segoe UI");
	font.setPointSizeF(std::min(grid / 4.0, 20.0));
	g->setFont(font);

	QPen pen(forecolor);
	g->setPen(pen);

	g->drawText(QRectF(X * grid, Y * grid, W * grid, H * grid), Qt::AlignCenter, str);
}

void renderPortType(const chipConfig &config, qreal W, qreal H, QPainter *g)
{
	if (!config.valid) return;

	qint32 R = config.rows, C = config.columns;
	qreal grid = getGridSize(W, H, R, C);
	g->save();

	g->translate((W - grid * C) / 2.0, (H - grid * R) / 2.0);

//	g->setBrush(Qt::green);
//	g->drawRect(QRectF(0, 0, 10.0, 20.0));

	QFont font("Segoe UI");

	for (qint32 i = 0; i < R; ++i) {
		renderPort(grid, -2.0, i, 2.0, 1.0, config.L[i], g);
		renderPort(grid, C, i, 2.0, 1.0, config.R[i], g);
	}
	for (qint32 j = 0; j < C; ++j) {
		renderPort(grid, j, -2.0, 1.0, 2.0, config.T[j], g);
		renderPort(grid, j, R, 1.0, 2.0, config.B[j], g);
	}

	g->restore();
}
