/*!	@file
@brief ViewCommanderクラスのコマンド(カーソル移動系)関数群

	2012/12/17	CViewCommander.cpp,CViewCommander_New.cppから分離
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
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "CMarkMgr.h"/// 2002/2/3 aroka 追加
#include "mem/CMemoryIterator.h"	// @@@ 2002.09.28 YAZAKI


void ViewCommander::Command_MOVECURSOR(LogicPoint pos, int option)
{
	if (pos.GetX2() < 0 || pos.GetY2() < 0) {
		ErrorBeep();
		return;
	}
	LayoutPoint layoutPos;
	GetDocument()->m_cLayoutMgr.LogicToLayout(pos, &layoutPos);
	Command_MOVECURSORLAYOUT(layoutPos, option);
}

void ViewCommander::Command_MOVECURSORLAYOUT(LayoutPoint pos, int option)
{
	if (pos.GetX2() < 0 || pos.GetY2() < 0) {
		ErrorBeep();
		return;
	}

	bool bSelect = (option & 0x01) == 0x01;
	bool bBoxSelect = (option & 0x02) == 0x02;

	auto& si = m_pCommanderView->GetSelectionInfo();

	if (bSelect || bBoxSelect) {
		if (!si.IsTextSelected()) {
			if (bBoxSelect) {
				Command_BEGIN_BOXSELECT();
			}else {
				si.BeginSelectArea();
			}
		}else {
			// 2014.01.08 追加
			if (bBoxSelect && !si.IsBoxSelecting()) {
				// 通常選択→矩形選択に変更。他のコマンドに合わせる
				Command_BEGIN_BOXSELECT();
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
	caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
}


/////////////////////////////////// 以下はコマンド群 (Oct. 17, 2000 jepro note) ///////////////////////////////////////////

// カーソル上移動
int ViewCommander::Command_UP(bool bSelect, bool bRepeat, int lines)
{
	auto& caret = GetCaret();
	// From Here Oct. 24, 2001 genta
	if (lines != 0) {
		caret.Cursor_UPDOWN(LayoutInt(lines), FALSE);
		return 1;
	}
	// To Here Oct. 24, 2001 genta


	int nRepeat = 0;

	// キーリピート時のスクロールを滑らかにするか
	auto& csGeneral = GetDllShareData().m_common.m_sGeneral;
	if (!csGeneral.m_nRepeatedScroll_Smooth) {
		LayoutInt i;
		if (!bRepeat) {
			i = LayoutInt(-1);
		}else {
			i = -1 * csGeneral.m_nRepeatedScrollLineNum;	// キーリピート時のスクロール行数
		}
		caret.Cursor_UPDOWN(i, bSelect);
		nRepeat = -1 * (Int)i;
	}else {
		++nRepeat;
		if (caret.Cursor_UPDOWN(LayoutInt(-1), bSelect) != 0 && bRepeat) {
			for (int i=0; i<csGeneral.m_nRepeatedScrollLineNum-1; ++i) {		// キーリピート時のスクロール行数
				::UpdateWindow(m_pCommanderView->GetHwnd());	// YAZAKI
				caret.Cursor_UPDOWN(LayoutInt(-1), bSelect);
				++nRepeat;
			}
		}
	}
	return nRepeat;
}


// カーソル下移動
int ViewCommander::Command_DOWN(bool bSelect, bool bRepeat)
{
	auto& caret = GetCaret();
	int nRepeat = 0;
	auto& csGeneral = GetDllShareData().m_common.m_sGeneral;
	// キーリピート時のスクロールを滑らかにするか
	if (!csGeneral.m_nRepeatedScroll_Smooth) {
		LayoutInt i;
		if (!bRepeat) {
			i = LayoutInt(1);
		}else {
			i = csGeneral.m_nRepeatedScrollLineNum;	// キーリピート時のスクロール行数
		}
		caret.Cursor_UPDOWN(i, bSelect);
		nRepeat = (Int)i;
	}else {
		++nRepeat;
		if (caret.Cursor_UPDOWN(LayoutInt(1), bSelect) != 0 && bRepeat) {
			for (int i=0; i<csGeneral.m_nRepeatedScrollLineNum-1; ++i) {	// キーリピート時のスクロール行数
				// ここで再描画。
				::UpdateWindow(m_pCommanderView->GetHwnd());	// YAZAKI
				caret.Cursor_UPDOWN(LayoutInt(1), bSelect);
				++nRepeat;
			}
		}
	}
	return nRepeat;
}


/*! @brief カーソル左移動

	@date 2004.03.28 Moca EOFだけの行以降の途中にカーソルがあると落ちるバグ修正．
			pcLayout == NULLかつキャレット位置が行頭以外の場合は
			2つのifのどちらにも当てはまらないが，そのあとのMoveCursorにて適正な
			位置に移動させられる．
	@date 2014.01.10 Moca キーリピート時、MoveCursorを一度にまとめる
*/
int ViewCommander::Command_LEFT(bool bSelect, bool bRepeat)
{
	bool bUnderlineDoNotOFF = true;		// アンダーラインを消去しない
	if (bSelect) {
		bUnderlineDoNotOFF = false;		// 選択状態ならアンダーライン消去を行う
	}
	bool bMoveCaretLine = false;
	int nRepeat = bRepeat ? 2 : 1;
	int nRes = 0;
	LayoutPoint ptCaretMove = GetCaret().GetCaretLayoutPos();
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	for (int nRepCount=0; nRepCount<nRepeat; ++nRepCount) {
		if (bSelect && ! selInfo.IsTextSelected()) {
			// 現在のカーソル位置から選択を開始する
			selInfo.BeginSelectArea();
		}
		if (!bSelect) {
			if (selInfo.IsTextSelected()) {
				this->Command_CANCEL_MODE(1);
				nRes = 1;
				continue; // 選択のキャンセルで左移動を 1消費。この後の移動処理はスキップする。
			}else if (selInfo.IsBoxSelecting()) {
				selInfo.SetBoxSelect(false);
			}
		}
		// (これから求める)カーソルの移動先。
		LayoutPoint ptPos(LayoutInt(0), ptCaretMove.GetY2());

		// 現在行のデータを取得
		const Layout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( ptCaretMove.GetY2() );
		// カーソルが左端にある
		if (ptCaretMove.GetX2() == (pcLayout ? pcLayout->GetIndent() : LayoutInt(0))) {
			if (0 < ptCaretMove.GetY2()
			   && ! selInfo.IsBoxSelecting()
			) {
				// 前のレイアウト行の、折り返し桁一つ手前または改行文字の手前に移動する。
				pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( ptCaretMove.GetY2() - LayoutInt(1) );
				CMemoryIterator it(pcLayout, GetDocument()->m_cLayoutMgr.GetTabSpace());
				while (!it.end()) {
					it.scanNext();
					if (it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL()) {
						ptPos.x += it.getColumnDelta();
						break;
					}
					it.addDelta();
				}
				ptPos.x += it.getColumn() - it.getColumnDelta();
				ptPos.y --;
			}else {
				if (0 < nRepCount) {
					GetCaret().MoveCursor( ptCaretMove, true, _CARETMARGINRATE, bUnderlineDoNotOFF );
					GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
				}
				nRes = 0;
				break; // これ以上左に動けぬ。
			}
			bUnderlineDoNotOFF = false;	// 行が変わるのでアンダーラインを消去する
			bMoveCaretLine = true;
		}
		//  2004.03.28 Moca EOFだけの行以降の途中にカーソルがあると落ちるバグ修正
		else if (pcLayout) {
			CMemoryIterator it(pcLayout, GetDocument()->m_cLayoutMgr.GetTabSpace());
			while (!it.end()) {
				it.scanNext();
				if ( it.getColumn() + it.getColumnDelta() > ptCaretMove.GetX2() - 1 ){
					ptPos.x += it.getColumnDelta();
					break;
				}
				it.addDelta();
			}
			ptPos.x += it.getColumn() - it.getColumnDelta();
			// Oct. 18, 2002 YAZAKI
			if (it.getIndex() >= pcLayout->GetLengthWithEOL()) {
				ptPos.x = ptCaretMove.GetX2() - LayoutInt(1);
			}
		}

		GetCaret().GetAdjustCursorPos( &ptPos );
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
			GetCaret().MoveCursor( ptPos, true, _CARETMARGINRATE, bUnderlineDoNotOFF );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
		ptCaretMove = ptPos;
		nRes = 1;
	}
	return nRes;
}


/* カーソル右移動
	@date 2014.01.10 Moca キーリピート時、MoveCursorを一度にまとめる
*/
void ViewCommander::Command_RIGHT(
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
	LayoutPoint ptCaretMove = GetCaret().GetCaretLayoutPos();
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
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
					this->Command_CANCEL_MODE(2);
					continue; // 選択のキャンセルで右移動を 1消費。この後の移動処理はスキップする。
				}else if (selInfo.IsBoxSelecting()) {
					selInfo.SetBoxSelect(false);
				}
			}
		}
//		2003.06.28 Moca [EOF]のみの行にカーソルがあるときに右を押しても選択を解除できない問題に対応

		// (これから求める)カーソルの移動先。
		LayoutPoint ptTo(0, 0);
		const LayoutPoint ptCaret = ptCaretMove;

		// 現在行のデータを取得
		const Layout* const pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(ptCaret.y);
		// 2004.04.02 EOF以降にカーソルがあったときに右を押しても何も起きなかったのを、EOFに移動するように
		if (pcLayout) {
			// キャレット位置のレイアウト行について。
			const LayoutInt x_wrap = pcLayout->CalcLayoutWidth(GetDocument()->m_cLayoutMgr); // 改行文字、または折り返しの位置。
			const bool wrapped = EOL_NONE == pcLayout->GetLayoutEol(); // 折り返しているか、改行文字で終わっているか。これにより x_wrapの意味が変わる。
			const bool nextline_exists = pcLayout->GetNextLayout() || pcLayout->GetLayoutEol() != EOL_NONE; // EOFのみの行も含め、キャレットが移動可能な次行が存在するか。

			// 現在のキャレットの右の位置(to_x)を求める。
			CMemoryIterator it(pcLayout, GetDocument()->m_cLayoutMgr.GetTabSpace());
			for (; !it.end(); it.scanNext(), it.addDelta()) {
				if (ptCaret.x < it.getColumn()) {
					break;
				}
			}
			const LayoutInt to_x = t_max(it.getColumn(), ptCaret.x + 1);

			// キャレットの右端(x_max)と、そこでの扱い(on_x_max)を決める。
			LayoutInt x_max;
			enum {
				STOP,
				MOVE_NEXTLINE_IMMEDIATELY, // 右端に止まらず次の行頭に移動する。(折り返しなど)
				MOVE_NEXTLINE_NEXTTIME, // 右端に止まり、次に次の行頭に移動する。(改行を超えるときなど)
				MOVE_NEXTLINE_NEXTTIME_AND_MOVE_RIGHT // 右端に止まり、次に次の行頭の一つ右に移動する。(折り返しなど)
			} on_x_max;

			if (selInfo.IsBoxSelecting()) {
				x_max = t_max(x_wrap, GetDocument()->m_cLayoutMgr.GetMaxLineKetas());
				on_x_max = STOP;
			}else if (GetDllShareData().m_common.m_sGeneral.m_bIsFreeCursorMode) {
				// フリーカーソルモードでは折り返し位置だけをみて、改行文字の位置はみない。
				if (wrapped) {
					if (nextline_exists) {
						x_max = x_wrap;
						on_x_max = MOVE_NEXTLINE_IMMEDIATELY;
					}else {
						// データのあるEOF行は折り返しではない
						x_max = t_max(x_wrap, GetDocument()->m_cLayoutMgr.GetMaxLineKetas());
						on_x_max = STOP;
					}
				}else {
					if (x_wrap < GetDocument()->m_cLayoutMgr.GetMaxLineKetas()) {
						x_max = GetDocument()->m_cLayoutMgr.GetMaxLineKetas();
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
				ptTo.x = pcLayout->GetNextLayout() ? pcLayout->GetNextLayout()->GetIndent() : LayoutInt(0);
				if (on_x_max == MOVE_NEXTLINE_NEXTTIME_AND_MOVE_RIGHT) {
					++nRepeat;
				}
				bUnderlineDoNotOFF = false;
				bMoveCaretLine = true;
			}else {
				ptTo.y = ptCaret.y;
				ptTo.x = t_min(to_x, x_max);
			}
		}else {
			// pcLayoutがNULLの場合はptPos.x=0に調整
			ptTo.y = ptCaret.y;
			ptTo.x = 0;
		}

		GetCaret().GetAdjustCursorPos( &ptTo );
		if (bSelect) {
			// 現在のカーソル位置によって選択範囲を変更
			selInfo.ChangeSelectAreaByCurrentCursor( ptTo );
		}

		if (bMoveCaretLine || nRepeat - 1 == nRepCount) {
			GetCaret().MoveCursor( ptTo, true, _CARETMARGINRATE, bUnderlineDoNotOFF );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
		ptCaretMove = ptTo;
	}
	return;
}


// カーソル上移動(２行づつ)
void ViewCommander::Command_UP2(bool bSelect)
{
	GetCaret().Cursor_UPDOWN(LayoutInt(-2), bSelect);
	return;
}


// カーソル下移動(２行づつ)
void ViewCommander::Command_DOWN2(bool bSelect)
{
	GetCaret().Cursor_UPDOWN(LayoutInt(2), bSelect);
	return;
}


// 単語の左端に移動
void ViewCommander::Command_WORDLEFT(bool bSelect)
{
	bool bUnderlineDoNotOFF = true;		// アンダーラインを消去しない
	if (bSelect) {
		bUnderlineDoNotOFF = false;		// 選択状態ならアンダーライン消去を行う
	}
	LogicInt nIdx;
	auto& si = m_pCommanderView->GetSelectionInfo();
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
	auto& csGeneral = GetDllShareData().m_common.m_sGeneral;

	const Layout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());
	if (!pcLayout) {
		bool bIsFreeCursorModeOld = csGeneral.m_bIsFreeCursorMode;	// フリーカーソルモードか
		csGeneral.m_bIsFreeCursorMode = false;
		// カーソル左移動
		Command_LEFT(bSelect, false);
		csGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	// フリーカーソルモードか
		return;
	}

	// 指定された桁に対応する行のデータ内の位置を調べる
	nIdx = m_pCommanderView->LineColumnToIndex(pcLayout, caret.GetCaretLayoutPos().GetX2());

	// 現在位置の左の単語の先頭位置を調べる
	LayoutPoint ptLayoutNew;
	int nResult = GetDocument()->m_cLayoutMgr.PrevWord(
		caret.GetCaretLayoutPos().GetY2(),
		nIdx,
		&ptLayoutNew,
		csGeneral.m_bStopsBothEndsWhenSearchWord
	);
	if (nResult) {
		// 行が変わった
		if (ptLayoutNew.y != caret.GetCaretLayoutPos().GetY2()) {
			pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(ptLayoutNew.GetY2());
			if (!pcLayout) {
				return;
			}
			bUnderlineDoNotOFF = false;
		}

		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
		/*
		ptLayoutNew.x = m_pCommanderView->LineIndexToColumn(pcLayout, ptLayoutNew.x);
		*/

		// カーソル移動
		caret.GetAdjustCursorPos(&ptLayoutNew);
		if (bSelect) {
			// 現在のカーソル位置によって選択範囲を変更
			si.ChangeSelectAreaByCurrentCursor(ptLayoutNew);
		}
		caret.MoveCursor(ptLayoutNew, true, _CARETMARGINRATE, bUnderlineDoNotOFF);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
	}else {
		bool bIsFreeCursorModeOld = csGeneral.m_bIsFreeCursorMode;	// フリーカーソルモードか
		csGeneral.m_bIsFreeCursorMode = false;
		// カーソル左移動
		Command_LEFT(bSelect, false);
		csGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	// フリーカーソルモードか
	}
	return;
}


// 単語の右端に移動
void ViewCommander::Command_WORDRIGHT(bool bSelect)
{
	bool bUnderlineDoNotOFF = true;	// アンダーラインを消去しない
	if (bSelect) {
		bUnderlineDoNotOFF = false;		// 選択状態ならアンダーライン消去を行う
	}
	LogicInt nIdx;
	LayoutInt nCurLine;
	auto& si = m_pCommanderView->GetSelectionInfo();
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
	bool	bTryAgain = false;
try_again:;
	auto& caret = GetCaret();
	nCurLine = caret.GetCaretLayoutPos().GetY2();
	const Layout* pcLayout;
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(nCurLine);
	if (!pcLayout) {
		return;
	}
	if (bTryAgain) {
		const wchar_t*	pLine = pcLayout->GetPtr();
		if (pLine[0] != L' ' && pLine[0] != WCODE::TAB) {
			return;
		}
	}
	// 指定された桁に対応する行のデータ内の位置を調べる
	nIdx = m_pCommanderView->LineColumnToIndex(pcLayout, caret.GetCaretLayoutPos().GetX2());

	auto& csGeneral = GetDllShareData().m_common.m_sGeneral;	
	// 現在位置の右の単語の先頭位置を調べる
	LayoutPoint ptLayoutNew;
	int nResult = GetDocument()->m_cLayoutMgr.NextWord(
		nCurLine,
		nIdx,
		&ptLayoutNew,
		csGeneral.m_bStopsBothEndsWhenSearchWord
	);
	if (nResult) {
		// 行が変わった
		if (ptLayoutNew.y != nCurLine) {
			pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(ptLayoutNew.GetY2());
			if (!pcLayout) {
				return;
			}
			bUnderlineDoNotOFF = false;
		}
		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
		/*
		ptLayoutNew.x = m_pCommanderView->LineIndexToColumn(pcLayout, ptLayoutNew.x);
		*/
		// カーソル移動
		caret.GetAdjustCursorPos(&ptLayoutNew);
		if (bSelect) {
			// 現在のカーソル位置によって選択範囲を変更
			si.ChangeSelectAreaByCurrentCursor(ptLayoutNew);
		}
		caret.MoveCursor(ptLayoutNew, true, _CARETMARGINRATE, bUnderlineDoNotOFF);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
	}else {
		bool bIsFreeCursorModeOld = csGeneral.m_bIsFreeCursorMode;	// フリーカーソルモードか
		csGeneral.m_bIsFreeCursorMode = false;
		// カーソル右移動
		Command_RIGHT(bSelect, false, false);
		csGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	// フリーカーソルモードか
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
void ViewCommander::Command_GOLINETOP(
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

	LayoutPoint ptCaretPos;
	auto& caret = GetCaret();
	if (lparam & 8) {
		// 改行単位指定の場合は、物理行頭位置から目的論理位置を求める
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			LogicPoint(0, caret.GetCaretLogicPos().y),
			&ptCaretPos
		);
	}else {
		const Layout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());
		ptCaretPos.x = pcLayout ? pcLayout->GetIndent() : LayoutInt(0);
		ptCaretPos.y = caret.GetCaretLayoutPos().GetY2();
	}
	if (!bLineTopOnly) {
		// 目的行のデータを取得
		// 改行単位指定で、先頭から空白が1折り返し行以上続いている場合は次の行データを取得
		LayoutInt nPosY_Layout;
		LogicInt  nPosX_Logic;

		nPosY_Layout = ptCaretPos.y - 1;
		const Layout*	pcLayout;
		bool			bZenSpace = m_pCommanderView->m_pTypeData->m_bAutoIndent_ZENSPACE;
		bool			bExtEol = GetDllShareData().m_common.m_sEdit.m_bEnableExtEol;
		
		LogicInt		nLineLen;
		do {
			++nPosY_Layout;
			const wchar_t*	pLine = GetDocument()->m_cLayoutMgr.GetLineStr(nPosY_Layout, &nLineLen, &pcLayout);
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
		while ((lparam & 8) && (nPosX_Logic >= nLineLen) && (GetDocument()->m_cLayoutMgr.GetLineCount() - 1 > nPosY_Layout));
		
		if (nPosX_Logic >= nLineLen) {
			/* 折り返し単位の行頭を探して物理行末まで到達した
			または、最終行のため改行コードに遭遇せずに行末に到達した */
			nPosX_Logic = 0;
		}
		
		if (nPosX_Logic == 0) nPosY_Layout = ptCaretPos.y;	// 物理行の移動なし
		
		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		LayoutInt nPosX_Layout = m_pCommanderView->LineIndexToColumn(pcLayout, nPosX_Logic);
		LayoutPoint ptPos(nPosX_Layout, nPosY_Layout);
		if (caret.GetCaretLayoutPos() != ptPos) {
			ptCaretPos = ptPos;
		}
	}

	// 2006.07.09 genta 新規関数にまとめた
	m_pCommanderView->MoveCursorSelecting(ptCaretPos, bSelect);
}


/*! 行末に移動(折り返し単位)
	@praram nOption	0x08 改行単位(合成可)
*/
void ViewCommander::Command_GOLINEEND(
	bool bSelect,
	int bIgnoreCurrentSelection,
	int nOption
	)
{
	if (nOption & 4) {
		bSelect = true;
	}
	auto& si = m_pCommanderView->GetSelectionInfo();
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
	LayoutPoint nPosXY = caret.GetCaretLayoutPos();
	if (nOption & 8) {
		// 改行単位の行末。1行中の最終レイアウト行を探す
		const Layout*	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(nPosXY.y);
		const Layout*	pcLayoutNext = pcLayout->GetNextLayout();
		while (pcLayout && pcLayoutNext && pcLayoutNext->GetLogicOffset() != 0) {
			pcLayout = pcLayoutNext;
			pcLayoutNext = pcLayoutNext->GetNextLayout();
			++nPosXY.y;
		}
	}
	nPosXY.x = LayoutInt(0);
	const Layout*	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(nPosXY.y);
	if (pcLayout)
		nPosXY.x = pcLayout->CalcLayoutWidth(GetDocument()->m_cLayoutMgr);

	// キャレット移動
	caret.GetAdjustCursorPos(&nPosXY);
	if (bSelect) {
		// 現在のカーソル位置によって選択範囲を変更
		si.ChangeSelectAreaByCurrentCursor(nPosXY);
	}
	caret.MoveCursor(nPosXY, true);
	caret.m_nCaretPosX_Prev = nPosXY.x;
}


// 半ページアップ		// Oct. 6, 2000 JEPRO added (実は従来のスクロールダウンそのもの)
void ViewCommander::Command_HalfPageUp(
	bool bSelect,
	LayoutYInt nScrollNum
	)
{
	if (nScrollNum <= 0) {
		nScrollNum = m_pCommanderView->GetTextArea().m_nViewRowNum / 2;
	}
	GetCaret().Cursor_UPDOWN( - (nScrollNum), bSelect );
	return;
}


// 半ページダウン		// Oct. 6, 2000 JEPRO added (実は従来のスクロールアップそのもの)
void ViewCommander::Command_HalfPageDown(
	bool bSelect,
	LayoutYInt nScrollNum
	)
{
	if (nScrollNum <= 0) {
		nScrollNum = m_pCommanderView->GetTextArea().m_nViewRowNum / 2;
	}
	GetCaret().Cursor_UPDOWN( nScrollNum, bSelect );
	return;
}


/*! １ページアップ

	@date 2000.10.10 JEPRO 作成
	@date 2001.12.13 hor 画面に対するカーソル位置はそのままで
		１ページアップに動作変更
	@date 2014.01.10 Moca カーソルが動かないときも画面をスクロールするように
*/	// Oct. 10, 2000 JEPRO added
void ViewCommander::Command_1PageUp(
	bool bSelect,
	LayoutYInt nScrollNum
	)
{
// GetCaret().Cursor_UPDOWN(-m_pCommanderView->GetTextArea().m_nViewRowNum, bSelect);

// 2001.12.03 hor
//		メモ帳ライクに、画面に対するカーソル位置はそのままで１ページアップ
	{
		const bool bDrawSwitchOld = m_pCommanderView->SetDrawSwitch(false);
		auto& textArea = m_pCommanderView->GetTextArea();
		LayoutInt nViewTopLine = textArea.GetViewTopLine();
		if (nScrollNum <= 0) {
			nScrollNum = m_pCommanderView->GetTextArea().m_nViewRowNum - 1;
		}
		GetCaret().Cursor_UPDOWN( -nScrollNum, bSelect );
		// Sep. 11, 2004 genta 同期スクロール処理のため
		// m_pCommanderView->RedrawAllではなくScrollAtを使うように
		m_pCommanderView->SyncScrollV( m_pCommanderView->ScrollAtV( nViewTopLine - nScrollNum ));
		m_pCommanderView->SetDrawSwitch(bDrawSwitchOld);
		m_pCommanderView->RedrawAll();
	}
	return;
}


/*!	１ページダウン

	@date 2000.10.10 JEPRO 作成
	@date 2001.12.13 hor 画面に対するカーソル位置はそのままで
		１ページダウンに動作変更
	@date 2014.01.10 Moca カーソルが動かないときも画面をスクロールするように
*/
void ViewCommander::Command_1PageDown(
	bool bSelect,
	LayoutYInt nScrollNum
	)
{
// GetCaret().Cursor_UPDOWN(m_pCommanderView->GetTextArea().m_nViewRowNum, bSelect);

// 2001.12.03 hor
//		メモ帳ライクに、画面に対するカーソル位置はそのままで１ページダウン
	{
		const bool bDrawSwitchOld = m_pCommanderView->SetDrawSwitch(false);
		LayoutInt nViewTopLine = m_pCommanderView->GetTextArea().GetViewTopLine();
		if (nScrollNum <= 0) {
			nScrollNum = m_pCommanderView->GetTextArea().m_nViewRowNum - 1;
		}
		GetCaret().Cursor_UPDOWN(nScrollNum, bSelect);
		// Sep. 11, 2004 genta 同期スクロール処理のため
		// m_pCommanderView->RedrawAllではなくScrollAtを使うように
		m_pCommanderView->SyncScrollV(m_pCommanderView->ScrollAtV(nViewTopLine + nScrollNum));
		m_pCommanderView->SetDrawSwitch(bDrawSwitchOld);
		m_pCommanderView->RedrawAll();
	}

	return;
}


// ファイルの先頭に移動
void ViewCommander::Command_GOFILETOP(bool bSelect)
{
	// 先頭へカーソルを移動
	// Sep. 8, 2000 genta
	m_pCommanderView->AddCurrentLineToHistory();

	// 2006.07.09 genta 新規関数にまとめた
	LayoutPoint pt(
		!m_pCommanderView->GetSelectionInfo().IsBoxSelecting()? LayoutInt(0): GetCaret().GetCaretLayoutPos().GetX2(),
		LayoutInt(0)
	);
	m_pCommanderView->MoveCursorSelecting(pt, bSelect);	// 通常は、(0, 0)へ移動。ボックス選択中は、(GetCaret().GetCaretLayoutPos().GetX2(), 0)へ移動
}


// ファイルの最後に移動
void ViewCommander::Command_GOFILEEND(bool bSelect)
{
	auto& si = m_pCommanderView->GetSelectionInfo();
// 2001.12.13 hor BOX選択中にファイルの最後にジャンプすると[EOF]の行が反転したままになるの修正
	if (!bSelect) {
		if (si.IsTextSelected()) {
			si.DisableSelectArea(true);	// 2001.12.21 hor Add
		}else if (si.IsBoxSelecting()) {
			si.SetBoxSelect(false);
		}
	}
	m_pCommanderView->AddCurrentLineToHistory();
	auto& caret = GetCaret();
	caret.Cursor_UPDOWN(GetDocument()->m_cLayoutMgr.GetLineCount() , bSelect);
	Command_DOWN(bSelect, true);
	if (!si.IsBoxSelecting()) {							// 2002/04/18 YAZAKI
		/*	2004.04.19 fotomo
			改行のない最終行で選択肢ながら文書末へ移動した場合に
			選択範囲が正しくない場合がある問題に対応
		*/
		Command_GOLINEEND(bSelect, 0, 0);				// 2001.12.21 hor Add
	}
	caret.MoveCursor(caret.GetCaretLayoutPos(), true);	// 2001.12.21 hor Add
	// 2002.02.16 hor 矩形選択中を除き直前のカーソル位置をリセット
	if (!(si.IsTextSelected() && si.IsBoxSelecting())) {
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
	}

	// 選択範囲情報メッセージを表示する	// 2009.05.06 ryoji 追加
	if (bSelect) {
		si.PrintSelectionInfoMsg();
	}
}


// カーソル行をウィンドウ中央へ
void ViewCommander::Command_CURLINECENTER(void)
{
	LayoutInt nViewTopLine;
	auto& textArea = m_pCommanderView->GetTextArea();
	nViewTopLine = GetCaret().GetCaretLayoutPos().GetY2() - (textArea.m_nViewRowNum / 2);

	// sui 02/08/09
	if (0 > nViewTopLine) nViewTopLine = LayoutInt(0);
	
	LayoutInt nScrollLines = nViewTopLine - textArea.GetViewTopLine();	// Sep. 11, 2004 genta 同期用に行数を記憶
	textArea.SetViewTopLine(nViewTopLine);
	// フォーカス移動時の再描画
	m_pCommanderView->RedrawAll();
	// sui 02/08/09

	// Sep. 11, 2004 genta 同期スクロールの関数化
	m_pCommanderView->SyncScrollV(nScrollLines);
}


// 移動履歴を前へたどる
void ViewCommander::Command_JUMPHIST_PREV(void)
{
	// 2001.12.13 hor
	// 移動履歴の最後に現在の位置を記憶する
	// (次の履歴が取得できないときは追加して戻る)
	if (!m_pCommanderView->m_cHistory->CheckNext()) {
		m_pCommanderView->AddCurrentLineToHistory();
		m_pCommanderView->m_cHistory->PrevValid();
	}

	if (m_pCommanderView->m_cHistory->CheckPrev()) {
		if (! m_pCommanderView->m_cHistory->PrevValid()) {
			::MessageBox(NULL, _T("Inconsistent Implementation"), _T("PrevValid"), MB_OK);
		}
		LayoutPoint pt;
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			m_pCommanderView->m_cHistory->GetCurrent().GetPosition(),
			&pt
		);
		// 2006.07.09 genta 選択を考慮
		m_pCommanderView->MoveCursorSelecting(pt, m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
	}
}


// 移動履歴を次へたどる
void ViewCommander::Command_JUMPHIST_NEXT(void)
{
	if (m_pCommanderView->m_cHistory->CheckNext()) {
		if (!m_pCommanderView->m_cHistory->NextValid()) {
			::MessageBox(NULL, _T("Inconsistent Implementation"), _T("NextValid"), MB_OK);
		}
		LayoutPoint pt;
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			m_pCommanderView->m_cHistory->GetCurrent().GetPosition(),
			&pt
		);
		// 2006.07.09 genta 選択を考慮
		m_pCommanderView->MoveCursorSelecting(pt, m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
	}
}


// 現在位置を移動履歴に登録する
void ViewCommander::Command_JUMPHIST_SET(void)
{
	m_pCommanderView->AddCurrentLineToHistory();
}


// 2001/06/20 Start by asa-o

// from CViewCommander_New.cpp
// テキストを１行下へスクロール
void ViewCommander::Command_WndScrollDown(void)
{
	LayoutInt	nCaretMarginY;

	auto& textArea = m_pCommanderView->GetTextArea();
	nCaretMarginY = textArea.m_nViewRowNum / _CARETMARGINRATE;
	if (nCaretMarginY < 1) {
		nCaretMarginY = LayoutInt(1);
	}

	nCaretMarginY += 2;

	bool bCaretOff = false;
	auto& caret = GetCaret();
	if (caret.GetCaretLayoutPos().GetY() > textArea.m_nViewRowNum + textArea.GetViewTopLine() - (nCaretMarginY + 1)) {
		bCaretOff = true;
	}

	// Sep. 11, 2004 genta 同期用に行数を記憶
	// Sep. 11, 2004 genta 同期スクロールの関数化
	m_pCommanderView->SyncScrollV(m_pCommanderView->ScrollAtV(textArea.GetViewTopLine() - LayoutInt(1)));

	// テキストが選択されていない
	if (!m_pCommanderView->GetSelectionInfo().IsTextSelected()) {
		// カーソルが画面外に出た
		if (caret.GetCaretLayoutPos().GetY() > textArea.m_nViewRowNum + textArea.GetViewTopLine() - nCaretMarginY) {
			if (caret.GetCaretLayoutPos().GetY() > GetDocument()->m_cLayoutMgr.GetLineCount() - nCaretMarginY) {
				caret.Cursor_UPDOWN((GetDocument()->m_cLayoutMgr.GetLineCount() - nCaretMarginY) - caret.GetCaretLayoutPos().GetY2(), FALSE);
			}else {
				caret.Cursor_UPDOWN(LayoutInt(-1), FALSE);
			}
			caret.ShowCaretPosInfo();
		}
	}
	if (bCaretOff) {
		caret.m_cUnderLine.CaretUnderLineOFF(true);
	}
	caret.m_cUnderLine.CaretUnderLineON(true, true);
}


// from CViewCommander_New.cpp
// テキストを１行上へスクロール
void ViewCommander::Command_WndScrollUp(void)
{
	LayoutInt	nCaretMarginY;

	auto& textArea = m_pCommanderView->GetTextArea();
	nCaretMarginY = textArea.m_nViewRowNum / _CARETMARGINRATE;
	if (nCaretMarginY < 1)
		nCaretMarginY = 1;

	bool bCaretOff = false;
	if (GetCaret().GetCaretLayoutPos().GetY2() < textArea.GetViewTopLine() + (nCaretMarginY + 1)) {
		bCaretOff = true;
	}

	// Sep. 11, 2004 genta 同期用に行数を記憶
	// Sep. 11, 2004 genta 同期スクロールの関数化
	m_pCommanderView->SyncScrollV(m_pCommanderView->ScrollAtV(textArea.GetViewTopLine() + LayoutInt(1)));

	// テキストが選択されていない
	if (!m_pCommanderView->GetSelectionInfo().IsTextSelected()) {
		// カーソルが画面外に出た
		if (GetCaret().GetCaretLayoutPos().GetY() < textArea.GetViewTopLine() + nCaretMarginY) {
			if (textArea.GetViewTopLine() == 1) {
				GetCaret().Cursor_UPDOWN(nCaretMarginY + 1, FALSE);
			}else {
				GetCaret().Cursor_UPDOWN(LayoutInt(1), FALSE);
			}
			GetCaret().ShowCaretPosInfo();
		}
	}
	if (bCaretOff) {
		GetCaret().m_cUnderLine.CaretUnderLineOFF(true);
	}
	GetCaret().m_cUnderLine.CaretUnderLineON(true, true);
}

// 2001/06/20 End


// from CViewCommander_New.cpp
/* 次の段落へ進む
	2002/04/26 段落の両端で止まるオプションを追加
	2002/04/19 新規
*/
void ViewCommander::Command_GONEXTPARAGRAPH(bool bSelect)
{
	CDocLine* pcDocLine;
	int nCaretPointer = 0;
	auto& docLineMgr = GetDocument()->m_cDocLineMgr;
	
	bool nFirstLineIsEmptyLine = false;
	// まずは、現在位置が空行（スペース、タブ、改行記号のみの行）かどうか判別
	if ((pcDocLine = docLineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2() + LogicInt(nCaretPointer)))) {
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		++nCaretPointer;
	}else {
		// EOF行でした。
		return;
	}

	// 次に、nFirstLineIsEmptyLineと異なるところまで読み飛ばす
	while ((pcDocLine = docLineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2() + LogicInt(nCaretPointer)))) {
		if (pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine) {
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
		if (GetDllShareData().m_common.m_sGeneral.m_bStopsBothEndsWhenSearchParagraph) {	// 段落の両端で止まる
		}else {
			// 仕上げに、空行じゃないところまで進む
			while ((pcDocLine = docLineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2() + LogicInt(nCaretPointer)))) {
				if (pcDocLine->IsEmptyLine()) {
					++nCaretPointer;
				}else {
					break;
				}
			}
		}
	}

	// EOFまで来たり、目的の場所まできたので移動終了。

	// 移動距離を計算
	LayoutPoint ptCaretPos_Layo;

	// 移動前の物理位置
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos(),
		&ptCaretPos_Layo
	);

	// 移動後の物理位置
	LayoutPoint ptCaretPos_Layo_CaretPointer;
	//int nCaretPosY_Layo_CaretPointer;
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos() + LogicPoint(0, nCaretPointer),
		&ptCaretPos_Layo_CaretPointer
	);

	GetCaret().Cursor_UPDOWN(ptCaretPos_Layo_CaretPointer.y - ptCaretPos_Layo.y, bSelect);
}


// from CViewCommander_New.cpp
/* 前の段落へ進む
	2002/04/26 段落の両端で止まるオプションを追加
	2002/04/19 新規
*/
void ViewCommander::Command_GOPREVPARAGRAPH(bool bSelect)
{
	auto& docLineMgr = GetDocument()->m_cDocLineMgr;
	CDocLine* pcDocLine;
	int nCaretPointer = -1;

	bool nFirstLineIsEmptyLine = false;
	// まずは、現在位置が空行（スペース、タブ、改行記号のみの行）かどうか判別
	if ((pcDocLine = docLineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2() + LogicInt(nCaretPointer)))) {
		nFirstLineIsEmptyLine = pcDocLine->IsEmptyLine();
		--nCaretPointer;
	}else {
		nFirstLineIsEmptyLine = true;
		--nCaretPointer;
	}

	// 次に、nFirstLineIsEmptyLineと異なるところまで読み飛ばす
	while ((pcDocLine = docLineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2() + LogicInt(nCaretPointer)))) {
		if (pcDocLine->IsEmptyLine() == nFirstLineIsEmptyLine) {
			--nCaretPointer;
		}else {
			break;
		}
	}

	/*	nFirstLineIsEmptyLineが空行だったら、今見ているところは非空行。すなわちおしまい。
		nFirstLineIsEmptyLineが非空行だったら、今見ているところは空行。
	*/
	auto& csGeneral = GetDllShareData().m_common.m_sGeneral;	
	if (nFirstLineIsEmptyLine) {
		// おしまい。
		if (csGeneral.m_bStopsBothEndsWhenSearchParagraph) {	// 段落の両端で止まる
			++nCaretPointer;	// 空行の最上行（段落の末端の次の行）で止まる。
		}else {
			// 仕上げに、空行じゃないところまで進む
			while ((pcDocLine = docLineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2() + LogicInt(nCaretPointer)))) {
				if (pcDocLine->IsEmptyLine()) {
					break;
				}else {
					--nCaretPointer;
				}
			}
			++nCaretPointer;	// 空行の最上行（段落の末端の次の行）で止まる。
		}
	}else {
		// いま見ているところは空行の1行目
		if (csGeneral.m_bStopsBothEndsWhenSearchParagraph) {	// 段落の両端で止まる
			++nCaretPointer;
		}else {
			++nCaretPointer;
		}
	}

	// EOFまで来たり、目的の場所まできたので移動終了。

	// 移動距離を計算
	LayoutPoint ptCaretPos_Layo;

	// 移動前の物理位置
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos(),
		&ptCaretPos_Layo
	);

	// 移動後の物理位置
	LayoutPoint ptCaretPos_Layo_CaretPointer;
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		GetCaret().GetCaretLogicPos() + LogicPoint(0, nCaretPointer),
		&ptCaretPos_Layo_CaretPointer
	);

	GetCaret().Cursor_UPDOWN(ptCaretPos_Layo_CaretPointer.y - ptCaretPos_Layo.y, bSelect);
}

void ViewCommander::Command_AUTOSCROLL()
{
	if (m_pCommanderView->m_nAutoScrollMode == 0) {
		GetCursorPos(&m_pCommanderView->m_cAutoScrollMousePos);
		ScreenToClient(m_pCommanderView->GetHwnd(), &m_pCommanderView->m_cAutoScrollMousePos);
		m_pCommanderView->m_bAutoScrollDragMode = false;
		m_pCommanderView->AutoScrollEnter();
	}else {
		m_pCommanderView->AutoScrollExit();
	}
}

void ViewCommander::Command_WHEELUP(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELUP);
}

void ViewCommander::Command_WHEELDOWN(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELDOWN);
}

void ViewCommander::Command_WHEELLEFT(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELLEFT);
}

void ViewCommander::Command_WHEELRIGHT(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELRIGHT);
}

void ViewCommander::Command_WHEELPAGEUP(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELPAGEUP);
}

void ViewCommander::Command_WHEELPAGEDOWN(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, false, F_WHEELPAGEDOWN);
}

void ViewCommander::Command_WHEELPAGELEFT(int zDelta)
{
	int zDelta2 = -1 * (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELPAGELEFT);
}

void ViewCommander::Command_WHEELPAGERIGHT(int zDelta)
{
	int zDelta2 = (zDelta == 0 ? 120: zDelta);
	WPARAM wParam = MAKELONG(0, zDelta2);
	LPARAM lParam = 0;
	m_pCommanderView->OnMOUSEWHEEL2(wParam, lParam, true, F_WHEELPAGERIGHT);
}

/*! 次の変更行へ
	変更行のブロックの先頭行と、変更行のブロックの末尾(次の行頭)に移動する
*/
void ViewCommander::Command_MODIFYLINE_NEXT( bool bSelect )
{
	auto& docLineMgr = GetDocument()->m_cDocLineMgr;
	LogicInt nYOld = GetCaret().GetCaretLogicPos().y;
	LogicPoint ptXY(0, nYOld);
	const CDocLine* pcDocLine = docLineMgr.GetLine(ptXY.GetY2());
	const int nSaveSeq = GetDocument()->m_cDocEditor.m_cOpeBuf.GetNoModifiedSeq();
	bool bModified = false;
	if (docLineMgr.GetLineCount() == 0) {
		return;
	}
	if (pcDocLine) {
		bModified = CModifyVisitor().IsLineModified(pcDocLine, nSaveSeq);
		ptXY.y++;
		pcDocLine = pcDocLine->GetNextLine();
	}
	for (int n=0; n<2; ++n) {
		while (pcDocLine) {
			if (CModifyVisitor().IsLineModified(pcDocLine, nSaveSeq) != bModified
				|| (
					ptXY.y == 0
					&& CModifyVisitor().IsLineModified(pcDocLine, nSaveSeq)
				)
			) {
				LayoutPoint ptLayout;
				GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
				m_pCommanderView->MoveCursorSelecting( ptLayout, bSelect );
				if (nYOld >= ptXY.GetY2()) {
					m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT1));
				}
				return;
			}
			ptXY.y++;
			pcDocLine = pcDocLine->GetNextLine();
		}
		if (n == 0 && bModified) {
			const CDocLine* pcDocLineLast = docLineMgr.GetDocLineBottom();
			bool bSkip = false;
			LogicPoint pos;
			if (pcDocLineLast) {
				if (pcDocLineLast->GetEol() == EOL_NONE) {
					// ぶら下がり[EOF]
					pos.x = pcDocLineLast->GetLengthWithoutEOL();
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
				LayoutPoint ptLayout;
				GetDocument()->m_cLayoutMgr.LogicToLayout(pos, &ptLayout);
				m_pCommanderView->MoveCursorSelecting( ptLayout, bSelect );
				return;
			}
		}
		if (n == 0) {
			ptXY.y = LogicInt(0);
			pcDocLine = docLineMgr.GetLine(ptXY.GetY2());
			bModified = false;
		}
		if (!GetDllShareData().m_common.m_sSearch.m_bSearchAll) {
			break;
		}
	}
	m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT2));
	AlertNotFound( m_pCommanderView->GetHwnd(), false, LS(STR_MODLINE_NEXT_NOT_FOUND));
}

/*! 前の変更行へ
	変更行のブロックの先頭行と、変更行のブロックの末尾(次の行頭)に移動する
	Command_MODIFYLINE_NEXTと同じ位置に止まる
*/
void ViewCommander::Command_MODIFYLINE_PREV( bool bSelect )
{
	auto& docLineMgr = GetDocument()->m_cDocLineMgr;
	LogicInt nYOld = GetCaret().GetCaretLogicPos().y;
	LogicInt nYOld2 = nYOld;
	LogicPoint ptXY(0, nYOld);
	const CDocLine* pcDocLine = docLineMgr.GetLine(ptXY.GetY2());
	const int nSaveSeq = GetDocument()->m_cDocEditor.m_cOpeBuf.GetNoModifiedSeq();
	bool bModified = false;
	bool bLast = false;
	if (!pcDocLine) {
		// [EOF]
		const CDocLine* pcDocLineLast = docLineMgr.GetLine(ptXY.GetY2() - 1);
		if (!pcDocLineLast) {
			// 1行もない
			return;
		}
		bModified = CModifyVisitor().IsLineModified(pcDocLineLast, nSaveSeq);
		ptXY.y--;
		pcDocLine = pcDocLineLast;
		bLast = true;
	}
	if (!bLast) {
		const CDocLine* pcDocLineLast = docLineMgr.GetDocLineBottom();
		if (pcDocLineLast && pcDocLineLast->GetEol() == EOL_NONE) {
			LogicPoint pos;
			pos.x = pcDocLine->GetLengthWithoutEOL();
			pos.y = docLineMgr.GetLineCount() - 1;
			if (GetCaret().GetCaretLogicPos() == pos) {
				// ぶら下がり[EOF]の位置だった
				bLast = true;
			}
		}
	}
	assert( pcDocLine );
	bModified = CModifyVisitor().IsLineModified(pcDocLine, nSaveSeq);
	nYOld2 = ptXY.y;
	ptXY.y--;
	pcDocLine = pcDocLine->GetPrevLine();
	if (pcDocLine && !bLast) {
		bModified = CModifyVisitor().IsLineModified(pcDocLine, nSaveSeq);
		nYOld2 = ptXY.y;
		ptXY.y--;
		pcDocLine = pcDocLine->GetPrevLine();
	}
	for (int n=0; n<2; ++n) {
		while (pcDocLine) {
			bool bModifiedTemp = CModifyVisitor().IsLineModified(pcDocLine, nSaveSeq);
			if (bModifiedTemp != bModified) {
				// 検出された位置の1行後ろ(MODIFYLINE_NEXTと同じ位置)に止まる
				ptXY.y = nYOld2;
				LayoutPoint ptLayout;
				GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
				m_pCommanderView->MoveCursorSelecting( ptLayout, bSelect );
				if (n == 1) {
					m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV1));
				}
				return;
			}
			nYOld2 = ptXY.y;
			ptXY.y--;
			pcDocLine = pcDocLine->GetPrevLine();
		}
		if (n == 0) {
			// 先頭行チェック
			const CDocLine* pcDocLineTemp = docLineMgr.GetDocLineTop();
			assert( pcDocLineTemp );
			if (CModifyVisitor().IsLineModified(pcDocLineTemp, nSaveSeq) != false) {
				if (GetCaret().GetCaretLogicPos() != LogicPoint(0,0)) {
					ptXY = LogicPoint(0,0);
					LayoutPoint ptLayout;
					GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
					m_pCommanderView->MoveCursorSelecting( ptLayout, bSelect );
					return;
				}
			}
			ptXY.y = docLineMgr.GetLineCount() - LogicInt(1);
			nYOld2 = ptXY.y;
			pcDocLine = docLineMgr.GetLine(ptXY.GetY2());
			bModified = false;
		}
		if (!GetDllShareData().m_common.m_sSearch.m_bSearchAll) {
			break;
		}
		if (n == 0) {
			const CDocLine* pcDocLineTemp = docLineMgr.GetDocLineBottom();
			assert( pcDocLineTemp );
			if (CModifyVisitor().IsLineModified(pcDocLineTemp, nSaveSeq) != false) {
				// 最終行が変更行の場合は、[EOF]に止まる
				LogicPoint pos;
				if (pcDocLineTemp->GetEol() != EOL_NONE) {
					pos.x = 0;
					pos.y = docLineMgr.GetLineCount();
					pos.y++;
				}else {
					pos.x = pcDocLine->GetLengthWithoutEOL();
					pos.y = docLineMgr.GetLineCount() - 1;
				}
				if (GetCaret().GetCaretLogicPos() != pos) {
					ptXY = pos;
					LayoutPoint ptLayout;
					GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY, &ptLayout);
					m_pCommanderView->MoveCursorSelecting( ptLayout, bSelect );
					m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV1));
					return;
				}
			}
		}
	}
	m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV2));
	AlertNotFound( m_pCommanderView->GetHwnd(), false, LS(STR_MODLINE_PREV_NOT_FOUND) );
}

