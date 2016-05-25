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

#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Color_RegexKeyword.h"


bool Color_RegexKeyword::BeginColor(const StringRef& str, int nPos)
{
	if (!str.IsValid()) {
		return false;
	}

	size_t nMatchLen;
	int nMatchColor;
	const EditView* pView = ColorStrategyPool::getInstance().GetCurrentView();
	// 正規表現キーワード
	if (pView->pRegexKeyword->RegexIsKeyword(str, nPos, &nMatchLen, &nMatchColor)) {
		this->nCommentEnd = nPos + nMatchLen;  // キーワード文字列の終端をセットする
		this->nCommentMode = ToColorIndexType_RegularExpression(nMatchColor);
		return true;
	}
	return false;
}


bool Color_RegexKeyword::EndColor(const StringRef& str, int nPos)
{
	return (nPos == this->nCommentEnd);
}

void Color_RegexKeyword::OnStartScanLogic()
{
	EditView* pView = ColorStrategyPool::getInstance().GetCurrentView();
	if (pTypeData->bUseRegexKeyword) {
		pView->pRegexKeyword->RegexKeyLineStart();
	}
}

