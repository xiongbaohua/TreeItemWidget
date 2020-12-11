#include "rcnamespace.h"

QIcon Rcspace::specialIcon(Rcspace::RcIconEnum iconEnum)
{
	//QPixmap pixmap(":Resources/risk.png");
	switch (iconEnum)
	{
	//case Rcspace::noIcon:
	//	return QIcon();
	//case Rcspace::riskNormal:
	//	return QIcon(pixmap.copy(80, 0, 16, 16));
	//case Rcspace::riskWarning:
	//	return QIcon(pixmap.copy(64, 0, 16, 16));
	//case Rcspace::riskMarginCall:
	//	return QIcon(pixmap.copy(48, 0, 16, 16));
	//case Rcspace::riskForceClose:
	//	return QIcon(pixmap.copy(32, 0, 16, 16));
	//case Rcspace::riskOverLoss:
	//	return QIcon(pixmap.copy(16, 0, 16, 16));
	//case Rcspace::riskAbnormal:
	//	return QIcon(pixmap.copy(0, 0, 16, 16));
	default:
		return QIcon();
	}
}

QVariant Rcspace::specialTextAlignment(Rcspace::RcTextAlignment alignEnum)
{
	switch (alignEnum)
	{
	case Rcspace::defaultTextAlign:
		return QVariant();
	case Rcspace::textAlignLeftTop:
		return (int)(Qt::AlignLeft | Qt::AlignTop);
	case Rcspace::textAlignLeftCenter:
		return (int)(Qt::AlignLeft | Qt::AlignVCenter);
	case Rcspace::textAlignLeftBottom:
		return (int)(Qt::AlignLeft | Qt::AlignBottom);
	case Rcspace::textAlignCenterTop:
		return (int)(Qt::AlignHCenter | Qt::AlignTop);
	case Rcspace::textAlignCenterCenter:
		return (int)(Qt::AlignHCenter | Qt::AlignVCenter);
	case Rcspace::textAlignCenterBottom:
		return (int)(Qt::AlignHCenter | Qt::AlignBottom);
	case Rcspace::textAlignRightTop:
		return (int)(Qt::AlignRight | Qt::AlignTop);
	case Rcspace::textAlignRightCenter:
		return (int)(Qt::AlignRight | Qt::AlignVCenter);
	case Rcspace::textAlignRightBottom:
		return (int)(Qt::AlignRight | Qt::AlignBottom);
	default:
		return QVariant();
	}
}
