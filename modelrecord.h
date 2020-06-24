#pragma once
#include <vector>
#include "recorddata.h"
#include "DisplayRecordData.h"

class ModelRecord
{
public:
	ModelRecord();
	virtual ~ModelRecord();
	virtual int recordSize() = 0;
	virtual RecordData* recordAt(int pos) = 0;
};
