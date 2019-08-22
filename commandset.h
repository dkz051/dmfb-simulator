#ifndef COMMANDSET_H
#define COMMANDSET_H

#include <QIntegerForSize>
#include <QVector>
#include <QMap>
#include <QColor>
#include <QMessageBox>

#include <utility>
#include <cmath>

#include "chipconfig.h"

static const double eps = 1e-8;
static const double radius = 0.4;

struct dropletStatus
{
	qreal t; // time
	qint32 x, y; // center position
	qreal rx, ry; // x- and y- radii
	qint32 a, r, g, b; // color components
};

struct droplet
{
	QVector<dropletStatus> route;
};

dropletStatus interpolation(dropletStatus a, dropletStatus b, qreal t, qreal &x, qreal &y);

bool loadFile(const QString &url, const chipConfig &config, QString &errorMsg, QVector<droplet> &result, qint64 &minTime, qint64 &maxTime);

void moveToPort(qint32 &x, qint32 &y, const chipConfig &config);

bool isPortType(qint32 x, qint32 y, const chipConfig &config, portType T);

bool getRealTimeStatus(const droplet &d, qreal time, dropletStatus &ans, qreal &x, qreal &y);

qreal progress(qreal t);

#endif // COMMANDSET_H
