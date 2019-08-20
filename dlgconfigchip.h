#ifndef DLGCONFIGCHIP_H
#define DLGCONFIGCHIP_H

#include <QDialog>

namespace Ui {
        class dlgConfigChip;
}

class dlgConfigChip : public QDialog
{
	Q_OBJECT

public:
        explicit dlgConfigChip(QWidget *parent = nullptr);
        ~dlgConfigChip();

private:
        Ui::dlgConfigChip *ui;
};

#endif // DLGCONFIGCHIP_H
