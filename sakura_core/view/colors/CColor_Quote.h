/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#pragma once

#include "view/colors/CColorStrategy.h"


class Color_Quote : public ColorStrategy {
public:
	Color_Quote(wchar_t cQuote)
		:
		m_cQuote(cQuote),
		m_nCOMMENTEND(-1),
		m_nColorTypeIndex(0)
	{
		m_szQuote[0] = cQuote;
		m_szQuote[1] = cQuote;
		m_szQuote[2] = cQuote;
	}
	virtual void Update(void);
	virtual EColorIndexType GetStrategyColor() const = 0;
	virtual LayoutColorInfo* GetStrategyColorInfo() const;
	virtual void InitStrategyStatus() { m_nCOMMENTEND = -1; }
	virtual void SetStrategyColorInfo(const LayoutColorInfo*);
	virtual bool BeginColor(const StringRef& cStr, int nPos);
	virtual bool EndColor(const StringRef& cStr, int nPos);
	virtual bool Disp() const { return m_pTypeData->m_ColorInfoArr[this->GetStrategyColor()].m_bDisp; }
	
	static bool IsCppRawString(const StringRef& cStr, int nPos);
	static int Match_Quote(wchar_t wcQuote, int nPos, const StringRef& cLineStr, StringLiteralType escapeType, bool* pbEscapeEnd = NULL);
	static int Match_QuoteStr(const wchar_t* szQuote, int nQuoteLen, int nPos, const StringRef& cLineStr, bool bEscape);
private:

	wchar_t m_cQuote;
	wchar_t m_szQuote[3];
	int m_nCOMMENTEND;
	std::wstring m_tag;

	StringLiteralType m_nStringType;
	StringLiteralType m_nEscapeType;
	bool* m_pbEscapeEnd;
	bool m_bEscapeEnd;
protected:
	int m_nColorTypeIndex;
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

