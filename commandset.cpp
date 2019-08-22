#include "commandset.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>

bool loadFile(const QString &url, const chipConfig &config, QString &errorMsg, QVector<drop> &result, qreal &totalTime)
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

	totalTime = 0.0;

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
			moment mnt;
			mnt.t = tk[1].toDouble();
			mnt.x = tk[2].toInt() - 1;
			mnt.y = config.rows - tk[3].toInt();

			if (!isPortType(mnt.x, mnt.y, config, portType::input)) {
				errorMsg = QString("On time %1, cannot place a drop at (%2, %3) which is not beside an input port.").arg(mnt.t).arg(tk[2]).arg(tk[3]);
				return false;
			}

			drop nDrop;
			nDrop.color = QColor(rand() & 0xff, rand() & 0xff, rand() & 0xff, 0xff);
			nDrop.route.push_back(mnt);

			result.push_back(nDrop);

			posMap[std::make_pair(mnt.x, mnt.y)] = count++;
			totalTime = std::max(totalTime, mnt.t);
		} else if (tk[0] == "output") {
			moment mnt;
			mnt.t = tk[1].toDouble();
			mnt.x = tk[2].toInt() - 1;
			mnt.y = config.rows - tk[3].toInt();

			if (!isPortType(mnt.x, mnt.y, config, portType::output)) {
				errorMsg = QString("On time %1, cannot remove the drop at (%2, %3) which is not beside an output port.").arg(mnt.t).arg(tk[2]).arg(tk[3]);
				return false;
			}

			qint32 id = findIdFromPosition(mnt.x, mnt.y);

			if (id < 0 || id >= result.size()) {
				errorMsg = QString("On time %1, cannot remove from position (%2, %3), no drop exists.").arg(mnt.t).arg(tk[2]).arg(tk[3]);
			}

			result[id].route.push_back(mnt);
			posMap.remove(std::make_pair(mnt.x, mnt.y));
			totalTime = std::max(totalTime, mnt.t);
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

			totalTime = std::max(totalTime, mnt2.t);
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
	qint32 D = qint32(std::lower_bound(d.route.begin(), d.route.end(), t, [](moment a, double b) -> bool { return a.t < b; }) - d.route.begin());
	if (D + 1 >= d.route.size()) return false; // out of range; no longer exists

	if (fabs(d.route[D + 1].t - d.route[D].t) < eps) {
		x = d.route[D + 1].x;
		y = d.route[D + 1].y;
		return true;
	}

	qreal pro = (t - d.route[D].t) / (d.route[D + 1].t - d.route[D].t);

	x = d.route[D].x + (d.route[D + 1].x - d.route[D].x) * progress(pro);
	y = d.route[D].y + (d.route[D + 1].y - d.route[D].y) * progress(pro);
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
