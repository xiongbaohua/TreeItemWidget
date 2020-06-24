#pragma once

#include <qtextcodec.h>
class GBKCodec
{
private:
	static QTextCodec* pCodec;
public:
	GBKCodec(){}
	~GBKCodec(){}
	static QString fromGBK8Bit(const char* str)
	{
		return pCodec->toUnicode(str);
	}
	static QString fromGBK8Bit(const QByteArray& str)
	{
		return pCodec->toUnicode(str);
	}
	static QByteArray toGBK8Bit(const QString& str)
	{
		return pCodec->fromUnicode(str);
	}
};
