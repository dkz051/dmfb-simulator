#include "utility.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>

// Enumerations & constants

const qreal eps = 1e-8;
const qreal radius = 0.4;

const qreal soundOffset = 0.3;
const qreal mergingTimeInterval = 1.6;
const qreal splitStretchInterval = 1.2;

const qint32 sndFxMove = 1;
const qint32 sndFxMerge = 2;
const qint32 sndFxSplitting = 4;
const qint32 sndFxSplit = 8;
const qreal inf = 1e100;

void ChipConfig::init(qint32 rows, qint32 columns) {
	if (rows < 3 || rows > 12 || columns < 3 || columns > 12 || (rows == 3 && columns == 3)) {
		valid = false;
		return;
	} else {
		valid = true;
	}

	this->rows = rows;
	this->columns = columns;
	L.clear();
	R.clear();
	T.clear();
	B.clear();
	L.resize(rows);
	R.resize(rows);
	T.resize(columns);
	B.resize(columns);
}

Contaminant::Contaminant(qint32 id, qint32 x, qint32 y) : id(id), x(x), y(y) {}

DropletStatus::DropletStatus() {}

DropletStatus::DropletStatus(qreal t, qint32 x, qint32 y, qreal rx, qreal ry, qint32 a, qint32 r, qint32 g, qint32 b) : t(t), x(x), y(y), rx(rx), ry(ry), a(a), r(r), g(g), b(b) {}

Command::Command(CommandType type, qint32 t, qint32 x1, qint32 y1, qint32 x2, qint32 y2, qint32 x3, qint32 y3) : type(type), t(t), x1(x1), y1(y1), x2(x2), y2(y2), x3(x3), y3(y3) {}

ErrorLog::ErrorLog(qint32 t, QString msg) : t(t), msg(msg) {}

DropletStatus interpolation(DropletStatus a, DropletStatus b, qreal t, qreal &x, qreal &y) {
	if (fabs(a.t - b.t) < eps) {
		x = b.x;
		y = b.y;
		return b;
	}
	DropletStatus ans = a;
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

void loadFile(const QString &url, const ChipConfig &config, QVector<droplet> &result, qint64 &minTime, qint64 &maxTime, soundList &sounds, ErrorLog &error) {
	QFile file(url);
	file.open(QFile::ReadOnly | QFile::Text);
	QTextStream fs(&file);

	QMap<std::pair<qint32, qint32>, qint32> posMap;

	QVector<Command> commandList;

	auto findIdFromPosition = [&](qint32 x, qint32 y) -> qint32 {
		auto pos = std::make_pair(x, y);
		if (!posMap.count(pos)) {
			return -1;
		} else {
			return posMap[pos];
		}
	};

	auto putDroplet = [&](qint32 x, qint32 y, qint32 id) -> bool {
		const qint32 dirx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
		const qint32 diry[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
		auto pos = std::make_pair(x, y);
		for (qint32 k = 0; k < 8; ++k) {
			qint32 xx = x + dirx[k], yy = y + diry[k];
			auto pok = std::make_pair(xx, yy);
			if (posMap.count(pok) && posMap[pok] != id) {
				return false;
			}
		}
		posMap[pos] = id;
		return true;
	};

	auto removeDroplet = [&](qint32 x, qint32 y) {
		posMap.remove(std::make_pair(x, y));
	};

	qint32 count = 0;

	minTime = maxTime = 0;

	error = ErrorLog(-2, "");

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
			commandList.push_back(Command(
				CommandType::Input,
				tokens[1].toInt(),
				tokens[2].toInt() - 1,
				config.rows - tokens[3].toInt()
			));
		} else if (tokens[0] == "output") {
			commandList.push_back(Command(
				CommandType::Output,
				tokens[1].toInt(),
				tokens[2].toInt() - 1,
				config.rows - tokens[3].toInt()
			));
		} else if (tokens[0] == "merge") {
			Command cmd(
				CommandType::Merging,
				tokens[1].toInt(),
				tokens[2].toInt() - 1,
				config.rows - tokens[3].toInt(),
				tokens[4].toInt() - 1,
				config.rows - tokens[5].toInt(),
				(tokens[2].toInt() + tokens[4].toInt() - 2) / 2,
				(config.rows * 2 - tokens[3].toInt() - tokens[5].toInt()) / 2
			);
			commandList.push_back(cmd);
			cmd.type = CommandType::Merged;
			++cmd.t;
			commandList.push_back(cmd);
		} else if (tokens[0] == "split") {
			Command cmd(
				CommandType::Splitting,
				tokens[1].toInt(),
				tokens[2].toInt() - 1,
				config.rows - tokens[3].toInt(),
				tokens[4].toInt() - 1,
				config.rows - tokens[5].toInt(),
				tokens[6].toInt() - 1,
				config.rows - tokens[7].toInt()
			);
			commandList.push_back(cmd);
			cmd.type = CommandType::Split;
			++cmd.t;
			commandList.push_back(cmd);
		} else if (tokens[0] == "mix") {
			for (qint32 i = 4, t = tokens[1].toInt(); i < tokens.size(); i += 2, ++t) {
				commandList.push_back(Command(
					CommandType::Mix,
					t,
					tokens[i - 2].toInt() - 1,
					config.rows - tokens[i - 1].toInt(),
					tokens[i].toInt() - 1,
					config.rows - tokens[i + 1].toInt()
				));
			}
		} else if (tokens[0] == "move") {
			commandList.push_back(Command(
				CommandType::Move,
				tokens[1].toInt(),
				tokens[2].toInt() - 1,
				config.rows - tokens[3].toInt(),
				tokens[4].toInt() - 1,
				config.rows - tokens[5].toInt()
			));
		}
	}

	std::sort(commandList.begin(), commandList.end(), [](Command a, Command b) -> bool { return a.t < b.t; });

	result.clear();
	sounds.clear();

	QVector<std::pair<qint32, qint32>> removeList;
	for (qint32 i = 0; i < commandList.size(); ++i) {
		Command &c = commandList[i];
		if (c.type == CommandType::Input) {
			maxTime = std::max(maxTime, c.t * qint64(1000));

			if (!isPortType(c.x1, c.y1, config, PortType::input)) {
				error = ErrorLog(c.t, QString("Cannot place a droplet on time %1, at (%2, %3): position not beside an input port.").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1));
				break;
			}
			DropletStatus mnt(c.t, c.x1, c.y1, radius, radius, 0xff, randint(0, 255), randint(0, 255), randint(0, 255));

			moveToPort(c.x1, c.y1, config);
			DropletStatus mnt0(c.t - 1, c.x1, c.y1, 0, 0, 0, mnt.r, mnt.g, mnt.b);

			if (!putDroplet(mnt.x, mnt.y, count++)) {
				error = ErrorLog(c.t, QString("Cannot place a droplet on time %1, at (%2, %3): static distance constraint failed.").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1));
				break;
			}

			result.push_back(droplet({mnt0, mnt}));

			minTime = std::min(minTime, qint64(mnt0.t * 1000));
		} else if (c.type == CommandType::Output) {
			maxTime = std::max(maxTime, qint64((c.t + 1) * 1000));

			qint32 id = findIdFromPosition(c.x1, c.y1);

			if (id < 0 || id >= result.size()) {
				error = ErrorLog(c.t, QString("Cannot output a droplet on time %1, at (%2, %3): no droplet here.").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1));
				break;
			}

			if (!isPortType(c.x1, c.y1, config, PortType::output)) {
				error = ErrorLog(c.t, QString("Cannot output the droplet on time %1, at (%2, %3): position not beside an input port.").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1));
				break;
			}

			auto iter = result[id].back();

			DropletStatus mnt(c.t, c.x1, c.y1, radius, radius, 0xff, iter.r, iter.g, iter.b);

			moveToPort(c.x1, c.y1, config);

			DropletStatus mnt1(c.t + 1, c.x1, c.y1, 0, 0, 0, iter.r, iter.g, iter.b);

			result[id].push_back(mnt);
			result[id].push_back(mnt1);
			removeList.push_back(std::make_pair(mnt.x, mnt.y));
		} else if (c.type == CommandType::Move || c.type == CommandType::Mix) {
			maxTime = std::max(maxTime, c.t * qint64(1000));

			qint32 id = findIdFromPosition(c.x1, c.y1);

			if (id < 0 || id >= result.size()) {
				error = ErrorLog(c.t, QString("Cannot %1 on time %2, at (%3, %4): no droplet here.").arg(c.type == CommandType::Move ? "move" : "mix").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1));
				break;
			}

			auto iter = result[id].back();

			DropletStatus mnt1(c.t, c.x1, c.y1, radius, radius, iter.a, iter.r, iter.g, iter.b),
				mnt2(c.t + 1, c.x2, c.y2, radius, radius, iter.a, iter.r, iter.g, iter.b);

			result[id].push_back(mnt1);
			removeList.push_back(std::make_pair(mnt1.x, mnt1.y));

			if (!putDroplet(mnt2.x, mnt2.y, id)) {
				error = ErrorLog(c.t, QString("Cannot %1 on time %2, from (%3, %4) to (%5, %6): dynamic distance constraint failed.").arg(c.type == CommandType::Move ? "move" : "mix").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1).arg(c.x2 + 1).arg(config.rows - c.y2));
				break;
			}

			result[id].push_back(mnt2);

			maxTime = std::max(maxTime, qint64(mnt2.t * 1000));

			sounds[mnt2.t - soundOffset] |= sndFxMove;
		} else if (c.type == CommandType::Merging) {
			maxTime = std::max(maxTime, c.t * qint64(1000));

			qint32 id1 = findIdFromPosition(c.x1, c.y1);
			qint32 id2 = findIdFromPosition(c.x2, c.y2);

			if (id1 < 0 || id1 >= result.size() || id2 < 0 || id2 >= result.size()) {
				error = ErrorLog(c.t, QString("Cannot merge on time %1, between (%2, %3) and (%4, %5): no droplet here.").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1).arg(c.x2 + 1).arg(config.rows - c.y2));
				break;
			}

			posMap.remove(std::make_pair(c.x1, c.y1));
			posMap.remove(std::make_pair(c.x2, c.y2));
			// Note that here we cannot use the remove-list

			auto iter = result[id1].back();
			auto jter = result[id2].back();
			DropletStatus s1(c.t, c.x1, c.y1, radius, radius, iter.a, iter.r, iter.g, iter.b);
			DropletStatus s2(c.t, c.x2, c.y2, radius, radius, jter.a, jter.r, jter.g, jter.b);

			result[id1].push_back(s1);
			result[id2].push_back(s2);

			DropletStatus s(
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

			assert(putDroplet(s1.x, s1.y, id2) && putDroplet(s2.x, s2.y, id2) && putDroplet(s.x, s.y, id2));

			maxTime = std::max(maxTime, qint64(s.t * 1000));
		} else if (c.type == CommandType::Merged) {
			maxTime = std::max(maxTime, qint64(c.t * 1000));

			qint32 id = findIdFromPosition(c.x3, c.y3);

			assert(id >= 0 && id < result.size());

			removeList.push_back(std::make_pair(c.x1, c.y1));
			removeList.push_back(std::make_pair(c.x2, c.y2));
			// (c.x3, c.y3) will be occupied later, so do not remove it here

			auto iter = result[id].back();

			DropletStatus s(
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

			posMap[std::make_pair(c.x3, c.y3)] = count++; // Do not use putDroplet here

			result.push_back(droplet({iter, s}));

			maxTime = std::max(maxTime, qint64(s.t * 1000));

			sounds[s.t - soundOffset] |= sndFxMerge;
		} else if (c.type == CommandType::Splitting) {
			maxTime = std::max(maxTime, c.t * qint64(1000));
			qint32 id = findIdFromPosition(c.x1, c.y1);

			if (id < 0 || id >= result.size()) {
				error = ErrorLog(c.t, QString("Cannot split on time %1, at (%2, %3): no droplet here.").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1));
				break;
			}

			auto iter = result[id].back();
			DropletStatus s(
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

			result[id].push_back(iter);

			if (!putDroplet(c.x1, c.y1, id) || !putDroplet(c.x2, c.y2, id) || !putDroplet(c.x3, c.y3, id)) {
				error = ErrorLog(c.t, QString("Cannot split on time %1, at (%2, %3): dynamic distance constraint failed.").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1));
				break;
			}

			result[id].push_back(s);

			maxTime = std::max(maxTime, qint64(s.t * 1000));
			sounds[s.t - soundOffset] |= sndFxSplitting;
		} else if (c.type == CommandType::Split) {
			qint32 id = findIdFromPosition(c.x1, c.y1);

			assert(id >= 0 && id < result.size());

			auto iter = result[id].back();
			DropletStatus u(
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

			qint32 nid1 = count++, nid2 = count++;

			result.push_back(droplet({iter, u}));
			result.push_back(droplet({iter, v}));

			removeDroplet(c.x1, c.y1);
			removeDroplet(c.x2, c.y2);
			removeDroplet(c.x3, c.y3);

			assert(putDroplet(u.x, u.y, nid1) && putDroplet(v.x, v.y, nid2));

			maxTime = std::max(maxTime, qint64(u.t * 1000));

			sounds[u.t - soundOffset] |= sndFxSplit;
		}

		if (i + 1 == commandList.size() || commandList[i + 1].t != commandList[i].t) {
			for (qint32 i = 0; i < removeList.size(); ++i) {
				removeDroplet(removeList[i].first, removeList[i].second);
			}
			removeList.clear();
		}
	}

	sounds[inf] = sndFxMove;

	qint32 timeMaximum = commandList.back().t + 1;
	for (qint32 i = 0; i < result.size(); ++i) {
		DropletStatus last = result[i].back();
		if (fabs(last.rx - radius) < eps && fabs(last.ry - radius) < eps) {
			last.t = timeMaximum;
			result[i].push_back(last); // Push a final state so that status is keeped when error occurs
		}
	}
}

void moveToPort(qint32 &x, qint32 &y, const ChipConfig &config) {
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

bool isPortType(qint32 x, qint32  y, const ChipConfig &config, PortType T) {
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

bool getRealTimeStatus(const droplet &d, qreal t, DropletStatus &ans, qreal &x, qreal &y) {
	DropletStatus mntTmp;
	mntTmp.t = t;

	qint32 D = qint32(std::lower_bound(d.begin(), d.end(), mntTmp, [](DropletStatus a, DropletStatus b) -> bool { return a.t < b.t; }) - d.begin());
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