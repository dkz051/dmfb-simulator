#include "ui.h"

#include <QFont>
#include <QPalette>
#include <QDebug>

static const qreal minGridSize = 4.0;

qreal getGridSize(qreal width, qreal height, qint32 rows, qint32 columns) {
	width *= 0.95;
	height *= 0.95;
	// Determine the size of a single grid.
	// Port identifier (Input/Output/Wash/Waste) shall take the place of 2 grids.
	// Grid size is truncated to 4 logical pixels, or at least 4 pixels.
	return std::max(std::min(width / (columns + 4), height / (rows + 4)), minGridSize);
}

void renderGrid(const chipConfig &config, qreal W, qreal H, QPainter *g) {
	if (!config.valid) return;

	qint32 R = config.rows, C = config.columns;
	qreal grid = getGridSize(W, H, R, C);
	g->save();

	g->translate((W - grid * C) / 2.0, (H - grid * R) / 2.0);

	g->setPen(QPen(Qt::black, 1.0));
	g->setPen(Qt::PenStyle::SolidLine);

	for (qint32 i = 0; i <= R; ++i) {
		g->drawLine(QPointF(0.0, i * grid), QPointF(C * grid, i * grid));
	}
	for (qint32 j = 0; j <= C; ++j) {
		g->drawLine(QPointF(j * grid, 0.0), QPointF(j * grid, R * grid));
	}

	g->restore();
}

void renderPortConfigMask(const chipConfig &config, qreal W, qreal H, QPainter *g) {
	if (!config.valid) return;

	qint32 R = config.rows, C = config.columns;
	qreal grid = getGridSize(W, H, R, C);
	g->save();

	g->translate((W - grid * C) / 2.0, (H - grid * R) / 2.0);

	g->setPen(Qt::PenStyle::NoPen);

	g->setBrush(QColor(192, 192, 192, 255));
	g->drawRect(QRectF(grid, grid, (R - 2) * grid, (C - 2) * grid));

	g->restore();
}

void renderPortConfigGrid(const chipConfig &config, qreal W, qreal H, QPainter *g) {
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

void renderPort(qreal grid, qreal X, qreal Y, qreal W, qreal H, portType T, QPainter *g, bool withText) {
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
	g->setPen(Qt::PenStyle::NoPen);
	g->drawRect(QRectF(X * grid, Y * grid, W * grid, H * grid));

	if (withText) {
		QFont font;
		font.setPointSizeF(std::min(grid / 4.0, 20.0));
		g->setFont(font);

		g->setPen(forecolor);

		g->drawText(QRectF(X * grid, Y * grid, W * grid, H * grid), Qt::AlignCenter, str);
	}
}

void renderPortType(const chipConfig &config, qreal W, qreal H, QPainter *g) {
	if (!config.valid) return;

	qint32 R = config.rows, C = config.columns;
	qreal grid = getGridSize(W, H, R, C);
	g->save();

	g->translate((W - grid * C) / 2.0, (H - grid * R) / 2.0);

	for (qint32 i = 0; i < R; ++i) {
		renderPort(grid, 0, i, 1.0, 1.0, config.L[i], g, false);
		renderPort(grid, -2.0, i, 2.0, 1.0, config.L[i], g, true);
		renderPort(grid, C - 1.0, i, 1.0, 1.0, config.R[i], g, false);
		renderPort(grid, C, i, 2.0, 1.0, config.R[i], g, true);
	}
	for (qint32 j = 0; j < C; ++j) {
		renderPort(grid, j, 0, 1.0, 1.0, config.T[j], g, false);
		renderPort(grid, j, -2.0, 1.0, 2.0, config.T[j], g, true);
		renderPort(grid, j, R - 1.0, 1.0, 1.0, config.B[j], g, false);
		renderPort(grid, j, R, 1.0, 2.0, config.B[j], g, true);
	}

	g->restore();
}

void renderDroplets(const chipConfig &config, const QVector<droplet> &droplets, qreal time, qreal W, qreal H, QPainter *g) {
	if (!config.valid) return;

	qint32 R = config.rows, C = config.columns;
	qreal grid = getGridSize(W, H, R, C);
	g->save();

	g->translate((W - grid * C) / 2.0, (H - grid * R) / 2.0);

	g->setClipping(true);
	g->setClipRect(QRectF(0.0, 0.0, C * grid, R * grid));

	for (qint32 i = 0; i < droplets.size(); ++i) {
		dropletStatus st;
		qreal x, y;
		if (!getRealTimeStatus(droplets[i], time, st, x, y)) {
			continue;
		};
		QColor color(st.r, st.g, st.b, st.a);
		g->setPen(color);
		g->setBrush(color);
		g->drawEllipse(QPointF((x + 0.5) * grid, (y + 0.5) * grid), st.rx * grid, st.ry * grid);
	}

	g->setClipping(false);
	g->restore();
}

void renderTime(const chipConfig &config, qreal time, qreal maxTime, qreal W, qreal H, QPainter *g) {
	if (!config.valid) return;

	qint32 R = config.rows, C = config.columns;
	qreal size = getGridSize(W, H, R, C) * 0.75;

	QFont font;
	font.setPointSizeF(std::max(size, 4.0));
	g->setFont(font);
	g->setPen(Qt::black);

	bool flag = false;
	if (time < 0.0) {
		flag = true;
		time = -time;
	}
	qint32 sec = qint32(time), dex = qint32((time - sec) * 100);

	g->drawText(QRectF(0.0, 0.0, W - size * 2, H), Qt::AlignRight | Qt::AlignTop, (flag ? "-" : "") + QString("%1").arg(sec));

	font.setPointSizeF(std::max(size, 4.0) * 0.6);
	g->setFont(font);
	g->drawText(QRectF(W - size * 2, 0.0, size * 2, H), Qt::AlignLeft | Qt::AlignTop, QString(".%1").arg(dex, 2, 10, QChar('0')));

	g->setPen(QColor(192, 192, 192, 255));
	g->drawText(QRectF(0.0, size * 0.5, W, H), Qt::AlignRight | Qt::AlignTop, QString("/%1").arg(maxTime, 1, 'f', 0, QChar('0')));
}
