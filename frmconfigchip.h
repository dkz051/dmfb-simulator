#ifndef FRMCONFIGCHIP_H
#define FRMCONFIGCHIP_H

#include <QTimer>
#include <QPainter>
#include <QMainWindow>

#include "utility.h"

namespace Ui {
	class frmConfigChip;
}

class frmConfigChip : public QMainWindow {
	Q_OBJECT

public:
	explicit frmConfigChip(QWidget *parent = nullptr);
	~frmConfigChip();

	void setDimensions(qint32 rows, qint32 columns);
	void refresh(QPainter *graphics);

protected:
	bool eventFilter(QObject *o, QEvent *e);

signals:
	void accepted(const ChipConfig &config);

private slots:
	void on_buttonBox_accepted();

private:
	Ui::frmConfigChip *ui;

	QTimer *timer;
	ChipConfig config;
};

#endif // FRMCONFIGCHIP_H
