#pragma once

#include "view/colors/ColorStrategy.h"

class Color_Heredoc : public ColorStrategy {
public:
	virtual EColorIndexType GetStrategyColor() const { return COLORIDX_HEREDOC; }
	virtual LayoutColorInfo* GetStrategyColorInfo() const;
	virtual void InitStrategyStatus() { nCommentEnd = 0; }
	virtual void SetStrategyColorInfo(const LayoutColorInfo*);
	virtual bool BeginColor(const StringRef& str, size_t nPos);
	virtual bool Disp() const { return pTypeData->colorInfoArr[COLORIDX_HEREDOC].bDisp; }
	virtual bool EndColor(const StringRef& str, size_t nPos);
private:
	std::wstring id;
	size_t nSize;
	const wchar_t* pszId;
	int nCommentEnd;
};

