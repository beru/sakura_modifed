#include "StdAfx.h"
#include "Layout.h"
#include "LayoutMgr.h"
#include "charset/charcode.h"
#include "extmodule/Bregexp.h" // LayoutMgrの定義で必要

Layout::~Layout()
{
	return;
}

void Layout::DUMP(void)
{
	DEBUG_TRACE(_T("\n\n■Layout::DUMP()======================\n"));
	DEBUG_TRACE(_T("ptLogicPos.y=%d\t\t対応する論理行番号\n"), ptLogicPos.y);
	DEBUG_TRACE(_T("ptLogicPos.x=%d\t\t対応する論理行の先頭からのオフセット\n"), ptLogicPos.x);
	DEBUG_TRACE(_T("nLength=%d\t\t対応する論理行のハイト数\n"), (int)nLength);
	DEBUG_TRACE(_T("nTypePrev=%d\t\tタイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 \n"), nTypePrev);
	DEBUG_TRACE(_T("======================\n"));
	return;
}

// レイアウト幅を計算。改行は含まない。
size_t Layout::CalcLayoutWidth(const LayoutMgr& layoutMgr) const
{
	// ソース
	const wchar_t* pText = pDocLine->GetPtr();
	size_t nTextLen = pDocLine->GetLengthWithoutEOL();

	// 計算
	size_t nWidth = GetIndent();
	for (int i=ptLogicPos.x; i<ptLogicPos.x+(int)nLength; ++i) {
		if (pText[i] == WCODE::TAB) {
			nWidth += layoutMgr.GetActualTabSpace(nWidth);
		}else {
			nWidth += NativeW::GetKetaOfChar(pText, nTextLen, i);
		}
	}
	return nWidth;
}

// オフセット値をレイアウト単位に変換して取得
int Layout::CalcLayoutOffset(
	const LayoutMgr& layoutMgr,
	int nStartPos,
	int nStartOffset) const
{
	int nRet = nStartOffset;
	if (this->GetLogicOffset()) {
		const wchar_t* pLine = this->pDocLine->GetPtr();
		size_t nLineLen = this->pDocLine->GetLengthWithEOL();
		const int nOffset = GetLogicOffset();
		for (int i=nStartPos; i<nOffset; ++i) {
			if (pLine[i] == WCODE::TAB) {
				nRet += layoutMgr.GetActualTabSpace(nRet);
			}else {
				nRet += NativeW::GetKetaOfChar(pLine, nLineLen, i);
			}
		}
	}
	return nRet;
}

