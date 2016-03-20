#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Color_Found.h"
#include "types/TypeSupport.h"
#include "view/ViewSelect.h"
#include <limits.h>


void Color_Select::OnStartScanLogic()
{
	m_nSelectLine	= LayoutInt(-1);
	m_nSelectStart	= LogicInt(-1);
	m_nSelectEnd	= LogicInt(-1);
}

bool Color_Select::BeginColor(const StringRef& str, int nPos)
{
	assert(0);
	return false;
}

bool Color_Select::BeginColorEx(const StringRef& str, int nPos, LayoutInt nLineNum, const Layout* pLayout)
{
	if (!str.IsValid()) return false;

	const EditView& view = *(ColorStrategyPool::getInstance().GetCurrentView());
	if (!view.GetSelectionInfo().IsTextSelected() || !TypeSupport(view, COLORIDX_SELECT).IsDisp()) {
		return false;
	}

	// 2011.12.27 レイアウト行頭で1回だけ確認してあとはメンバー変数をみる
	if (m_nSelectLine == nLineNum) {
		return (m_nSelectStart <= nPos && nPos < m_nSelectEnd);
	}
	m_nSelectLine = nLineNum;
	LayoutRange selectArea = view.GetSelectionInfo().GetSelectAreaLine(nLineNum, pLayout);
	LayoutInt nSelectFrom = selectArea.GetFrom().x;
	LayoutInt nSelectTo = selectArea.GetTo().x;
	if (nSelectFrom == nSelectTo || nSelectFrom == -1) {
		m_nSelectStart = -1;
		m_nSelectEnd = -1;
		return false;
	}
	LogicInt nIdxFrom = view.LineColumnToIndex(pLayout, nSelectFrom) + pLayout->GetLogicOffset();
	LogicInt nIdxTo = view.LineColumnToIndex(pLayout, nSelectTo) + pLayout->GetLogicOffset();
	m_nSelectStart = nIdxFrom;
	m_nSelectEnd = nIdxTo;
	if (m_nSelectStart <= nPos && nPos < m_nSelectEnd) {
		return true;
	}
	return false;
}

bool Color_Select::EndColor(const StringRef& str, int nPos)
{
	// マッチ文字列終了検出
	return (m_nSelectEnd <= nPos);
}


Color_Found::Color_Found()
	:
	validColorNum(0)
{
}

void Color_Found::OnStartScanLogic()
{
	m_nSearchResult	= 1;
	m_nSearchStart	= LogicInt(-1);
	m_nSearchEnd	= LogicInt(-1);

	this->validColorNum = 0;
	for (int color=COLORIDX_SEARCH; color<=COLORIDX_SEARCHTAIL; ++color) {
		if (m_pTypeData->colorInfoArr[color].bDisp) {
			this->highlightColors[this->validColorNum++] = EColorIndexType(color);
		}
	}
}

bool Color_Found::BeginColor(const StringRef& str, int nPos)
{
	if (!str.IsValid()) return false;
	const EditView* pView = ColorStrategyPool::getInstance().GetCurrentView();
	if (!pView->m_bCurSrchKeyMark || this->validColorNum == 0) {
		return false;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//        検索ヒットフラグ設定 -> bSearchStringMode            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2002.02.08 hor 正規表現の検索文字列マークを少し高速化
	if (pView->m_curSearchOption.bWordOnly || (m_nSearchResult && m_nSearchStart < nPos)) {
		m_nSearchResult = pView->IsSearchString(
			str,
			LogicInt(nPos),
			&m_nSearchStart,
			&m_nSearchEnd
		);
	}
	// マッチ文字列検出
	return (m_nSearchResult && m_nSearchStart == nPos);
}

bool Color_Found::EndColor(const StringRef& str, int nPos)
{
	// マッチ文字列終了検出
	return (m_nSearchEnd <= nPos); //+ == では行頭文字の場合、m_nSearchEndも０であるために文字色の解除ができないバグを修正 2003.05.03 かろと
}

