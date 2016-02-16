/*!	@file
@brief ViewCommanderクラスのコマンド(モード切り替え系)関数群

	2012/12/15	CViewCommander.cpp,CViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2007, Moca

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"


/*! 挿入／上書きモード切り替え

	@date 2005.10.02 genta InsMode関数化
*/
void ViewCommander::Command_CHGMOD_INS(void)
{
	// 挿入モードか？
	m_pCommanderView->SetInsMode(!m_pCommanderView->IsInsMode());
	// キャレットの表示・更新
	GetCaret().ShowEditCaret();
	// キャレットの行桁位置を表示する
	GetCaret().ShowCaretPosInfo();
}


// from CViewCommander_New.cpp
/*! 入力する改行コードを設定

	@author moca
	@date 2003.06.23 新規作成
*/
void ViewCommander::Command_CHGMOD_EOL(EolType e)
{
	if (EolType::None < e && e < EolType::CodeMax) {
		GetDocument()->m_docEditor.SetNewLineCode(e);
		// ステータスバーを更新するため
		// キャレットの行桁位置を表示する関数を呼び出す
		GetCaret().ShowCaretPosInfo();
	}
}


// 文字コードセット指定
void ViewCommander::Command_CHG_CHARSET(
	ECodeType	eCharSet,	// [in] 設定する文字コードセット
	bool		bBom		// [in] 設定するBOM(Unicode系以外は無視)
	)
{
	if (eCharSet == CODE_NONE || eCharSet ==  CODE_AUTODETECT) {
		// 文字コードが指定されていないならば
		// 文字コードの確認
		eCharSet = GetDocument()->GetDocumentEncoding();	// 設定する文字コードセット
		bBom     = GetDocument()->GetDocumentBomExist();	// 設定するBOM
		int nRet = GetEditWindow()->m_dlgSetCharSet.DoModal(G_AppInstance(), m_pCommanderView->GetHwnd(), 
						&eCharSet, &bBom);
		if (!nRet) {
			return;
		}
	}

	// 文字コードの設定
	GetDocument()->m_docFile.SetCodeSetChg(eCharSet, CodeTypeName(eCharSet).UseBom() & bBom);

	// ステータス表示
	GetCaret().ShowCaretPosInfo();
}


/** 各種モードの取り消し
	@param whereCursorIs 選択をキャンセルした後、キャレットをどこに置くか。0=動かさない。1=左上。2=右下。
*/
void ViewCommander::Command_CANCEL_MODE(int whereCursorIs)
{
	bool bBoxSelect = false;
	auto& selInfo = m_pCommanderView->GetSelectionInfo();
	auto& caret = GetCaret();
	if (selInfo.IsTextSelected()) {
		// 選択解除後のカーソル位置を決める。
		LayoutPoint ptTo;
		LayoutRange rcMoveTo = GetSelect();
		if (selInfo.IsBoxSelecting()) { // 矩形選択ではキャレットが改行の後ろに取り残されないように、左上。
			bBoxSelect = true;
			// 2点を対角とする矩形を求める
			LayoutRange rcSel;
			TwoPointToRange(
				&rcSel,
				GetSelect().GetFrom(),	// 範囲選択開始
				GetSelect().GetTo()		// 範囲選択終了
			);
			// 2013.04.22 Moca 左上固定はやめる
			rcMoveTo = rcSel;
		}
		if (whereCursorIs == 1) { // 左上
			ptTo = rcMoveTo.GetFrom();
		}else if (whereCursorIs == 2) { // 右下
			ptTo = rcMoveTo.GetTo();
		}else {
			ptTo = caret.GetCaretLayoutPos();
		}

		// 現在の選択範囲を非選択状態に戻す
		selInfo.DisableSelectArea(true);

		// カーソルを移動
		auto& layoutMgr = GetDocument()->m_layoutMgr;
		if (ptTo.y >= layoutMgr.GetLineCount()) {
			// ファイルの最後に移動
			Command_GOFILEEND(false);
		}else {
			if (!GetDllShareData().m_common.m_general.m_bIsFreeCursorMode && bBoxSelect) {
				// 2013.04.22 Moca 矩形選択のとき左上固定をやめたので代わりにEOLより右だった場合にEOLに補正する
				const Layout*	pLayout = layoutMgr.SearchLineByLayoutY(ptTo.y);
				if (pLayout) {
					ptTo.x = t_min(ptTo.x, pLayout->CalcLayoutWidth(layoutMgr));
				}
			}

			caret.MoveCursor(ptTo, true);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		}
	}else {
		// 2011.12.05 Moca 選択中の未選択状態でもLockの解除と描画が必要
		if (selInfo.IsTextSelecting()
			|| selInfo.IsBoxSelecting()
		) {
			selInfo.DisableSelectArea(true);
			caret.m_cUnderLine.CaretUnderLineON(true, false);
			selInfo.PrintSelectionInfoMsg();
		}
	}
}

