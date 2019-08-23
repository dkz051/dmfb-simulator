#include "utility.h"

#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QStringList>

// Enumerations & constants

const qreal eps = 1e-8;
const qreal inf = 1e100;

const qreal radius = 0.4;
const qreal rContaminant = 0.2;
const qint32 contaminationDots = 10;

const qreal acceleration = 1.0;

const qreal soundOffset = 0.3;
const qreal mergingTimeInterval = 1.6;
const qreal splitStretchInterval = 1.2;

const qint32 sndFxMove = 1;
const qint32 sndFxMerge = 2;
const qint32 sndFxSplitting = 4;
const qint32 sndFxSplit = 8;

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

Contaminant::Contaminant(qint32 time, qint32 id, qint32 x, qint32 y) : time(time), id(id), x(x), y(y) {}

DropletStatus::DropletStatus() {}

DropletStatus::DropletStatus(qreal t, qint32 x, qint32 y, qreal rx, qreal ry, qint32 a, qint32 h, qint32 s, qint32 v) : t(t), x(x), y(y), rx(rx), ry(ry), a(a), h(h), s(s), v(v) {}

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
	ans.h += (b.h - a.h) * p;
	ans.s += (b.s - a.s) * p;
	ans.v += (b.v - a.v) * p;
	ans.x += (b.x - a.x) * p;
	ans.y += (b.y - a.y) * p;
	ans.rx += (b.rx - a.rx) * p;
	ans.ry += (b.ry - a.ry) * p;
	x = a.x + (b.x - a.x) * p;
	y = a.y + (b.y - a.y) * p;
	return ans;
}

void loadFile(const QString &url, const ChipConfig &config, QVector<Droplet> &droplets, qint64 &minTime, qint64 &maxTime, SoundList &sounds, ErrorLog &error, ContaminantList &contaminants) {
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

	droplets.clear();
	sounds.clear();
	contaminants.clear();

	QVector<std::pair<qint32, qint32>> removeList;
	for (qint32 i = 0; i < commandList.size(); ++i) {
		Command &c = commandList[i];
		if (c.type == CommandType::Input) {
			maxTime = std::max(maxTime, c.t * qint64(1000));

			if (!isPortType(c.x1, c.y1, config, PortType::input)) {
				error = ErrorLog(c.t, QString("Cannot place a droplet on time %1, at (%2, %3): position not beside an input port.").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1));
				break;
			}
			DropletStatus mnt(c.t, c.x1, c.y1, radius, radius, 0xff, randInt(0, 359), randInt(127, 255), randInt(127, 255));

			moveToPort(c.x1, c.y1, config);
			DropletStatus mnt0(c.t - 1, c.x1, c.y1, 0, 0, 0, mnt.h, mnt.s, mnt.v);

			if (!putDroplet(mnt.x, mnt.y, count++)) {
				error = ErrorLog(c.t, QString("Cannot place a droplet on time %1, at (%2, %3): static distance constraint failed.").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1));
				break;
			}

			droplets.push_back(Droplet({mnt0, mnt}));

			minTime = std::min(minTime, qint64(mnt0.t * 1000));
			contaminants.push_back(Contaminant(c.t, count - 1, mnt.x, mnt.y));
		} else if (c.type == CommandType::Output) {
			maxTime = std::max(maxTime, qint64((c.t + 1) * 1000));

			qint32 id = findIdFromPosition(c.x1, c.y1);

			if (id < 0 || id >= droplets.size()) {
				error = ErrorLog(c.t, QString("Cannot output a droplet on time %1, at (%2, %3): no droplet here.").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1));
				break;
			}

			if (!isPortType(c.x1, c.y1, config, PortType::output)) {
				error = ErrorLog(c.t, QString("Cannot output the droplet on time %1, at (%2, %3): position not beside an input port.").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1));
				break;
			}

			auto iter = droplets[id].back();

			DropletStatus mnt(c.t, c.x1, c.y1, radius, radius, 0xff, iter.h, iter.s, iter.v);

			moveToPort(c.x1, c.y1, config);

			DropletStatus mnt1(c.t + 1, c.x1, c.y1, 0, 0, 0, iter.h, iter.s, iter.v);

			droplets[id].push_back(mnt);
			droplets[id].push_back(mnt1);
			removeList.push_back(std::make_pair(mnt.x, mnt.y));
		} else if (c.type == CommandType::Move || c.type == CommandType::Mix) {
			maxTime = std::max(maxTime, c.t * qint64(1000));

			qint32 id = findIdFromPosition(c.x1, c.y1);

			if (id < 0 || id >= droplets.size()) {
				error = ErrorLog(c.t, QString("Cannot %1 on time %2, at (%3, %4): no droplet here.").arg(c.type == CommandType::Move ? "move" : "mix").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1));
				break;
			}

			auto iter = droplets[id].back();

			DropletStatus mnt1(c.t, c.x1, c.y1, radius, radius, iter.a, iter.h, iter.s, iter.v),
				mnt2(c.t + 1, c.x2, c.y2, radius, radius, iter.a, iter.h, iter.s, iter.v);

			droplets[id].push_back(mnt1);
			removeList.push_back(std::make_pair(mnt1.x, mnt1.y));

			if (!putDroplet(mnt2.x, mnt2.y, id)) {
				error = ErrorLog(c.t, QString("Cannot %1 on time %2, from (%3, %4) to (%5, %6): dynamic distance constraint failed.").arg(c.type == CommandType::Move ? "move" : "mix").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1).arg(c.x2 + 1).arg(config.rows - c.y2));
				break;
			}

			droplets[id].push_back(mnt2);

			maxTime = std::max(maxTime, qint64(mnt2.t * 1000));

			sounds[mnt2.t - soundOffset] |= sndFxMove;

			contaminants.push_back(Contaminant(c.t + 1, id, mnt2.x, mnt2.y));
		} else if (c.type == CommandType::Merging) {
			maxTime = std::max(maxTime, c.t * qint64(1000));

			qint32 id1 = findIdFromPosition(c.x1, c.y1);
			qint32 id2 = findIdFromPosition(c.x2, c.y2);

			if (id1 < 0 || id1 >= droplets.size() || id2 < 0 || id2 >= droplets.size()) {
				error = ErrorLog(c.t, QString("Cannot merge on time %1, between (%2, %3) and (%4, %5): no droplet here.").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1).arg(c.x2 + 1).arg(config.rows - c.y2));
				break;
			}

			posMap.remove(std::make_pair(c.x1, c.y1));
			posMap.remove(std::make_pair(c.x2, c.y2));
			// Note that here we cannot use the remove-list

			auto iter = droplets[id1].back();
			auto jter = droplets[id2].back();
			DropletStatus s1(c.t, c.x1, c.y1, radius, radius, iter.a, iter.h, iter.s, iter.v);
			DropletStatus s2(c.t, c.x2, c.y2, radius, radius, jter.a, jter.h, jter.s, jter.v);

			droplets[id1].push_back(s1);
			droplets[id2].push_back(s2);

			DropletStatus s(
				c.t + mergingTimeInterval,
				c.x3,
				c.y3,
				radius * (abs(s1.x - s2.x) + 1),
				radius * (abs(s1.y - s2.y) + 1),
				(s1.a + s2.a) / 2,
				((s1.h + s2.h) / 2 + randInt(0, 1) * 180) % 360,
				(s1.s + s2.s) / 2,
				(s1.v + s2.v) / 2
			);

			droplets[id1].push_back(s);
			droplets[id2].push_back(s);

			posMap[std::make_pair(s1.x, s1.y)] = id2;
			posMap[std::make_pair(s2.x, s2.y)] = id2;
			posMap[std::make_pair(s.x, s.y)] = id2;

			assert(putDroplet(s1.x, s1.y, id2) && putDroplet(s2.x, s2.y, id2) && putDroplet(s.x, s.y, id2));

			maxTime = std::max(maxTime, qint64(s.t * 1000));
		} else if (c.type == CommandType::Merged) {
			maxTime = std::max(maxTime, qint64(c.t * 1000));

			qint32 id = findIdFromPosition(c.x3, c.y3);

			assert(id >= 0 && id < droplets.size());

			removeList.push_back(std::make_pair(c.x1, c.y1));
			removeList.push_back(std::make_pair(c.x2, c.y2));
			// (c.x3, c.y3) will be occupied later, so do not remove it here

			auto iter = droplets[id].back();

			DropletStatus s(
				c.t + 1,
				c.x3,
				c.y3,
				radius,
				radius,
				iter.a,
				iter.h,
				iter.s,
				iter.v
			);

			posMap[std::make_pair(c.x3, c.y3)] = count++; // Do not use putDroplet here

			droplets.push_back(Droplet({iter, s}));

			maxTime = std::max(maxTime, qint64(s.t * 1000));

			sounds[s.t - soundOffset] |= sndFxMerge;
			contaminants.push_back(Contaminant(c.t, count - 1, c.x3, c.y3));
		} else if (c.type == CommandType::Splitting) {
			maxTime = std::max(maxTime, c.t * qint64(1000));
			qint32 id = findIdFromPosition(c.x1, c.y1);

			if (id < 0 || id >= droplets.size()) {
				error = ErrorLog(c.t, QString("Cannot split on time %1, at (%2, %3): no droplet here.").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1));
				break;
			}

			auto iter = droplets[id].back();
			DropletStatus s(
				c.t + splitStretchInterval,
				c.x1,
				c.y1,
				radius * (abs(c.x2 - c.x3) + 1),
				radius * (abs(c.y2 - c.y3) + 1),
				iter.a,
				iter.h,
				iter.s,
				iter.v
			); // s: before split

			droplets[id].push_back(iter);

			if (!putDroplet(c.x1, c.y1, id) || !putDroplet(c.x2, c.y2, id) || !putDroplet(c.x3, c.y3, id)) {
				error = ErrorLog(c.t, QString("Cannot split on time %1, at (%2, %3): dynamic distance constraint failed.").arg(c.t).arg(c.x1 + 1).arg(config.rows - c.y1));
				break;
			}

			droplets[id].push_back(s);

			maxTime = std::max(maxTime, qint64(s.t * 1000));
			sounds[s.t - soundOffset] |= sndFxSplitting;
		} else if (c.type == CommandType::Split) {
			qint32 id = findIdFromPosition(c.x1, c.y1);

			assert(id >= 0 && id < droplets.size());

			auto iter = droplets[id].back();
			DropletStatus u(
				c.t + 1,
				c.x2,
				c.y2,
				radius,
				radius,
				iter.a,
				randInt(0, 359),
				iter.s <= 191 ? randInt(127, 2 * iter.s - 127) : randInt(2 * iter.s - 255, 255),
				iter.v <= 191 ? randInt(127, 2 * iter.v - 127) : randInt(2 * iter.v - 255, 255)
			), v(
				c.t + 1,
				c.x3,
				c.y3,
				radius,
				radius,
				iter.a,
				(2 * iter.h - u.h + 360) % 360,
				2 * iter.s - u.s,
				2 * iter.v - u.v
			);

			qint32 nid1 = count++, nid2 = count++;

			droplets.push_back(Droplet({iter, u}));
			droplets.push_back(Droplet({iter, v}));

			removeDroplet(c.x1, c.y1);
			removeDroplet(c.x2, c.y2);
			removeDroplet(c.x3, c.y3);

			assert(putDroplet(u.x, u.y, nid1) && putDroplet(v.x, v.y, nid2));

			maxTime = std::max(maxTime, qint64(u.t * 1000));

			sounds[u.t - soundOffset] |= sndFxSplit;
			contaminants.push_back(Contaminant(c.t + 1, nid1, c.x2, c.y2));
			contaminants.push_back(Contaminant(c.t + 1, nid2, c.x3, c.y3));
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
	for (qint32 i = 0; i < droplets.size(); ++i) {
		DropletStatus last = droplets[i].back();
		if (fabs(last.rx - radius) < eps && fabs(last.ry - radius) < eps) {
			last.t = timeMaximum;
			droplets[i].push_back(last); // Push a final state so that status is keeped when error occurs
		}
	}

	std::sort(contaminants.begin(), contaminants.end(), [](Contaminant a, Contaminant b) -> bool { return a.time < b.time; });
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

bool getRealTimeStatus(const Droplet &d, qreal t, DropletStatus &ans, qreal &x, qreal &y) {
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

qint32 randInt(qint32 L, qint32 R) {
	if (L > R) {
		std::swap(L, R);
	}
	qint32 ans = qint32(rand() / qreal(RAND_MAX) * (R - L + 1) + L - 0.5);
	return ans;
}

qreal randReal(qreal L, qreal R) {
	if (L > R) {
		std::swap(L, R);
	}
	return rand() / qreal(RAND_MAX) * (R - L) + L;
}
