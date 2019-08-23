#ifndef UI_H
#define UI_H

#include <QPainter>

#include "chipconfig.h"
#include "commandset.h"

qreal getGridSize(qreal width, qreal height, qint32 rows, qint32 columns);

void renderGrid(const chipConfig &config, qreal W, qreal H, QPainter *g);
void renderPortConfigGrid(const chipConfig &config, qreal W, qreal H, QPainter *g);
void renderPortConfigMask(const chipConfig &config, qreal W, qreal H, QPainter *g);
void renderPortType(const chipConfig &config, qreal W, qreal H, QPainter *g);
void renderDroplets(const chipConfig &config, const QVector<droplet> &droplets, qreal time, qreal W, qreal H, QPainter *g);
void renderTime(const chipConfig &config, qreal time, qreal maxTime, qreal W, qreal H, QPainter *g);
void renderGridAxisNumber(const chipConfig &config, qreal W, qreal H, QPainter *g);
void renderContaminants(const chipConfig &config, qreal W, qreal H, const contaminantList &contaminants, QPainter *g);

#endif // UI_H
