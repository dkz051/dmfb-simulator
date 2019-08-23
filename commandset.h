#ifndef COMMANDSET_H
#define COMMANDSET_H

#include <cmath>
#include <utility>

#include <QMap>
#include <QColor>
#include <QVector>
#include <QMessageBox>
#include <QIntegerForSize>

#include "chipconfig.h"

static const qreal eps = 1e-8;
static const qreal radius = 0.4;

static const qreal soundOffset = 0.3;
static const qreal mergingTimeInterval = 1.6;
static const qreal splitStretchInterval = 1.2;

const qint32 sndFxMove = 1;
const qint32 sndFxMerge = 2;
const qint32 sndFxSplitting = 4;
const qint32 sndFxSplit = 8;

struct dropletStatus {
	qreal t; // time
	qint32 x, y; // center position
	qreal rx, ry; // x- and y- radii
	qint32 a, r, g, b; // color components
	dropletStatus();
	dropletStatus(qreal t, qint32 x, qint32 y, qreal rx, qreal ry, qint32 a, qint32 r, qint32 g, qint32 b);
};

enum commandType {
	Input, Output, Move, Mix, Merging, Merged, Splitting, Split
};

struct command {
	commandType type;
	qint32 t, x1, y1, x2, y2, x3, y3;
	command(commandType type, qint32 t, qint32 x1, qint32 y1, qint32 x2 = -1, qint32 y2 = -1, qint32 x3 = -1, qint32 y3 = -1);
};

struct errorLog {
	qint32 t;
	QString msg;
	errorLog(qint32 t, QString msg);
};

typedef QVector<dropletStatus> droplet;

typedef QMap<qreal, qint32> soundList;

dropletStatus interpolation(dropletStatus a, dropletStatus b, qreal t, qreal &x, qreal &y);

void loadFile(const QString &url, const chipConfig &config, QVector<droplet> &result, qint64 &minTime, qint64 &maxTime, soundList &sounds, errorLog &error);

void moveToPort(qint32 &x, qint32 &y, const chipConfig &config);

bool isPortType(qint32 x, qint32 y, const chipConfig &config, portType T);

bool getRealTimeStatus(const droplet &d, qreal time, dropletStatus &ans, qreal &x, qreal &y);

qreal easing(qreal t);

// Random integer within interval [L, R], both L and R included
qint32 randint(qint32 L, qint32 R);

#endif // COMMANDSET_H
