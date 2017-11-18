#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Color_RegexKeyword.h"


bool Color_RegexKeyword::BeginColor(const StringRef& str, size_t nPos)
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


bool Color_RegexKeyword::EndColor(const StringRef& str, size_t nPos)
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

