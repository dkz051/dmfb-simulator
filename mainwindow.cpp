#include "mainwindow.h"

#include <string>

#include <QFile>
#include <QDebug>
#include <QStack>
#include <QQueue>
#include <QMimeData>
#include <QDropEvent>
#include <QTextStream>
#include <QMessageBox>
#include <QFileDialog>
#include <QDragEnterEvent>

#include "dlgabout.h"
#include "dlgnewchip.h"
#include "frmconfigchip.h"

#include "ui_mainwindow.h"

#include "ui.h"
#include "utility.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent), ui(new Ui::MainWindow),
	dataLoaded(false),
	sndMove("qrc:/sounds/move.wav"), sndMerge("qrc:/sounds/merge.wav"), sndSplitting("qrc:/sounds/splitting.wav"), sndSplit("qrc:/sounds/split.wav"), sndError("qrc:/sounds/error.wav"),
	error(-2, ""),
	timerRun(this), timerWash(this) {
	ui->setupUi(this);

	timerRun.setInterval(25);
	connect(&timerRun, SIGNAL(timeout()), this, SLOT(onRunTimeout()));

	timerWash.setInterval(25);
	connect(&timerWash, SIGNAL(timeout()), this, SLOT(onWashTimeout()));

	ui->picDisplay->installEventFilter(this);

	ui->lblWashObstacleHints->setVisible(false);

	sndMove.setLoops(1);
	sndMerge.setLoops(1);
	sndSplit.setLoops(1);
	sndSplitting.setLoops(1);
	sndError.setLoops(1);

	config.valid = false;
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_actionNewChip_triggered() {
	dlgNewChip wndNewChip(this);
	connect(&wndNewChip, SIGNAL(accepted(qint32, qint32)), this, SLOT(onDlgNewChipAccepted(qint32, qint32)));
	wndNewChip.exec();
}

void MainWindow::onDlgNewChipAccepted(qint32 rows, qint32 columns) {
	ui->actionLoadCommandFile->setEnabled(false);
	ui->actionStart->setEnabled(false);
	ui->actionPause->setEnabled(false);
	ui->actionStep->setEnabled(false);
	ui->actionRevert->setEnabled(false);
	ui->actionReset->setEnabled(false);
	ui->actionWash->setEnabled(false);
	ui->lblWashObstacleHints->setVisible(false);

	dataLoaded = false;

	clearObstacles();
	clearContaminants();

	frmConfigChip *wndConfigChip = new frmConfigChip(this);
	connect(wndConfigChip, SIGNAL(accepted(const ChipConfig &)), this, SLOT(onDlgConfigChipAccepted(const ChipConfig &)));
	wndConfigChip->setDimensions(rows, columns);
	wndConfigChip->show();
}

void MainWindow::onDlgConfigChipAccepted(const ChipConfig &config) {
	displayTime = 0;
	this->config = config;
	ui->actionLoadCommandFile->setEnabled(true);

	clearObstacles();
	clearContaminants();
	selectFile();
}

void MainWindow::on_actionExit_triggered() {
	this->close();
}

void MainWindow::on_actionAboutDmfbSimulator_triggered() {
	dlgAbout wndAbout(this);
	wndAbout.exec();
}

void MainWindow::on_actionLoadCommandFile_triggered() {
	selectFile();
}

void MainWindow::selectFile() {
	QFileDialog *fileDlg = new QFileDialog(this);
	fileDlg->setWindowTitle(tr("Open Command File"));
	fileDlg->setDirectory(".");
	fileDlg->setNameFilter("All Files (*.*)");
	fileDlg->setFileMode(QFileDialog::ExistingFile);
	if (fileDlg->exec()) {
		QStringList fileName = fileDlg->selectedFiles();
		if (fileName.empty()) return;
		loadFile(fileName[0]);
	}
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
	if (ui->actionLoadCommandFile->isEnabled() && e->mimeData()->hasFormat("text/uri-list")) {
		e->acceptProposedAction();
	}
}

void MainWindow::dropEvent(QDropEvent *e) {
	QList<QUrl> urls = e->mimeData()->urls();
	if (urls.empty()) return;
	loadFile(urls.first().toLocalFile());
}

void MainWindow::loadFile(const QString &url) {
	::loadFile(url, config, droplets, minTime, maxTime, sounds, error, contaminants);

	srand(quint32(QDateTime::currentMSecsSinceEpoch()));

	randSeed = quint32(rand());

	clearObstacles();
	clearContaminants();

	washColor = QColor::fromHsv(randInt(0, 359), randInt(127, 255), randInt(127, 255), 0xff);

	maxTime = (maxTime / 1000) * 1000; // Truncate to seconds (in case any command failed)

	ui->actionStart->setEnabled(true);
	ui->actionStep->setEnabled(true);

	if (config.hasWash) {
		ui->actionWash->setEnabled(true);
	}

	displayTime = minTime;
	dataLoaded = true;

	this->update();
}

void MainWindow::render() {
	this->update();
}

void MainWindow::onRunTimeout() {
	qint64 thisTime = QDateTime::currentMSecsSinceEpoch();
	qint64 lastDisplay = displayTime;

	displayTime += (thisTime - lastTime) * runAcceleration;
	lastTime = thisTime;

	auto iter = sounds.lowerBound(lastDisplay / 1000.0);
	auto jter = sounds.lowerBound(displayTime / 1000.0);

	if (iter != jter) {
		playSound(iter.value());
	}

	auto kter = std::lower_bound(contaminants.begin(), contaminants.end(), lastDisplay / 1000.0, [](Contaminant a, qreal t) -> bool { return a.time < t; });
	auto lter = std::lower_bound(contaminants.begin(), contaminants.end(), displayTime / 1000.0, [](Contaminant a, qreal t) -> bool { return a.time < t; });

	for (auto it = kter; it != lter; ++it) {
		contamination[it->x][it->y].insert(it->id);
	}

	if (displayTime > maxTime) {
		displayTime = maxTime;
		on_actionPause_triggered();
	}

	if (floor(lastDisplay / 1000.0) < error.t && floor(displayTime / 1000.0) >= error.t) {
		on_actionPause_triggered();
		sndError.play();
		QMessageBox::warning(this, tr("Error"), error.msg);
	}

	render();
}

void MainWindow::on_actionStart_triggered() {
	timerRun.start();
	lastTime = QDateTime::currentMSecsSinceEpoch();

	ui->actionStart->setEnabled(false);
	ui->actionPause->setEnabled(true);
	ui->actionStep->setEnabled(false);
	ui->actionRevert->setEnabled(false);
	ui->actionReset->setEnabled(false);

	ui->actionNewChip->setEnabled(false);
	ui->actionLoadCommandFile->setEnabled(false);
	ui->lblWashObstacleHints->setVisible(false);
}

void MainWindow::on_actionPause_triggered() {
	timerRun.stop();

	displayTime = qint32(floor(displayTime / 1000.0)) * 1000; // truncate to last second

	ui->actionStart->setEnabled(true);
	ui->actionPause->setEnabled(false);
	ui->actionStep->setEnabled(true);
	ui->actionRevert->setEnabled(true);
	ui->actionReset->setEnabled(true);

	ui->actionNewChip->setEnabled(true);
	ui->actionLoadCommandFile->setEnabled(true);

	if (config.hasWash) {
		ui->actionWash->setEnabled(true);
		ui->lblWashObstacleHints->setVisible(true);
	}

	render();
}

void MainWindow::on_actionStep_triggered() {
	if (displayTime / 1000 == error.t) {
		on_actionPause_triggered();
		sndError.play();
		QMessageBox::warning(this, tr("Error"), error.msg);
	}

	displayTime = qint32(floor(displayTime / 1000.0) + 1.0) * 1000;
	displayTime = std::min(displayTime, maxTime);

	auto iter = std::lower_bound(contaminants.begin(), contaminants.end(), displayTime / 1000.0, [](Contaminant a, qreal t) -> bool { return a.time < t; });
	auto jter = std::upper_bound(contaminants.begin(), contaminants.end(), displayTime / 1000.0, [](qreal t, Contaminant a) -> bool { return t < a.time; });

	for (auto it = iter; it != jter; ++it) {
		contamination[it->x][it->y].insert(it->id);
	}

	render();
}

void MainWindow::on_actionRevert_triggered() {
	displayTime = qint32(ceil(displayTime / 1000.0) - 1.0) * 1000;
	displayTime = std::max(displayTime, minTime);
	render();
}

void MainWindow::on_actionReset_triggered() {
	displayTime = minTime;
	ui->actionNewChip->setEnabled(true);
	ui->actionLoadCommandFile->setEnabled(true);

	if (config.hasWash) {
		ui->actionWash->setEnabled(true);
		ui->lblWashObstacleHints->setVisible(true);
	}
	render();
}

bool MainWindow::eventFilter(QObject *o, QEvent *e) {
	if (o == ui->picDisplay) {
		QWidget *p = static_cast<QWidget *>(o);
		if (e->type() == QEvent::Paint) {
			QPainter painter(p);
			painter.eraseRect(painter.window());
			painter.setRenderHints(QPainter::Antialiasing);

			qreal W = p->width(), H = p->height();

			renderPortType(config, W, H, &painter);
			renderGridAxisNumber(config, W, H, &painter);

			if (dataLoaded) {
				renderTime(config, displayTime / 1000.0, maxTime / 1000.0, W, H, &painter);
				renderWashObstacles(config, W, H, obstacles, &painter);
				renderContaminants(config, W, H, randSeed, droplets, contamination, &painter);
				renderDroplets(config, droplets, displayTime / 1000.0, W, H, &painter);
				if (!timerRun.isActive() && displayTime == maxTime) {
					renderContaminantCount(config, W, H, contamination, &painter);
				}
				if (timerWash.isActive()) {
					renderWash(config, W, H, curWashTime / 1000.0, steps, washColor, &painter);
				}
			}
			renderGrid(config, W, H, &painter);
			return true;
		} else if (e->type() == QEvent::MouseButtonPress) {
			if (timerRun.isActive() || timerWash.isActive() || !config.hasWash) {
				return false; // Cannot set obstacles when running/washing or if no wash/waste port exists
			}

			QMouseEvent *ev = static_cast<QMouseEvent *>(e);

			qreal W = p->width(), H = p->height();
			qint32 R = config.rows, C = config.columns;

			qreal grid = getGridSize(W, H, R, C);

			qint32 X = qint32(floor((ev->x() - (W - grid * C) / 2.0) / grid));
			qint32 Y = qint32(floor((ev->y() - (H - grid * R) / 2.0) / grid));

			if (X < 0 || X >= config.columns || Y < 0 || Y >= config.rows) {
				return false;
			}

			if (isPortType(X, Y, config, PortType::wash) || isPortType(X, Y, config, PortType::waste)) {
				obstacles[X][Y] = false;
			} else {
				obstacles[X][Y] = !obstacles[X][Y];
			}

			p->update();
			return true;
		}
	}
	return false;
}

void MainWindow::playSound(qint32 sounds) {
	sndMove.stop();
	sndMerge.stop();
	sndSplit.stop();
	sndSplitting.stop();
	if (sounds & sndFxMove) {
		sndMove.play();
	}
	if (sounds & sndFxMerge) {
		sndMerge.play();
	}
	if (sounds & sndFxSplit) {
		sndSplit.play();
	}
	if (sounds & sndFxSplitting) {
		sndSplitting.play();
	}
}

void MainWindow::clearObstacles() {
	obstacles.clear();
	if (config.rows > 0 && config.columns > 0) {
		obstacles.resize(config.columns);
		for (qint32 i = 0; i < config.columns; ++i) {
			obstacles[i].resize(config.rows);
			for (qint32 j = 0; j < config.rows; ++j) {
				obstacles[i][j] = false;
			}
		}
	}
}

void MainWindow::clearContaminants() {
	contamination.clear();

	if (config.rows > 0 && config.columns > 0) {
		contamination.resize(config.columns);

		for (qint32 i = 0; i < config.columns; ++i) {
			contamination[i].resize(config.rows);
		}
	}
}

bool MainWindow::wash(QVector<Position> &steps) {
	// Step 1: mark obstacles
	auto ob = obstacles;
	for (qint32 i = 0; i < droplets.size(); ++i) {
		DropletStatus pos;
		qreal x, y;
		if (getRealTimeStatus(droplets[i], displayTime / 1000.0, pos, x, y)) {
			for (qint32 dX = qint32(ceil(x - pos.rx)); dX <= qint32(floor(x + pos.rx)); ++dX) {
				for (qint32 dY = qint32(ceil(y - pos.ry)); dY <= qint32(floor(y + pos.ry)); ++dY) {
					for (qint32 k = 0; k < 8; ++k) {
						qint32 xx = dX + dirX[k], yy = dY + dirY[k];
						if (xx >= 0 && xx < config.columns && yy >= 0 && yy < config.rows) {
							ob[xx][yy] = true;
						}
						ob[pos.x][pos.y] = true;
					}
				}
			}
		}
	}
	// Step 2: find [wash input] and [waste] ports
	qint32 sx = -1, sy = -1, tx = -1, ty = -1;
	for (qint32 i = 0; i < config.columns; ++i) {
		for (qint32 j = 0; j < config.rows; ++j) {
			if (isPortType(i, j, config, PortType::wash)) {
				sx = i;
				sy = j;
			} else if (isPortType(i, j, config, PortType::waste)) {
				tx = i;
				ty = j;
			}
		}
	}
	assert(sx >= 0 && sy >= 0 && tx >= 0 && ty >= 0);
	if (ob[sx][sy] || ob[tx][ty]) {
		QMessageBox::warning(this, tr("Error washing"), tr("Cannot wash the chip: no valid route."));
		return false;
	}
	// Step 3: initialize [distance] matrix
	QVector<QVector<qint32>> distance;
	distance.resize(config.columns);
	for (qint32 i = 0; i < config.columns; ++i) {
		distance[i].resize(config.rows);
		for (qint32 j = 0; j < config.rows; ++j) {
			distance[i][j] = -1;
		}
	}
	distance[sx][sy] = 0;
	// Step 4: find the route
	QVector<QVector<bool>> washed;
	washed.resize(config.columns);
	for (qint32 i = 0; i < config.columns; ++i) {
		washed[i].resize(config.rows);
		for (qint32 j = 0; j < config.rows; ++j) {
			washed[i][j] = false;
		}
	}

	steps.clear();
	QQueue<Position> queue;
	queue.enqueue(Position(sx, sy));
	steps.push_back(Position(sx, sy));

	qint32 ssx = sx, ssy = sy;
	moveToPort(ssx, ssy, config);
	steps.push_front(Position(ssx, ssy));
	while (!queue.empty()) {
		Position pos = queue.dequeue();
		for (qint32 k = 0; k < 4; ++k) {
			qint32 nx = pos.first + dirX[k], ny = pos.second + dirY[k];
			if (nx < 0 || nx >= config.columns || ny < 0 || ny >= config.rows) {
				continue;
			}
			if (ob[nx][ny]) {
				continue;
			}
			if (distance[nx][ny] == -1 || distance[nx][ny] > distance[pos.first][pos.second] + 1) {
				distance[nx][ny] = distance[pos.first][pos.second] + 1;

				if (contamination[nx][ny].size() > 0 && !washed[nx][ny]) {
					washRoute(config, steps, nx, ny, ob);
					washed[nx][ny] = true;
					queue.push_front(Position(nx, ny));
				} else {
					queue.enqueue(Position(nx, ny));
				}
			}
		}
	}
	if (distance[tx][ty] == -1) {
		QMessageBox::warning(this, tr("Error washing"), tr("Cannot wash the chip: no valid route."));
		return false;
	}
	if (steps.size() <= 2 && !contamination[sx][sy].size()) {
		QMessageBox::information(this, tr("Hint"), tr("Nothing to wash."));
		return false;
	}
	washRoute(config, steps, tx, ty, ob);
	moveToPort(tx, ty, config);
	steps.push_back(Position(tx, ty));

	return true;
}

void MainWindow::washRoute(const ChipConfig &config, QVector<Position> &steps, qint32 tx, qint32 ty, const QVector<QVector<bool>> &obstacles) {
	QVector<QVector<qint32>> distance;
	QVector<QVector<qint32>> ancestor;
	distance.resize(config.columns);
	for (qint32 i = 0; i < config.columns; ++i) {
		distance[i].resize(config.rows);
		for (qint32 j = 0; j < config.rows; ++j) {
			distance[i][j] = -1;
		}
	}
	ancestor.resize(config.columns);
	for (qint32 i = 0; i < config.columns; ++i) {
		ancestor[i].resize(config.rows);
		for (qint32 j = 0; j < config.rows; ++j) {
			ancestor[i][j] = -1;
		}
	}
	auto iter = steps.back();
	QQueue<Position> queue;
	queue.push_back(iter);
	distance[iter.first][iter.second] = 0;
	while (!queue.empty()) {
		Position pos = queue.dequeue();
		for (qint32 k = 0; k < 4; ++k) {
			Position npos(pos.first + dirX[k], pos.second + dirY[k]);
			if (npos.first < 0 || npos.first >= config.columns || npos.second < 0 || npos.second >= config.rows) {
				continue;
			}
			if (obstacles[npos.first][npos.second]) {
				continue;
			}
			if (distance[npos.first][npos.second] == -1 || distance[npos.first][npos.second] > distance[pos.first][pos.second] + 1) {
				distance[npos.first][npos.second] = distance[pos.first][pos.second] + 1;
				ancestor[npos.first][npos.second] = k;
				queue.enqueue(npos);
				if (npos.first == tx && npos.second == ty) {
					queue.clear();
					break;
				}
			}
		}
	}
	if (distance[tx][ty] == -1) {
		QMessageBox::warning(this, tr("Failed!"), tr("Assertion failed!"));
	}
	QStack<Position> stk;
	for (qint32 x = tx, y = ty; x != iter.first || y != iter.second; ) {
		stk.push(Position(x, y));
		qint32 dir = ancestor[x][y];
		x -= dirX[dir];
		y -= dirY[dir];
	}
	while (!stk.empty()) {
		steps.push_back(stk.pop());
	}
}

void MainWindow::on_actionWash_triggered() {
	if (timerRun.isActive()) {
		on_actionPause_triggered();
	}
	if (wash(steps)) {
		lastWashTime = QDateTime::currentMSecsSinceEpoch();
		curWashTime = 0;

		ui->actionNewChip->setEnabled(false);
		ui->actionLoadCommandFile->setEnabled(false);
		ui->actionStart->setEnabled(false);
		ui->actionPause->setEnabled(false);
		ui->actionStep->setEnabled(false);
		ui->actionRevert->setEnabled(false);
		ui->actionReset->setEnabled(false);

		ui->actionWash->setEnabled(false);

	//	clearContamination(0);
		timerWash.start();
	}
}

void MainWindow::onWashTimeout() {
	qint64 thisWashTime = QDateTime::currentMSecsSinceEpoch();
	qint64 lastDisplayWashTime = curWashTime;
	curWashTime += (thisWashTime - lastWashTime) * washAcceleration;
	lastWashTime = thisWashTime;

	if (curWashTime >= (steps.size() - 1) * 1000) {
		curWashTime = (steps.size() - 1) * 1000;

		ui->actionNewChip->setEnabled(true);
		ui->actionLoadCommandFile->setEnabled(true);
		ui->actionStart->setEnabled(true);
		ui->actionPause->setEnabled(false);
		ui->actionStep->setEnabled(true);
		ui->actionRevert->setEnabled(true);
		ui->actionReset->setEnabled(true);

		if (config.hasWash) {
			ui->actionWash->setEnabled(true);
		}
		timerWash.stop();
	}

	if (lastDisplayWashTime / 1000 != curWashTime / 1000) {
		clearContamination(qint32(curWashTime / 1000));
	}

	ui->picDisplay->update();
}

void MainWindow::clearContamination(qint32 second) {
	if (second + 1 >= steps.size()) { // back() element is the waste port (which is out of range)
		return;
	}
	Position pos = steps[second];
	contamination[pos.first][pos.second].clear();
}
