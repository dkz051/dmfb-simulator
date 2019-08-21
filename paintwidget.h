#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>

#include "chipconfig.h"

class paintWidget : public QWidget
{
	Q_OBJECT
public:
	using QWidget::QWidget;

	chipConfig config;
	portType currentType;

signals:

public slots:
};

class previewWidget : public paintWidget
{
	Q_OBJECT
public:
	using paintWidget::paintWidget;

protected:
	void paintEvent(QPaintEvent *e);
	void mousePressEvent(QMouseEvent *e);
};

class displayWidget : public paintWidget
{
	Q_OBJECT
public:
	using paintWidget::paintWidget;
protected:
	void paintEvent(QPaintEvent *e);
	void mousePressEvent(QMouseEvent *e);
};

#endif // PAINTWIDGET_H
