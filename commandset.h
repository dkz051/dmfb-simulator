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

struct moment
{
	qreal t;
	qint32 x, y;
};

struct drop
{
	QColor color;
	QVector<moment> route;
};

bool loadFile(const QString &url, const chipConfig &config, QString &errorMsg, QVector<drop> &result, qint64 &minTime, qint64 &maxTime);

void moveToPort(qint32 &x, qint32 &y, const chipConfig &config);

bool isPortType(qint32 x, qint32 y, const chipConfig &config, portType T);

bool getRealTimePosition(const drop &d, qreal time, qreal &x, qreal &y);

qreal progress(qreal t);

#endif // COMMANDSET_H
