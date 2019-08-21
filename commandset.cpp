#include "commandset.h"

command::command(commandType type, qint32 time, qint32 x1, qint32 y1, qint32 x2, qint32 y2) : type(type), time(time), x1(x1), y1(y1), x2(x2), y2(y2) {}

void coordinateTransform(qint32 x, qint32 y, qint32 rows, qint32 &ansx, qint32 &ansy)
{
	ansx = x - 1;
	ansy = rows - y;
}

bool assertPortType(qint32 x, qint32 y, const chipConfig &config, portType type)
{
	if ((x != 0 && x != config.columns) || (y != 0 && y != config.rows)) {
		return false;
	} else if (x == 0 && y + 1 < config.rows && config.L[y] != type) {
		return false;
	} else if (y + 1 == config.rows && x + 1 < config.columns && config.B[x] != type) {
		return false;
	} else if (x + 1 == config.columns && y > 0 && config.R[y] != type) {
		return false;
	} else if (y == 0 && x > 0 && config.T[x] != type) {
		return false;
	} else {
		return true;
	}
}

bool status::load(const cmdTimeSet &s, const chipConfig &config, QString &errorMsg)
{
	time = 0;
	grid.clear();
	if (!s.count(0)) return true;
	for (qint32 i = 0; i < s[0].size(); ++i) {
		qint32 x = s[0][i].x1, y = s[0][i].y1;
		coordinateTransform(x, y, config.rows, x, y);
		if (!assertPortType(x, y, config, portType::input)) {
			errorMsg = QString("Command: Input %1,%2,%3: Position specified is not next to an input port.").arg(time).arg(s[0][i].x1).arg(s[0][i].y1);
			return false;
		} else {
			grid[id++] = (drop){qreal(x), qreal(y), QColor(rand() & 0xff, rand() & 0xff, rand() & 0xff, 0xff)};
		}
	}
	return true;
}

bool status::next(const cmdTimeSet &s, const chipConfig &config, QString &errorMsg)
{
	++time;
}

void status::reset()
{
	id = 0;
	time = 0;
	grid.clear();
}

qint32 findDrop(const status &s, qint32 x, qint32 y) {
	for (QMap<qint32, drop>::const_iterator iter = s.grid.begin(); iter != s.grid.end(); ++iter) {
		if (fabs(iter->x - x) < eps && fabs(iter->y - y) < eps) return iter.key();
	}
	return -1;
}
