#ifndef DLGNEWCHIP_H
#define DLGNEWCHIP_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
	class dlgNewChip;
}

class dlgNewChip : public QDialog {
	Q_OBJECT

public:
	explicit dlgNewChip(QWidget *parent = nullptr);
	~dlgNewChip();

signals:
	void accepted(qint32 rows, qint32 columns);

private slots:
	void on_buttonBox_accepted();

private:
	Ui::dlgNewChip *ui;
};

#endif // DLGNEWCHIP_H
