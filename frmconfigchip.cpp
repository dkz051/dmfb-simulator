#include "frmconfigchip.h"
#include "ui_frmconfigchip.h"

frmConfigChip::frmConfigChip(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::frmConfigChip)
{
	ui->setupUi(this);
	this->setWindowFlags(Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
	this->config.init();
}

void frmConfigChip::setDimensions(qint32 rows, qint32 columns)
{
	this->rows = rows;
	this->columns = columns;
	this->config.init(rows, columns);
}

frmConfigChip::~frmConfigChip()
{
	delete ui;
}

void frmConfigChip::refresh(QPainter *graphics)
{
	qint32 W = ui->graphicsView->width();
	qint32 H = ui->graphicsView->height();

	// Determine the size of a single grid.
	// Port identifier (Input/Output/Wash/Waste) shall take the place of 2 grids.
	// Grid size is truncated to 10 logical pixels.
	qreal grid = (std::min(W / (columns + 4), H / (rows + 4)) / 10) * 10.0;
	graphics->save();

	graphics->translate((W - grid * columns) / 2.0, (H - grid * rows) / 2.0);
	graphics->scale(grid, grid);

	for (qint32 i = 0; i <= rows; ++i) {
		graphics->drawLine(0, i, columns, i);
	}
	for (qint32 j = 0; j <= columns; ++j) {
		graphics->drawLine(j, 0, j, rows);
	}

	graphics->restore();
}
