#include "commandset.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>

bool loadFile(const QString &url, const chipConfig &config, QString &errorMsg, QVector<drop> &result, qint64 &minTime, qint64 &maxTime)
{
	QFile file(url);
	file.open(QFile::ReadOnly | QFile::Text);
	QTextStream fs(&file);

	QVector<QStringList> lineList;
	QMap<std::pair<qint32, qint32>, qint32> posMap;

	auto findIdFromPosition = [&](qint32 x, qint32 y) -> qint32 {
		auto pos = std::make_pair(x, y);
		if (!posMap.count(pos)) {
			return -1;
		} else {
			return posMap[pos];
		}
	};

	qint32 count = 0;

	minTime = maxTime = 0;

	while (!fs.atEnd()) {
		QStringList tokens = fs.readLine()
		                     .replace(',', ' ')
		                     .replace(';', ' ')
		                     .simplified()
		                     .toLower()
		                     .split(' ', QString::SkipEmptyParts);

		if (tokens.empty()) continue;
		assert(tokens.size() % 2 == 0);
		lineList.push_back(tokens);
	}

	std::sort(lineList.begin(), lineList.end(), [](QStringList a, QStringList b) -> bool { return a[1].toInt() < b[1].toInt(); });

	result.clear();

	for (qint32 i = 0; i < lineList.size(); ++i) {
		QStringList &tk = lineList[i];
		if (tk[0] == "input") {
			moment mnt, mnt0;
			mnt.t = tk[1].toDouble();
			mnt.x = tk[2].toInt() - 1;
			mnt.y = config.rows - tk[3].toInt();

			if (!isPortType(mnt.x, mnt.y, config, portType::input)) {
				errorMsg = QString("On time %1, cannot place a drop at (%2, %3) which is not beside an input port.").arg(mnt.t).arg(tk[2]).arg(tk[3]);
				return false;
			}

			mnt0.t = mnt.t - 1.0;
			mnt0.x = mnt.x;
			mnt0.y = mnt.y;
			moveToPort(mnt0.x, mnt0.y, config);

			drop nDrop;
			nDrop.color = QColor(rand() & 0xff, rand() & 0xff, rand() & 0xff, 0xff);

			nDrop.route.push_back(mnt0);
			nDrop.route.push_back(mnt);

			result.push_back(nDrop);

			posMap[std::make_pair(mnt.x, mnt.y)] = count++;

			maxTime = std::max(maxTime, qint64(mnt.t * 1000));
			minTime = std::min(minTime, qint64(mnt0.t * 1000));
		} else if (tk[0] == "output") {
			moment mnt, mnt1;
			mnt.t = tk[1].toDouble();
			mnt.x = tk[2].toInt() - 1;
			mnt.y = config.rows - tk[3].toInt();

			if (!isPortType(mnt.x, mnt.y, config, portType::output)) {
				errorMsg = QString("On time %1, cannot remove the drop at (%2, %3) which is not beside an output port.").arg(mnt.t).arg(tk[2]).arg(tk[3]);
				return false;
			}

			mnt1.t = mnt.t + 1.0;
			mnt1.x = mnt.x;
			mnt1.y = mnt.y;
			moveToPort(mnt1.x, mnt1.y, config);

			qint32 id = findIdFromPosition(mnt.x, mnt.y);

			if (id < 0 || id >= result.size()) {
				errorMsg = QString("On time %1, cannot remove from position (%2, %3), no drop exists.").arg(mnt.t).arg(tk[2]).arg(tk[3]);
			}

			result[id].route.push_back(mnt);
			result[id].route.push_back(mnt1);
			posMap.remove(std::make_pair(mnt.x, mnt.y));

			maxTime = std::max(maxTime, qint64(mnt1.t * 1000));
		} else if (tk[0] == "move") {
			moment mnt1, mnt2;
			mnt1.t = tk[1].toDouble();
			mnt1.x = tk[2].toInt() - 1;
			mnt1.y = config.rows - tk[3].toInt();

			mnt2.t = mnt1.t + 1.0;
			mnt2.x = tk[4].toInt() - 1;
			mnt2.y = config.rows - tk[5].toInt();

			qint32 id = findIdFromPosition(mnt1.x, mnt1.y);

			if (id < 0 || id >= result.size()) {
				errorMsg = QString("On time %1, cannot move from position (%2, %3) to position (%4, %5), no drop exists.").arg(mnt1.t).arg(tk[2]).arg(tk[3]).arg(tk[4]).arg(tk[5]);
				return false;
			}

			result[id].route.push_back(mnt1);
			result[id].route.push_back(mnt2);
			posMap.remove(std::make_pair(mnt1.x, mnt1.y));
			posMap[std::make_pair(mnt2.x, mnt2.y)] = id;

			maxTime = std::max(maxTime, qint64(mnt2.t * 1000));
		} else if (tk[0] == "mix") {
			// TODO...
		} else if (tk[0] == "merge") {
			// TODO...
		} else if (tk[0] == "split") {
			// TODO...
		}
	}

	return true;
}

void moveToPort(qint32 &x, qint32 &y, const chipConfig &config)
{
	if (x == 0 && y + 1 < config.rows) {
		--x;
	} else if (y + 1 == config.rows && x + 1 < config.columns) {
		++y;
	} else if (x + 1 == config.columns && y > 0) {
		++x;
	} else if (y == 0 && x > 0) {
		--y;
	}
}

bool isPortType(qint32 x, qint32 y, const chipConfig &config, portType T)
{
	if (x > 0 && x + 1 < config.columns && y > 0 && y + 1 < config.rows) {
		return false;
	}
	if (x == 0 && y + 1 < config.rows) {
		return config.L[y] == T;
	} else if (y + 1 == config.rows && x + 1 < config.columns) {
		return config.B[x] == T;
	} else if (x + 1 == config.columns && y > 0) {
		return config.R[y] == T;
	} else if (y == 0 && x > 0) {
		return config.T[x] == T;
	}
	return false;
}

bool getRealTimePosition(const drop &d, qreal t, qreal &x, qreal &y)
{
	moment mntTmp;
	mntTmp.t = t;

	qint32 D = qint32(std::lower_bound(d.route.begin(), d.route.end(), mntTmp, [](moment a, moment b) -> bool { return a.t < b.t; }) - d.route.begin());
	if (D >= d.route.size()) return false; // out of range; no longer exists
	if (D <= 0) return false; // not present yet

	if (fabs(d.route[D].t - d.route[D - 1].t) < eps) {
		x = d.route[D].x;
		y = d.route[D].y;
		return true;
	}

	qreal pro = (t - d.route[D - 1].t) / (d.route[D].t - d.route[D - 1].t);

	x = d.route[D - 1].x + (d.route[D].x - d.route[D - 1].x) * progress(pro);
	y = d.route[D - 1].y + (d.route[D].y - d.route[D - 1].y) * progress(pro);
	return true;
}

qreal progress(qreal t)
{
	if (t < 0.5) {
		return pow(t * 2.0, 3.0) / 2.0;
	} else {
		return 1.0 - pow((1.0 - t) * 2.0, 3.0) / 2.0;
	}
}
