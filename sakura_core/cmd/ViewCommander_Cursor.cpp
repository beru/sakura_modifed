/*!	@file
@brief ViewCommanderクラスのコマンド(カーソル移動系)関数群

	2012/12/17	ViewCommander.cpp,ViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, asa-o, hor
	Copyright (C) 2002, hor, YAZAKI, oak
	Copyright (C) 2003, Moca
	Copyright (C) 2004, Moca, genta, fotomo
	Copyright (C) 2006, genta
	Copyright (C) 2007, kobake, maru
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "MarkMgr.h"/// 2002/2/3 aroka 追加
#include "mem/MemoryIterator.h"	// @@@ 2002.09.28 YAZAKI


void ViewCommander::Command_MoveCursor(Point pos, int option)
{
	if (pos.x < 0 || pos.y < 0) {
		ErrorBeep();
		return;
	}
	Point layoutPos = GetDocument().layoutMgr.LogicToLayout(pos);
	Command_MoveCursorLayout(layoutPos, option);
}

void ViewCommander::Command_MoveCursorLayout(Point pos, int option)
{
	if (pos.x < 0 || pos.y < 0) {
		ErrorBeep();
		return;
	}

	bool bSelect = (option & 0x01) == 0x01;
	bool bBoxSelect = (option & 0x02) == 0x02;

	auto& si = view.GetSelectionInfo();

	if (bSelect || bBoxSelect) {
		if (!si.IsTextSelected()) {
			if (bBoxSelect) {
				Command_Begin_BoxSelect();
			}else {
				si.BeginSelectArea();
			}
		}else {
			// 2014.01.08 追加
			if (bBoxSelect && !si.IsBoxSelecting()) {
				// 通常選択→矩形選択に変更。他のコマンドに合わせる
				Command_Begin_BoxSelect();
			}
		}
	}else {
		if (si.IsTextSelected()) {
			si.DisableSelectArea(true);
		}else if (si.IsBoxSelecting()) {
			si.SetBoxSelect(false);
		}
	}
	auto& caret = GetCaret();
	caret.GetAdjustCursorPos(&pos);
	// 選択
	if (bSelect || bBoxSelect) {
		si.ChangeSelectAreaByCurrentCursor(pos);
	}
	caret.MoveCursor(pos, true);
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
}


/////////////////////////////////// 以下はコマンド群 (Oct. 17, 2000 jepro note) ///////////////////////////////////////////

// カーソル上移動
int ViewCommander::Command_Up(bool bSelect, bool bRepeat, int lines)
{
	auto& caret = GetCaret();
	// From Here Oct. 24, 2001 genta
	if (lines != 0) {
		caret.Cursor_UPDOWN(lines, false);
		return 1;
	}
	// To Here Oct. 24, 2001 genta


	int nRepeat = 0;

	// キーリピート時のScrollを滑らかにするか
	auto& csGeneral = GetDllShareData().common.general;
	if (!csGeneral.nRepeatedScroll_Smooth) {
		int i;
		if (!bRepeat) {
			i = -1;
		}else {
			i = -1 * csGeneral.nRepeatedScrollLineNum;	// キーリピート時のScroll行数
		}
		caret.Cursor_UPDOWN(i, bSelect);
		nRepeat = -1 * i;
	}else {
		++nRepeat;
		if (caret.Cursor_UPDOWN(-1, bSelect) != 0 && bRepeat) {
			for (int i=0; i<csGeneral.nRepeatedScrollLineNum-1; ++i) {		// キーリピート時のScroll行数
				::UpdateWindow(view.GetHwnd());	// YAZAKI
				caret.Cursor_UPDOWN(-1, bSelect);
				++nRepeat;
			}
		}
	}
	return nRepeat;
}


// カーソル下移動
int ViewCommander::Command_Down(bool bSelect, bool bRepeat)
{
	auto& caret = GetCaret();
	int nRepeat = 0;
	auto& csGeneral = GetDllShareData().common.general;
	// キーリピート時のScrollを滑らかにするか
	if (!csGeneral.nRepeatedScroll_Smooth) {
		int i;
		if (!bRepeat) {
			i = 1;
		}else {
			i = csGeneral.nRepeatedScrollLineNum;	// キーリピート時のScroll行数
		}
		caret.Cursor_UPDOWN(i, bSelect);
		nRepeat = i;
	}else {
		++nRepeat;
		if (caret.Cursor_UPDOWN(1, bSelect) != 0 && bRepeat) {
			for (int i=0; i<csGeneral.nRepeatedScrollLineNum-1; ++i) {	// キーリピート時のScroll行数
				// ここで再描画。
				::UpdateWindow(view.GetHwnd());	// YAZAKI
				caret.Cursor_UPDOWN(1, bSelect);
				++nRepeat;
			}
		}
	}
	return nRepeat;
}


/*! @brief カーソル左移動

	@date 2004.03.28 Moca EOFだけの行以降の途中にカーソルがあると落ちるバグ修正．
			pLayout == NULLかつキャレット位置が行頭以外の場合は
			2つのifのどちらにも当てはまらないが，そのあとのMoveCursorにて適正な
			位置に移動させられる．
	@date 2014.01.10 Moca キーリピート時、MoveCursorを一度にまとめる
*/
int ViewCommander::Command_Left(bool bSelect, bool bRepeat)
{
	bool bUnderlineDoNotOFF = true;		// アンダーラインを消去しない
	if (bSelect) {
		bUnderlineDoNotOFF = false;		// 選択状態ならアンダーライン消去を行う
	}
	bool bMoveCaretLine = false;
	int nRepeat = bRepeat ? 2 : 1;
	int nRes = 0;
	auto& caret = GetCaret();
	Point ptCaretMove = caret.GetCaretLayoutPos();
	auto& selInfo = view.GetSelectionInfo();
	for (int nRepCount=0; nRepCount<nRepeat; ++nRepCount) {
		if (bSelect && ! selInfo.IsTextSelected()) {
			// 現在のカーソル位置から選択を開始する
			selInfo.BeginSelectArea();
		}
		if (!bSelect) {
			if (selInfo.IsTextSelected()) {
				this->Command_Cancel_Mode(1);
				nRes = 1;
				continue; // 選択のキャンセルで左移動を 1消費。この後の移動処理はスキップする。
			}else if (selInfo.IsBoxSelecting()) {
				selInfo.SetBoxSelect(false);
			}
		}
		// (これから求める)カーソルの移動先。
		Point ptPos(0, ptCaretMove.y);

		auto& layoutMgr = GetDocument().layoutMgr;
		// 現在行のデータを取得
		const Layout* pLayout = layoutMgr.SearchLineByLayoutY(ptCaretMove.y);
		// カーソルが左端にある
		if (ptCaretMove.x == (pLayout ? pLayout->GetIndent() : 0)) {
			if (0 < ptCaretMove.y
			   && ! selInfo.IsBoxSelecting()
			) {
				// 前のレイアウト行の、折り返し桁一つ手前または改行文字の手前に移動する。
				pLayout = layoutMgr.SearchLineByLayoutY(ptCaretMove.y - 1);
				MemoryIterator it(pLayout, layoutMgr.GetTabSpace());
				while (!it.end()) {
					it.scanNext();
					if (it.getIndex() + it.getIndexDelta() > pLayout->GetLengthWithoutEOL()) {
						ptPos.x += it.getColumnDelta();
						break;
					}
					it.addDelta();
				}
				ptPos.x += it.getColumn() - it.getColumnDelta();
				ptPos.y --;
			}else {
				if (0 < nRepCount) {
					caret.MoveCursor( ptCaretMove, true, _CARETMARGINRATE, bUnderlineDoNotOFF );
					caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
				}
				nRes = 0;
				break; // これ以上左に動けぬ。
			}
			bUnderlineDoNotOFF = false;	// 行が変わるのでアンダーラインを消去する
			bMoveCaretLine = true;
		}
		//  2004.03.28 Moca EOFだけの行以降の途中にカーソルがあると落ちるバグ修正
		else if (pLayout) {
			MemoryIterator it(pLayout, layoutMgr.GetTabSpace());
			while (!it.end()) {
				it.scanNext();
				if ((int)(it.getColumn() + it.getColumnDelta()) > ptCaretMove.x - 1){
					ptPos.x += it.getColumnDelta();
					break;
				}
				it.addDelta();
			}
			ptPos.x += it.getColumn() - it.getColumnDelta();
			// Oct. 18, 2002 YAZAKI
			if (it.getIndex() >= pLayout->GetLengthWithEOL()) {
				ptPos.x = ptCaretMove.x - 1;
			}
		}

		caret.GetAdjustCursorPos( &ptPos );
		if (bSelect) {
			/*	現在のカーソル位置によって選択範囲を変更．
				2004.04.02 Moca 
				キャレット位置が不正だった場合にMoveCursorの移動結果が
				引数で与えた座標とは異なることがあるため，
				ptPosの代わりに実際の移動結果を使うように．
			*/
			selInfo.ChangeSelectAreaByCurrentCursor(ptPos);
		}
		if (bMoveCaretLine || nRepeat - 1 == nRepCount) {
			caret.MoveCursor( ptPos, true, _CARETMARGINRATE, bUnderlineDoNotOFF );
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		}
		ptCaretMove = ptPos;
		nRes = 1;
	}
	return nRes;
}


/* カーソル右移動
	@date 2014.01.10 Moca キーリピート時、MoveCursorを一度にまとめる
*/
void ViewCommander::Command_Right(
	bool bSelect,
	bool bIgnoreCurrentSelection,
	bool bRepeat
	)
{
	bool bUnderlineDoNotOFF = true;	// アンダーラインを消去しない
	if (bSelect) {
		bUnderlineDoNotOFF = false;		// 選択状態ならアンダーライン消去を行う
	}
	bool bMoveCaretLine = false;
	int nRepeat = bRepeat ? 2 : 1; // 移動する回数
	auto& caret = GetCaret();
	Point ptCaretMove = caret.GetCaretLayoutPos();
	auto& selInfo = view.GetSelectionInfo();
	for (int nRepCount=0; nRepCount<nRepeat; ++nRepCount) {
		// 2003.06.28 Moca [EOF]のみの行にカーソルがあるときに右を押しても選択を解除できない問題に
		// 対応するため、現在行のデータを取得を移動
		if (!bIgnoreCurrentSelection) {
			if (bSelect && ! selInfo.IsTextSelected()) {
				// 現在のカーソル位置から選択を開始する
				selInfo.BeginSelectArea();
			}
			if (!bSelect) {
				if (selInfo.IsTextSelected()) {
					this->Command_Cancel_Mode(2);
					continue; // 選択のキャンセルで右移動を 1消費。この後の移動処理はスキップする。
				}else if (selInfo.IsBoxSelecting()) {
					selInfo.SetBoxSelect(false);
				}
			}
		}
//		2003.06.28 Moca [EOF]のみの行にカーソルがあるときに右を押しても選択を解除できない問題に対応

		// (これから求める)カーソルの移動先。
		Point ptTo(0, 0);
		const Point ptCaret = ptCaretMove;

		auto& layoutMgr = GetDocument().layoutMgr;
		// 現在行のデータを取得
		const Layout* const pcLayout = layoutMgr.SearchLineByLayoutY(ptCaret.y);
		// 2004.04.02 EOF以降にカーソルがあったときに右を押しても何も起きなかったのを、EOFに移動するように
		if (pcLayout) {
			// キャレット位置のレイアウト行について。
			const size_t x_wrap = pcLayout->CalcLayoutWidth(layoutMgr); // 改行文字、または折り返しの位置。
			const bool wrapped = EolType::None == pcLayout->GetLayoutEol(); // 折り返しているか、改行文字で終わっているか。これにより x_wrapの意味が変わる。
			const bool nextline_exists = pcLayout->GetNextLayout() || pcLayout->GetLayoutEol() != EolType::None; // EOFのみの行も含め、キャレットが移動可能な次行が存在するか。

			// 現在のキャレットの右の位置(to_x)を求める。
			MemoryIterator it(pcLayout, layoutMgr.GetTabSpace());
			for (; !it.end(); it.scanNext(), it.addDelta()) {
				if (ptCaret.x < (int)it.getColumn()) {
					break;
				}
			}
			ASSERT_GE(ptCaret.x, -2);
			const size_t to_x = t_max(it.getColumn(), (size_t)ptCaret.x + 1);

			// キャレットの右端(x_max)と、そこでの扱い(on_x_max)を決める。
			size_t x_max;
			enum {
				STOP,
				MOVE_NEXTLINE_IMMEDIATELY, // 右端に止まらず次の行頭に移動する。(折り返しなど)
				MOVE_NEXTLINE_NEXTTIME, // 右端に止まり、次に次の行頭に移動する。(改行を超えるときなど)
				MOVE_NEXTLINE_NEXTTIME_AND_MOVE_RIGHT // 右端に止まり、次に次の行頭の一つ右に移動する。(折り返しなど)
			} on_x_max;

			if (selInfo.IsBoxSelecting()) {
				x_max = t_max(x_wrap, layoutMgr.GetMaxLineKetas());
				on_x_max = STOP;
			}else if (GetDllShareData().common.general.bIsFreeCursorMode) {
				// フリーカーソルモードでは折り返し位置だけをみて、改行文字の位置はみない。
				if (wrapped) {
					if (nextline_exists) {
						x_max = x_wrap;
						on_x_max = MOVE_NEXTLINE_IMMEDIATELY;
					}else {
						// データのあるEOF行は折り返しではない
						x_max = t_max(x_wrap, layoutMgr.GetMaxLineKetas());
						on_x_max = STOP;
					}
				}else {
					if (x_wrap < layoutMgr.GetMaxLineKetas()) {
						x_max = layoutMgr.GetMaxLineKetas();
						on_x_max = MOVE_NEXTLINE_IMMEDIATELY;
					}else { // 改行文字がぶら下がっているときは例外。
						x_max = x_wrap;
						on_x_max = MOVE_NEXTLINE_NEXTTIME;
					}
				}
			}else {
				x_max = x_wrap;
				on_x_max = wrapped ? MOVE_NEXTLINE_IMMEDIATELY : MOVE_NEXTLINE_NEXTTIME;
			}

			// キャレットの移動先を決める。
			if (nextline_exists
				&& (on_x_max == MOVE_NEXTLINE_IMMEDIATELY && x_max <= to_x
					|| on_x_max == MOVE_NEXTLINE_NEXTTIME && x_max < to_x
					|| on_x_max == MOVE_NEXTLINE_NEXTTIME_AND_MOVE_RIGHT && x_max < to_x
				)
			) {
				ptTo.y = ptCaret.y + 1;
				ptTo.x = pcLayout->GetNextLayout() ? pcLayout->GetNextLayout()->GetIndent() : 0;
				if (on_x_max == MOVE_NEXTLINE_NEXTTIME_AND_MOVE_RIGHT) {
					++nRepeat;
				}
				bUnderlineDoNotOFF = false;
				bMoveCaretLine = true;
			}else {
				ptTo.y = ptCaret.y;
				ptTo.x = (int)t_min(to_x, x_max);
			}
		}else {
			// pLayoutがNULLの場合はptPos.x=0に調整
			ptTo.y = ptCaret.y;
			ptTo.x = 0;
		}

		caret.GetAdjustCursorPos( &ptTo );
		if (bSelect) {
			// 現在のカーソル位置によって選択範囲を変更
			selInfo.ChangeSelectAreaByCurrentCursor( ptTo );
		}

		if (bMoveCaretLine || nRepeat - 1 == nRepCount) {
			caret.MoveCursor( ptTo, true, _CARETMARGINRATE, bUnderlineDoNotOFF );
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		}
		ptCaretMove = ptTo;
	}
	return;
}


// カーソル上移動(２行づつ)
void ViewCommander::Command_Up2(bool bSelect)
{
	GetCaret().Cursor_UPDOWN(-2, bSelect);
	return;
}


// カーソル下移動(２行づつ)
void ViewCommander::Command_Down2(bool bSelect)
{
	GetCaret().Cursor_UPDOWN(2, bSelect);
	return;
}


// 単語の左端に移動
void ViewCommander::Command_WordLeft(bool bSelect)
{
	bool bUnderlineDoNotOFF = true;		// アンダーラインを消去しない
	if (bSelect) {
		bUnderlineDoNotOFF = false;		// 選択状態ならアンダーライン消去を行う
	}
	auto& si = view.GetSelectionInfo();
	if (bSelect) {
		if (!si.IsTextSelected()) {		// テキストが選択されているか
			// 現在のカーソル位置から選択を開始する
			si.BeginSelectArea();
		}
	}else {
		if (si.IsTextSelected()) {		// テキストが選択されているか
			// 現在の選択範囲を非選択状態に戻す
			si.DisableSelectArea(true);
		}else if (si.IsBoxSelecting()) {
			si.SetBoxSelect(false);
		}
	}

	auto& caret = GetCaret();
	auto& csGeneral = GetDllShareData().common.general;
	auto& layoutMgr = GetDocument().layoutMgr;
	const Layout* pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
	if (!pLayout) {
		bool bIsFreeCursorModeOld = csGeneral.bIsFreeCursorMode;	// フリーカーソルモードか
		csGeneral.bIsFreeCursorMode = false;
		// カーソル左移動
		Command_Left(bSelect, false);
		csGeneral.bIsFreeCursorMode = bIsFreeCursorModeOld;	// フリーカーソルモードか
		return;
	}

	// 指定された桁に対応する行のデータ内の位置を調べる
	size_t nIdx = view.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().x);
	// 現在位置の左の単語の先頭位置を調べる
	Point ptLayoutNew;
	int nResult = layoutMgr.PrevWord(
		caret.GetCaretLayoutPos().y,
		nIdx,
		&ptLayoutNew,
		csGeneral.bStopsBothEndsWhenSearchWord
	);
	if (nResult) {
		// 行が変わった
		if (ptLayoutNew.y != caret.GetCaretLayoutPos().y) {
			pLayout = layoutMgr.SearchLineByLayoutY(ptLayoutNew.y);
			if (!pLayout) {
				return;
			}
			bUnderlineDoNotOFF = false;
		}

		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
		/*
		ptLayoutNew.x = view.LineIndexToColumn(pLayout, ptLayoutNew.x);
		*/

		// カーソル移動
		caret.GetAdjustCursorPos(&ptLayoutNew);
		if (bSelect) {
			// 現在のカーソル位置によって選択範囲を変更
			si.ChangeSelectAreaByCurrentCursor(ptLayoutNew);
		}
		caret.MoveCursor(ptLayoutNew, true, _CARETMARGINRATE, bUnderlineDoNotOFF);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
	}else {
		bool bIsFreeCursorModeOld = csGeneral.bIsFreeCursorMode;	// フリーカーソルモードか
		csGeneral.bIsFreeCursorMode = false;
		// カーソル左移動
		Command_Left(bSelect, false);
		csGeneral.bIsFreeCursorMode = bIsFreeCursorModeOld;	// フリーカーソルモードか
	}
	return;
}


// 単語の右端に移動
void ViewCommander::Command_WordRight(bool bSelect)
{
	bool bUnderlineDoNotOFF = true;	// アンダーラインを消去しない
	if (bSelect) {
		bUnderlineDoNotOFF = false;		// 選択状態ならアンダーライン消去を行う
	}
	int nCurLine;
	auto& si = view.GetSelectionInfo();
	if (bSelect) {
		if (!si.IsTextSelected()) {	// テキストが選択されているか
			// 現在のカーソル位置から選択を開始する
			si.BeginSelectArea();
		}
	}else {
		if (si.IsTextSelected()) {	// テキストが選択されているか
			// 現在の選択範囲を非選択状態に戻す
			si.DisableSelectArea(true);
		}else if (si.IsBoxSelecting()) {
			si.SetBoxSelect(false);
		}
	}
	bool bTryAgain = false;
try_again:;
	auto& caret = GetCaret();
	nCurLine = caret.GetCaretLayoutPos().y;
	const Layout* pLayout;
	auto& layoutMgr = GetDocument().layoutMgr;
	pLayout = layoutMgr.SearchLineByLayoutY(nCurLine);
	if (!pLayout) {
		return;
	}
	if (bTryAgain) {
		const wchar_t*	pLine = pLayout->GetPtr();
		if (pLine[0] != L' ' && pLine[0] != WCODE::TAB) {
			return;
		}
	}
	// 指定された桁に対応する行のデータ内の位置を調べる
	size_t nIdx = view.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().x);
	auto& csGeneral = GetDllShareData().common.general;	
	// 現在位置の右の単語の先頭位置を調べる
	Point ptLayoutNew;
	int nResult = layoutMgr.NextWord(
		nCurLine,
		nIdx,
		&ptLayoutNew,
		csGeneral.bStopsBothEndsWhenSearchWord
	);
	if (nResult) {
		// 行が変わった
		if (ptLayoutNew.y != nCurLine) {
			pLayout = layoutMgr.SearchLineByLayoutY(ptLayoutNew.y);
			if (!pLayout) {
				return;
			}
			bUnderlineDoNotOFF = false;
		}
		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
		/*
		ptLayoutNew.x = view.LineIndexToColumn(pLayout, ptLayoutNew.x);
		*/
		// カーソル移動
		caret.GetAdjustCursorPos(&ptLayoutNew);
		if (bSelect) {
			// 現在のカーソル位置によって選択範囲を変更
			si.ChangeSelectAreaByCurrentCursor(ptLayoutNew);
		}
		caret.MoveCursor(ptLayoutNew, true, _CARETMARGINRATE, bUnderlineDoNotOFF);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
	}else {
		bool bIsFreeCursorModeOld = csGeneral.bIsFreeCursorMode;	// フリーカーソルモードか
		csGeneral.bIsFreeCursorMode = false;
		// カーソル右移動
		Command_Right(bSelect, false, false);
		csGeneral.bIsFreeCursorMode = bIsFreeCursorModeOld;	// フリーカーソルモードか
		if (!bTryAgain) {
			bTryAgain = true;
			goto try_again;
		}
	}
	return;
}


/*! @brief 行頭に移動

	@date Oct. 29, 2001 genta マクロ用機能拡張(パラメータ追加) + goto排除
	@date May. 15, 2002 oak   改行単位移動
	@date Oct.  7, 2002 YAZAKI 冗長な引数 bLineTopOnly を削除
	@date Jun. 18, 2007 maru 行頭判定に全角空白のインデント設定も考慮する
*/
void ViewCommander::Command_GoLineTop(
	bool	bSelect,	// [in] 選択の有無。true: 選択しながら移動。false: 選択しないで移動。
	int		lparam		/* [in] マクロから使用する拡張フラグ
								  @li 0: キー操作と同一(default)
								  @li 1: カーソル位置に関係なく行頭に移動(合成可)
								  @li 4: 選択して移動(合成可)
								  @li 8: 改行単位で先頭に移動(合成可)
						*/
	)
{
	using namespace WCODE;

	// lparamの解釈
	bool bLineTopOnly = ((lparam & 1) != 0);
	if (lparam & 4) {
		bSelect = true;
	}

	Point ptCaretPos;
	auto& caret = GetCaret();
	auto& layoutMgr = GetDocument().layoutMgr;
	if (lparam & 8) {
		// 改行単位指定の場合は、物理行頭位置から目的論理位置を求める
		ptCaretPos = layoutMgr.LogicToLayout(Point(0, caret.GetCaretLogicPos().y));
	}else {
		const Layout* pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
		ptCaretPos.x = pLayout ? pLayout->GetIndent() : 0;
		ptCaretPos.y = caret.GetCaretLayoutPos().y;
	}
	if (!bLineTopOnly) {
		// 目的行のデータを取得
		// 改行単位指定で、先頭から空白が1折り返し行以上続いている場合は次の行データを取得
		int nPosY_Layout;
		size_t nPosX_Logic;
		nPosY_Layout = ptCaretPos.y - 1;
		const Layout* pLayout;
		bool bZenSpace = view.pTypeData->bAutoIndent_ZENSPACE;
		bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
		size_t nLineLen;
		do {
			++nPosY_Layout;
			ASSERT_GE(nPosY_Layout, 0);
			const wchar_t*	pLine = layoutMgr.GetLineStr(nPosY_Layout, &nLineLen, &pLayout);
			if (!pLine) {
				return;
			}
			for (nPosX_Logic=0; nPosX_Logic<nLineLen; ++nPosX_Logic) {
				if (WCODE::IsIndentChar(pLine[nPosX_Logic], bZenSpace != 0)) continue;
				
				if (WCODE::IsLineDelimiter(pLine[nPosX_Logic], bExtEol)) {
					nPosX_Logic = 0;	// 空白またはタブおよび改行だけの行だった
				}
				break;
			}
		}
		while ((lparam & 8) && (nPosX_Logic >= nLineLen) && ((int)layoutMgr.GetLineCount() - 1 > nPosY_Layout));
		
		if (nPosX_Logic >= nLineLen) {
			/* 折り返し単位の行頭を探して物理行末まで到達した
			または、最終行のため改行コードに遭遇せずに行末に到達した */
			nPosX_Logic = 0;
		}
		
		if (nPosX_Logic == 0) nPosY_Layout = ptCaretPos.y;	// 物理行の移動なし
		
		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		size_t nPosX_Layout = view.LineIndexToColumn(pLayout, nPosX_Logic);
		Point ptPos((int)nPosX_Layout, nPosY_Layout);
		if (caret.GetCaretLayoutPos() != ptPos) {
			ptCaretPos = ptPos;
		}
	}

	// 2006.07.09 genta 新規関数にまとめた
	view.MoveCursorSelecting(ptCaretPos, bSelect);
}


/*! 行末に移動(折り返し単位)
	@praram nOption	0x08 改行単位(合成可)
*/
void ViewCommander::Command_GoLineEnd(
	bool bSelect,
	int bIgnoreCurrentSelection,
	int nOption
	)
{
	if (nOption & 4) {
		bSelect = true;
	}
	auto& si = view.GetSelectionInfo();
	if (!bIgnoreCurrentSelection) {
		if (bSelect) {
			if (!si.IsTextSelected()) {	// テキストが選択されているか
				// 現在のカーソル位置から選択を開始する
				si.BeginSelectArea();
			}
		}else {
			if (si.IsTextSelected()) {	// テキストが選択されているか
				// 現在の選択範囲を非選択状態に戻す
				si.DisableSelectArea(true);
			}else if (si.IsBoxSelecting()) {
				si.SetBoxSelect(false);
			}
		}
	}

	// 現在行のデータから、そのレイアウト幅を取得
	auto& caret = GetCaret();
	auto& layoutMgr = GetDocument().layoutMgr;
	Point nPosXY = caret.GetCaretLayoutPos();
	if (nOption & 8) {
		// 改行単位の行末。1行中の最終レイアウト行を探す
		const Layout*	pLayout = layoutMgr.SearchLineByLayoutY(nPosXY.y);
		const Layout*	pLayoutNext = pLayout->GetNextLayout();
		while (pLayout && pLayoutNext && pLayoutNext->GetLogicOffset() != 0) {
			pLayout = pLayoutNext;
			pLayoutNext = pLayoutNext->GetNextLayout();
			++nPosXY.y;
		}
	}
	nPosXY.x = 0;
	const Layout* pLayout = layoutMgr.SearchLineByLayoutY(nPosXY.y);
	if (pLayout) {
		nPosXY.x = pLayout->CalcLayoutWidth(layoutMgr);
	}

	// キャレット移動
	caret.GetAdjustCursorPos(&nPosXY);
	if (bSelect) {
		// 現在のカーソル位置によって選択範囲を変更
		si.ChangeSelectAreaByCurrentCursor(nPosXY);
	}
	caret.MoveCursor(nPosXY, true);
	caret.nCaretPosX_Prev = nPosXY.x;
}


// 半ページアップ		// Oct. 6, 2000 JEPRO added (実は従来のScroll Downそのもの)
void ViewCommander::Command_HalfPageUp(
	bool bSelect,
	int nScrollNum
	)
{
	if (nScrollNum <= 0) {
		nScrollNum = view.GetTextArea().nViewRowNum / 2;
	}
	GetCaret().Cursor_UPDOWN( - (nScrollNum), bSelect );
	return;
}


// 半ページダウン		// Oct. 6, 2000 JEPRO added (実は従来のScroll Upそのもの)
void ViewCommander::Command_HalfPageDown(
	bool bSelect,
	int nScrollNum
	)
{
	if (nScrollNum <= 0) {
		nScrollNum = view.GetTextArea().nViewRowNum / 2;
	}
	GetCaret().Cursor_UPDOWN( nScrollNum, bSelect );
	return;
}


/*! １ページアップ

	@date 2000.10.10 JEPRO 作成
	@date 2001.12.13 hor 画面に対するカーソル位置はそのままで
		１ページアップに動作変更
	@date 2014.01.10 Moca カーソルが動かないときも画面をScrollするように
*/	// Oct. 10, 2000 JEPRO added
void ViewCommander::Command_1PageUp(
	bool bSelect,
	int nScrollNum
	)
{
// GetCaret().Cursor_UPDOWN(-view.GetTextArea().nViewRowNum, bSelect);

// 2001.12.03 hor
//		メモ帳ライクに、画面に対するカーソル位置はそのままで１ページアップ
	{
		const bool bDrawSwitchOld = view.SetDrawSwitch(false);
		auto& textArea = view.GetTextArea();
		int nViewTopLine = textArea.GetViewTopLine();
		if (nScrollNum <= 0) {
			nScrollNum = view.GetTextArea().nViewRowNum - 1;
		}
		GetCaret().Cursor_UPDOWN( -nScrollNum, bSelect );
		// Sep. 11, 2004 genta 同期Scroll処理のため
		// view.RedrawAllではなくScrollAtを使うように
		view.SyncScrollV( view.ScrollAtV( nViewTopLine - nScrollNum ));
		view.SetDrawSwitch(bDrawSwitchOld);
		view.RedrawAll();
	}
	return;
}


/*!	１ページダウン

	@date 2000.10.10 JEPRO 作成
	@date 2001.12.13 hor 画面に対するカーソル位置はそのままで
		１ページダウンに動作変更
	@date 2014.01.10 Moca カーソルが動かないときも画面をScrollするように
*/
void ViewCommander::Command_1PageDown(
	bool bSelect,
	int nScrollNum
	)
{
// GetCaret().Cursor_UPDOWN(view.GetTextArea().nViewRowNum, bSelect);

// 2001.12.03 hor
//		メモ帳ライクに、画面に対するカーソル位置はそのままで１ページダウン
	{
		const bool bDrawSwitchOld = view.SetDrawSwitch(false);
		int nViewTopLine = view.GetTextArea().GetViewTopLine();
		if (nScrollNum <= 0) {
			nScrollNum = view.GetTextArea().nViewRowNum - 1;
		}
		GetCaret().Cursor_UPDOWN(nScrollNum, bSelect);
		// Sep. 11, 2004 genta 同期Scroll処理のため
		// view.RedrawAllではなくScrollAtを使うように
		view.SyncScrollV(view.ScrollAtV(nViewTopLine + nScrollNum));
		view.SetDrawSwitch(bDrawSwitchOld);
		view.RedrawAll();
	}

	return;
}


// ファイルの先頭に移動
void ViewCommander::Command_GoFileTop(bool bSelect)
{
	// 先頭へカーソルを移動
	// Sep. 8, 2000 genta
	view.AddCurrentLineToHistory();

	// 2006.07.09 genta 新規関数にまとめた
	Point pt(
		!view.GetSelectionInfo().IsBoxSelecting()? 0: GetCaret().GetCaretLayoutPos().x,
		0
	);
	view.MoveCursorSelecting(pt, bSelect);	// 通常は、(0, 0)へ移動。ボックス選択中は、(GetCaret().GetCaretLayoutPos().x, 0)へ移動
}


// ファイルの最後に移動
void ViewCommander::Command_GoFileEnd(bool bSelect)
{
	auto& si = view.GetSelectionInfo();
// 2001.12.13 hor BOX選択中にファイルの最後にジャンプすると[EOF]の行が反転したままになるの修正
	if (!bSelect) {
		if (si.IsTextSelected()) {
			si.DisableSelectArea(true);	// 2001.12.21 hor Add
		}else if (si.IsBoxSelecting()) {
			si.SetBoxSelect(false);
		}
	}
	view.AddCurrentLineToHistory();
	auto& caret = GetCaret();
	caret.Cursor_UPDOWN((int)GetDocument().layoutMgr.GetLineCount() , bSelect);
	Command_Down(bSelect, true);
	if (!si.IsBoxSelecting()) {							// 2002/04/18 YAZAKI
		/*	2004.04.19 fotomo
			改行のない最終行で選択肢ながら文書末へ移動した場合に
			選択範囲が正しくない場合がある問題に対応
		*/
		Command_GoLineEnd(bSelect, 0, 0);				// 2001.12.21 hor Add
	}
	caret.MoveCursor(caret.GetCaretLayoutPos(), true);	// 2001.12.21 hor Add
	// 2002.02.16 hor 矩形選択中を除き直前のカーソル位置をリセット
	if (!(si.IsTextSelected() && si.IsBoxSelecting())) {
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
	}

	// 選択範囲情報メッセージを表示する	// 2009.05.06 ryoji 追加
	if (bSelect) {
		si.PrintSelectionInfoMsg();
	}
}


// カーソル行をウィンドウ中央へ
void ViewCommander::Command_CurLineCenter(void)
{
	int nViewTopLine;
	auto& textArea = view.GetTextArea();
	nViewTopLine = GetCaret().GetCaretLayoutPos().y - (textArea.nViewRowNum / 2);

	// sui 02/08/09
	if (0 > nViewTopLine) {
		nViewTopLine = 0;
	}
	
	int nScrollLines = nViewTopLine - textArea.GetViewTopLine();	// Sep. 11, 2004 genta 同期用に行数を記憶
	textArea.SetViewTopLine(nViewTopLine);
	// フォーカス移動時の再描画
	view.RedrawAll();
	// sui 02/08/09

	// Sep. 11, 2004 genta 同期Scrollの関数化
	view.SyncScrollV(nScrollLines);
}


// 移動履歴を前へたどる
void ViewCommander::Command_JumpHist_Prev(void)
{
	// 2001.12.13 hor
	// 移動履歴の最後に現在の位置を記憶する
	// (次の履歴が取得できないときは追加して戻る)
	if (!view.pHistory->CheckNext()) {
		view.AddCurrentLineToHistory();
		view.pHistory->PrevValid();
	}

	if (view.pHistory->CheckPrev()) {
		if (! view.pHistory->PrevValid()) {
			::MessageBox(NULL, _T("Inconsistent Implementation"), _T("PrevValid"), MB_OK);
		}
		Point pt = GetDocument().layoutMgr.LogicToLayout(view.pHistory->GetCurrent().GetPosition());
		// 2006.07.09 genta 選択を考慮
		view.MoveCursorSelecting(pt, view.GetSelectionInfo().bSelectingLock);
	}
}


// 移動履歴を次へたどる
void ViewCommander::Command_JumpHist_Next(void)
{
	if (view.pHistory->CheckNext()) {
		if (!view.pHistory->NextValid()) {
			::MessageBox(NULL, _T("Inconsistent Implementation"), _T("NextValid"), MB_OK);
		}
		Point pt = GetDocument().layoutMgr.LogicToLayout(view.pHistory->GetCurrent().GetPosition());
		// 2006.07.09 genta 選択を考慮
		view.MoveCursorSelecting(pt, view.GetSelectionInfo().bSelectingLock);
	}
}


// 現在位置を移動履歴に登録する
void ViewCommander::Command_JumpHist_Set(void)
{
	view.AddCurrentLineToHistory();
}


// 2001/06/20 Start by asa-o

// from ViewCommander_New.cpp
// テキストを１行下へScroll
void ViewCommander::Command_WndScrollDown(void)
{
	int nCaretMarginY;

	auto& textArea = view.GetTextArea();
	nCaretMarginY = textArea.nViewRowNum / _CARETMARGINRATE;
	if (nCaretMarginY < 1) {
		nCaretMarginY = 1;
	}

	nCaretMarginY += 2;

	bool bCaretOff = false;
	auto& caret = GetCaret();
	if (caret.GetCaretLayoutPos().GetY() > textArea.nViewRowNum + textArea.GetViewTopLine() - (nCaretMarginY + 1)) {
		bCaretOff = true;
	}

	// Sep. 11, 2004 genta 同期用に行数を記憶
	// Sep. 11, 2004 genta 同期Scrollの関数化
	view.SyncScrollV(view.ScrollAtV(textArea.GetViewTopLine() - 1));

	// テキストが選択されていない
	if (!view.GetSelectionInfo().IsTextSelected()) {
		// カーソルが画面外に出た
		if (caret.GetCaretLayoutPos().GetY() > textArea.nViewRowNum + textArea.GetViewTopLine() - nCaretMarginY) {
			if (caret.GetCaretLayoutPos().GetY() > (int)GetDocument().layoutMgr.GetLineCount() - nCaretMarginY) {
				caret.Cursor_UPDOWN((GetDocument().layoutMgr.GetLineCount() - nCaretMarginY) - caret.GetCaretLayoutPos().y, false);
			}else {
				caret.Cursor_UPDOWN(-1, false);
			}
			caret.ShowCaretPosInfo();
		}
	}
	if (bCaretOff) {
		caret.underLine.CaretUnderLineOFF(true);
	}
	caret.underLine.CaretUnderLineON(true, true);
}


// from ViewCommander_New.cpp
// テキストを１行上へScroll
void ViewCommander::Command_WndScrollUp(void)
{
	auto& textArea = view.GetTextArea();
	int nCaretMarginY = textArea.nViewRowNum / _CARETMARGINRATE;
	if (nCaretMarginY < 1)
		nCaretMarginY = 1;

	bool bCaretOff = false;
	auto& caret = GetCaret();
	if (caret.GetCaretLayoutPos().y < textArea.GetViewTopLine() + (nCaretMarginY + 1)) {
		bCaretOff = true;
	}

	// Sep. 11, 2004 genta 同期用に行数を記憶
	// Sep. 11, 2004 genta 同期Scrollの関数化
	view.SyncScrollV(view.ScrollAtV(textArea.GetViewTopLine() + 1));

	// テキストが選択されていない
	if (!view.GetSelectionInfo().IsTextSelected()) {
		// カーソルが画面外に出た
		if (caret.GetCaretLayoutPos().GetY() < textArea.GetViewTopLine() + nCaretMarginY) {
			if (textArea.GetViewTopLine() == 1) {
				caret.Cursor_UPDOWN(nCaretMarginY + 1, false);
			}else {
				caret.Cursor_UPDOWN(1, false);
			}
			caret.ShowCaretPosInfo();
		}
	}
	if (bCaretOff) {
		caret.underLine.CaretUnderLineOFF(true);
	}
	caret.underLine.CaretUnderLineON(true, true);
}

// 2001/06/20 End


// from ViewCommander_New.cpp
/* 次の段落へ進む
	2002/04/26 段落の両端で止まるオプションを追加
	2002/04/19 新規
*/
void ViewCommander::Command_GoNextParagraph(bool bSelect)
{
	DocLine* pDocLine;
	int nCaretPointer = 0;
	auto& docLineMgr = GetDocument().docLineMgr;
	auto& caret = GetCaret();
	
	bool nFirstLineIsEmptyLine = false;
	// まずは、現在位置が空行（スペース、タブ、改行記号のみの行）かどうか判別
	if ((pDocLine = docLineMgr.GetLine(caret.GetCaretLogicPos().y + nCaretPointer))) {
		nFirstLineIsEmptyLine = pDocLine->IsEmptyLine();
		++nCaretPointer;
	}else {
		// EOF行でした。
		return;
	}

	// 次に、nFirstLineIsEmptyLineと異なるところまで読み飛ばす
	while ((pDocLine = docLineMgr.GetLine(caret.GetCaretLogicPos().y + nCaretPointer))) {
		if (pDocLine->IsEmptyLine() == nFirstLineIsEmptyLine) {
			++nCaretPointer;
		}else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLineが空行だったら、今見ているところは非空行。すなわちおしまい。
		nFirstLineIsEmptyLineが非空行だったら、今見ているところは空行。
	*/
	if (nFirstLineIsEmptyLine) {
		// おしまい。
	}else {
		// いま見ているところは空行の1行目
		if (GetDllShareData().common.general.bStopsBothEndsWhenSearchParagraph) {	// 段落の両端で止まる
		}else {
			// 仕上げに、空行じゃないところまで進む
			while ((pDocLine = docLineMgr.GetLine(caret.GetCaretLogicPos().y + nCaretPointer))) {
				if (pDocLine->IsEmptyLine()) {
					++nCaretPointer;
				}else {
					break;
				}
			}
		}
	}

	// EOFまで来たり、目的の場所まできたので移動終了。

	// 移動距離を計算
	Point ptCaretPos_Layo;

	// 移動前の物理位置
	ptCaretPos_Layo = GetDocument().layoutMgr.LogicToLayout(caret.GetCaretLogicPos());

	// 移動後の物理位置
	Point ptCaretPos_Layo_CaretPointer;
	ptCaretPos_Layo_CaretPointer = GetDocument().layoutMgr.LogicToLayout(caret.GetCaretLogicPos() + Point(0, nCaretPointer));

	caret.Cursor_UPDOWN(ptCaretPos_Layo_CaretPointer.y - ptCaretPos_Layo.y, bSelect);
}


// from ViewCommander_New.cpp
/* 前の段落へ進む
	2002/04/26 段落の両端で止まるオプションを追加
	2002/04/19 新規
*/
void ViewCommander::Command_GoPrevParagraph(bool bSelect)
{
	auto& docLineMgr = GetDocument().docLineMgr;
	DocLine* pDocLine;
	int nCaretPointer = -1;

	bool nFirstLineIsEmptyLine = false;
	auto& caret = GetCaret();
	// まずは、現在位置が空行（スペース、タブ、改行記号のみの行）かどうか判別
	if ((pDocLine = docLineMgr.GetLine(caret.GetCaretLogicPos().y + nCaretPointer))) {
		nFirstLineIsEmptyLine = pDocLine->IsEmptyLine();
		--nCaretPointer;
	}else {
		nFirstLineIsEmptyLine = true;
		--nCaretPointer;
	}

	// 次に、nFirstLineIsEmptyLineと異なるところまで読み飛ばす
	while ((pDocLine = docLineMgr.GetLine(caret.GetCaretLogicPos().y + nCaretPointer))) {
		if (pDocLine->IsEmptyLine() == nFirstLineIsEmptyLine) {
			--nCaretPointer;
		}else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLineが空行だったら、今見ているところは非空行。すなわちおしまい。
		nFirstLineIsEmptyLineが非空行だったら、今見ているところは空行。
	*/
	auto& csGeneral = GetDllShareData().common.general;	
	if (nFirstLineIsEmptyLine) {
		// おしまい。
		if (csGeneral.bStopsBothEndsWhenSearchParagraph) {	// 段落の両端で止まる
			++nCaretPointer;	// 空行の最上行（段落の末端の次の行）で止まる。
		}else {
			// 仕上げに、空行じゃないところまで進む
			while ((pDocLine = docLineMgr.GetLine(caret.GetCaretLogicPos().y + nCaretPointer))) {
				if (pDocLine->IsEmptyLine()) {
					break;
				}else {
					--nCaretPointer;
				}
			}
			++nCaretPointer;	// 空行の最上行（段落の末端の次の行）で止まる。
		}
	}else {
		// いま見ているところは空行の1行目
		if (csGeneral.bStopsBothEndsWhenSearchParagraph) {	// 段落の両端で止まる
			++nCaretPointer;
		}else {
			++nCaretPointer;
		}
	}

	// EOFまで来たり、目的の場所まできたので移動終了。

	// 移動距離を計算
	
	// 移動前の物理位置
	Point ptCaretPos_Layo = GetDocument().layoutMgr.LogicToLayout(caret.GetCaretLogicPos());
	// 移動後の物理位置
	Point ptCaretPos_Layo_CaretPointer = GetDocument().layoutMgr.LogicToLayout(caret.GetCaretLogicPos() + Point(0, nCaretPointer));
	
	caret.Cursor_UPDOWN(ptCaretPos_Layo_CaretPointer.y - ptCaretPos_Layo.y, bSelect);
}

void ViewCommander::Command_AutoScroll()
{
	if (view.nAutoScrollMode == 0) {
		GetCursorPos(&view.autoScrollMousePos);
		ScreenToClient(view.GetHwnd(), &view.autoScrollMousePos);
		view.bAutoScrollDragMode = false;
		view.AutoScrollEnter();
	}else {
		view.AutoScrollExit();
	}
}

void ViewCommander::Command_WheelUp(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELUP);
}

void ViewCommander::Command_WheelDown(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELDOWN);
}

void ViewCommander::Command_WheelLeft(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELLEFT);
}

void ViewCommander::Command_WheelRight(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELRIGHT);
}

void ViewCommander::Command_WheelPageUp(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELPAGEUP);
}

void ViewCommander::Command_WheelPageDown(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELPAGEDOWN);
}

void ViewCommander::Command_WheelPageLeft(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELPAGELEFT);
}

void ViewCommander::Command_WheelPageRight(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	view.OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELPAGERIGHT);
}

/*! 次の変更行へ
	変更行のブロックの先頭行と、変更行のブロックの末尾(次の行頭)に移動する
*/
void ViewCommander::Command_ModifyLine_Next( bool bSelect )
{
	auto& docLineMgr = GetDocument().docLineMgr;
	int nYOld = GetCaret().GetCaretLogicPos().y;
	Point ptXY(0, nYOld);
	const DocLine* pDocLine = docLineMgr.GetLine(ptXY.y);
	const int nSaveSeq = GetDocument().docEditor.opeBuf.GetNoModifiedSeq();
	bool bModified = false;
	if (docLineMgr.GetLineCount() == 0) {
		return;
	}
	if (pDocLine) {
		bModified = ModifyVisitor().IsLineModified(pDocLine, nSaveSeq);
		ptXY.y++;
		pDocLine = pDocLine->GetNextLine();
	}
	for (int n=0; n<2; ++n) {
		while (pDocLine) {
			if (ModifyVisitor().IsLineModified(pDocLine, nSaveSeq) != bModified
				|| (
					ptXY.y == 0
					&& ModifyVisitor().IsLineModified(pDocLine, nSaveSeq)
				)
			) {
				Point ptLayout = GetDocument().layoutMgr.LogicToLayout(ptXY);
				view.MoveCursorSelecting(ptLayout, bSelect);
				if (nYOld >= ptXY.y) {
					view.SendStatusMessage(LS(STR_ERR_SRNEXT1));
				}
				return;
			}
			ptXY.y++;
			pDocLine = pDocLine->GetNextLine();
		}
		if (n == 0 && bModified) {
			const DocLine* pDocLineLast = docLineMgr.GetDocLineBottom();
			bool bSkip = false;
			Point pos;
			if (pDocLineLast) {
				if (pDocLineLast->GetEol() == EolType::None) {
					// ぶら下がり[EOF]
					pos.x = pDocLineLast->GetLengthWithoutEOL();
					pos.y = docLineMgr.GetLineCount() - 1;
					if (GetCaret().GetCaretLogicPos() == pos) {
						bSkip = true;
					}
				}else {
					// 単独[EOF]
					pos = ptXY;
				}
			}else {
				bSkip = true;
			}
			if (!bSkip) {
				Point ptLayout = GetDocument().layoutMgr.LogicToLayout(pos);
				view.MoveCursorSelecting( ptLayout, bSelect );
				return;
			}
		}
		if (n == 0) {
			ptXY.y = 0;
			pDocLine = docLineMgr.GetLine(ptXY.y);
			bModified = false;
		}
		if (!GetDllShareData().common.search.bSearchAll) {
			break;
		}
	}
	view.SendStatusMessage(LS(STR_ERR_SRNEXT2));
	AlertNotFound( view.GetHwnd(), false, LS(STR_MODLINE_NEXT_NOT_FOUND));
}

/*! 前の変更行へ
	変更行のブロックの先頭行と、変更行のブロックの末尾(次の行頭)に移動する
	Command_ModifyLine_Nextと同じ位置に止まる
*/
void ViewCommander::Command_ModifyLine_Prev( bool bSelect )
{
	auto& docLineMgr = GetDocument().docLineMgr;
	int nYOld = GetCaret().GetCaretLogicPos().y;
	int nYOld2 = nYOld;
	Point ptXY(0, nYOld);
	const DocLine* pDocLine = docLineMgr.GetLine(ptXY.y);
	const int nSaveSeq = GetDocument().docEditor.opeBuf.GetNoModifiedSeq();
	bool bModified = false;
	bool bLast = false;
	if (!pDocLine) {
		// [EOF]
		const DocLine* pDocLineLast = docLineMgr.GetLine(ptXY.y - 1);
		if (!pDocLineLast) {
			// 1行もない
			return;
		}
		bModified = ModifyVisitor().IsLineModified(pDocLineLast, nSaveSeq);
		ptXY.y--;
		pDocLine = pDocLineLast;
		bLast = true;
	}
	if (!bLast) {
		const DocLine* pDocLineLast = docLineMgr.GetDocLineBottom();
		if (pDocLineLast && pDocLineLast->GetEol() == EolType::None) {
			Point pos;
			pos.x = pDocLine->GetLengthWithoutEOL();
			pos.y = docLineMgr.GetLineCount() - 1;
			if (GetCaret().GetCaretLogicPos() == pos) {
				// ぶら下がり[EOF]の位置だった
				bLast = true;
			}
		}
	}
	assert( pDocLine );
	bModified = ModifyVisitor().IsLineModified(pDocLine, nSaveSeq);
	nYOld2 = ptXY.y;
	ptXY.y--;
	pDocLine = pDocLine->GetPrevLine();
	if (pDocLine && !bLast) {
		bModified = ModifyVisitor().IsLineModified(pDocLine, nSaveSeq);
		nYOld2 = ptXY.y;
		ptXY.y--;
		pDocLine = pDocLine->GetPrevLine();
	}
	auto& layoutMgr = GetDocument().layoutMgr;
	for (int n=0; n<2; ++n) {
		while (pDocLine) {
			bool bModifiedTemp = ModifyVisitor().IsLineModified(pDocLine, nSaveSeq);
			if (bModifiedTemp != bModified) {
				// 検出された位置の1行後ろ(MODIFYLINE_NEXTと同じ位置)に止まる
				ptXY.y = nYOld2;
				Point ptLayout = layoutMgr.LogicToLayout(ptXY);
				view.MoveCursorSelecting(ptLayout, bSelect);
				if (n == 1) {
					view.SendStatusMessage(LS(STR_ERR_SRPREV1));
				}
				return;
			}
			nYOld2 = ptXY.y;
			ptXY.y--;
			pDocLine = pDocLine->GetPrevLine();
		}
		if (n == 0) {
			// 先頭行チェック
			const DocLine* pDocLineTemp = docLineMgr.GetDocLineTop();
			assert( pDocLineTemp );
			if (ModifyVisitor().IsLineModified(pDocLineTemp, nSaveSeq) != false) {
				if (GetCaret().GetCaretLogicPos() != Point(0,0)) {
					ptXY = Point(0,0);
					Point ptLayout = layoutMgr.LogicToLayout(ptXY);
					view.MoveCursorSelecting(ptLayout, bSelect);
					return;
				}
			}
			ptXY.y = docLineMgr.GetLineCount() - 1;
			nYOld2 = ptXY.y;
			pDocLine = docLineMgr.GetLine(ptXY.y);
			bModified = false;
		}
		if (!GetDllShareData().common.search.bSearchAll) {
			break;
		}
		if (n == 0) {
			const DocLine* pDocLineTemp = docLineMgr.GetDocLineBottom();
			assert( pDocLineTemp );
			if (ModifyVisitor().IsLineModified(pDocLineTemp, nSaveSeq) != false) {
				// 最終行が変更行の場合は、[EOF]に止まる
				Point pos;
				if (pDocLineTemp->GetEol() != EolType::None) {
					pos.x = 0;
					pos.y = docLineMgr.GetLineCount();
					pos.y++;
				}else {
					pos.x = pDocLine->GetLengthWithoutEOL();
					pos.y = docLineMgr.GetLineCount() - 1;
				}
				if (GetCaret().GetCaretLogicPos() != pos) {
					ptXY = pos;
					Point ptLayout = layoutMgr.LogicToLayout(ptXY);
					view.MoveCursorSelecting(ptLayout, bSelect);
					view.SendStatusMessage(LS(STR_ERR_SRPREV1));
					return;
				}
			}
		}
	}
	view.SendStatusMessage(LS(STR_ERR_SRPREV2));
	AlertNotFound( view.GetHwnd(), false, LS(STR_MODLINE_PREV_NOT_FOUND) );
}

