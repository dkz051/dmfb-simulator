#ifndef WNDCONFIGCHIP_H
#define WNDCONFIGCHIP_H

#include <QDialog>

namespace Ui {
	class wndConfigChip;
}

class wndConfigChip : public QDialog
{
	Q_OBJECT

public:
	explicit wndConfigChip(QWidget *parent = nullptr);
	~wndConfigChip();

private:
	Ui::wndConfigChip *ui;
};

#endif // WNDCONFIGCHIP_H
