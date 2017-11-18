#pragma once

#include "view/colors/ColorStrategy.h"

class Color_Url : public ColorStrategy {
public:
	Color_Url() : nCommentEnd(0) { }
	virtual EColorIndexType GetStrategyColor() const { return COLORIDX_URL; }
	virtual void InitStrategyStatus() { nCommentEnd = 0; }
	virtual bool BeginColor(const StringRef& str, size_t nPos);
	virtual bool EndColor(const StringRef& str, size_t nPos);
	virtual bool Disp() const { return pTypeData->colorInfoArr[COLORIDX_URL].bDisp; }
private:
	int nCommentEnd;
};

