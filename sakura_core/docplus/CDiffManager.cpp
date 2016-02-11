#include "StdAfx.h"
#include "docplus/CDiffManager.h"
#include "types/CTypeSupport.h"
#include "window/CEditWnd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     DiffLineGetter                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EDiffMark DiffLineGetter::GetLineDiffMark() const { return (EDiffMark)m_pcDocLine->m_sMark.m_cDiffmarked; }

/*! 行の差分マークに対応した色を返す -> pnColorIndex
	
	色設定が無い場合は pnColorIndex を変更せずに false を返す。	
*/
bool DiffLineGetter::GetDiffColor(EColorIndexType* pnColorIndex) const
{
	EDiffMark type = GetLineDiffMark();
	EditView* pView = &EditWnd::getInstance()->GetActiveView();

	// DIFF差分マーク表示	//@@@ 2002.05.25 MIK
	if (type) {
		switch (type) {
		case MARK_DIFF_APPEND:	// 追加
			if (TypeSupport(pView, COLORIDX_DIFF_APPEND).IsDisp()) {
				*pnColorIndex = COLORIDX_DIFF_APPEND;
				return true;
			}
			break;
		case MARK_DIFF_CHANGE:	// 変更
			if (TypeSupport(pView, COLORIDX_DIFF_CHANGE).IsDisp()) {
				*pnColorIndex = COLORIDX_DIFF_CHANGE;
				return true;
			}
			break;
		case MARK_DIFF_DELETE:	// 削除
		case MARK_DIFF_DEL_EX:	// 削除
			if (TypeSupport(pView, COLORIDX_DIFF_DELETE).IsDisp()) {
				*pnColorIndex = COLORIDX_DIFF_DELETE;
				return true;
			}
			break;
		}
	}
	return false;
}


/*! DIFFマーク描画

	引数は仮。（無駄な引数ありそう）
*/
bool DiffLineGetter::DrawDiffMark(Graphics& gr, int y, int nLineHeight, COLORREF color) const
{
	EDiffMark type = GetLineDiffMark();

	if (type) {	// DIFF差分マーク表示	//@@@ 2002.05.25 MIK
		int	cy = y + nLineHeight / 2;

		gr.PushPen(color, 0);

		switch (type) {
		case MARK_DIFF_APPEND:	// 追加
			::MoveToEx(gr, 3, cy, NULL);
			::LineTo  (gr, 6, cy);
			::MoveToEx(gr, 4, cy - 2, NULL);
			::LineTo  (gr, 4, cy + 3);
			break;

		case MARK_DIFF_CHANGE:	// 変更
			::MoveToEx(gr, 3, cy - 4, NULL);
			::LineTo  (gr, 3, cy);
			::MoveToEx(gr, 3, cy + 2, NULL);
			::LineTo  (gr, 3, cy + 3);
			break;

		case MARK_DIFF_DELETE:	// 削除
			cy -= 3;
			::MoveToEx(gr, 3, cy, NULL);
			::LineTo  (gr, 5, cy);
			::LineTo  (gr, 3, cy + 2);
			::LineTo  (gr, 3, cy);
			::LineTo  (gr, 7, cy + 4);
			break;
		
		case MARK_DIFF_DEL_EX:	// 削除(EOF)
			cy += 3;
			::MoveToEx(gr, 3, cy, NULL);
			::LineTo  (gr, 5, cy);
			::LineTo  (gr, 3, cy - 2);
			::LineTo  (gr, 3, cy);
			::LineTo  (gr, 7, cy - 4);
			break;
		}

		gr.PopPen();

		return true;
	}
	return false;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     DiffLineSetter                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void DiffLineSetter::SetLineDiffMark(EDiffMark mark) { m_pcDocLine->m_sMark.m_cDiffmarked = mark; }


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       DiffLineMgr                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	差分表示の全解除
	@author	MIK
	@date	2002.05.25
*/
void DiffLineMgr::ResetAllDiffMark()
{
	DocLine* pDocLine = m_pcDocLineMgr->GetDocLineTop();
	while (pDocLine) {
		pDocLine->m_sMark.m_cDiffmarked = MARK_DIFF_NONE;
		pDocLine = pDocLine->GetNextLine();
	}

	DiffManager::getInstance()->SetDiffUse(false);
}

/*! 差分検索
	@author	MIK
	@date	2002.05.25
*/
bool DiffLineMgr::SearchDiffMark(
	LogicInt			nLineNum,		// 検索開始行
	eSearchDirection	bPrevOrNext,	// 0==前方検索 1==後方検索
	LogicInt*			pnLineNum 		// マッチ行
	)
{
	LogicInt nLinePos = nLineNum;

	// 前方検索
	if (bPrevOrNext == eSearchDirection::Backward) {
		--nLinePos;
		const DocLine* pDocLine = m_pcDocLineMgr->GetLine(nLinePos);
		while (pDocLine) {
			if (DiffLineGetter(pDocLine).GetLineDiffMark() != 0) {
				*pnLineNum = nLinePos;				// マッチ行
				return true;
			}
			--nLinePos;
			pDocLine = pDocLine->GetPrevLine();
		}
	// 後方検索
	}else {
		++nLinePos;
		const DocLine* pDocLine = m_pcDocLineMgr->GetLine(nLinePos);
		while (pDocLine) {
			if (DiffLineGetter(pDocLine).GetLineDiffMark() != 0) {
				*pnLineNum = nLinePos;				// マッチ行
				return true;
			}
			++nLinePos;
			pDocLine = pDocLine->GetNextLine();
		}
	}
	return false;
}

/*!	差分情報を行範囲指定で登録する。
	@author	MIK
	@date	2002/05/25
*/
void DiffLineMgr::SetDiffMarkRange(EDiffMark nMode, LogicInt nStartLine, LogicInt nEndLine)
{
	DiffManager::getInstance()->SetDiffUse(true);

	if (nStartLine < LogicInt(0)) {
		nStartLine = LogicInt(0);
	}
	// 最終行より後に削除行あり
	LogicInt nLines = m_pcDocLineMgr->GetLineCount();
	if (nLines <= nEndLine) {
		nEndLine = nLines - LogicInt(1);
		DocLine* pCDocLine = m_pcDocLineMgr->GetLine(nEndLine);
		if (pCDocLine) {
			DiffLineSetter(pCDocLine).SetLineDiffMark(MARK_DIFF_DEL_EX);
		}
	}

	// 行範囲にマークをつける
	for (LogicInt i=nStartLine; i<=nEndLine; ++i) {
		DocLine* pCDocLine = m_pcDocLineMgr->GetLine(i);
		if (pCDocLine) {
			DiffLineSetter(pCDocLine).SetLineDiffMark(nMode);
		}
	}

	return;
}

