#ifndef CHIPCONFIG_H
#define CHIPCONFIG_H

#include <QIntegerForSize>
#include <QVector>

enum portType {
	none, input, output, wash, waste
};

struct chipConfig {
	qint32 rows;
	qint32 columns;
	QVector<portType> L, T, R, B;
	bool valid;

	void init(qint32 rows = -1, qint32 columns = -1);
};

#endif // CHIPCONFIG_H
