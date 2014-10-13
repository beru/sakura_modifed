/*!	@file
@brief CViewCommanderクラスのコマンド(ジャンプ&ブックマーク)関数群

	2012/12/17	CViewCommander.cpp,CViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, hor, YAZAKI, MIK
	Copyright (C) 2006, genta

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"


// from CViewCommander_New.cpp
/*!	検索開始位置へ戻る
	@author	ai
	@date	02/06/26
*/
void CViewCommander::Command_JUMP_SRCHSTARTPOS(void)
{
	if (m_pCommanderView->m_ptSrchStartPos_PHY.BothNatural()) {
		CLayoutPoint pt;
		// 範囲選択中か
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			m_pCommanderView->m_ptSrchStartPos_PHY,
			&pt
		);
		// 2006.07.09 genta 選択状態を保つ
		m_pCommanderView->MoveCursorSelecting(pt, m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
	}else {
		ErrorBeep();
	}
	return;
}


/*! 指定行へジャンプダイアログの表示
	2002.2.2 YAZAKI
*/
void CViewCommander::Command_JUMP_DIALOG(void)
{
	if (!GetEditWindow()->m_cDlgJump.DoModal(
			G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument()
		)
	) {
		return;
	}
}


// 指定行ヘジャンプ
void CViewCommander::Command_JUMP(void)
{
	auto& layoutMgr = GetDocument()->m_cLayoutMgr;
	if (0 == layoutMgr.GetLineCount()) {
		ErrorBeep();
		return;
	}

	int nMode;
	int bValidLine;
	int nCurrentLine;
	int nCommentBegin = 0;

	auto& dlgJump = GetEditWindow()->m_cDlgJump;

	// 行番号
	int	nLineNum; //$$ 単位混在
	nLineNum = dlgJump.m_nLineNum;

	if (!dlgJump.m_bPLSQL) {	// PL/SQLソースの有効行か
		// 行番号の表示 false=折り返し単位／true=改行単位
		if (GetDllShareData().m_bLineNumIsCRLF_ForJump) {
			if (CLogicInt(0) >= nLineNum) {
				nLineNum = CLogicInt(1);
			}
			/*
			  カーソル位置変換
			  ロジック位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			CLayoutPoint ptPosXY;
			layoutMgr.LogicToLayout(
				CLogicPoint(0, nLineNum - 1),
				&ptPosXY
			);
			nLineNum = (Int)ptPosXY.y + 1;
		}else {
			if (0 >= nLineNum) {
				nLineNum = 1;
			}
			if (nLineNum > layoutMgr.GetLineCount()) {
				nLineNum = (Int)layoutMgr.GetLineCount();
			}
		}
		// Sep. 8, 2000 genta
		m_pCommanderView->AddCurrentLineToHistory();
		// 2006.07.09 genta 選択状態を解除しないように
		m_pCommanderView->MoveCursorSelecting(
			CLayoutPoint(0, nLineNum - 1),
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock,
			_CARETMARGINRATE / 3
		);
		return;
	}
	if (0 >= nLineNum) {
		nLineNum = 1;
	}
	nMode = 0;
	nCurrentLine = dlgJump.m_nPLSQL_E2 - 1;

	int	nLineCount; //$$ 単位混在
	nLineCount = dlgJump.m_nPLSQL_E1 - 1;

	// 行番号の表示 false=折り返し単位／true=改行単位
	if (!m_pCommanderView->m_pTypeData->m_bLineNumIsCRLF) { // レイアウト単位
		/*
		  カーソル位置変換
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		  →
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		*/
		CLogicPoint ptPosXY;
		layoutMgr.LayoutToLogic(
			CLayoutPoint(0, nLineCount),
			&ptPosXY
		);
		nLineCount = ptPosXY.y;
	}

	auto& lineMgr = GetDocument()->m_cDocLineMgr;
	for (; nLineCount <  lineMgr.GetLineCount(); ++nLineCount) {
		CLogicInt nLineLen;
		const wchar_t* pLine = lineMgr.GetLine(CLogicInt(nLineCount))->GetDocLineStrWithEOL(&nLineLen);
		bValidLine = FALSE;
		CLogicInt i;
		for (i = CLogicInt(0); i < nLineLen; ++i) {
			wchar_t let = pLine[i];
			if (1
				&& let != L' '
				&& let != WCODE::TAB
			) {
				break;
			}
		}
		CLogicInt nBgn = i;
		wchar_t let = 0;
		wchar_t prevLet;
		for (i = nBgn; i < nLineLen; ++i) {
			// シングルクォーテーション文字列読み込み中
			prevLet = let;
			let = pLine[i];
			if (20 == nMode) {
				bValidLine = TRUE;
				if (let == L'\'') {
					if (i > 0 && L'\\' == prevLet) {
					}else {
						nMode = 0;
						continue;
					}
				}else {
				}
			}else
			// ダブルクォーテーション文字列読み込み中
			if (21 == nMode) {
				bValidLine = TRUE;
				if (let == L'"') {
					if (i > 0 && L'\\' == prevLet) {
					}else {
						nMode = 0;
						continue;
					}
				}else {
				}
			}else
			// コメント読み込み中
			if (8 == nMode) {
				if (i < nLineLen - 1 && let == L'*' &&  L'/' == pLine[i + 1]) {
					if (/*nCommentBegin != nLineCount &&*/ nCommentBegin != 0) {
						bValidLine = TRUE;
					}
					++i;
					nMode = 0;
					continue;
				}else {
				}
			}else
			// ノーマルモード
			if (0 == nMode) {
				// 空白やタブ記号等を飛ばす
				if (0
					|| let == L'\t'
					|| let == L' '
					|| WCODE::IsLineDelimiter(let)
				) {
					continue;
				}else
				if (i < nLineLen - 1 && let == L'-' &&  L'-' == pLine[i + 1]) {
					bValidLine = TRUE;
					break;
				}else
				if (i < nLineLen - 1 && let == L'/' &&  L'*' == pLine[i + 1]) {
					++i;
					nMode = 8;
					nCommentBegin = nLineCount;
					continue;
				}else
				if (let == L'\'') {
					nMode = 20;
					continue;
				}else
				if (let == L'"') {
					nMode = 21;
					continue;
				}else {
					bValidLine = TRUE;
				}
			}
		}
		// コメント読み込み中
		if (8 == nMode) {
			if (nCommentBegin != 0) {
				bValidLine = TRUE;
			}
			// コメントブロック内の改行だけの行
			if (WCODE::IsLineDelimiter(pLine[nBgn])) {
				bValidLine = FALSE;
			}
		}
		if (bValidLine) {
			++nCurrentLine;
			if (nCurrentLine >= nLineNum) {
				break;
			}
		}
	}
	/*
	  カーソル位置変換
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	  →
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	*/
	CLayoutPoint ptPos;
	layoutMgr.LogicToLayout(
		CLogicPoint(0, nLineCount),
		&ptPos
	);
	// Sep. 8, 2000 genta
	m_pCommanderView->AddCurrentLineToHistory();
	// 2006.07.09 genta 選択状態を解除しないように
	m_pCommanderView->MoveCursorSelecting(ptPos, m_pCommanderView->GetSelectionInfo().m_bSelectingLock, _CARETMARGINRATE / 3);
}


// from CViewCommander_New.cpp
//! ブックマークの設定・解除を行う(トグル動作)
void CViewCommander::Command_BOOKMARK_SET(void)
{
	CDocLine* pCDocLine;
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	auto& sSelect = selInfo.m_sSelect;
	auto& lineMgr = GetDocument()->m_cDocLineMgr;
	if (selInfo.IsTextSelected()
		&& sSelect.GetFrom().y < sSelect.GetTo().y
	) {
		CLogicPoint ptFrom;
		CLogicPoint ptTo;
		auto& layoutMgr = GetDocument()->m_cLayoutMgr;
		layoutMgr.LayoutToLogic(
			CLayoutPoint(CLayoutInt(0), sSelect.GetFrom().y),
			&ptFrom
		);
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			CLayoutPoint(CLayoutInt(0), sSelect.GetTo().y),
			&ptTo
		);
		for (CLogicInt nY = ptFrom.GetY2(); nY <= ptTo.y; nY++) {
			pCDocLine = lineMgr.GetLine(nY);
			CBookmarkSetter cBookmark(pCDocLine);
			if (pCDocLine) cBookmark.SetBookmark(!cBookmark.IsBookmarked());
		}
	}else {
		pCDocLine = lineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2());
		CBookmarkSetter cBookmark(pCDocLine);
		if (pCDocLine) cBookmark.SetBookmark(!cBookmark.IsBookmarked());
	}

	// 2002.01.16 hor 分割したビューも更新
	GetEditWindow()->Views_Redraw();
}


// from CViewCommander_New.cpp
//! 次のブックマークを探し，見つかったら移動する
void CViewCommander::Command_BOOKMARK_NEXT(void)
{
	int		nYOld;				// hor
	BOOL	bFound	=	FALSE;	// hor
	BOOL	bRedo	=	TRUE;	// hor

	CLogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	CLogicInt tmp_y;

	nYOld = ptXY.y;					// hor

re_do:;								// hor
	if (CBookmarkManager(&GetDocument()->m_cDocLineMgr).SearchBookMark(ptXY.GetY2(), SEARCH_FORWARD, &tmp_y)) {
		ptXY.y = tmp_y;
		bFound = TRUE;
		CLayoutPoint ptLayout;
		GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
		// 2006.07.09 genta 新規関数にまとめた
		m_pCommanderView->MoveCursorSelecting(ptLayout, m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
	}
    // 2002.01.26 hor
	if (GetDllShareData().m_Common.m_sSearch.m_bSearchAll) {
		if (!bFound	&&		// 見つからなかった
			bRedo			// 最初の検索
		) {
			ptXY.y = -1;	// 2002/06/01 MIK
			bRedo = FALSE;
			goto re_do;		// 先頭から再検索
		}
	}
	if (bFound) {
		if (nYOld >= ptXY.y) m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT1));
	}else {
		m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT2));
		AlertNotFound(m_pCommanderView->GetHwnd(), false, LS(STR_BOOKMARK_NEXT_NOT_FOUND));
	}
	return;
}


// from CViewCommander_New.cpp
//! 前のブックマークを探し，見つかったら移動する．
void CViewCommander::Command_BOOKMARK_PREV(void)
{
	int		nYOld;				// hor
	BOOL	bFound	=	FALSE;	// hor
	BOOL	bRedo	=	TRUE;	// hor

	CLogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	CLogicInt tmp_y;

	nYOld = ptXY.y;						// hor

re_do:;								// hor
	if (CBookmarkManager(&GetDocument()->m_cDocLineMgr).SearchBookMark(ptXY.GetY2(), SEARCH_BACKWARD, &tmp_y)) {
		ptXY.y = tmp_y;
		bFound = TRUE;				// hor
		CLayoutPoint ptLayout;
		GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
		// 2006.07.09 genta 新規関数にまとめた
		m_pCommanderView->MoveCursorSelecting(ptLayout, m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
	}
    // 2002.01.26 hor
	if (GetDllShareData().m_Common.m_sSearch.m_bSearchAll) {
		if (!bFound	&&	// 見つからなかった
			bRedo		// 最初の検索
		) {
			// 2011.02.02 m_cLayoutMgr→m_cDocLineMgr
			ptXY.y = GetDocument()->m_cDocLineMgr.GetLineCount();	// 2002/06/01 MIK
			bRedo = FALSE;
			goto re_do;	// 末尾から再検索
		}
	}
	if (bFound) {
		if (nYOld <= ptXY.y) {
			m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV1));
		}
	}else {
		m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV2));
		AlertNotFound(m_pCommanderView->GetHwnd(), false, LS(STR_BOOKMARK_PREV_NOT_FOUND));
	}
	return;
}


// from CViewCommander_New.cpp
//! ブックマークをクリアする
void CViewCommander::Command_BOOKMARK_RESET(void)
{
	CBookmarkManager(&GetDocument()->m_cDocLineMgr).ResetAllBookMark();
	// 2002.01.16 hor 分割したビューも更新
	GetEditWindow()->Views_Redraw();
}


// from CViewCommander_New.cpp
// 指定パターンに一致する行をマーク 2002.01.16 hor
// キーマクロで記録できるように	2002.02.08 hor
void CViewCommander::Command_BOOKMARK_PATTERN(void)
{
	// 検索or置換ダイアログから呼び出された
	if (!m_pCommanderView->ChangeCurRegexp(false)) {
		return;
	}
	CBookmarkManager(&GetDocument()->m_cDocLineMgr).MarkSearchWord(
		m_pCommanderView->m_sSearchPattern
	);
	// 2002.01.16 hor 分割したビューも更新
	GetEditWindow()->Views_Redraw();
}

