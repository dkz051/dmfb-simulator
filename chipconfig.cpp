#include "chipconfig.h"

void chipConfig::init(qint32 rows, qint32 columns)
{
	this->rows = rows;
	this->columns = columns;
	this->waste = 0;
	this->output = 0;
	this->input.clear();
	this->wash.clear();
}
