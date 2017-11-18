#pragma once

#include "view/colors/ColorStrategy.h"


class Color_Quote : public ColorStrategy {
public:
	Color_Quote(wchar_t cQuote)
		:
		cQuote(cQuote),
		nCommentEnd(-1),
		nColorTypeIndex(0)
	{
		szQuote[0] = cQuote;
		szQuote[1] = cQuote;
		szQuote[2] = cQuote;
	}
	virtual void Update(void);
	virtual EColorIndexType GetStrategyColor() const = 0;
	virtual LayoutColorInfo* GetStrategyColorInfo() const;
	virtual void InitStrategyStatus() { nCommentEnd = -1; }
	virtual void SetStrategyColorInfo(const LayoutColorInfo*);
	virtual bool BeginColor(const StringRef& str, size_t nPos);
	virtual bool EndColor(const StringRef& str, size_t nPos);
	virtual bool Disp() const { return pTypeData->colorInfoArr[this->GetStrategyColor()].bDisp; }
	
	static size_t Match_Quote(wchar_t wcQuote, int nPos, const StringRef& lineStr, StringLiteralType escapeType, bool* pbEscapeEnd = NULL);
	static size_t Match_QuoteStr(const wchar_t* szQuote, size_t nQuoteLen, size_t nPos, const StringRef& lineStr, bool bEscape);
private:

	wchar_t cQuote;
	wchar_t szQuote[3];
	int nCommentEnd;
	std::wstring tag;

	StringLiteralType nStringType;
	StringLiteralType nEscapeType;
	bool* pbEscapeEnd;
	bool bEscapeEnd;
protected:
	int nColorTypeIndex;
};


class Color_SingleQuote : public Color_Quote {
public:
	Color_SingleQuote() : Color_Quote(L'\'') { }
	virtual EColorIndexType GetStrategyColor() const { return COLORIDX_SSTRING; }
};

class Color_DoubleQuote : public Color_Quote {
public:
	Color_DoubleQuote() : Color_Quote(L'"') { }
	virtual EColorIndexType GetStrategyColor() const { return COLORIDX_WSTRING; }
};

