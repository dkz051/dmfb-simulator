#ifndef UTILITY_H
#define UTILITY_H

#include <cmath>
#include <utility>

#include <QMap>
#include <QColor>
#include <QVector>
#include <QMessageBox>
#include <QIntegerForSize>

extern const qreal eps;
extern const qreal inf;

extern const qreal radius;
extern const qreal rContaminant;
extern const qint32 contaminationDots;

extern const qreal acceleration;

extern const qreal soundOffset;
extern const qreal mergingTimeInterval;
extern const qreal splitStretchInterval;

extern const qint32 sndFxMove;
extern const qint32 sndFxMerge;
extern const qint32 sndFxSplitting;
extern const qint32 sndFxSplit;

enum CommandType {
	Input, Output, Move, Mix,
	Merging, Merged, Splitting, Split
};

enum PortType {
	none, input, output, wash, waste
};

struct ChipConfig {
	qint32 rows;
	qint32 columns;
	QVector<PortType> L, T, R, B;
	bool valid;

	void init(qint32 rows = -1, qint32 columns = -1);
};

struct Contaminant {
	qint32 time, id, x, y;
	Contaminant(qint32 time, qint32 id, qint32 x, qint32 y);
};

struct DropletStatus {
	qreal t; // time
	qint32 x, y; // center position
	qreal rx, ry; // x- and y- radii
	qint32 a, h, s, v; // color components
	DropletStatus();
	DropletStatus(qreal t, qint32 x, qint32 y, qreal rx, qreal ry, qint32 a, qint32 h, qint32 s, qint32 v);
};

struct Command {
	CommandType type;
	qint32 t, x1, y1, x2, y2, x3, y3;
	Command(CommandType type, qint32 t, qint32 x1, qint32 y1, qint32 x2 = -1, qint32 y2 = -1, qint32 x3 = -1, qint32 y3 = -1);
};

struct ErrorLog {
	qint32 t;
	QString msg;
	ErrorLog(qint32 t, QString msg);
};

typedef QVector<Contaminant> ContaminantList;
typedef QVector<DropletStatus> Droplet;
typedef QMap<qreal, qint32> SoundList;

DropletStatus interpolation(DropletStatus a, DropletStatus b, qreal t, qreal &x, qreal &y);

void loadFile(const QString &url, const ChipConfig &config, QVector<Droplet> &droplets, qint64 &minTime, qint64 &maxTime, SoundList &sounds, ErrorLog &error, ContaminantList &contaminants);

void moveToPort(qint32 &x, qint32 &y, const ChipConfig &config);

bool isPortType(qint32 x, qint32 y, const ChipConfig &config, PortType T);

bool getRealTimeStatus(const Droplet &d, qreal time, DropletStatus &ans, qreal &x, qreal &y);

qreal easing(qreal t);

// Random integer within interval [L, R], both L and R included
qint32 randInt(qint32 L, qint32 R);

qreal randReal(qreal L, qreal R);

#endif // UTILITY_H
