#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"
#include "docplus/FuncListManager.h"

// ViewCommanderクラスのコマンド(ジャンプ&ブックマーク)関数群

/*!	検索開始位置へ戻る */
void ViewCommander::Command_Jump_SrchStartPos(void)
{
	if (view.ptSrchStartPos_PHY.BothNatural()) {
		// 範囲選択中か
		Point pt = GetDocument().layoutMgr.LogicToLayout(view.ptSrchStartPos_PHY);
		// 選択状態を保つ
		view.MoveCursorSelecting(pt, view.GetSelectionInfo().bSelectingLock);
	}else {
		ErrorBeep();
	}
}

/*! 指定行へジャンプダイアログの表示 */
void ViewCommander::Command_Jump_Dialog(void)
{
	if (!GetEditWindow().dlgJump.DoModal(
			G_AppInstance(), view.GetHwnd(), (LPARAM)&GetDocument()
		)
	) {
		return;
	}
}


// 指定行ヘジャンプ
void ViewCommander::Command_Jump(void)
{
	auto& layoutMgr = GetDocument().layoutMgr;
	if (layoutMgr.GetLineCount() == 0) {
		ErrorBeep();
		return;
	}

	int nMode;
	bool bValidLine;
	size_t nCommentBegin = 0;

	auto& dlgJump = GetEditWindow().dlgJump;
	// 行番号
	size_t nLineNum = dlgJump.nLineNum; //$$ 単位混在
	if (!dlgJump.bPLSQL) {	// PL/SQLソースの有効行か
		// 行番号の表示 false=折り返し単位／true=改行単位
		if (GetDllShareData().bLineNumIsCRLF_ForJump) {
			if (0 == nLineNum) {
				nLineNum = 1;
			}
			/*
			  カーソル位置変換
			  ロジック位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			Point ptPosXY = layoutMgr.LogicToLayout(Point(0, (int)nLineNum - 1));
			nLineNum = ptPosXY.y + 1;
		}else {
			if (0 == nLineNum) {
				nLineNum = 1;
			}
			if (nLineNum > layoutMgr.GetLineCount()) {
				nLineNum = layoutMgr.GetLineCount();
			}
		}
		view.AddCurrentLineToHistory();
		// 選択状態を解除しないように
		view.MoveCursorSelecting(
			Point(0, (int)nLineNum - 1),
			view.GetSelectionInfo().bSelectingLock,
			_CARETMARGINRATE / 3
		);
		return;
	}
	if (0 == nLineNum) {
		nLineNum = 1;
	}
	nMode = 0;
	ASSERT_GE(dlgJump.nPLSQL_E2, 1);
	size_t nCurrentLine = dlgJump.nPLSQL_E2 - 1;
	ASSERT_GE(dlgJump.nPLSQL_E1, 1);
	size_t nLineCount = dlgJump.nPLSQL_E1 - 1;; //$$ 単位混在

	// 行番号の表示 false=折り返し単位／true=改行単位
	if (!view.pTypeData->bLineNumIsCRLF) { // レイアウト単位
		/*
		  カーソル位置変換
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		  →
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		*/
		Point ptPosXY = layoutMgr.LayoutToLogic(Point(0, (int)nLineCount));
		nLineCount = ptPosXY.y;
	}

	auto& lineMgr = GetDocument().docLineMgr;
	for (; nLineCount<lineMgr.GetLineCount(); ++nLineCount) {
		size_t nLineLen;
		const wchar_t* pLine = lineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		bValidLine = false;
		size_t i;
		for (i=0; i<nLineLen; ++i) {
			wchar_t let = pLine[i];
			if (1
				&& let != L' '
				&& let != WCODE::TAB
			) {
				break;
			}
		}
		size_t nBgn = i;
		wchar_t let = 0;
		wchar_t prevLet;
		auto& csEdit = GetDllShareData().common.edit;
		for (i=nBgn; i<nLineLen; ++i) {
			// シングルクォーテーション文字列読み込み中
			prevLet = let;
			let = pLine[i];
			if (nMode == 20) {
				bValidLine = true;
				if (let == L'\'') {
					if (i > 0 && prevLet == L'\\') {
					}else {
						nMode = 0;
						continue;
					}
				}else {
				}
			}else
			// ダブルクォーテーション文字列読み込み中
			if (nMode == 21) {
				bValidLine = true;
				if (let == L'"') {
					if (i > 0 && prevLet == L'\\') {
					}else {
						nMode = 0;
						continue;
					}
				}else {
				}
			}else
			// コメント読み込み中
			if (nMode == 8) {
				if (i < nLineLen - 1 && let == L'*' &&  pLine[i + 1] == L'/') {
					if (/*nCommentBegin != nLineCount &&*/ nCommentBegin != 0) {
						bValidLine = true;
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
					|| WCODE::IsLineDelimiter(pLine[i], csEdit.bEnableExtEol)
				) {
					continue;
				}else
				if (i < nLineLen - 1 && let == L'-' &&  pLine[i + 1] == L'-') {
					bValidLine = true;
					break;
				}else
				if (i < nLineLen - 1 && let == L'/' &&  pLine[i + 1] == L'*') {
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
					bValidLine = true;
				}
			}
		}
		// コメント読み込み中
		if (nMode == 8) {
			if (nCommentBegin != 0) {
				bValidLine = true;
			}
			// コメントブロック内の改行だけの行
			if (WCODE::IsLineDelimiter(pLine[nBgn], csEdit.bEnableExtEol)) {
				bValidLine = false;
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
	Point ptPos = layoutMgr.LogicToLayout(Point(0, (int)nLineCount));
	view.AddCurrentLineToHistory();
	// 選択状態を解除しないように
	view.MoveCursorSelecting(ptPos, view.GetSelectionInfo().bSelectingLock, _CARETMARGINRATE / 3);
}


// ブックマークの設定・解除を行う(トグル動作)
void ViewCommander::Command_Bookmark_Set(void)
{
	DocLine* pDocLine;
	auto& selInfo = view.GetSelectionInfo();
	auto& select = selInfo.select;
	auto& lineMgr = GetDocument().docLineMgr;
	if (selInfo.IsTextSelected()
		&& select.GetFrom().y < select.GetTo().y
	) {
		auto& layoutMgr = GetDocument().layoutMgr;
		Point ptFrom = layoutMgr.LayoutToLogic(Point(0, select.GetFrom().y));
		Point ptTo = layoutMgr.LayoutToLogic(Point(0, select.GetTo().y));
		for (int nY=ptFrom.y; nY<=ptTo.y; ++nY) {
			pDocLine = lineMgr.GetLine(nY);
			if (pDocLine) {
				BookmarkSetter bookmark(pDocLine);
				bookmark.SetBookmark(!bookmark.IsBookmarked());
			}
		}
	}else {
		pDocLine = lineMgr.GetLine(GetCaret().GetCaretLogicPos().y);
		if (pDocLine) {
			BookmarkSetter bookmark(pDocLine);
			bookmark.SetBookmark(!bookmark.IsBookmarked());
		}
	}

	// 分割したビューも更新
	GetEditWindow().Views_Redraw();
}


// 次のブックマークを探し，見つかったら移動する
void ViewCommander::Command_Bookmark_Next(void)
{
	int		nYOld;
	bool	bFound	=	false;
	bool	bRedo	=	true;

	Point ptXY(0, GetCaret().GetCaretLogicPos().y);
	int tmp_y;

	nYOld = ptXY.y;

re_do:;
	if (BookmarkManager(GetDocument().docLineMgr).SearchBookMark(ptXY.y, SearchDirection::Forward, &tmp_y)) {
		ptXY.y = tmp_y;
		bFound = true;
		Point ptLayout = GetDocument().layoutMgr.LogicToLayout(ptXY);
		view.MoveCursorSelecting(ptLayout, view.GetSelectionInfo().bSelectingLock);
	}
	if (GetDllShareData().common.search.bSearchAll) {
		if (!bFound		// 見つからなかった
			&& bRedo	// 最初の検索
		) {
			ptXY.y = -1;
			bRedo = false;
			goto re_do;		// 先頭から再検索
		}
	}
	if (bFound) {
		if (nYOld >= ptXY.y) {
			view.SendStatusMessage(LS(STR_ERR_SRNEXT1));
		}
	}else {
		view.SendStatusMessage(LS(STR_ERR_SRNEXT2));
		AlertNotFound(view.GetHwnd(), false, LS(STR_BOOKMARK_NEXT_NOT_FOUND));
	}
}

// 前のブックマークを探し，見つかったら移動する．
void ViewCommander::Command_Bookmark_Prev(void)
{
	int		nYOld;
	bool	bFound	=	false;
	bool	bRedo	=	true;

	Point ptXY(0, GetCaret().GetCaretLogicPos().y);
	int tmp_y;

	nYOld = ptXY.y;

re_do:;
	auto& docLineMgr = GetDocument().docLineMgr;
	if (BookmarkManager(docLineMgr).SearchBookMark(ptXY.y, SearchDirection::Backward, &tmp_y)) {
		ptXY.y = tmp_y;
		bFound = true;
		Point ptLayout = GetDocument().layoutMgr.LogicToLayout(ptXY);
		view.MoveCursorSelecting(ptLayout, view.GetSelectionInfo().bSelectingLock);
	}
	if (GetDllShareData().common.search.bSearchAll) {
		if (!bFound		// 見つからなかった
			&& bRedo	// 最初の検索
		) {
			ptXY.y = (int)docLineMgr.GetLineCount();
			bRedo = false;
			goto re_do;	// 末尾から再検索
		}
	}
	if (bFound) {
		if (nYOld <= ptXY.y) {
			view.SendStatusMessage(LS(STR_ERR_SRPREV1));
		}
	}else {
		view.SendStatusMessage(LS(STR_ERR_SRPREV2));
		AlertNotFound(view.GetHwnd(), false, LS(STR_BOOKMARK_PREV_NOT_FOUND));
	}
	return;
}


// ブックマークをクリアする
void ViewCommander::Command_Bookmark_Reset(void)
{
	BookmarkManager(GetDocument().docLineMgr).ResetAllBookMark();
	// 分割したビューも更新
	GetEditWindow().Views_Redraw();
}

void ViewCommander::Command_Bookmark_Pattern(void)
{
	// 検索or置換ダイアログから呼び出された
	if (!view.ChangeCurRegexp(false)) {
		return;
	}
	BookmarkManager(GetDocument().docLineMgr).MarkSearchWord(
		view.searchPattern
	);
	// 分割したビューも更新
	GetEditWindow().Views_Redraw();
}

// 次の関数リストマークを探し，見つかったら移動する
void ViewCommander::Command_FuncList_Next(void)
{
	Point ptXY(0, GetCaret().GetCaretLogicPos().y);
	int nYOld = ptXY.y;

	for (int n=0; n<2; ++n) {
		if (FuncListManager().SearchFuncListMark(
				GetDocument().docLineMgr,
				ptXY.y, SearchDirection::Forward, &ptXY.y)) {
			Point ptLayout = GetDocument().layoutMgr.LogicToLayout(ptXY);
			view.MoveCursorSelecting( ptLayout,
				view.GetSelectionInfo().bSelectingLock );
			if (nYOld >= ptXY.y) {
				view.SendStatusMessage(LS(STR_ERR_SRNEXT1));
			}
			return;
		}
		if (!GetDllShareData().common.search.bSearchAll) {
			break;
		}
		ptXY.y = -1;
	}
	view.SendStatusMessage(LS(STR_ERR_SRNEXT2));
	AlertNotFound( view.GetHwnd(), false, LS(STR_FUCLIST_NEXT_NOT_FOUND));
}

// 前のブックマークを探し，見つかったら移動する．
void ViewCommander::Command_FuncList_Prev(void)
{

	Point ptXY(0,GetCaret().GetCaretLogicPos().y);
	int nYOld = ptXY.y;
	auto& docLineMgr = GetDocument().docLineMgr;
	for (int n=0; n<2; ++n) {
		if (FuncListManager().SearchFuncListMark(
			docLineMgr,
			ptXY.y,
			SearchDirection::Backward,
			&ptXY.y
			)
		) {
			Point ptLayout = GetDocument().layoutMgr.LogicToLayout(ptXY);
			view.MoveCursorSelecting(
				ptLayout,
				view.GetSelectionInfo().bSelectingLock
				);
			if (nYOld <= ptXY.y) {
				view.SendStatusMessage(LS(STR_ERR_SRPREV1));
			}
			return;
		}
		if (!GetDllShareData().common.search.bSearchAll) {
			break;
		}
		ptXY.y = (int)docLineMgr.GetLineCount();
	}
	view.SendStatusMessage(LS(STR_ERR_SRPREV2));
	AlertNotFound( view.GetHwnd(), false, LS(STR_FUCLIST_PREV_NOT_FOUND) );
}

