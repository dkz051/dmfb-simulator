#include "commandset.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>

dropletStatus interpolation(dropletStatus a, dropletStatus b, qreal t, qreal &x, qreal &y) {
	if (fabs(a.t - b.t) < eps) {
		x = b.x;
		y = b.y;
		return b;
	}
	dropletStatus ans = a;
	qreal p = progress((t - a.t) / (b.t - a.t));
	ans.a += (b.a - a.a) * p;
	ans.r += (b.r - a.r) * p;
	ans.g += (b.g - a.g) * p;
	ans.b += (b.b - a.b) * p;
	ans.x += (b.x - a.x) * p;
	ans.y += (b.y - a.y) * p;
	ans.rx += (b.rx - a.rx) * p;
	ans.ry += (b.ry - a.ry) * p;
	x = a.x + (b.x - a.x) * p;
	y = a.y + (b.y - a.y) * p;
	return ans;
}

bool loadFile(const QString &url, const chipConfig &config, QString &errorMsg, QVector<droplet> &result, qint64 &minTime, qint64 &maxTime) {
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
			dropletStatus mnt, mnt0;
			mnt.t = tk[1].toDouble();
			mnt.x = tk[2].toInt() - 1;
			mnt.y = config.rows - tk[3].toInt();
			mnt.rx = mnt.ry = radius;

			if (!isPortType(mnt.x, mnt.y, config, portType::input)) {
				errorMsg = QString("On time %1, cannot place a drop at (%2, %3) which is not beside an input port.").arg(mnt.t).arg(tk[2]).arg(tk[3]);
				return false;
			}

			mnt0.t = mnt.t - 1.0;
			mnt0.x = mnt.x;
			mnt0.y = mnt.y;
			mnt0.rx = mnt0.ry = 0.0;
			moveToPort(mnt0.x, mnt0.y, config);

			mnt0.a = 0;
			mnt.a = 0xff;
			mnt0.r = mnt.r = randint(0, 255);
			mnt0.g = mnt.g = randint(0, 255);
			mnt0.b = mnt.b = randint(0, 255);

			droplet nDrop;

			nDrop.route.push_back(mnt0);
			nDrop.route.push_back(mnt);

			result.push_back(nDrop);

			posMap[std::make_pair(mnt.x, mnt.y)] = count++;

			maxTime = std::max(maxTime, qint64(mnt.t * 1000));
			minTime = std::min(minTime, qint64(mnt0.t * 1000));
		} else if (tk[0] == "output") {
			dropletStatus mnt, mnt1;
			mnt.t = tk[1].toDouble();
			mnt.x = tk[2].toInt() - 1;
			mnt.y = config.rows - tk[3].toInt();
			mnt.rx = mnt.ry = radius;

			if (!isPortType(mnt.x, mnt.y, config, portType::output)) {
				errorMsg = QString("On time %1, cannot remove the drop at (%2, %3) which is not beside an output port.").arg(mnt.t).arg(tk[2]).arg(tk[3]);
				return false;
			}

			mnt1.t = mnt.t + 1.0;
			mnt1.x = mnt.x;
			mnt1.y = mnt.y;
			mnt1.rx = mnt1.ry = 0.0;
			moveToPort(mnt1.x, mnt1.y, config);

			qint32 id = findIdFromPosition(mnt.x, mnt.y);

			if (id < 0 || id >= result.size()) {
				errorMsg = QString("On time %1, cannot remove from position (%2, %3), no drop exists.").arg(mnt.t).arg(tk[2]).arg(tk[3]);
			}

			mnt.a = 0xff;
			mnt1.a = 0;
			mnt.r = mnt1.r = result[id].route.back().r;
			mnt.g = mnt1.g = result[id].route.back().g;
			mnt.b = mnt1.b = result[id].route.back().b;

			result[id].route.push_back(mnt);
			result[id].route.push_back(mnt1);
			posMap.remove(std::make_pair(mnt.x, mnt.y));

			maxTime = std::max(maxTime, qint64(mnt1.t * 1000));
		} else if (tk[0] == "move") {
			dropletStatus mnt1, mnt2;
			mnt1.t = tk[1].toDouble();
			mnt1.x = tk[2].toInt() - 1;
			mnt1.y = config.rows - tk[3].toInt();
			mnt1.rx = mnt1.ry = radius;

			mnt2.t = mnt1.t + 1.0;
			mnt2.x = tk[4].toInt() - 1;
			mnt2.y = config.rows - tk[5].toInt();
			mnt2.rx = mnt2.ry = radius;

			qint32 id = findIdFromPosition(mnt1.x, mnt1.y);

			if (id < 0 || id >= result.size()) {
				errorMsg = QString("On time %1, cannot move from position (%2, %3) to position (%4, %5), no drop exists.").arg(mnt1.t).arg(tk[2]).arg(tk[3]).arg(tk[4]).arg(tk[5]);
				return false;
			}

			mnt1.a = mnt2.a = result[id].route.back().a;
			mnt1.r = mnt2.r = result[id].route.back().r;
			mnt1.g = mnt2.g = result[id].route.back().g;
			mnt1.b = mnt2.b = result[id].route.back().b;

			result[id].route.push_back(mnt1);
			result[id].route.push_back(mnt2);
			posMap.remove(std::make_pair(mnt1.x, mnt1.y));
			posMap[std::make_pair(mnt2.x, mnt2.y)] = id;

			maxTime = std::max(maxTime, qint64(mnt2.t * 1000));
		} else if (tk[0] == "mix") {
			dropletStatus mnt;
			mnt.t = tk[1].toDouble();
			mnt.x = tk[2].toInt() - 1;
			mnt.y = config.rows - tk[3].toInt();
			mnt.rx = mnt.ry = radius;

			qint32 id = findIdFromPosition(mnt.x, mnt.y);

			if (id < 0 || id >= result.size()) {
				errorMsg = QString("On time %1, cannot mix at position (%2, %3), no drop exists.").arg(mnt.t).arg(tk[2]).arg(tk[3]);
				return false;
			}

			mnt.a = result[id].route.back().a;
			mnt.r = result[id].route.back().r;
			mnt.g = result[id].route.back().g;
			mnt.b = result[id].route.back().b;

			result[id].route.push_back(mnt);
			posMap.remove(std::make_pair(mnt.x, mnt.y));

			for (qint32 i = 4; i < tk.size(); i += 2) {
				mnt.t += 1.0;
				mnt.x = tk[i].toInt() - 1;
				mnt.y = config.rows - tk[i + 1].toInt();
			}
			result[id].route.push_back(mnt);

			posMap[std::make_pair(mnt.x, mnt.y)] = id;
			maxTime = std::max(maxTime, qint64(mnt.t * 1000));
		} else if (tk[0] == "merge") {
			dropletStatus s1, s2; // s1, s2: before merge
			s1.t = s2.t = tk[1].toDouble();
			s1.x = tk[2].toInt() - 1;
			s1.y = config.rows - tk[3].toInt();
			s2.x = tk[4].toInt() - 1;
			s2.y = config.rows - tk[5].toInt();
			s1.rx = s1.ry = s2.rx = s2.ry = radius;

			qint32 id1 = findIdFromPosition(s1.x, s1.y);
			qint32 id2 = findIdFromPosition(s2.x, s2.y);

			if (id1 < 0 || id1 >= result.size()) {
				errorMsg = QString("On time %1, cannot merge the droplet at position (%2, %3), no drop exists.").arg(s1.t).arg(tk[2]).arg(tk[3]);
				return false;
			}

			if (id2 < 0 || id2 >= result.size()) {
				errorMsg = QString("On time %1, cannot merge the droplet at position (%2, %3), no drop exists.").arg(s2.t).arg(tk[4]).arg(tk[5]);
				return false;
			}

			s1.a = result[id1].route.back().a;
			s1.r = result[id1].route.back().r;
			s1.g = result[id1].route.back().g;
			s1.b = result[id1].route.back().b;

			s2.a = result[id2].route.back().a;
			s2.r = result[id2].route.back().r;
			s2.g = result[id2].route.back().g;
			s2.b = result[id2].route.back().b;

			result[id1].route.push_back(s1);
			result[id2].route.push_back(s2);

			posMap.remove(std::make_pair(s1.x, s1.y));
			posMap.remove(std::make_pair(s2.x, s2.y));

			s1.a = (s1.a + s2.a) / 2;
			s1.r = (s1.r + s2.r) / 2;
			s1.g = (s1.g + s2.g) / 2;
			s1.b = (s1.b + s2.b) / 2;

			s1.t += 1.6;
			if (s1.x == s2.x) {
				s1.ry = 3.0 * radius;
			} else {
				s1.rx = 3.0 * radius;
			}

			s1.x = (s1.x + s2.x) / 2;
			s1.y = (s1.y + s2.y) / 2;

			result[id1].route.push_back(s1);
			result[id2].route.push_back(s1);

			droplet nDrop;
			nDrop.route.push_back(s1);

			s2 = s1;
			s2.t += 0.4;
			s2.rx = s2.ry = radius;

			nDrop.route.push_back(s2);

			result.push_back(nDrop);

			posMap[std::make_pair(s2.x, s2.y)] = count++;
			maxTime = std::max(maxTime, qint64(s2.t * 1000));
		} else if (tk[0] == "split") {
			dropletStatus s, u, v; // s: before merge
			s.t = s.t = tk[1].toDouble();
			s.x = tk[2].toInt() - 1;
			s.y = config.rows - tk[3].toInt();
			s.rx = s.ry = 0.4;

			qint32 id = findIdFromPosition(s.x, s.y);

			if (id < 0 || id >= result.size()) {
				errorMsg = QString("On time %1, cannot merge the droplet at position (%2, %3), no drop exists.").arg(s.t).arg(tk[2]).arg(tk[3]);
				return false;
			}

			s.a = result[id].route.back().a;
			s.r = result[id].route.back().r;
			s.g = result[id].route.back().g;
			s.b = result[id].route.back().b;

			result[id].route.push_back(s);

			posMap.remove(std::make_pair(s.x, s.y));

			u.a = s.a;
			u.r = randint(s.r <= 127 ? 0 : 2 * s.r - 255, s.r >= 128 ? 255: 2 * s.r);
			u.g = randint(s.g <= 127 ? 0 : 2 * s.g - 255, s.g >= 128 ? 255: 2 * s.g);
			u.b = randint(s.b <= 127 ? 0 : 2 * s.b - 255, s.b >= 128 ? 255: 2 * s.b);

			u.t = s.t + 2.0;
			u.x = tk[4].toInt() - 1;
			u.y = config.rows - tk[5].toInt();
			u.rx = u.ry = 0.4;

			v = u;
			v.x = tk[6].toInt() - 1;
			v.y = config.rows - tk[7].toInt();

			v.r = s.r * 2 - u.r;
			v.g = s.g * 2 - u.g;
			v.b = s.b * 2 - u.b;

			if (u.x == v.x) {
				s.ry = 3.0 * radius;
			} else {
				s.rx = 3.0 * radius;
			}

			s.t += 0.8;
			result[id].route.push_back(s);

			qint32 nid1 = count++, nid2 = count++;
			droplet nd1, nd2;
			nd1.route.push_back(s);
			nd1.route.push_back(u);
			nd2.route.push_back(s);
			nd2.route.push_back(v);

			result.push_back(nd1);
			result.push_back(nd2);

			posMap[std::make_pair(u.x, u.y)] = nid1;
			posMap[std::make_pair(v.x, v.y)] = nid2;
			maxTime = std::max(maxTime, qint64(u.t * 1000));
		}
	}

	return true;
}

void moveToPort(qint32 &x, qint32 &y, const chipConfig &config) {
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

bool isPortType(qint32 x, qint32  y, const chipConfig &config, portType T) {
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

bool getRealTimeStatus(const droplet &d, qreal t, dropletStatus &ans, qreal &x, qreal &y) {
	dropletStatus mntTmp;
	mntTmp.t = t;

	qint32 D = qint32(std::lower_bound(d.route.begin(), d.route.end(), mntTmp, [](dropletStatus a, dropletStatus b) -> bool { return a.t < b.t; }) - d.route.begin());
	if (D >= d.route.size()) return false; // out of range; no longer exists
	if (D <= 0) return false; // not present yet

	ans = interpolation(d.route[D - 1], d.route[D], t, x, y);
	return true;
}

qreal progress(qreal t) {
	if (t < 0.5) {
		return pow(t * 2.0, 3.0) / 2.0;
	} else {
		return 1.0 - pow((1.0 - t) * 2.0, 3.0) / 2.0;
	}
}

qint32 randint(qint32 L, qint32 R) {
	return qint32(rand() / double(RAND_MAX) * (R - L)) + L;
}
