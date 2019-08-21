#ifndef UI_H
#define UI_H

#include <QPainter>
#include "chipconfig.h"

qreal getGridSize(qreal width, qreal height, qint32 rows, qint32 columns);

void renderGrid(const chipConfig &config, qreal W, qreal H, QPainter *g);
void renderPortConfigGrid(const chipConfig &config, qreal W, qreal H, QPainter *g);
void renderPortType(const chipConfig &config, qreal W, qreal H, QPainter *g);

#endif // UI_H
