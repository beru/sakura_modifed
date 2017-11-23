#include "StdAfx.h"
#include "docplus/DiffManager.h"
#include "types/TypeSupport.h"
#include "window/EditWnd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     DiffLineGetter                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

DiffMark DiffLineGetter::GetLineDiffMark() const { return (DiffMark)pDocLine->mark.diffMarked; }

/*! 行の差分マークに対応した色を返す -> pColorIndex
	
	色設定が無い場合は pColorIndex を変更せずに false を返す。	
*/
bool DiffLineGetter::GetDiffColor(EColorIndexType* pColorIndex) const
{
	DiffMark type = GetLineDiffMark();
	EditView& view = EditWnd::getInstance().GetActiveView();

	// DIFF差分マーク表示	
	if (type != DiffMark::None) {
		switch (type) {
		case DiffMark::Append:	// 追加
			if (TypeSupport(view, COLORIDX_DIFF_APPEND).IsDisp()) {
				*pColorIndex = COLORIDX_DIFF_APPEND;
				return true;
			}
			break;
		case DiffMark::Change:	// 変更
			if (TypeSupport(view, COLORIDX_DIFF_CHANGE).IsDisp()) {
				*pColorIndex = COLORIDX_DIFF_CHANGE;
				return true;
			}
			break;
		case DiffMark::Delete:	// 削除
		case DiffMark::DeleteEx:	// 削除
			if (TypeSupport(view, COLORIDX_DIFF_DELETE).IsDisp()) {
				*pColorIndex = COLORIDX_DIFF_DELETE;
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
bool DiffLineGetter::DrawDiffMark(
	Graphics& gr,
	int y,
	unsigned int nLineHeight,
	COLORREF color
	) const
{
	DiffMark type = GetLineDiffMark();

	if (type != DiffMark::None) {	// DIFF差分マーク表示	//@@@ 2002.05.25 MIK
		int	cy = y + nLineHeight / 2;

		gr.PushPen(color, 0);

		switch (type) {
		case DiffMark::Append:	// 追加
			::MoveToEx(gr, 3, cy, NULL);
			::LineTo  (gr, 6, cy);
			::MoveToEx(gr, 4, cy - 2, NULL);
			::LineTo  (gr, 4, cy + 3);
			break;

		case DiffMark::Change:	// 変更
			::MoveToEx(gr, 3, cy - 4, NULL);
			::LineTo  (gr, 3, cy);
			::MoveToEx(gr, 3, cy + 2, NULL);
			::LineTo  (gr, 3, cy + 3);
			break;

		case DiffMark::Delete:	// 削除
			cy -= 3;
			::MoveToEx(gr, 3, cy, NULL);
			::LineTo  (gr, 5, cy);
			::LineTo  (gr, 3, cy + 2);
			::LineTo  (gr, 3, cy);
			::LineTo  (gr, 7, cy + 4);
			break;
		
		case DiffMark::DeleteEx:	// 削除(EOF)
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

void DiffLineSetter::SetLineDiffMark(DiffMark mark) { pDocLine->mark.diffMarked = mark; }


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       DiffLineMgr                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	差分表示の全解除 */
void DiffLineMgr::ResetAllDiffMark()
{
	DocLine* pDocLine = docLineMgr.GetDocLineTop();
	while (pDocLine) {
		pDocLine->mark.diffMarked = DiffMark::None;
		pDocLine = pDocLine->GetNextLine();
	}

	DiffManager::getInstance().SetDiffUse(false);
}

/*! 差分検索 */
bool DiffLineMgr::SearchDiffMark(
	size_t			nLineNum,		// 検索開始行
	SearchDirection	bPrevOrNext,	// 0==前方検索 1==後方検索
	size_t*			pnLineNum 		// マッチ行
	)
{
	size_t nLinePos = nLineNum;

	// 前方検索
	if (bPrevOrNext == SearchDirection::Backward) {
		--nLinePos;
		const DocLine* pDocLine = docLineMgr.GetLine(nLinePos);
		while (pDocLine) {
			if (DiffLineGetter(pDocLine).GetLineDiffMark() != DiffMark::None) {
				*pnLineNum = nLinePos;				// マッチ行
				return true;
			}
			--nLinePos;
			pDocLine = pDocLine->GetPrevLine();
		}
	// 後方検索
	}else {
		++nLinePos;
		const DocLine* pDocLine = docLineMgr.GetLine(nLinePos);
		while (pDocLine) {
			if (DiffLineGetter(pDocLine).GetLineDiffMark() != DiffMark::None) {
				*pnLineNum = nLinePos;				// マッチ行
				return true;
			}
			++nLinePos;
			pDocLine = pDocLine->GetNextLine();
		}
	}
	return false;
}

/*!	差分情報を行範囲指定で登録する */
void DiffLineMgr::SetDiffMarkRange(DiffMark nMode, size_t nStartLine, size_t nEndLine)
{
	DiffManager::getInstance().SetDiffUse(true);

	if (nStartLine < 0) {
		nStartLine = 0;
	}
	// 最終行より後に削除行あり
	size_t nLines = docLineMgr.GetLineCount();
	if (nLines <= nEndLine) {
		nEndLine = nLines - 1;
		DocLine* pDocLine = docLineMgr.GetLine(nEndLine);
		if (pDocLine) {
			DiffLineSetter(pDocLine).SetLineDiffMark(DiffMark::DeleteEx);
		}
	}

	// 行範囲にマークをつける
	for (size_t i=nStartLine; i<=nEndLine; ++i) {
		DocLine* pDocLine = docLineMgr.GetLine(i);
		if (pDocLine) {
			DiffLineSetter(pDocLine).SetLineDiffMark(nMode);
		}
	}

	return;
}

