#include "chipconfig.h"

void chipConfig::init(qint32 rows, qint32 columns) {
	if (rows < 3 || rows > 12 || columns < 3 || columns > 12 || (rows == 3 && columns == 3)) {
		valid = false;
		return;
	} else {
		valid = true;
	}

	this->rows = rows;
	this->columns = columns;
	L.clear();
	R.clear();
	T.clear();
	B.clear();
	L.resize(rows);
	R.resize(rows);
	T.resize(columns);
	B.resize(columns);
}
