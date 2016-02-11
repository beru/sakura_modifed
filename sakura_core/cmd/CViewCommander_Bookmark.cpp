/*!	@file
@brief ViewCommanderクラスのコマンド(ジャンプ&ブックマーク)関数群

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
#include "docplus/CFuncListManager.h"


// from CViewCommander_New.cpp
/*!	検索開始位置へ戻る
	@author	ai
	@date	02/06/26
*/
void ViewCommander::Command_JUMP_SRCHSTARTPOS(void)
{
	if (m_pCommanderView->m_ptSrchStartPos_PHY.BothNatural()) {
		LayoutPoint pt;
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
void ViewCommander::Command_JUMP_DIALOG(void)
{
	if (!GetEditWindow()->m_cDlgJump.DoModal(
			G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument()
		)
	) {
		return;
	}
}


// 指定行ヘジャンプ
void ViewCommander::Command_JUMP(void)
{
	auto& layoutMgr = GetDocument()->m_cLayoutMgr;
	if (layoutMgr.GetLineCount() == 0) {
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
			if (LogicInt(0) >= nLineNum) {
				nLineNum = LogicInt(1);
			}
			/*
			  カーソル位置変換
			  ロジック位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			LayoutPoint ptPosXY;
			layoutMgr.LogicToLayout(
				LogicPoint(0, nLineNum - 1),
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
			LayoutPoint(0, nLineNum - 1),
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
		LogicPoint ptPosXY;
		layoutMgr.LayoutToLogic(
			LayoutPoint(0, nLineCount),
			&ptPosXY
		);
		nLineCount = ptPosXY.y;
	}

	auto& lineMgr = GetDocument()->m_docLineMgr;
	for (; nLineCount<lineMgr.GetLineCount(); ++nLineCount) {
		LogicInt nLineLen;
		const wchar_t* pLine = lineMgr.GetLine(LogicInt(nLineCount))->GetDocLineStrWithEOL(&nLineLen);
		bValidLine = FALSE;
		LogicInt i;
		for (i=LogicInt(0); i<nLineLen; ++i) {
			wchar_t let = pLine[i];
			if (1
				&& let != L' '
				&& let != WCODE::TAB
			) {
				break;
			}
		}
		LogicInt nBgn = i;
		wchar_t let = 0;
		wchar_t prevLet;
		for (i=nBgn; i<nLineLen; ++i) {
			// シングルクォーテーション文字列読み込み中
			prevLet = let;
			let = pLine[i];
			if (nMode == 20) {
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
			if (nMode == 21) {
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
			if (nMode == 8) {
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
			if (nMode == 0) {
				// 空白やタブ記号等を飛ばす
				if (0
					|| let == L'\t'
					|| let == L' '
					|| WCODE::IsLineDelimiter(pLine[i], GetDllShareData().m_common.m_sEdit.m_bEnableExtEol)
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
		if (nMode == 8) {
			if (nCommentBegin != 0) {
				bValidLine = TRUE;
			}
			// コメントブロック内の改行だけの行
			if (WCODE::IsLineDelimiter(pLine[nBgn], GetDllShareData().m_common.m_sEdit.m_bEnableExtEol)) {
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
	LayoutPoint ptPos;
	layoutMgr.LogicToLayout(
		LogicPoint(0, nLineCount),
		&ptPos
	);
	// Sep. 8, 2000 genta
	m_pCommanderView->AddCurrentLineToHistory();
	// 2006.07.09 genta 選択状態を解除しないように
	m_pCommanderView->MoveCursorSelecting(ptPos, m_pCommanderView->GetSelectionInfo().m_bSelectingLock, _CARETMARGINRATE / 3);
}


// from CViewCommander_New.cpp
// ブックマークの設定・解除を行う(トグル動作)
void ViewCommander::Command_BOOKMARK_SET(void)
{
	DocLine* pCDocLine;
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	auto& sSelect = selInfo.m_sSelect;
	auto& lineMgr = GetDocument()->m_docLineMgr;
	if (selInfo.IsTextSelected()
		&& sSelect.GetFrom().y < sSelect.GetTo().y
	) {
		LogicPoint ptFrom;
		LogicPoint ptTo;
		auto& layoutMgr = GetDocument()->m_cLayoutMgr;
		layoutMgr.LayoutToLogic(
			LayoutPoint(LayoutInt(0), sSelect.GetFrom().y),
			&ptFrom
		);
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			LayoutPoint(LayoutInt(0), sSelect.GetTo().y),
			&ptTo
		);
		for (LogicInt nY=ptFrom.GetY2(); nY<=ptTo.y; ++nY) {
			pCDocLine = lineMgr.GetLine(nY);
			BookmarkSetter cBookmark(pCDocLine);
			if (pCDocLine) cBookmark.SetBookmark(!cBookmark.IsBookmarked());
		}
	}else {
		pCDocLine = lineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2());
		BookmarkSetter cBookmark(pCDocLine);
		if (pCDocLine) cBookmark.SetBookmark(!cBookmark.IsBookmarked());
	}

	// 2002.01.16 hor 分割したビューも更新
	GetEditWindow()->Views_Redraw();
}


// from CViewCommander_New.cpp
// 次のブックマークを探し，見つかったら移動する
void ViewCommander::Command_BOOKMARK_NEXT(void)
{
	int		nYOld;				// hor
	BOOL	bFound	=	FALSE;	// hor
	BOOL	bRedo	=	TRUE;	// hor

	LogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	LogicInt tmp_y;

	nYOld = ptXY.y;					// hor

re_do:;								// hor
	if (BookmarkManager(&GetDocument()->m_docLineMgr).SearchBookMark(ptXY.GetY2(), SearchDirection::Forward, &tmp_y)) {
		ptXY.y = tmp_y;
		bFound = TRUE;
		LayoutPoint ptLayout;
		GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
		// 2006.07.09 genta 新規関数にまとめた
		m_pCommanderView->MoveCursorSelecting(ptLayout, m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
	}
    // 2002.01.26 hor
	if (GetDllShareData().m_common.m_sSearch.m_bSearchAll) {
		if (!bFound	&&		// 見つからなかった
			bRedo			// 最初の検索
		) {
			ptXY.y = -1;	// 2002/06/01 MIK
			bRedo = FALSE;
			goto re_do;		// 先頭から再検索
		}
	}
	if (bFound) {
		if (nYOld >= ptXY.y) {
			m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT1));
		}
	}else {
		m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT2));
		AlertNotFound(m_pCommanderView->GetHwnd(), false, LS(STR_BOOKMARK_NEXT_NOT_FOUND));
	}
	return;
}


// from CViewCommander_New.cpp
// 前のブックマークを探し，見つかったら移動する．
void ViewCommander::Command_BOOKMARK_PREV(void)
{
	int		nYOld;				// hor
	BOOL	bFound	=	FALSE;	// hor
	BOOL	bRedo	=	TRUE;	// hor

	LogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	LogicInt tmp_y;

	nYOld = ptXY.y;						// hor

re_do:;								// hor
	if (BookmarkManager(&GetDocument()->m_docLineMgr).SearchBookMark(ptXY.GetY2(), SearchDirection::Backward, &tmp_y)) {
		ptXY.y = tmp_y;
		bFound = TRUE;				// hor
		LayoutPoint ptLayout;
		GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
		// 2006.07.09 genta 新規関数にまとめた
		m_pCommanderView->MoveCursorSelecting(ptLayout, m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
	}
    // 2002.01.26 hor
	if (GetDllShareData().m_common.m_sSearch.m_bSearchAll) {
		if (!bFound	&&	// 見つからなかった
			bRedo		// 最初の検索
		) {
			// 2011.02.02 m_cLayoutMgr→m_docLineMgr
			ptXY.y = GetDocument()->m_docLineMgr.GetLineCount();	// 2002/06/01 MIK
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
// ブックマークをクリアする
void ViewCommander::Command_BOOKMARK_RESET(void)
{
	BookmarkManager(&GetDocument()->m_docLineMgr).ResetAllBookMark();
	// 2002.01.16 hor 分割したビューも更新
	GetEditWindow()->Views_Redraw();
}


// from CViewCommander_New.cpp
// 指定パターンに一致する行をマーク 2002.01.16 hor
// キーマクロで記録できるように	2002.02.08 hor
void ViewCommander::Command_BOOKMARK_PATTERN(void)
{
	// 検索or置換ダイアログから呼び出された
	if (!m_pCommanderView->ChangeCurRegexp(false)) {
		return;
	}
	BookmarkManager(&GetDocument()->m_docLineMgr).MarkSearchWord(
		m_pCommanderView->m_sSearchPattern
	);
	// 2002.01.16 hor 分割したビューも更新
	GetEditWindow()->Views_Redraw();
}



//! 次の関数リストマークを探し，見つかったら移動する
void ViewCommander::Command_FUNCLIST_NEXT(void)
{
	LogicPoint	ptXY(0, GetCaret().GetCaretLogicPos().y);
	int nYOld = ptXY.y;

	for (int n=0; n<2; ++n) {
		if (FuncListManager().SearchFuncListMark(&GetDocument()->m_docLineMgr,
				ptXY.GetY2(), SearchDirection::Forward, &ptXY.y)) {
			LayoutPoint ptLayout;
			GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY,&ptLayout);
			m_pCommanderView->MoveCursorSelecting( ptLayout,
				m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
			if (nYOld >= ptXY.y) {
				m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT1));
			}
			return;
		}
		if (!GetDllShareData().m_common.m_sSearch.m_bSearchAll) {
			break;
		}
		ptXY.y = -1;
	}
	m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT2));
	AlertNotFound( m_pCommanderView->GetHwnd(), false, LS(STR_FUCLIST_NEXT_NOT_FOUND));
	return;
}



//! 前のブックマークを探し，見つかったら移動する．
void ViewCommander::Command_FUNCLIST_PREV(void)
{

	LogicPoint	ptXY(0,GetCaret().GetCaretLogicPos().y);
	int nYOld = ptXY.y;

	for (int n=0; n<2; ++n) {
		if (FuncListManager().SearchFuncListMark(
			&GetDocument()->m_docLineMgr,
			ptXY.GetY2(),
			SearchDirection::Backward,
			&ptXY.y
			)
		) {
			LayoutPoint ptLayout;
			GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
			m_pCommanderView->MoveCursorSelecting(
				ptLayout,
				m_pCommanderView->GetSelectionInfo().m_bSelectingLock
				);
			if (nYOld <= ptXY.y) {
				m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV1));
			}
			return;
		}
		if (!GetDllShareData().m_common.m_sSearch.m_bSearchAll) {
			break;
		}
		ptXY.y= GetDocument()->m_docLineMgr.GetLineCount();
	}
	m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV2));
	AlertNotFound( m_pCommanderView->GetHwnd(), false, LS(STR_FUCLIST_PREV_NOT_FOUND) );
	return;
}

