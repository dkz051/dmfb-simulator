#ifndef FRMCONFIGCHIP_H
#define FRMCONFIGCHIP_H

#include <QMainWindow>
#include <QPainter>
#include "chipconfig.h"

namespace Ui {
	class frmConfigChip;
}

class frmConfigChip : public QMainWindow
{
	Q_OBJECT

public:
	explicit frmConfigChip(QWidget *parent = nullptr);
	~frmConfigChip();

	void setDimensions(qint32 rows, qint32 columns);
	void refresh(QPainter *graphics);

private:
	Ui::frmConfigChip *ui;
	qint32 rows, columns;

	chipConfig config;
};

#endif // FRMCONFIGCHIP_H
