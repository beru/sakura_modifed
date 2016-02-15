/*!	@file
@brief ViewCommanderクラスのコマンド(編集系 単語/行単位)関数群

	2012/12/16	CViewCommander.cppから分離
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
#include "CViewCommander.h"
#include "CViewCommander_inline.h"


// 単語の左端まで削除
void ViewCommander::Command_WordDeleteToStart(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	// 矩形選択状態では実行不能(★★もろ手抜き★★)
	if (selInfo.IsTextSelected()) {
		// 矩形範囲選択中か
		if (selInfo.IsBoxSelecting()) {
			ErrorBeep();
			return;
		}
	}

	// 単語の左端に移動
	ViewCommander::Command_WORDLEFT(true);
	if (!selInfo.IsTextSelected()) {
		ErrorBeep();
		return;
	}

	if (!m_pCommanderView->m_bDoing_UndoRedo) {	// アンドゥ・リドゥの実行中か
		MoveCaretOpe*	pOpe = new MoveCaretOpe();
		GetDocument()->m_layoutMgr.LayoutToLogic(
			GetSelect().GetTo(),
			&pOpe->m_ptCaretPos_PHY_Before
		);
		pOpe->m_ptCaretPos_PHY_After = pOpe->m_ptCaretPos_PHY_Before;	// 操作後のキャレット位置

		// 操作の追加
		GetOpeBlk()->AppendOpe(pOpe);
	}

	// 削除
	m_pCommanderView->DeleteData(true);
}



// 単語の右端まで削除
void ViewCommander::Command_WordDeleteToEnd(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();

	// 矩形選択状態では実行不能((★★もろ手抜き★★))
	if (selInfo.IsTextSelected()) {
		// 矩形範囲選択中か
		if (selInfo.IsBoxSelecting()) {
			ErrorBeep();
			return;
		}
	}
	// 単語の右端に移動
	ViewCommander::Command_WORDRIGHT(true);
	if (!selInfo.IsTextSelected()) {
		ErrorBeep();
		return;
	}
	if (!m_pCommanderView->m_bDoing_UndoRedo) {	// アンドゥ・リドゥの実行中か
		MoveCaretOpe*	pOpe = new MoveCaretOpe();
		GetDocument()->m_layoutMgr.LayoutToLogic(
			GetSelect().GetFrom(),
			&pOpe->m_ptCaretPos_PHY_Before
		);
		pOpe->m_ptCaretPos_PHY_After = pOpe->m_ptCaretPos_PHY_Before;	// 操作後のキャレット位置
		// 操作の追加
		GetOpeBlk()->AppendOpe(pOpe);
	}
	// 削除
	m_pCommanderView->DeleteData(true);
}


// 単語切り取り
void ViewCommander::Command_WordCut(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	if (selInfo.IsTextSelected()) {
		// 切り取り(選択範囲をクリップボードにコピーして削除)
		Command_CUT();
		return;
	}
	// 現在位置の単語選択
	Command_SELECTWORD();
	// 切り取り(選択範囲をクリップボードにコピーして削除)
	if (!selInfo.IsTextSelected()) {
		// 単語選択で選択できなかったら、次の文字を選ぶことに挑戦。
		Command_RIGHT(true, false, false);
	}
	Command_CUT();
	return;
}


// 単語削除
void ViewCommander::Command_WordDelete(void)
{
	if (m_pCommanderView->GetSelectionInfo().IsTextSelected()) {
		// 削除
		m_pCommanderView->DeleteData(true);
		return;
	}
	// 現在位置の単語選択
	Command_SELECTWORD();
	// 削除
	m_pCommanderView->DeleteData(true);
	return;
}


// 行頭まで切り取り(改行単位)
void ViewCommander::Command_LineCutToStart(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// テキストが選択されているか
		// 切り取り(選択範囲をクリップボードにコピーして削除)
		Command_CUT();
		return;
	}
	pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());	// 指定された物理行のレイアウトデータ(Layout)へのポインタを返す
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;
	GetDocument()->m_layoutMgr.LogicToLayout(LogicPoint(0, pLayout->GetLogicLineNo()), &ptPos);
	if (GetCaret().GetCaretLayoutPos() == ptPos) {
		ErrorBeep();
		return;
	}

	// 選択範囲の変更
	// 2005.06.24 Moca
	LayoutRange range(ptPos, GetCaret().GetCaretLayoutPos());
	selInfo.SetSelectArea(range);

	// 切り取り(選択範囲をクリップボードにコピーして削除)
	Command_CUT();
}



// 行末まで切り取り(改行単位)
void ViewCommander::Command_LineCutToEnd(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// テキストが選択されているか
		// 切り取り(選択範囲をクリップボードにコピーして削除)
		Command_CUT();
		return;
	}
	// 指定された物理行のレイアウトデータ(Layout)へのポインタを返す
	pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(
		GetCaret().GetCaretLayoutPos().GetY2()
	);
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;
	auto& docLineRef = *pLayout->GetDocLineRef();
	if (docLineRef.GetEol() == EolType::None) {	// 改行コードの種類
		GetDocument()->m_layoutMgr.LogicToLayout(
			LogicPoint(
				docLineRef.GetLengthWithEOL(),
				pLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}else {
		GetDocument()->m_layoutMgr.LogicToLayout(
			LogicPoint(
				docLineRef.GetLengthWithEOL() - docLineRef.GetEol().GetLen(),
				pLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}

	if (GetCaret().GetCaretLayoutPos().GetY2() == ptPos.y && GetCaret().GetCaretLayoutPos().GetX2() >= ptPos.x) {
		ErrorBeep();
		return;
	}

	// 選択範囲の変更
	// 2005.06.24 Moca
	LayoutRange range(GetCaret().GetCaretLayoutPos(), ptPos);
	selInfo.SetSelectArea(range);

	// 切り取り(選択範囲をクリップボードにコピーして削除)
	Command_CUT();
}


// 行頭まで削除(改行単位)
void ViewCommander::Command_LineDeleteToStart(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// テキストが選択されているか
		m_pCommanderView->DeleteData(true);
		return;
	}
	pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());	// 指定された物理行のレイアウトデータ(Layout)へのポインタを返す
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;

	GetDocument()->m_layoutMgr.LogicToLayout(LogicPoint(0, pLayout->GetLogicLineNo()), &ptPos);
	if (GetCaret().GetCaretLayoutPos() == ptPos) {
		ErrorBeep();
		return;
	}

	// 選択範囲の変更
	// 2005.06.24 Moca
	LayoutRange range(ptPos, GetCaret().GetCaretLayoutPos());
	selInfo.SetSelectArea(range);

	// 選択領域削除
	m_pCommanderView->DeleteData(true);
}


// 行末まで削除(改行単位)
void ViewCommander::Command_LineDeleteToEnd(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	Layout* pLayout;
	if (selInfo.IsTextSelected()) {	// テキストが選択されているか
		m_pCommanderView->DeleteData(true);
		return;
	}

	auto& caretLayoutPos = GetCaret().GetCaretLayoutPos();
	pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(caretLayoutPos.GetY2());	// 指定された物理行のレイアウトデータ(Layout)へのポインタを返す
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	LayoutPoint ptPos;

	auto& docLineRef = *pLayout->GetDocLineRef();
	if (docLineRef.GetEol() == EolType::None) {	// 改行コードの種類
		GetDocument()->m_layoutMgr.LogicToLayout(
			LogicPoint(
				docLineRef.GetLengthWithEOL(),
				pLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}else {
		GetDocument()->m_layoutMgr.LogicToLayout(
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
	m_pCommanderView->DeleteData(true);
}


// 行切り取り(折り返し単位)
void ViewCommander::Command_CUT_LINE(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	if (selInfo.IsTextSelected()) {	// テキストが選択されているか
		ErrorBeep();
		return;
	}

	const Layout* pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().y);
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	// 2007.10.04 ryoji 処理簡素化
	m_pCommanderView->CopyCurLine(
		GetDllShareData().m_common.m_edit.m_bAddCRLFWhenCopy,
		EolType::Unknown,
		GetDllShareData().m_common.m_edit.m_bEnableLineModePaste
	);
	Command_DELETE_LINE();
	return;
}


// 行削除(折り返し単位)
void ViewCommander::Command_DELETE_LINE(void)
{
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	if (selInfo.IsMouseSelecting()) {	// マウスによる範囲選択中
		ErrorBeep();
		return;
	}

	if (selInfo.IsTextSelected()) {	// テキストが選択されているか
		ErrorBeep();
		return;
	}
	const Layout* pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());
	if (!pLayout) {
		ErrorBeep();
		return;
	}
	GetSelect().SetFrom(LayoutPoint(LayoutInt(0), GetCaret().GetCaretLayoutPos().GetY2()  ));	// 範囲選択開始位置
	GetSelect().SetTo  (LayoutPoint(LayoutInt(0), GetCaret().GetCaretLayoutPos().GetY2() + 1));	// 範囲選択終了位置

	LayoutPoint ptCaretPos_OLD = GetCaret().GetCaretLayoutPos();

	Command_DELETE();
	pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());
	if (pLayout) {
		// 2003-04-30 かろと
		// 行削除した後、フリーカーソルでないのにカーソル位置が行端より右になる不具合対応
		// フリーカーソルモードでない場合は、カーソル位置を調整する
		if (!GetDllShareData().m_common.m_general.m_bIsFreeCursorMode) {
			LogicInt nIndex;

			LayoutInt tmp;
			nIndex = m_pCommanderView->LineColumnToIndex2(pLayout, ptCaretPos_OLD.GetX2(), &tmp);
			ptCaretPos_OLD.x = tmp;

			if (ptCaretPos_OLD.x > 0) {
				ptCaretPos_OLD.x--;
			}else {
				ptCaretPos_OLD.x = m_pCommanderView->LineIndexToColumn(pLayout, nIndex);
			}
		}
		// 操作前の位置へカーソルを移動
		GetCaret().MoveCursor(ptCaretPos_OLD, true);
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if (!m_pCommanderView->m_bDoing_UndoRedo) {	// アンドゥ・リドゥの実行中か
			// 操作の追加
			GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
	}
	return;
}


// 行の二重化(折り返し単位)
void ViewCommander::Command_DUPLICATELINE(void)
{
	int			bCRLF;
	int			bAddCRLF;
	NativeW		memBuf;

	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	if (selInfo.IsTextSelected()) {	// テキストが選択されているか
		// 現在の選択範囲を非選択状態に戻す
		selInfo.DisableSelectArea(true);
	}

	const Layout* pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(GetCaret().GetCaretLayoutPos().GetY2());
	if (!pLayout) {
		ErrorBeep();
		return;
	}

	if (!m_pCommanderView->m_bDoing_UndoRedo) {	// アンドゥ・リドゥの実行中か
		// 操作の追加
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}

	LayoutPoint ptCaretPosOld = GetCaret().GetCaretLayoutPos() + LayoutPoint(0, 1);

	// 行頭に移動(折り返し単位)
	Command_GOLINETOP(selInfo.m_bSelectingLock, 0x1 /* カーソル位置に関係なく行頭に移動 */);

	if (!m_pCommanderView->m_bDoing_UndoRedo) {	// アンドゥ・リドゥの実行中か
		// 操作の追加
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
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
	bCRLF = (EolType::None == pLayout->GetLayoutEol()) ? FALSE : TRUE;

	bAddCRLF = FALSE;
	if (!bCRLF) {
		if (GetCaret().GetCaretLayoutPos().GetY2() == GetDocument()->m_layoutMgr.GetLineCount() - 1) {
			bAddCRLF = TRUE;
		}
	}

	memBuf.SetString(pLayout->GetPtr(), pLayout->GetLengthWithoutEOL() + pLayout->GetLayoutEol().GetLen());	// ※pLayout->GetLengthWithEOL()は、EOLの長さを必ず1にするので使えない。
	if (bAddCRLF) {
		// 現在、Enterなどで挿入する改行コードの種類を取得
		Eol cWork = GetDocument()->m_docEditor.GetNewLineCode();
		memBuf.AppendString(cWork.GetValue2(), cWork.GetLen());
	}

	// 現在位置にデータを挿入
	LayoutPoint ptLayoutNew;
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		memBuf.GetStringPtr(),
		memBuf.GetStringLength(),
		&ptLayoutNew,
		true
	);

	// カーソルを移動
	GetCaret().MoveCursor(ptCaretPosOld, true);
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	if (!m_pCommanderView->m_bDoing_UndoRedo) {	// アンドゥ・リドゥの実行中か
		// 操作の追加
		GetOpeBlk()->AppendOpe(
			new MoveCaretOpe(
				GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
			)
		);
	}
	return;
}

