#pragma once

#include "view/colors/ColorStrategy.h"


class Color_RegexKeyword : public ColorStrategy {
public:
	Color_RegexKeyword() : nCommentEnd(0), nCommentMode(ToColorIndexType_RegularExpression(0)) { }
	virtual EColorIndexType GetStrategyColor() const { return nCommentMode; }
	virtual void InitStrategyStatus() { nCommentEnd = 0; nCommentMode = ToColorIndexType_RegularExpression(0); }
	virtual bool BeginColor(const StringRef& str, size_t nPos);
	virtual bool EndColor(const StringRef& str, size_t nPos);
	virtual bool Disp() const { return pTypeData->bUseRegexKeyword; }
	virtual void OnStartScanLogic();
private:
	int nCommentEnd;
	EColorIndexType nCommentMode;
};

