/*!	@file
@brief ViewCommanderクラスのコマンド(選択系/矩形選択系)関数群

	2012/12/20	ViewCommander.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2005, Moca
	Copyright (C) 2007, kobake, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"


// 現在位置の単語選択
bool ViewCommander::Command_SelectWord(const Point* pptCaretPos)
{
	auto& si = view.GetSelectionInfo();
	if (si.IsTextSelected()) {	// テキストが選択されているか
		// 現在の選択範囲を非選択状態に戻す
		si.DisableSelectArea(true);
	}
	auto& caret = GetCaret();
	Point ptCaretPos = ((!pptCaretPos) ? caret.GetCaretLayoutPos() : *pptCaretPos);
	auto& layoutMgr = GetDocument().layoutMgr;
	const Layout* pLayout = layoutMgr.SearchLineByLayoutY(ptCaretPos.y);
	if (!pLayout) {
		return false;	// 単語選択に失敗
	}
	// 指定された桁に対応する行のデータ内の位置を調べる
	int nIdx = view.LineColumnToIndex(pLayout, ptCaretPos.x);

	// 現在位置の単語の範囲を調べる
	Range range;
	if (layoutMgr.WhereCurrentWord(ptCaretPos.y, nIdx, &range, NULL, NULL)) {

		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
		/*
		pLayout = layoutMgr.SearchLineByLayoutY(range.GetFrom().y);
		range.SetFromX(view.LineIndexToColumn(pLayout, range.GetFrom().x));
		pLayout = layoutMgr.SearchLineByLayoutY(range.GetTo().y);
		range.SetToX(view.LineIndexToColumn(pLayout, range.GetTo().x));
		*/

		// 選択範囲の変更
		// 2005.06.24 Moca
		si.SetSelectArea(range);
		// 選択領域描画
		si.DrawSelectArea();

		// 単語の先頭にカーソルを移動
		caret.MoveCursor(range.GetTo(), true);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		return true;	// 単語選択に成功。
	}else {
		return false;	// 単語選択に失敗
	}
}


// すべて選択
void ViewCommander::Command_SelectAll(void)
{
	auto& si = view.GetSelectionInfo();
	if (si.IsTextSelected()) {	// テキストが選択されているか
		// 現在の選択範囲を非選択状態に戻す
		si.DisableSelectArea(true);
	}

	// 先頭へカーソルを移動
	// Sep. 8, 2000 genta
	view.AddCurrentLineToHistory();
	GetCaret().nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;

	// Jul. 29, 2006 genta 選択位置の末尾を正確に取得する
	// マクロから取得した場合に正しい範囲が取得できないため
	//int nX, nY;
	Range range;
	range.SetFrom(Point(0, 0));
	GetDocument().layoutMgr.GetEndLayoutPos(range.GetToPointer());
	si.SetSelectArea(range);

	// 選択領域描画
	si.DrawSelectArea(false);
}


/*!	1行選択
	@brief カーソル位置を1行選択する
	@param lparam [in] マクロから使用する拡張フラグ（拡張用に予約）

	note 改行単位で選択を行う。

	@date 2007.11.15 nasukoji	新規作成
*/
void ViewCommander::Command_SelectLine(int lparam)
{
	// 改行単位で1行選択する
	Command_GoLineTop(false, 0x9);	// 物理行頭に移動

	auto& si = view.GetSelectionInfo();
	si.bBeginLineSelect = true;		// 行単位選択中

	Point ptCaret;

	auto& layoutMgr = GetDocument().layoutMgr;
	auto& caret = GetCaret();
	// 最下行（物理行）でない
	if (caret.GetCaretLogicPos().y < GetDocument().docLineMgr.GetLineCount()) {
		// 1行先の物理行からレイアウト行を求める
		layoutMgr.LogicToLayout(Point(0, caret.GetCaretLogicPos().y + 1), &ptCaret);

		// カーソルを次の物理行頭へ移動する
		view.MoveCursorSelecting(ptCaret, true);

		// 移動後のカーソル位置を取得する
		ptCaret = caret.GetCaretLayoutPos().Get();
	}else {
		// カーソルを最下行（レイアウト行）へ移動する
		view.MoveCursorSelecting(Point(0, layoutMgr.GetLineCount()), true);
		Command_GoLineEnd(true, 0, 0);	// 行末に移動

		// 選択するものが無い（[EOF]のみの行）時は選択状態としない
		if (
			!si.IsTextSelected()
			&& (caret.GetCaretLogicPos().y >= GetDocument().docLineMgr.GetLineCount())
		) {
			// 現在の選択範囲を非選択状態に戻す
			si.DisableSelectArea(true);
		}
	}
	
	if (si.bBeginLineSelect) {
		// 範囲選択開始行・カラムを記憶
		si.select.SetTo(ptCaret);
		si.selectBgn.SetTo(ptCaret);
	}

	return;
}

// 範囲選択開始
void ViewCommander::Command_Begin_Select(void)
{
	auto& si = view.GetSelectionInfo();
	if (!si.IsTextSelected()) {	// テキストが選択されているか
		// 現在のカーソル位置から選択を開始する
		si.BeginSelectArea();
	}

	// ロックの解除切り替え
	if (si.bSelectingLock) {
		si.bSelectingLock = false;	// 選択状態のロック解除
	}else {
		si.bSelectingLock = true;		// 選択状態のロック
	}
	if (GetSelect().IsOne()) {
		GetCaret().underLine.CaretUnderLineOFF(true);
	}
	si.PrintSelectionInfoMsg();
	return;
}


// 矩形範囲選択開始
void ViewCommander::Command_Begin_BoxSelect(bool bSelectingLock)
{
	if (!GetDllShareData().common.view.bFontIs_FixedPitch) {	// 現在のフォントは固定幅フォントである
		return;
	}

	auto& si = view.GetSelectionInfo();
//@@@ 2002.01.03 YAZAKI 範囲選択中にShift+F6を実行すると選択範囲がクリアされない問題に対処
	if (si.IsTextSelected()) {	// テキストが選択されているか
		// 現在の選択範囲を非選択状態に戻す
		si.DisableSelectArea(true);
	}

	// 現在のカーソル位置から選択を開始する
	si.BeginSelectArea();

	si.bSelectingLock = bSelectingLock;	// 選択状態のロック
	si.SetBoxSelect(true);	// 矩形範囲選択中

	si.PrintSelectionInfoMsg();
	GetCaret().underLine.CaretUnderLineOFF(true);
	return;
}

