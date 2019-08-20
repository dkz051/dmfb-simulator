#ifndef CHIPCONFIG_H
#define CHIPCONFIG_H

#include <QIntegerForSize>
#include <QList>

struct chipConfig
{
	qint32 rows;
	qint32 columns;
	qint32 waste;
	qint32 output;
	QList<qint32> input;
	QList<qint32> wash;

	void init(qint32 rows = 8, qint32 columns = 8);
};

#endif // CHIPCONFIG_H
