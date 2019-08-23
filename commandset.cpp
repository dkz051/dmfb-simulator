#include "commandset.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>

static const qreal inf = 1e100;

command::command(commandType type, qint32 t, qint32 x1, qint32 y1, qint32 x2, qint32 y2, qint32 x3, qint32 y3) : type(type), t(t), x1(x1), y1(y1), x2(x2), y2(y2), x3(x3), y3(y3) {}

dropletStatus::dropletStatus() {}

dropletStatus::dropletStatus(qreal t, qint32 x, qint32 y, qreal rx, qreal ry, qint32 a, qint32 r, qint32 g, qint32 b) : t(t), x(x), y(y), rx(rx), ry(ry), a(a), r(r), g(g), b(b) {}

dropletStatus interpolation(dropletStatus a, dropletStatus b, qreal t, qreal &x, qreal &y) {
	if (fabs(a.t - b.t) < eps) {
		x = b.x;
		y = b.y;
		return b;
	}
	dropletStatus ans = a;
	qreal p = easing((t - a.t) / (b.t - a.t));
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

bool loadFile(const QString &url, const chipConfig &config, QString &errorMsg, QVector<droplet> &result, qint64 &minTime, qint64 &maxTime, soundList &sounds) {
	QFile file(url);
	file.open(QFile::ReadOnly | QFile::Text);
	QTextStream fs(&file);

	QMap<std::pair<qint32, qint32>, qint32> posMap;

	QVector<command> commandList;

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

		if (tokens[0] == "input") {
			commandList.push_back(command(
				commandType::Input,
				tokens[1].toInt(),
				tokens[2].toInt() - 1,
				config.rows - tokens[3].toInt()
			));
		} else if (tokens[0] == "output") {
			commandList.push_back(command(
				commandType::Output,
				tokens[1].toInt(),
				tokens[2].toInt() - 1,
				config.rows - tokens[3].toInt()
			));
		} else if (tokens[0] == "merge") {
			command cmd(
				commandType::Merging,
				tokens[1].toInt(),
				tokens[2].toInt() - 1,
				config.rows - tokens[3].toInt(),
				tokens[4].toInt() - 1,
				config.rows - tokens[5].toInt(),
				(tokens[2].toInt() + tokens[4].toInt() - 2) / 2,
				(config.rows * 2 - tokens[3].toInt() - tokens[5].toInt()) / 2
			);
			commandList.push_back(cmd);
			cmd.type = commandType::Merged;
			++cmd.t;
			commandList.push_back(cmd);
		} else if (tokens[0] == "split") {
			command cmd(
				commandType::Splitting,
				tokens[1].toInt(),
				tokens[2].toInt() - 1,
				config.rows - tokens[3].toInt(),
				tokens[4].toInt() - 1,
				config.rows - tokens[5].toInt(),
				tokens[6].toInt() - 1,
				config.rows - tokens[7].toInt()
			);
			commandList.push_back(cmd);
			cmd.type = commandType::Split;
			++cmd.t;
			commandList.push_back(cmd);
		} else if (tokens[0] == "mix") {
			for (qint32 i = 4, t = tokens[1].toInt(); i < tokens.size(); i += 2, ++t) {
				commandList.push_back(command(
					commandType::Mix,
					t,
					tokens[i - 2].toInt() - 1,
					config.rows - tokens[i - 1].toInt(),
					tokens[i].toInt() - 1,
					config.rows - tokens[i + 1].toInt()
				));
			}
		} else if (tokens[0] == "move") {
			commandList.push_back(command(
				commandType::Move,
				tokens[1].toInt(),
				tokens[2].toInt() - 1,
				config.rows - tokens[3].toInt(),
				tokens[4].toInt() - 1,
				config.rows - tokens[5].toInt()
			));
		}
	}

	std::sort(commandList.begin(), commandList.end(), [](command a, command b) -> bool { return a.t < b.t; });

	result.clear();
	sounds.clear();

	for (qint32 i = 0; i < commandList.size(); ++i) {
		command &c = commandList[i];
		if (c.type == commandType::Input) {
			if (!isPortType(c.x1, c.y1, config, portType::input)) {
				errorMsg = QString("Cannot place the droplet at time %1: position specified is not beside an input port.").arg(c.t);
				return false;
			}
			dropletStatus mnt(c.t, c.x1, c.y1, radius, radius, 0xff, randint(0, 255), randint(0, 255), randint(0, 255));

			moveToPort(c.x1, c.y1, config);
			dropletStatus mnt0(c.t - 1, c.x1, c.y1, 0, 0, 0, mnt.r, mnt.g, mnt.b);

			result.push_back(droplet({mnt0, mnt}));

			posMap[std::make_pair(mnt.x, mnt.y)] = count++;

			maxTime = std::max(maxTime, qint64(mnt.t * 1000));
			minTime = std::min(minTime, qint64(mnt0.t * 1000));
		} else if (c.type == commandType::Output) {
			if (!isPortType(c.x1, c.y1, config, portType::output)) {
				errorMsg = QString("Cannot output the droplet at time %1: position specified is not beside an output port.").arg(c.t);
				return false;
			}

			qint32 id = findIdFromPosition(c.x1, c.y1);

			if (id < 0 || id >= result.size()) {
				errorMsg = QString("Cannot output at time %1: no droplet exists at the position specified.").arg(c.t).arg(c.x1).arg(c.y1);
				return false;
			}

			auto iter = result[id].back();

			dropletStatus mnt(c.t, c.x1, c.y1, radius, radius, 0xff, iter.r, iter.g, iter.b);

			moveToPort(c.x1, c.y1, config);

			dropletStatus mnt1(c.t + 1, c.x1, c.y1, 0, 0, 0, iter.r, iter.g, iter.b);

			result[id].push_back(mnt);
			result[id].push_back(mnt1);
			posMap.remove(std::make_pair(mnt.x, mnt.y));

			maxTime = std::max(maxTime, qint64(mnt1.t * 1000));
		} else if (c.type == commandType::Move || c.type == commandType::Mix) {
			qint32 id = findIdFromPosition(c.x1, c.y1);

			if (id < 0 || id >= result.size()) {
				errorMsg = QString("Cannot %1 a droplet at time %2: no droplet exists at the position specified.").arg(c.type == commandType::Move ? "move" : "mix").arg(c.t);
				return false;
			}

			auto iter = result[id].back();

			dropletStatus mnt1(c.t, c.x1, c.y1, radius, radius, iter.a, iter.r, iter.g, iter.b),
					mnt2(c.t + 1, c.x2, c.y2, radius, radius, iter.a, iter.r, iter.g, iter.b);

			result[id].push_back(mnt1);
			result[id].push_back(mnt2);
			posMap.remove(std::make_pair(mnt1.x, mnt1.y));
			posMap[std::make_pair(mnt2.x, mnt2.y)] = id;

			maxTime = std::max(maxTime, qint64(mnt2.t * 1000));

			sounds[mnt2.t - soundOffset] |= sndFxMove;
		} else if (c.type == commandType::Merging) {
			qint32 id1 = findIdFromPosition(c.x1, c.y1);
			qint32 id2 = findIdFromPosition(c.x2, c.y2);

			if (id1 < 0 || id1 >= result.size() || id2 < 0 || id2 >= result.size()) {
				errorMsg = QString("Cannot merge at time %1: no drop exists at the position specified.").arg(c.t);
				return false;
			}

			posMap.remove(std::make_pair(c.x1, c.y1));
			posMap.remove(std::make_pair(c.x2, c.y2));

			auto iter = result[id1].back();
			auto jter = result[id2].back();
			dropletStatus s1(c.t, c.x1, c.y1, radius, radius, iter.a, iter.r, iter.g, iter.b);
			dropletStatus s2(c.t, c.x2, c.y2, radius, radius, jter.a, jter.r, jter.g, jter.b);

			result[id1].push_back(s1);
			result[id2].push_back(s2);

			dropletStatus s(
				c.t + mergingTimeInterval,
				c.x3,
				c.y3,
				radius * (abs(s1.x - s2.x) + 1),
				radius * (abs(s1.y - s2.y) + 1),
				(s1.a + s2.a) / 2,
				(s1.r + s2.r) / 2,
				(s1.g + s2.g) / 2,
				(s1.b + s2.b) / 2
			);

			result[id1].push_back(s);
			result[id2].push_back(s);

			posMap[std::make_pair(s1.x, s1.y)] = id2;
			posMap[std::make_pair(s2.x, s2.y)] = id2;
			posMap[std::make_pair(s.x, s.y)] = id2;

			maxTime = std::max(maxTime, qint64(s.t * 1000));
		} else if (c.type == commandType::Merged) {
			qint32 id = findIdFromPosition(c.x1, c.y1);

			assert(id >= 0 && id < result.size());

			posMap.remove(std::make_pair(c.x1, c.y1));
			posMap.remove(std::make_pair(c.x2, c.y2));
			posMap.remove(std::make_pair(c.x3, c.y3));

			auto iter = result[id].back();

			dropletStatus s(
				c.t + 1,
				c.x3,
				c.y3,
				radius,
				radius,
				iter.a,
				iter.r,
				iter.g,
				iter.b
			);
			result[id].push_back(s);

			posMap[std::make_pair(c.x3, c.y3)] = count++;

			result.push_back(droplet({s}));

			maxTime = std::max(maxTime, qint64(s.t * 1000));

			sounds[s.t - soundOffset] |= sndFxMerge;
		} else if (c.type == commandType::Splitting) {
			qint32 id = findIdFromPosition(c.x1, c.y1);

			if (id < 0 || id >= result.size()) {
				errorMsg = QString("Cannot split at time %1: no droplet exists at the position specified.").arg(c.t);
				return false;
			}

			auto iter = result[id].back();
			dropletStatus s(
				c.t + splitStretchInterval,
				c.x1,
				c.y1,
				radius * (abs(c.x2 - c.x3) + 1),
				radius * (abs(c.y2 - c.y3) + 1),
				iter.a,
				iter.r,
				iter.g,
				iter.b
			); // s: before split

			result[id].push_back(s);

			posMap[std::make_pair(c.x1, c.y1)] = id;
			posMap[std::make_pair(c.x2, c.y2)] = id;
			posMap[std::make_pair(c.x3, c.y3)] = id;

			maxTime = std::max(maxTime, qint64(s.t * 1000));
			sounds[s.t - soundOffset] |= sndFxSplitting;
		} else if (c.type == commandType::Split) {
			qint32 id = findIdFromPosition(c.x1, c.y1);

			assert(id >= 0 && id < result.size());

			auto iter = result[id].back();

/*			dropletStatus u, v;

			u.a = s.a;
			u.r = randint(s.r <= 127 ? 0 : 2 * s.r - 255, s.r >= 128 ? 255: 2 * s.r);
			u.g = randint(s.g <= 127 ? 0 : 2 * s.g - 255, s.g >= 128 ? 255: 2 * s.g);
			u.b = randint(s.b <= 127 ? 0 : 2 * s.b - 255, s.b >= 128 ? 255: 2 * s.b);

			u.t = s.t + 2.0;
			u.x = tk[4].toInt() - 1;
			u.y = config.rows - tk[5].toInt();
			u.rx = u.ry = 0.4;
*/
			dropletStatus u(
				c.t + 1,
				c.x2,
				c.y2,
				radius,
				radius,
				iter.a,
				iter.r <= 127 ? randint(0, 2 * iter.r) : randint(2 * iter.r - 255, 255),
				iter.g <= 127 ? randint(0, 2 * iter.g) : randint(2 * iter.g - 255, 255),
				iter.b <= 127 ? randint(0, 2 * iter.b) : randint(2 * iter.b - 255, 255)
			), v(
				c.t + 1,
				c.x3,
				c.y3,
				radius,
				radius,
				iter.a,
				2 * iter.r - u.r,
				2 * iter.g - u.g,
				2 * iter.b - u.b
			);

/*			v = u;
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
			result[id].push_back(s);*/

			qint32 nid1 = count++, nid2 = count++;
			/*droplet nd1, nd2;
			nd1.push_back(s);
			nd1.push_back(u);
			nd2.push_back(s);
			nd2.push_back(v);

			result.push_back(nd1);
			result.push_back(nd2);*/

			result.push_back(droplet({iter, u}));
			result.push_back(droplet({iter, v}));

			posMap[std::make_pair(u.x, u.y)] = nid1;
			posMap[std::make_pair(v.x, v.y)] = nid2;
			maxTime = std::max(maxTime, qint64(u.t * 1000));

			sounds[u.t - soundOffset] |= sndFxSplit;
		}
	}

	sounds[inf] = sndFxMove;
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

	qint32 D = qint32(std::lower_bound(d.begin(), d.end(), mntTmp, [](dropletStatus a, dropletStatus b) -> bool { return a.t < b.t; }) - d.begin());
	if (D >= d.size()) return false; // out of range; no longer exists
	if (D <= 0) return false; // not present yet

	ans = interpolation(d[D - 1], d[D], t, x, y);
	return true;
}

qreal easing(qreal t) {
	if (t < 0.5) {
		return pow(t * 2.0, 3.0) / 2.0;
	} else {
		return 1.0 - pow((1.0 - t) * 2.0, 3.0) / 2.0;
	}
}

qint32 randint(qint32 L, qint32 R) {
	return qint32(rand() / double(RAND_MAX) * (R - L)) + L;
}
