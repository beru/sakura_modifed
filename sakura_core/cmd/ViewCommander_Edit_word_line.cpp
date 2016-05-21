/*!	@file
@brief ViewCommanderクラスのコマンド(編集系 単語/行単位)関数群

	2012/12/16	ViewCommander.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, かろと
	Copyright (C) 2005, Moca
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"


// 単語の左端まで削除
void ViewCommander::Command_WordDeleteToStart(void)
{
	auto& selInfo = view.GetSelectionInfo();
	// 矩形選択状態では実行不能(★★もろ手抜き★★)
	if (selInfo.IsTextSelected()) {
		// 矩形範囲選択中か
		if (selInfo.IsBoxSelecting()) {
			ErrorBeep();
			return;
		}
	}

	// 単語の左端に移動
	ViewCommander::Command_WordLeft(true);
	if (!selInfo.IsTextSelected()) {
		ErrorBeep();
		return;
	}

	if (!view.bDoing_UndoRedo) {	// Undo, Redoの実行中か
		MoveCaretOpe* pOpe = new MoveCaretOpe();
		GetDocument().layoutMgr.LayoutToLogic(
			GetSelect().GetTo(),
			&pOpe->ptCaretPos_PHY_Before
		);
		pOpe->ptCaretPos_PHY_After = pOpe->ptCaretPos_PHY_Before;	// 操作後のキャレット位置

		// 操作の追加
		GetOpeBlk()->AppendOpe(pOpe);
	}

	// 削除
	view.DeleteData(true);
}



// 単語の右端まで削除
void ViewCommander::Command_WordDeleteToEnd(void)
{
	auto& selInfo = view.GetSelectionInfo();

	// 矩形選択状態では実行不能((★★もろ手抜き★★))
	if (selInfo.IsTextSelected()) {
		// 矩形範囲選択中か
		if (selInfo.IsBoxSelecting()) {
			ErrorBeep();
			return;
		}
	}
	// 単語の右端に移動
	ViewCommander::Command_WordRight(true);
	if (!selInfo.IsTextSelected()) {
		ErrorBeep();
		return;
	}
	if (!view.bDoing_UndoRedo) {	// Undo, Redoの実行中か
		MoveCaretOpe* pOpe = new MoveCaretOpe();
		GetDocument().layoutMgr.LayoutToLogic(
			GetSelect().GetFrom(),
			&pOpe->ptCaretPos_PHY_Before
		);
		pOpe->ptCaretPos_PHY_After = pOpe->ptCaretPos_PHY_Before;	// 操作後のキャレット位置
		// 操作の追加
		GetOpeBlk()->AppendOpe(pOpe);
	}
	// 削除
	view.DeleteData(true);
}


// 単語切り取り
void ViewCommander::Command_WordCut(void)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsTextSelected()) {
		// 切り取り(選択範囲をクリップボードにコピーして削除)
		Command_Cut();
		return;
	}
	// 現在位置の単語選択
	Command_SelectWord();
	// 切り取り(選択範囲をクリップボードにコピーして削除)
	if (!selInfo.IsTextSelected()) {
		// 単語選択で選択できなかったら、次の文字を選ぶことに挑戦。
		Command_Right(true, false, false);
	}
	Command_Cut();
	return;
}


// 単語削除
void ViewCommander::Command_WordDelete(void)
{
	if (view.GetSelectionInfo().IsTextSelected()) {
		// 削除
		view.DeleteData(true);
		return;
	}
	// 現在位置の単語選択
	Command_SelectWord();
	// 削除
	view.DeleteData(true);
	return;
}


// 行頭まで切り取り(改行単位)
void ViewCommander::Command_LineCutToStart(void)
{
	auto& selInfo = view.GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// テキストが選択されているか
		// 切り取り(選択範囲をクリップボードにコピーして削除)
		Command_Cut();
		return;
	}
	auto& layoutMgr = GetDocument().layoutMgr;
	auto& caret = GetCaret();
	pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());	// 指定された物理行のレイアウトデータ(Layout)へのポインタを返す
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;
	layoutMgr.LogicToLayout(LogicPoint(0, pLayout->GetLogicLineNo()), &ptPos);
	if (caret.GetCaretLayoutPos() == ptPos) {
		ErrorBeep();
		return;
	}

	// 選択範囲の変更
	// 2005.06.24 Moca
	LayoutRange range(ptPos, caret.GetCaretLayoutPos());
	selInfo.SetSelectArea(range);

	// 切り取り(選択範囲をクリップボードにコピーして削除)
	Command_Cut();
}



// 行末まで切り取り(改行単位)
void ViewCommander::Command_LineCutToEnd(void)
{
	auto& selInfo = view.GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// テキストが選択されているか
		// 切り取り(選択範囲をクリップボードにコピーして削除)
		Command_Cut();
		return;
	}
	auto& layoutMgr = GetDocument().layoutMgr;
	auto& caret = GetCaret();
	// 指定された物理行のレイアウトデータ(Layout)へのポインタを返す
	pLayout = layoutMgr.SearchLineByLayoutY(
		caret.GetCaretLayoutPos().GetY2()
	);
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;
	auto& docLineRef = *pLayout->GetDocLineRef();
	if (docLineRef.GetEol() == EolType::None) {	// 改行コードの種類
		layoutMgr.LogicToLayout(
			LogicPoint(
				docLineRef.GetLengthWithEOL(),
				pLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}else {
		layoutMgr.LogicToLayout(
			LogicPoint(
				docLineRef.GetLengthWithEOL() - docLineRef.GetEol().GetLen(),
				pLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}

	if (caret.GetCaretLayoutPos().GetY2() == ptPos.y && caret.GetCaretLayoutPos().GetX2() >= ptPos.x) {
		ErrorBeep();
		return;
	}

	// 選択範囲の変更
	// 2005.06.24 Moca
	LayoutRange range(caret.GetCaretLayoutPos(), ptPos);
	selInfo.SetSelectArea(range);

	// 切り取り(選択範囲をクリップボードにコピーして削除)
	Command_Cut();
}


// 行頭まで削除(改行単位)
void ViewCommander::Command_LineDeleteToStart(void)
{
	auto& selInfo = view.GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// テキストが選択されているか
		view.DeleteData(true);
		return;
	}
	auto& layoutMgr = GetDocument().layoutMgr;
	auto& caret = GetCaret();
	pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());	// 指定された物理行のレイアウトデータ(Layout)へのポインタを返す
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;

	layoutMgr.LogicToLayout(LogicPoint(0, pLayout->GetLogicLineNo()), &ptPos);
	if (caret.GetCaretLayoutPos() == ptPos) {
		ErrorBeep();
		return;
	}

	// 選択範囲の変更
	// 2005.06.24 Moca
	LayoutRange range(ptPos, caret.GetCaretLayoutPos());
	selInfo.SetSelectArea(range);

	// 選択領域削除
	view.DeleteData(true);
}


// 行末まで削除(改行単位)
void ViewCommander::Command_LineDeleteToEnd(void)
{
	auto& selInfo = view.GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// テキストが選択されているか
		view.DeleteData(true);
		return;
	}

	auto& caretLayoutPos = GetCaret().GetCaretLayoutPos();
	pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(caretLayoutPos.GetY2());	// 指定された物理行のレイアウトデータ(Layout)へのポインタを返す
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;

	auto& docLineRef = *pLayout->GetDocLineRef();
	if (docLineRef.GetEol() == EolType::None) {	// 改行コードの種類
		GetDocument().layoutMgr.LogicToLayout(
			LogicPoint(
				docLineRef.GetLengthWithEOL(),
				pLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}else {
		GetDocument().layoutMgr.LogicToLayout(
			LogicPoint(
				docLineRef.GetLengthWithEOL() - docLineRef.GetEol().GetLen(),
				pLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}

	if (caretLayoutPos.GetY2() == ptPos.y
		&& caretLayoutPos.GetX2() >= ptPos.x
	) {
		ErrorBeep();
		return;
	}

	// 選択範囲の変更
	// 2005.06.24 Moca
	LayoutRange range(caretLayoutPos, ptPos);
	selInfo.SetSelectArea(range);

	// 選択領域削除
	view.DeleteData(true);
}


// 行切り取り(折り返し単位)
void ViewCommander::Command_Cut_Line(void)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	if (selInfo.IsTextSelected()) {	// テキストが選択されているか
		ErrorBeep();
		return;
	}

	const Layout* pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().y);
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	// 2007.10.04 ryoji 処理簡素化
	view.CopyCurLine(
		GetDllShareData().common.edit.bAddCRLFWhenCopy,
		EolType::Unknown,
		GetDllShareData().common.edit.bEnableLineModePaste
	);
	Command_Delete_Line();
	return;
}


// 行削除(折り返し単位)
void ViewCommander::Command_Delete_Line(void)
{
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	if (selInfo.IsTextSelected()) {	// テキストが選択されているか
		ErrorBeep();
		return;
	}
	auto& caret = GetCaret();
	const Layout* pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());
	if (!pLayout) {
		ErrorBeep();
		return;
	}
	GetSelect().SetFrom(LayoutPoint(LayoutInt(0), caret.GetCaretLayoutPos().GetY2()  ));	// 範囲選択開始位置
	GetSelect().SetTo  (LayoutPoint(LayoutInt(0), caret.GetCaretLayoutPos().GetY2() + 1));	// 範囲選択終了位置

	LayoutPoint ptCaretPos_OLD = caret.GetCaretLayoutPos();

	Command_Delete();
	pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());
	if (pLayout) {
		// 2003-04-30 かろと
		// 行削除した後、フリーカーソルでないのにカーソル位置が行端より右になる不具合対応
		// フリーカーソルモードでない場合は、カーソル位置を調整する
		if (!GetDllShareData().common.general.bIsFreeCursorMode) {
			LogicInt nIndex;

			LayoutInt tmp;
			nIndex = view.LineColumnToIndex2(pLayout, ptCaretPos_OLD.GetX2(), &tmp);
			ptCaretPos_OLD.x = tmp;

			if (ptCaretPos_OLD.x > 0) {
				ptCaretPos_OLD.x--;
			}else {
				ptCaretPos_OLD.x = view.LineIndexToColumn(pLayout, nIndex);
			}
		}
		// 操作前の位置へカーソルを移動
		caret.MoveCursor(ptCaretPos_OLD, true);
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		if (!view.bDoing_UndoRedo) {	// Undo, Redoの実行中か
			// 操作の追加
			GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					caret.GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
	}
	return;
}


// 行の二重化(折り返し単位)
void ViewCommander::Command_DuplicateLine(void)
{
	NativeW		memBuf;
	auto& selInfo = view.GetSelectionInfo();
	if (selInfo.IsTextSelected()) {	// テキストが選択されているか
		// 現在の選択範囲を非選択状態に戻す
		selInfo.DisableSelectArea(true);
	}

	auto& caret = GetCaret();
	auto& layoutMgr = GetDocument().layoutMgr;
	const Layout* pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	if (!view.bDoing_UndoRedo) {	// Undo, Redoの実行中か
		// 操作の追加
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				caret.GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}

	LayoutPoint ptCaretPosOld = caret.GetCaretLayoutPos() + LayoutPoint(0, 1);

	// 行頭に移動(折り返し単位)
	Command_GoLineTop(selInfo.bSelectingLock, 0x1 /* カーソル位置に関係なく行頭に移動 */);

	if (!view.bDoing_UndoRedo) {	// Undo, Redoの実行中か
		// 操作の追加
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				caret.GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}

	/* 二重化したい行を調べる
	||	・改行で終わっている
	||	・改行で終わっていない
	||	・最終行である
	||	→折り返しでない
	||	・最終行でない
	||	→折り返しである
	*/
	bool bCRLF = (pLayout->GetLayoutEol() != EolType::None);
	bool bAddCRLF = false;
	if (!bCRLF) {
		if (caret.GetCaretLayoutPos().GetY2() == layoutMgr.GetLineCount() - 1) {
			bAddCRLF = true;
		}
	}

	memBuf.SetString(pLayout->GetPtr(), pLayout->GetLengthWithoutEOL() + pLayout->GetLayoutEol().GetLen());	// ※pLayout->GetLengthWithEOL()は、EOLの長さを必ず1にするので使えない。
	if (bAddCRLF) {
		// 現在、Enterなどで挿入する改行コードの種類を取得
		Eol cWork = GetDocument().docEditor.GetNewLineCode();
		memBuf.AppendString(cWork.GetValue2(), cWork.GetLen());
	}

	// 現在位置にデータを挿入
	LayoutPoint ptLayoutNew;
	view.InsertData_CEditView(
		caret.GetCaretLayoutPos(),
		memBuf.GetStringPtr(),
		memBuf.GetStringLength(),
		&ptLayoutNew,
		true
	);

	// カーソルを移動
	caret.MoveCursor(ptCaretPosOld, true);
	caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

	if (!view.bDoing_UndoRedo) {	// Undo, Redoの実行中か
		// 操作の追加
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				caret.GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}
	return;
}

