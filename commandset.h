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

// [mix] command is split into (n-1) move commands
enum commandType
{
	Input, Move, Split, Merge, Output
};

struct command
{
	command(commandType type, qint32 time, qint32 x1, qint32 y1, qint32 x2, qint32 y2);
	commandType type;
	qint32 time;
	qint32 x1, y1;
	qint32 x2, y2;
};

typedef QVector<command> commandSet;
typedef QMap<qint32, commandSet> cmdTimeSet;

struct drop
{
	qreal x, y;
	QColor color;
};

void coordinateTransform(qint32 x, qint32 y, qint32 rows, qint32 &ansx, qint32 &ansy);

bool assertPortType(qint32 x, qint32 y, const chipConfig &config, portType type);

struct status
{
	qint32 time;
	qint32 id;
	QMap<qint32, drop> grid;
	bool load(const cmdTimeSet &s, const chipConfig &config, QString &errorMsg);
//	bool prev(const cmdTimeSet &s, const chipConfig &config, QString &errorMsg);
	bool next(const cmdTimeSet &s, const chipConfig &config, QString &errorMsg);
//	status step(const cmdTimeSet &s, const chipConfig &config, qreal delta);
	void reset();
};

qint32 findDrop(const status &s, qint32 x, qint32 y);

#endif // COMMANDSET_H
