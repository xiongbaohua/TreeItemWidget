#ifndef __RCNAMESPACE_H__
#define __RCNAMESPACE_H__

#include <qicon.h>
#include <qpixmap.h>
#include <qvariant.h>

namespace Rcspace{
	///Icon 枚举类型,最大只支持16种
	enum RcIconEnum
	{
		riskAbnormal = 0,	//风险-异常
		riskOverLoss,		//风险-穿仓
		riskForceClose,		//风险-强平
		riskMarginCall,		//风险-追保
		riskWarning,		//风险-警示
		riskNormal,			//风险-正常
		noIcon,				//no-Icon
	};
	///TextAlignment 枚举类型，最大只支持16种
	enum RcTextAlignment
	{
		defaultTextAlign = 0,	//no specialized
		textAlignLeftTop,		//left-top
		textAlignLeftCenter,	//left-center
		textAlignLeftBottom,	//left-bottom
		textAlignCenterTop,		//center-top
		textAlignCenterCenter,	//center-center
		textAlignCenterBottom,	//center-center
		textAlignRightTop,		//right-top
		textAlignRightCenter,	//right-center
		textAlignRightBottom,	//right-bottom
	};

	QIcon specialIcon(RcIconEnum iconEnum);
	QVariant specialTextAlignment(RcTextAlignment alignEnum);
}

#endif