#pragma once

#include "view/colors/ColorStrategy.h"

class Color_Numeric : public ColorStrategy {
public:
	Color_Numeric() : nCommentEnd(0) { }
	virtual EColorIndexType GetStrategyColor() const { return COLORIDX_DIGIT; }
	virtual void InitStrategyStatus() { nCommentEnd = 0; }
	virtual bool BeginColor(const StringRef& str, size_t nPos);
	virtual bool EndColor(const StringRef& str, size_t nPos);
	virtual bool Disp() const { return pTypeData->colorInfoArr[COLORIDX_DIGIT].bDisp; }
private:
	int nCommentEnd;
};

