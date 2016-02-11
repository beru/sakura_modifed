#include "StdAfx.h"
#include "CViewParser.h"
#include "doc/CEditDoc.h"
#include "doc/layout/CLayout.h"
#include "view/CEditView.h"
#include "charset/charcode.h"

/*
	カーソル直前の単語を取得 単語の長さを返します
	単語区切り
*/
int ViewParser::GetLeftWord(NativeW* pcmemWord, int nMaxWordLen) const
{
	LogicInt	nLineLen;
	LogicInt	nIdx;
	LogicInt	nIdxTo;

	int			nCharChars;
	const Layout* pcLayout;

	LayoutInt nCurLine = m_pEditView->GetCaret().GetCaretLayoutPos().GetY2();
	const wchar_t* pLine = m_pEditView->m_pEditDoc->m_layoutMgr.GetLineStr(nCurLine, &nLineLen, &pcLayout);
	if (!pLine) {
//		return 0;
		nIdxTo = LogicInt(0);
	}else {
		// 指定された桁に対応する行のデータ内の位置を調べる Ver1
		nIdxTo = m_pEditView->LineColumnToIndex(pcLayout, m_pEditView->GetCaret().GetCaretLayoutPos().GetX2());
	}
	if (nIdxTo == 0 || !pLine) {
		if (nCurLine <= 0) {
			return 0;
		}
		--nCurLine;
		pLine = m_pEditView->m_pEditDoc->m_layoutMgr.GetLineStr(nCurLine, &nLineLen);
		if (!pLine) {
			return 0;
		}
		bool bExtEol = GetDllShareData().m_common.m_edit.m_bEnableExtEol;
		if (WCODE::IsLineDelimiter(pLine[nLineLen - 1], bExtEol)) {
			return 0;
		}

		nCharChars = &pLine[nLineLen] - NativeW::GetCharPrev(pLine, nLineLen, &pLine[nLineLen]);
		if (nCharChars == 0) {
			return 0;
		}
		nIdxTo = nLineLen;
		nIdx = nIdxTo - LogicInt(nCharChars);
	}else {
		nCharChars = &pLine[nIdxTo] - NativeW::GetCharPrev(pLine, nLineLen, &pLine[nIdxTo]);
		if (nCharChars == 0) {
			return 0;
		}
		nIdx = nIdxTo - LogicInt(nCharChars);
	}

	if (nCharChars == 1) {
		if (WCODE::IsWordDelimiter(pLine[nIdx])) {
			return 0;
		}
	}

	// 現在位置の単語の範囲を調べる
	NativeW cmemWord;
	LayoutRange sRange;
	bool bResult = m_pEditView->m_pEditDoc->m_layoutMgr.WhereCurrentWord(
		nCurLine,
		nIdx,
		&sRange,
		&cmemWord,
		pcmemWord
	);
	if (bResult) {
		pcmemWord->AppendString(&pLine[nIdx], nCharChars);
		return pcmemWord->GetStringLength();
	}else {
		return 0;
	}
}


/*!
	キャレット位置の単語を取得
	単語区切り

	@param[out] pcmemWord キャレット位置の単語
	@return true: 成功，false: 失敗
	
	@date 2006.03.24 fon (CEditView::Command_SELECTWORDを流用)
*/
bool ViewParser::GetCurrentWord(
	NativeW* pcmemWord
) const
{
	const Layout* pcLayout = m_pEditView->m_pEditDoc->m_layoutMgr.SearchLineByLayoutY(m_pEditView->GetCaret().GetCaretLayoutPos().GetY2());
	if (!pcLayout) {
		return false;	// 単語選択に失敗
	}
	
	// 指定された桁に対応する行のデータ内の位置を調べる
	LogicInt nIdx = m_pEditView->LineColumnToIndex(pcLayout, m_pEditView->GetCaret().GetCaretLayoutPos().GetX2());
	
	// 現在位置の単語の範囲を調べる
	LayoutRange sRange;
	bool bResult = m_pEditView->m_pEditDoc->m_layoutMgr.WhereCurrentWord(
		m_pEditView->GetCaret().GetCaretLayoutPos().GetY2(),
		nIdx,
		&sRange,
		pcmemWord,
		NULL
	);

	return bResult;
}

