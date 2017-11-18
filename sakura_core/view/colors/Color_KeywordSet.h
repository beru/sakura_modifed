#pragma once

#include "view/colors/ColorStrategy.h"

class Color_KeywordSet : public ColorStrategy {
public:
	Color_KeywordSet();
	virtual EColorIndexType GetStrategyColor() const { return (EColorIndexType)(COLORIDX_KEYWORD1 + nKeywordIndex); }
	virtual void InitStrategyStatus() { nCommentEnd = 0; }
	virtual bool BeginColor(const StringRef& str, size_t nPos);
	virtual bool EndColor(const StringRef& str, size_t nPos);
	virtual bool Disp() const { return true; }
private:
	int nKeywordIndex;
	int nCommentEnd;
};

