#ifndef FRMCONFIGCHIP_H
#define FRMCONFIGCHIP_H

#include <QMainWindow>
#include <QPainter>
#include <QGraphicsScene>
#include <QTimer>

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

public slots:
	void timerRefresh();

protected:

signals:
	void accepted(const chipConfig &config);

private slots:
	void on_optInput_clicked();

	void on_optOutput_clicked();

	void on_optWash_clicked();

	void on_optWaste_clicked();

	void on_optNone_clicked();

	void on_buttonBox_accepted();

private:
	Ui::frmConfigChip *ui;
	qint32 rows, columns;

	QTimer *timer;
};


#endif // FRMCONFIGCHIP_H
