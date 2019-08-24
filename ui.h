#ifndef UI_H
#define UI_H

#include <QPainter>

#include "utility.h"

qreal getGridSize(qreal width, qreal height, qint32 rows, qint32 columns);

void renderGrid(const ChipConfig &config, qreal W, qreal H, QPainter *g);
void renderPortConfigGrid(const ChipConfig &config, qreal W, qreal H, QPainter *g);
void renderPortConfigMask(const ChipConfig &config, qreal W, qreal H, QPainter *g);
void renderPortType(const ChipConfig &config, qreal W, qreal H, QPainter *g);
void renderDroplets(const ChipConfig &config, const QVector<Droplet> &droplets, qreal time, qreal W, qreal H, QPainter *g);
void renderTime(const ChipConfig &config, qreal time, qreal maxTime, qreal W, qreal H, QPainter *g);
void renderGridAxisNumber(const ChipConfig &config, qreal W, qreal H, QPainter *g);
void renderContaminants(const ChipConfig &config, qreal W, qreal H, quint32 randSeed, const QVector<Droplet> &droplets, const QVector<QVector<QSet<qint32>>> &contaminants, QPainter *g);
void renderContaminantCount(const ChipConfig &config, qreal W, qreal H, const QVector<QVector<QSet<qint32>>> &contaminants, QPainter *g);
void renderWashObstacles(const ChipConfig &config, qreal W, qreal H, const QVector<QVector<bool>> &obstacles, QPainter *g);
void renderWash(const ChipConfig &config, qreal W, qreal H, qreal time, const QVector<Position> &steps, QColor color, QPainter *g);

#endif // UI_H
