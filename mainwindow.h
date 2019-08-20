#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "chipconfig.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
	void on_actionNewChip_triggered();

	void on_dlgNewChip_accepted(qint32 rows, qint32 columns);
	void on_dlgConfigChip_accepted(const chipConfig &config);

	void on_actionExit_triggered();

	void on_actionAboutDmfbSimulator_triggered();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
