#pragma once
#include <qvariant.h>

///用于扩展TreeItem，装载displayRole数据
class RecordData
{
public:
	RecordData();
	virtual ~RecordData();
	virtual QVariant data(int column, int role = Qt::DisplayRole) = 0;
	virtual bool setData(int column, const QVariant& value, int role = Qt::EditRole) = 0;
};

