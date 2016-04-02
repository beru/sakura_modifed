/*!	@file
@brief ViewCommanderクラスのコマンド(検索系 基本形)関数群

	2012/12/17	ViewCommander.cpp,ViewCommander_New.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, hor, YAZAKI
	Copyright (C) 2002, hor, YAZAKI, novice, Azumaiya, Moca
	Copyright (C) 2003, かろと
	Copyright (C) 2004, Moca
	Copyright (C) 2005, かろと, Moca, D.S.Koba
	Copyright (C) 2006, genta, ryoji, かろと, yukihane
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2009, ryoji, genta
	Copyright (C) 2010, ryoji
	Copyright (C) 2011, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "dlg/DlgCancel.h"// 2002/2/8 hor
#include "SearchAgent.h"
#include "util/window.h"
#include "util/string_ex2.h"
#include <limits.h>
#include "sakura_rc.h"

/*!
検索(ボックス)コマンド実行.
ツールバーの検索ボックスにフォーカスを移動する.
	@date 2006.06.04 yukihane 新規作成
*/
void ViewCommander::Command_SEARCH_BOX(void)
{
	GetEditWindow().m_toolbar.SetFocusSearchBox();
}


// 検索(単語検索ダイアログ)
void ViewCommander::Command_SEARCH_DIALOG(void)
{
	// 現在カーソル位置単語または選択範囲より検索等のキーを取得
	NativeW memCurText;
	m_view.GetCurrentTextForSearchDlg(memCurText);	// 2006.08.23 ryoji ダイアログ専用関数に変更

	auto& dlgFind = GetEditWindow().m_dlgFind;
	// 検索文字列を初期化
	if (0 < memCurText.GetStringLength()) {
		dlgFind.m_strText = memCurText.GetStringPtr();
	}
	// 検索ダイアログの表示
	if (!dlgFind.GetHwnd()) {
		dlgFind.DoModeless(G_AppInstance(), m_view.GetHwnd(), (LPARAM)&GetEditWindow().GetActiveView());
	}else {
		// アクティブにする
		ActivateFrameWindow(dlgFind.GetHwnd());
		dlgFind.SetItemText(IDC_COMBO_TEXT, memCurText.GetStringT());
	}
	return;
}


/*! 次を検索
	@param bChangeCurRegexp 共有データの検索文字列を使う
	@date 2003.05.22 かろと 無限マッチ対策．行頭・行末処理見直し．
	@date 2004.05.30 Moca bChangeCurRegexp=trueで従来通り。falseで、CEditViewの現在設定されている検索パターンを使う
*/
void ViewCommander::Command_SEARCH_NEXT(
	bool			bChangeCurRegexp,
	bool			bRedraw,
	bool			bReplaceAll,
	HWND			hwndParent,
	const WCHAR*	pszNotFoundMessage,
	LogicRange*		pSelectLogic		// [out] 選択範囲のロジック版。マッチ範囲を返す。すべて置換/高速モードで使用
	)
{
	bool		bSelecting;
	bool		bFlag1 = false;
	bool		bSelectingLock_Old = false;
	bool		bFound = false;
	bool		bDisableSelect = false;
	bool		b0Match = false;		// 長さ０でマッチしているか？フラグ by かろと
	LogicInt	nIdx(0);
	LayoutInt	nLineNum(0);

	LayoutRange	rangeA;
	rangeA.Set(GetCaret().GetCaretLayoutPos());

	LayoutRange	selectBgn_Old;
	LayoutRange	select_Old;
	LayoutInt	nLineNumOld(0);

	// bFastMode
	LogicInt nLineNumLogic(0);

	bool	bRedo = false;	// hor
	int		nIdxOld = 0;	// hor
	int		nSearchResult;
	auto& layoutMgr = GetDocument().m_layoutMgr;

	if (pSelectLogic) {
		pSelectLogic->Clear(-1);
	}

	bSelecting = false;
	// 2002.01.16 hor
	// 共通部分のくくりだし
	// 2004.05.30 Moca CEditViewの現在設定されている検索パターンを使えるように
	if (bChangeCurRegexp && !m_view.ChangeCurRegexp()) {
		return;
	}
	if (m_view.m_strCurSearchKey.size() == 0) {
		goto end_of_func;
	}

	auto& si = m_view.GetSelectionInfo();
	auto& caret = GetCaret();
	// 検索開始位置を調整
	bFlag1 = false;
	if (!pSelectLogic && si.IsTextSelected()) {	// テキストが選択されているか
		// 矩形範囲選択中でない & 選択状態のロック
		if (!si.IsBoxSelecting() && si.m_bSelectingLock) {
			bSelecting = true;
			bSelectingLock_Old = si.m_bSelectingLock;

			selectBgn_Old = si.m_selectBgn; // 範囲選択(原点)
			select_Old = GetSelect();

			if (PointCompare(si.m_selectBgn.GetFrom(), caret.GetCaretLayoutPos()) >= 0) {
				// カーソル移動
				caret.SetCaretLayoutPos(GetSelect().GetFrom());
				if (GetSelect().IsOne()) {
					// 現在、長さ０でマッチしている場合は１文字進める(無限マッチ対策) by かろと
					b0Match = true;
				}
				bFlag1 = true;
			}else {
				// カーソル移動
				caret.SetCaretLayoutPos(GetSelect().GetTo());
				if (GetSelect().IsOne()) {
					// 現在、長さ０でマッチしている場合は１文字進める(無限マッチ対策) by かろと
					b0Match = true;
				}
			}
		}else {
			// カーソル移動
			caret.SetCaretLayoutPos(GetSelect().GetTo());
			if (GetSelect().IsOne()) {
				// 現在、長さ０でマッチしている場合は１文字進める(無限マッチ対策) by かろと
				b0Match = true;
			}

			// 現在の選択範囲を非選択状態に戻す
			si.DisableSelectArea(bRedraw, false);
			bDisableSelect = true;
		}
	}
	if (!pSelectLogic) {
		nLineNum = caret.GetCaretLayoutPos().GetY2();
		LogicInt nLineLen = LogicInt(0); // 2004.03.17 Moca pLine == NULL のとき、nLineLenが未設定になり落ちるバグ対策
		const Layout*	pLayout;
		const wchar_t*	pLine = layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);

		// 指定された桁に対応する行のデータ内の位置を調べる
		// 2002.02.08 hor EOFのみの行からも次検索しても再検索可能に (2/2)
		nIdx = pLayout ? m_view.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().GetX2()) : LogicInt(0);
		if (b0Match) {
			// 現在、長さ０でマッチしている場合は物理行で１文字進める(無限マッチ対策)
			if (nIdx < nLineLen) {
				// 2005-09-02 D.S.Koba GetSizeOfChar
				nIdx += LogicInt(NativeW::GetSizeOfChar(pLine, nLineLen, nIdx) == 2 ? 2 : 1);
			}else {
				// 念のため行末は別処理
				++nIdx;
			}
		}
	}else {
		nLineNumLogic = caret.GetCaretLogicPos().GetY2();
		nIdx = caret.GetCaretLogicPos().GetX2();
	}

	nLineNumOld = nLineNum;	// hor
	bRedo		= true;		// hor
	nIdxOld		= nIdx;		// hor

re_do:;
	// 現在位置より後ろの位置を検索する
	// 2004.05.30 Moca 引数をGetShareData()からメンバ変数に変更。他のプロセス/スレッドに書き換えられてしまわないように。
	if (!pSelectLogic) {
		nSearchResult = layoutMgr.SearchWord(
			nLineNum,						// 検索開始レイアウト行
			nIdx,							// 検索開始データ位置
			SearchDirection::Forward,		// 0==前方検索 1==後方検索
			&rangeA,						// マッチレイアウト範囲
			m_view.m_searchPattern
		);
	}else {
		auto& docLineMgr = GetDocument().m_docLineMgr;
		nSearchResult = SearchAgent(docLineMgr).SearchWord(
			LogicPoint(nIdx, nLineNumLogic),
			SearchDirection::Forward,		// 0==前方検索 1==後方検索
			pSelectLogic,
			m_view.m_searchPattern
		);
	}
	if (nSearchResult) {
		// 指定された行のデータ内の位置に対応する桁の位置を調べる
		if (bFlag1 && rangeA.GetFrom() == caret.GetCaretLayoutPos()) {
			LogicRange logicRange;
			layoutMgr.LayoutToLogic(rangeA, &logicRange);

			nLineNum = rangeA.GetTo().GetY2();
			nIdx     = logicRange.GetTo().GetX2();
			if (logicRange.GetFrom() == logicRange.GetTo()) { // 幅0マッチでの無限ループ対策。
				nIdx += 1; // wchar_t一個分進めるだけでは足りないかもしれないが。
			}
			goto re_do;
		}

		if (bSelecting) {
			// 現在のカーソル位置によって選択範囲を変更
			si.ChangeSelectAreaByCurrentCursor(rangeA.GetTo());
			si.m_bSelectingLock = bSelectingLock_Old;	// 選択状態のロック
		}else if (!pSelectLogic) {
			// 選択範囲の変更
			// 2005.06.24 Moca
			si.SetSelectArea(rangeA);

			if (bRedraw) {
				// 選択領域描画
				si.DrawSelectArea();
			}
		}

		// カーソル移動
		// Sep. 8, 2000 genta
		if (!bReplaceAll) m_view.AddCurrentLineToHistory();	// 2002.02.16 hor すべて置換のときは不要
		if (!pSelectLogic) {
			caret.MoveCursor(rangeA.GetFrom(), bRedraw);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		}else {
			caret.MoveCursorFastMode(pSelectLogic->GetFrom());
		}
		bFound = true;
	}else {
		if (bSelecting) {
			si.m_bSelectingLock = bSelectingLock_Old;	// 選択状態のロック

			// 選択範囲の変更
			si.m_selectBgn = selectBgn_Old; // 範囲選択(原点)
			si.m_selectOld = select_Old;	// 2011.12.24
			GetSelect().SetFrom(select_Old.GetFrom());
			GetSelect().SetTo(rangeA.GetFrom());

			// カーソル移動
			caret.MoveCursor(rangeA.GetFrom(), bRedraw);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();

			if (bRedraw) {
				// 選択領域描画
				si.DrawSelectArea();
			}
		}else {
			if (bDisableSelect) {
				// 2011.12.21 ロジックカーソル位置の修正/カーソル線・対括弧の表示
				LogicPoint ptLogic;
				layoutMgr.LayoutToLogic(caret.GetCaretLayoutPos(), &ptLogic);
				caret.SetCaretLogicPos(ptLogic);
				m_view.DrawBracketCursorLine(bRedraw);
			}
		}
	}

end_of_func:;
// From Here 2002.01.26 hor 先頭（末尾）から再検索
	if (GetDllShareData().common.search.bSearchAll) {
		if (!bFound	&&		// 見つからなかった
			bRedo	&&		// 最初の検索
			!bReplaceAll	// 全て置換の実行中じゃない
		) {
			nLineNum	= LayoutInt(0);
			nIdx		= LogicInt(0);
			bRedo		= false;
			goto re_do;		// 先頭から再検索
		}
	}

	if (bFound) {
		if (!pSelectLogic && ((nLineNumOld > nLineNum)||(nLineNumOld == nLineNum && nIdxOld > nIdx)))
			m_view.SendStatusMessage(LS(STR_ERR_SRNEXT1));
	}else {
		caret.ShowEditCaret();	// 2002/04/18 YAZAKI
		caret.ShowCaretPosInfo();	// 2002/04/18 YAZAKI
		if (!bReplaceAll) {
			m_view.SendStatusMessage(LS(STR_ERR_SRNEXT2));
		}
// To Here 2002.01.26 hor

		// 検索／置換  見つからないときメッセージを表示
		if (!pszNotFoundMessage) {
			NativeW keyName;
			auto& curSearchKey = m_view.m_strCurSearchKey;
			LimitStringLengthW(curSearchKey.c_str(), curSearchKey.size(), _MAX_PATH, keyName);
			if ((size_t)keyName.GetStringLength() < curSearchKey.size()) {
				keyName.AppendString(L"...");
			}
			AlertNotFound(
				hwndParent,
				bReplaceAll,
				LS(STR_ERR_SRNEXT3),
				keyName.GetStringPtr()
			);
		}else {
			AlertNotFound(hwndParent, bReplaceAll, _T("%ls"), pszNotFoundMessage);
		}
	}
}


// 前を検索
void ViewCommander::Command_SEARCH_PREV(bool bReDraw, HWND hwndParent)
{
	bool		bSelectingLock_Old = false;
	bool		bFound = false;
	bool		bRedo = false;			// hor
	bool		bDisableSelect = false;
	LayoutInt	nLineNumOld(0);
	LogicInt	nIdxOld(0);
	LayoutInt	nLineNum(0);
	LogicInt	nIdx(0);

	auto& caret = GetCaret();
	auto& layoutMgr = GetDocument().m_layoutMgr;

	LayoutRange rangeA;
	rangeA.Set(caret.GetCaretLayoutPos());

	LayoutRange selectBgn_Old;
	LayoutRange select_Old;

	bool bSelecting = false;
	// 2002.01.16 hor
	// 共通部分のくくりだし
	if (!m_view.ChangeCurRegexp()) {
		return;
	}
	if (m_view.m_strCurSearchKey.size() == 0) {
		goto end_of_func;
	}
	auto& si = m_view.GetSelectionInfo();
	if (si.IsTextSelected()) {	// テキストが選択されているか
		selectBgn_Old = si.m_selectBgn; // 範囲選択(原点)
		select_Old = GetSelect();
		
		bSelectingLock_Old = si.m_bSelectingLock;

		// 矩形範囲選択中か
		if (!si.IsBoxSelecting() && si.m_bSelectingLock) {	// 選択状態のロック
			bSelecting = true;
		}else {
			// 現在の選択範囲を非選択状態に戻す
			si.DisableSelectArea(bReDraw, false);
			bDisableSelect = true;
		}
	}

	nLineNum = caret.GetCaretLayoutPos().GetY2();
	const Layout* pLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
	if (!pLayout) {
		// pLayoutはNULLとなるのは、[EOF]から前検索した場合
		// １行前に移動する処理
		--nLineNum;
		if (nLineNum < 0) {
			goto end_of_func;
		}
		pLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
		if (!pLayout) {
			goto end_of_func;
		}
		// カーソル左移動はやめて nIdxは行の長さとしないと[EOF]から改行を前検索した時に最後の改行を検索できない 2003.05.04 かろと
		Layout* pLayout = layoutMgr.SearchLineByLayoutY(nLineNum);
		nIdx = LogicInt(pLayout->GetDocLineRef()->GetLengthWithEOL() + 1);		// 行末のヌル文字(\0)にマッチさせるために+1 2003.05.16 かろと
	}else {
		// 指定された桁に対応する行のデータ内の位置を調べる
		nIdx = m_view.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().GetX2());
	}

	bRedo		=	true;		// hor
	nLineNumOld	=	nLineNum;	// hor
	nIdxOld		=	nIdx;		// hor
re_do:;							// hor
	// 現在位置より前の位置を検索する
	if (layoutMgr.SearchWord(
			nLineNum,								// 検索開始レイアウト行
			nIdx,									// 検索開始データ位置
			SearchDirection::Backward,				// 0==前方検索 1==後方検索
			&rangeA,								// マッチレイアウト範囲
			m_view.m_searchPattern
		)
	) {
		if (bSelecting) {
			// 現在のカーソル位置によって選択範囲を変更
			si.ChangeSelectAreaByCurrentCursor(rangeA.GetFrom());
			si.m_bSelectingLock = bSelectingLock_Old;	// 選択状態のロック
		}else {
			// 選択範囲の変更
			// 2005.06.24 Moca
			si.SetSelectArea(rangeA);

			if (bReDraw) {
				// 選択領域描画
				si.DrawSelectArea();
			}
		}
		// カーソル移動
		// Sep. 8, 2000 genta
		m_view.AddCurrentLineToHistory();
		caret.MoveCursor(rangeA.GetFrom(), bReDraw);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		bFound = true;
	}else {
		if (bSelecting) {
			si.m_bSelectingLock = bSelectingLock_Old;	// 選択状態のロック
			// 選択範囲の変更
			si.m_selectBgn = selectBgn_Old;
			GetSelect() = select_Old;

			// カーソル移動
			caret.MoveCursor(rangeA.GetFrom(), bReDraw);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
			// 選択領域描画
			si.DrawSelectArea();
		}else {
			if (bDisableSelect) {
				m_view.DrawBracketCursorLine(bReDraw);
			}
		}
	}
end_of_func:;
// From Here 2002.01.26 hor 先頭（末尾）から再検索
	if (GetDllShareData().common.search.bSearchAll) {
		if (!bFound	&&	// 見つからなかった
			bRedo		// 最初の検索
		) {
			nLineNum	= layoutMgr.GetLineCount() - LayoutInt(1);
			nIdx		= LogicInt(MAXLINEKETAS);
			bRedo		= false;
			goto re_do;	// 末尾から再検索
		}
	}
	if (bFound) {
		if ((nLineNumOld < nLineNum)||(nLineNumOld == nLineNum && nIdxOld < nIdx))
			m_view.SendStatusMessage(LS(STR_ERR_SRPREV1));
	}else {
		m_view.SendStatusMessage(LS(STR_ERR_SRPREV2));
// To Here 2002.01.26 hor

		// 検索／置換  見つからないときメッセージを表示
		NativeW keyName;
		auto& curSearchKey = m_view.m_strCurSearchKey;
		LimitStringLengthW(curSearchKey.c_str(), curSearchKey.size(), _MAX_PATH, keyName);
		if ((size_t)keyName.GetStringLength() < curSearchKey.size()) {
			keyName.AppendString(L"...");
		}
		AlertNotFound(
			hwndParent,
			false,
			LS(STR_ERR_SRPREV3),	// Jan. 25, 2001 jepro メッセージを若干変更
			keyName.GetStringPtr()
		);
	}
	return;
}


// 置換(置換ダイアログ)
void ViewCommander::Command_REPLACE_DIALOG(void)
{
	bool bSelected = false;

	// 現在カーソル位置単語または選択範囲より検索等のキーを取得
	NativeW memCurText;
	m_view.GetCurrentTextForSearchDlg(memCurText);	// 2006.08.23 ryoji ダイアログ専用関数に変更

	auto& dlgReplace = GetEditWindow().m_dlgReplace;

	// 検索文字列を初期化
	if (0 < memCurText.GetStringLength()) {
		dlgReplace.m_strText = memCurText.GetStringPtr();
	}
	if (0 < GetDllShareData().searchKeywords.replaceKeys.size()) {
		if (dlgReplace.nReplaceKeySequence < GetDllShareData().common.search.nReplaceKeySequence) {
			dlgReplace.m_strText2 = GetDllShareData().searchKeywords.replaceKeys[0];	// 2006.08.23 ryoji 前回の置換後文字列を引き継ぐ
		}
	}
	
	if (m_view.GetSelectionInfo().IsTextSelected() && !GetSelect().IsLineOne()) {
		bSelected = true;	// 選択範囲をチェックしてダイアログ表示
	}else {
		bSelected = false;	// ファイル全体をチェックしてダイアログ表示
	}
	// 置換オプションの初期化
	dlgReplace.m_nReplaceTarget = 0;	// 置換対象
	dlgReplace.m_bPaste = false;		// 貼り付ける？
// To Here 2001.12.03 hor

	// 置換ダイアログの表示
	// From Here Jul. 2, 2001 genta 置換ウィンドウの2重開きを抑止
	if (!::IsWindow(dlgReplace.GetHwnd())) {
		dlgReplace.DoModeless(G_AppInstance(), m_view.GetHwnd(), (LPARAM)&m_view, bSelected);
	}else {
		// アクティブにする
		ActivateFrameWindow(dlgReplace.GetHwnd());
		dlgReplace.SetItemText(IDC_COMBO_TEXT, memCurText.GetStringT());
	}
	// To Here Jul. 2, 2001 genta 置換ウィンドウの2重開きを抑止
	return;
}


/*! 置換実行
	
	@date 2002/04/08 親ウィンドウを指定するように変更。
	@date 2003.05.17 かろと 長さ０マッチの無限置換回避など
	@date 2011.12.18 Moca オプション・検索キーをDllShareDataからm_dlgReplace/EditViewベースに変更。文字列長制限の撤廃
*/
void ViewCommander::Command_REPLACE(HWND hwndParent)
{
	if (!hwndParent) {	// 親ウィンドウが指定されていなければ、CEditViewが親。
		hwndParent = m_view.GetHwnd();
	}
	auto& dlgReplace = GetEditWindow().m_dlgReplace;
	// 2002.02.10 hor
	int nPaste			=	dlgReplace.m_bPaste;
	int nReplaceTarget	=	dlgReplace.m_nReplaceTarget;

	if (nPaste && nReplaceTarget == 3) {
		// 置換対象：行削除のときは、クリップボードから貼り付けを無効にする
		nPaste = FALSE;
	}

	// From Here 2001.12.03 hor
	if (nPaste && !GetDocument().m_docEditor.IsEnablePaste()) {
		OkMessage(hwndParent, LS(STR_ERR_CEDITVIEW_CMD10));
		dlgReplace.CheckButton(IDC_CHK_PASTE, false);
		dlgReplace.EnableItem(IDC_COMBO_TEXT2, true);
		return;	// 失敗return;
	}

	// 2002.01.09 hor
	// 選択エリアがあれば、その先頭にカーソルを移す
	if (m_view.GetSelectionInfo().IsTextSelected()) {
		if (m_view.GetSelectionInfo().IsBoxSelecting()) {
			GetCaret().MoveCursor(GetSelect().GetFrom(), true);
		}else {
			Command_LEFT(false, false);
		}
	}
	// To Here 2002.01.09 hor
	
	// 矩形選択？
//			bBeginBoxSelect = m_view.GetSelectionInfo().IsBoxSelecting();

	// カーソル左移動
	//HandleCommand(F_LEFT, true, 0, 0, 0, 0);	//？？？
	// To Here 2001.12.03 hor

	// テキスト選択解除
	// 現在の選択範囲を非選択状態に戻す
	m_view.GetSelectionInfo().DisableSelectArea(true);

	// 2004.06.01 Moca 検索中に、他のプロセスによってreplaceKeysが書き換えられても大丈夫なように
	const NativeW memRepKey(dlgReplace.m_strText2.c_str());

	// 次を検索
	Command_SEARCH_NEXT(true, true, false, hwndParent, nullptr);

	bool	bRegularExp = m_view.m_curSearchOption.bRegularExp;
	int 	nFlag       = m_view.m_curSearchOption.bLoHiCase ? 0x01 : 0x00;
	auto& caret = GetCaret();

	// テキストが選択されているか
	if (m_view.GetSelectionInfo().IsTextSelected()) {
		// From Here 2001.12.03 hor
		LayoutPoint ptTmp(0, 0);
		if (nPaste || !bRegularExp) {
			// 正規表現時は 後方参照($&)で実現するので、正規表現は除外
			if (nReplaceTarget == 1) {	// 挿入位置へ移動
				ptTmp = GetSelect().GetTo() - GetSelect().GetFrom();
				GetSelect().Clear(-1);
			}else if (nReplaceTarget == 2) {	// 追加位置へ移動
				// 正規表現を除外したので、「検索後の文字が改行やったら次の行の先頭へ移動」の処理を削除
				caret.MoveCursor(GetSelect().GetTo(), false);
				GetSelect().Clear(-1);
			}else {
				// 位置指定ないので、何もしない
			}
		}
		auto& layoutMgr = GetDocument().m_layoutMgr;
		// 行削除 選択範囲を行全体に拡大。カーソル位置を行頭へ(正規表現でも実行)
		if (nReplaceTarget == 3) {
			LogicPoint lineHome;
			layoutMgr.LayoutToLogic(GetSelect().GetFrom(), &lineHome);
			lineHome.x = LogicXInt(0); // 行頭
			LayoutRange selectFix;
			layoutMgr.LogicToLayout(lineHome, selectFix.GetFromPointer());
			lineHome.y++; // 次行の行頭
			layoutMgr.LogicToLayout(lineHome, selectFix.GetToPointer());
			caret.GetAdjustCursorPos(selectFix.GetToPointer());
			m_view.GetSelectionInfo().SetSelectArea(selectFix);
			m_view.GetSelectionInfo().DrawSelectArea();
			caret.MoveCursor(selectFix.GetFrom(), false);
			caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();
		}
		// コマンドコードによる処理振り分け
		// テキストを貼り付け
		if (nPaste) {
			Command_PASTE(0);
		} else if (nReplaceTarget == 3) {
			// 行削除
			Command_INSTEXT( false, L"", LogicInt(0), true );
		}else if (bRegularExp) { // 検索／置換  1==正規表現
			// 先読みに対応するために物理行末までを使うように変更 2005/03/27 かろと
			// 2002/01/19 novice 正規表現による文字列置換
			Bregexp regexp;

			if (!InitRegexp(m_view.GetHwnd(), regexp, true)) {
				return;	// 失敗return;
			}

			// 物理行、物理行長、物理行での検索マッチ位置
			const Layout* pLayout = layoutMgr.SearchLineByLayoutY(GetSelect().GetFrom().GetY2());
			const wchar_t* pLine = pLayout->GetDocLineRef()->GetPtr();
			LogicInt nIdx = m_view.LineColumnToIndex(pLayout, GetSelect().GetFrom().GetX2()) + pLayout->GetLogicOffset();
			LogicInt nLen = pLayout->GetDocLineRef()->GetLengthWithEOL();
			// 正規表現で選択始点・終点への挿入を記述
			// Jun. 6, 2005 かろと
			// →これでは「検索の後ろの文字が改行だったら次の行頭へ移動」が処理できない
			// → Oct. 30, 「検索の後ろの文字が改行だったら・・」の処理をやめる（誰もしらないみたいなので）
			// Nov. 9, 2005 かろと 正規表現で選択始点・終点への挿入方法を変更(再)
			NativeW memMatchStr; memMatchStr.SetString(L"$&");
			NativeW memRepKey2;
			if (nReplaceTarget == 1) {	// 選択始点へ挿入
				memRepKey2 = memRepKey;
				memRepKey2 += memMatchStr;
			}else if (nReplaceTarget == 2) { // 選択終点へ挿入
				memRepKey2 = memMatchStr;
				memRepKey2 += memRepKey;
			}else {
				memRepKey2 = memRepKey;
			}
			regexp.Compile(m_view.m_strCurSearchKey.c_str(), memRepKey2.GetStringPtr(), nFlag);
			if (regexp.Replace(pLine, nLen, nIdx)) {
				// From Here Jun. 6, 2005 かろと
				// 物理行末までINSTEXTする方法は、キャレット位置を調整する必要があり、
				// キャレット位置の計算が複雑になる。（置換後に改行がある場合に不具合発生）
				// そこで、INSTEXTする文字列長を調整する方法に変更する（実はこっちの方がわかりやすい）
				LogicInt matchLen = regexp.GetMatchLen();
				LogicInt nIdxTo = nIdx + matchLen;		// 検索文字列の末尾
				if (matchLen == 0) {
					// ０文字マッチの時(無限置換にならないように１文字進める)
					if (nIdxTo < nLen) {
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nIdxTo += LogicInt(NativeW::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1);
					}
					// 無限置換しないように、１文字増やしたので１文字選択に変更
					// 選択始点・終点への挿入の場合も０文字マッチ時は動作は同じになるので
					layoutMgr.LogicToLayout(LogicPoint(nIdxTo, pLayout->GetLogicLineNo()), GetSelect().GetToPointer());	// 2007.01.19 ryoji 行位置も取得する
				}
				// 行末から検索文字列末尾までの文字数
				LogicInt colDiff = nLen - nIdxTo;
				// Oct. 22, 2005 Karoto
				// \rを置換するとその後ろの\nが消えてしまう問題の対応
				if (colDiff < pLayout->GetDocLineRef()->GetEol().GetLen()) {
					// 改行にかかっていたら、行全体をINSTEXTする。
					colDiff = LogicInt(0);
					layoutMgr.LogicToLayout(LogicPoint(nLen, pLayout->GetLogicLineNo()), GetSelect().GetToPointer());	// 2007.01.19 ryoji 追加
				}
				// 置換後文字列への書き換え(行末から検索文字列末尾までの文字を除く)
				Command_INSTEXT(false, regexp.GetString(), regexp.GetStringLen() - colDiff, true);
				// To Here Jun. 6, 2005 かろと
			}
		}else {
			Command_INSTEXT(false, memRepKey.GetStringPtr(), memRepKey.GetStringLength(), true);
		}

		// 挿入後の検索開始位置を調整
		if (nReplaceTarget == 1) {
			caret.SetCaretLayoutPos(caret.GetCaretLayoutPos() + ptTmp);
		}

		// To Here 2001.12.03 hor
		/* 最後まで置換した時にOK押すまで置換前の状態が表示されるので、
		** 置換後、次を検索する前に書き直す 2003.05.17 かろと
		*/
		m_view.Redraw();

		// 次を検索
		Command_SEARCH_NEXT(true, true, false, hwndParent, LSW(STR_ERR_CEDITVIEW_CMD11));
	}
}


/*! すべて置換実行

	@date 2003.05.22 かろと 無限マッチ対策．行頭・行末処理など見直し
	@date 2006.03.31 かろと 行置換機能追加
	@date 2007.01.16 ryoji 行置換機能を全置換のオプションに変更
	@date 2009.09.20 genta 左下～右上で矩形選択された領域の置換が行われない
	@date 2010.09.17 ryoji ラインモード貼り付け処理を追加
	@date 2011.12.18 Moca オプション・検索キーをDllShareDataからm_dlgReplace/EditViewベースに変更。文字列長制限の撤廃
	@date 2013.05.10 Moca fastMode
*/
void ViewCommander::Command_REPLACE_ALL()
{
	// m_sSearchOption選択のための先に適用
	if (!m_view.ChangeCurRegexp()) {
		return;
	}

	auto& dlgReplace = GetEditWindow().m_dlgReplace;
	// 2002.02.10 hor
	bool bPaste			= dlgReplace.m_bPaste;
	BOOL nReplaceTarget	= dlgReplace.m_nReplaceTarget;
	bool bRegularExp	= m_view.m_curSearchOption.bRegularExp;
	bool bSelectedArea	= dlgReplace.bSelectedArea;
	bool bConsecutiveAll = dlgReplace.bConsecutiveAll;	// 「すべて置換」は置換の繰返し	// 2007.01.16 ryoji
	if (bPaste && nReplaceTarget == 3) {
		// 置換対象：行削除のときは、クリップボードから貼り付けを無効にする
		bPaste = false;
	}

	dlgReplace.m_bCanceled = false;
	dlgReplace.m_nReplaceCnt = 0;

	// From Here 2001.12.03 hor
	if (bPaste && !GetDocument().m_docEditor.IsEnablePaste()) {
		OkMessage(m_view.GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD10));
		dlgReplace.CheckButton(IDC_CHK_PASTE, false);
		dlgReplace.EnableItem(IDC_COMBO_TEXT2, true);
		return;	// TRUE;
	}
	// To Here 2001.12.03 hor
	bool bBeginBoxSelect; // 矩形選択？
	if (m_view.GetSelectionInfo().IsTextSelected()) {
		bBeginBoxSelect = m_view.GetSelectionInfo().IsBoxSelecting();
	}else {
		bSelectedArea = false;
		bBeginBoxSelect = false;
	}

	// 表示処理ON/OFF
	bool bDisplayUpdate = false;
	const bool bDrawSwitchOld = m_view.SetDrawSwitch(bDisplayUpdate);

	auto& layoutMgr = GetDocument().m_layoutMgr;
	auto& docLineMgr = GetDocument().m_docLineMgr;
	bool bFastMode = false;
	if (((Int)docLineMgr.GetLineCount() * 10 < (Int)layoutMgr.GetLineCount())
		&& !(bSelectedArea || bPaste)
	) {
		// 1行あたり10レイアウト行以上で、選択・ペーストでない場合
		bFastMode = true;
	}
	int	nAllLineNum; // $$単位混在
	if (bFastMode) {
		nAllLineNum = (Int)layoutMgr.GetLineCount();
	}else {
		nAllLineNum = (Int)docLineMgr.GetLineCount();
	}
	int	nAllLineNumOrg = nAllLineNum;
	int	nAllLineNumLogicOrg = (Int)docLineMgr.GetLineCount();

	// 進捗表示&中止ダイアログの作成
	DlgCancel	dlgCancel;
	HWND		hwndCancel = dlgCancel.DoModeless(G_AppInstance(), m_view.GetHwnd(), IDD_REPLACERUNNING);
	::EnableWindow(m_view.GetHwnd(), FALSE);
	::EnableWindow(::GetParent(m_view.GetHwnd()), FALSE);
	::EnableWindow(::GetParent(::GetParent(m_view.GetHwnd())), FALSE);
	//<< 2002/03/26 Azumaiya
	// 割り算掛け算をせずに進歩状況を表せるように、シフト演算をする。
	int nShiftCount;
	for (nShiftCount=0; 300<nAllLineNum; ++nShiftCount) {
		nAllLineNum /= 2;
	}
	//>> 2002/03/26 Azumaiya

	// プログレスバー初期化
	HWND		hwndProgress = ::GetDlgItem(hwndCancel, IDC_PROGRESS_REPLACE);
	Progress_SetRange(hwndProgress, 0, nAllLineNum + 1);
	int			nNewPos = 0;
	int			nOldPos = -1;
	Progress_SetPos(hwndProgress, nNewPos);

	// 置換個数初期化
	int			nReplaceNum = 0;
	HWND		hwndStatic = ::GetDlgItem(hwndCancel, IDC_STATIC_KENSUU);
	TCHAR szLabel[64];
	_itot(nReplaceNum, szLabel, 10);
	::SendMessage(hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel);

	LayoutRange rangeA;	// 選択範囲
	LogicPoint ptColLineP;

	// From Here 2001.12.03 hor
	auto& caret = GetCaret();
	if (bSelectedArea) {
		// 選択範囲置換
		// 選択範囲開始位置の取得
		rangeA = GetSelect();

		// From Here 2007.09.20 genta 矩形範囲の選択置換ができない
		// 左下～右上と選択した場合，m_nSelectColumnTo < m_nSelectColumnFrom となるが，
		// 範囲チェックで colFrom < colTo を仮定しているので，
		// 矩形選択の場合は左上～右下指定になるよう桁を入れ換える．
		if (bBeginBoxSelect && rangeA.GetTo().x < rangeA.GetFrom().x)
			t_swap(rangeA.GetFromPointer()->x, rangeA.GetToPointer()->x);
		// To Here 2007.09.20 genta 矩形範囲の選択置換ができない

		layoutMgr.LayoutToLogic(
			rangeA.GetTo(),
			&ptColLineP
		);
		// 選択範囲開始位置へ移動
		caret.MoveCursor(rangeA.GetFrom(), bDisplayUpdate);
	}else {
		// ファイル全体置換
		// ファイルの先頭に移動
	//	HandleCommand(F_GOFILETOP, bDisplayUpdate, 0, 0, 0, 0);
		Command_GOFILETOP(bDisplayUpdate);
	}

	LayoutPoint ptLast = caret.GetCaretLayoutPos();
	LogicPoint ptLastLogic = caret.GetCaretLogicPos();

	// テキスト選択解除
	// 現在の選択範囲を非選択状態に戻す
	m_view.GetSelectionInfo().DisableSelectArea(bDisplayUpdate);

	LogicRange selectLogic;	// 置換文字列GetSelect()のLogic単位版
	// 次を検索
	Command_SEARCH_NEXT(true, bDisplayUpdate, true, 0, NULL, bFastMode ? &selectLogic : nullptr);
	// To Here 2001.12.03 hor

	//<< 2002/03/26 Azumaiya
	// 速く動かすことを最優先に組んでみました。
	// ループの外で文字列の長さを特定できるので、一時変数化。
	const wchar_t* szREPLACEKEY;		// 置換後文字列。
	bool		bColumnSelect = false;	// 矩形貼り付けを行うかどうか。
	bool		bLineSelect = false;	// ラインモード貼り付けを行うかどうか
	NativeW	memClip;				// 置換後文字列のデータ（データを格納するだけで、ループ内ではこの形ではデータを扱いません）。

	// クリップボードからのデータ貼り付けかどうか。
	if (bPaste) {
		// クリップボードからデータを取得。
		if (!m_view.MyGetClipboardData(memClip, &bColumnSelect, GetDllShareData().common.edit.bEnableLineModePaste ? &bLineSelect : nullptr)) {
			ErrorBeep();
			m_view.SetDrawSwitch(bDrawSwitchOld);

			::EnableWindow(m_view.GetHwnd(), TRUE);
			::EnableWindow(::GetParent(m_view.GetHwnd()), TRUE);
			::EnableWindow(::GetParent(::GetParent(m_view.GetHwnd())), TRUE);
			return;
		}

		// 矩形貼り付けが許可されていて、クリップボードのデータが矩形選択のとき。
		if (GetDllShareData().common.edit.bAutoColumnPaste && bColumnSelect) {
			// マウスによる範囲選択中
			if (m_view.GetSelectionInfo().IsMouseSelecting()) {
				ErrorBeep();
				m_view.SetDrawSwitch(bDrawSwitchOld);
				::EnableWindow(m_view.GetHwnd(), TRUE);
				::EnableWindow(::GetParent(m_view.GetHwnd()), TRUE);
				::EnableWindow(::GetParent(::GetParent(m_view.GetHwnd())), TRUE);
				return;
			}

			// 現在のフォントは固定幅フォントである
			if (!GetDllShareData().common.view.bFontIs_FixedPitch) {
				m_view.SetDrawSwitch(bDrawSwitchOld);
				::EnableWindow(m_view.GetHwnd(), TRUE);
				::EnableWindow(::GetParent(m_view.GetHwnd()), TRUE);
				::EnableWindow(::GetParent(::GetParent(m_view.GetHwnd())), TRUE);
				return;
			}
		}else { // クリップボードからのデータは普通に扱う。
			bColumnSelect = false;
		}
	}else {
		// 2004.05.14 Moca 全置換の途中で他のウィンドウで置換されるとまずいのでコピーする
		memClip.SetString(dlgReplace.m_strText2.c_str());
	}

	LogicInt nReplaceKey;			// 置換後文字列の長さ。
	szREPLACEKEY = memClip.GetStringPtr(&nReplaceKey);

	// 行コピー（MSDEVLineSelect形式）のテキストで末尾が改行になっていなければ改行を追加する
	// ※レイアウト折り返しの行コピーだった場合は末尾が改行になっていない
	if (bLineSelect) {
		if (!WCODE::IsLineDelimiter(szREPLACEKEY[nReplaceKey - 1], GetDllShareData().common.edit.bEnableExtEol)) {
			memClip.AppendString(GetDocument().m_docEditor.GetNewLineCode().GetValue2());
			szREPLACEKEY = memClip.GetStringPtr(&nReplaceKey);
		}
	}

	if (GetDllShareData().common.edit.bConvertEOLPaste) {
		LogicInt nConvertedTextLen = ConvertEol(szREPLACEKEY, nReplaceKey, NULL);
		std::vector<wchar_t> szConvertedText(nConvertedTextLen);
		wchar_t* pszConvertedText = &szConvertedText[0];
		ConvertEol(szREPLACEKEY, nReplaceKey, pszConvertedText);
		memClip.SetString(pszConvertedText, nConvertedTextLen);
		szREPLACEKEY = memClip.GetStringPtr(&nReplaceKey);
	}

	// 取得にステップがかかりそうな変数などを、一時変数化する。
	// とはいえ、これらの操作をすることによって得をするクロック数は合わせても 1 ループで数十だと思います。
	// 数百クロック毎ループのオーダーから考えてもそんなに得はしないように思いますけど・・・。
	bool& bCancel = dlgCancel.m_bCANCEL;

	// クラス関係をループの中で宣言してしまうと、毎ループごとにコンストラクタ、デストラクタが
	// 呼ばれて遅くなるので、ここで宣言。
	Bregexp regexp;
	// 初期化も同様に毎ループごとにやると遅いので、最初に済ましてしまう。
	if (bRegularExp && !bPaste) {
		if (!InitRegexp(m_view.GetHwnd(), regexp, true)) {
			m_view.SetDrawSwitch(bDrawSwitchOld);
			::EnableWindow(m_view.GetHwnd(), TRUE);
			::EnableWindow(::GetParent(m_view.GetHwnd()), TRUE);
			::EnableWindow(::GetParent(::GetParent(m_view.GetHwnd())), TRUE);
			return;
		}

		// Nov. 9, 2005 かろと 正規表現で選択始点・終点への挿入方法を変更(再)
		NativeW memRepKey2;
		NativeW memMatchStr;
		memMatchStr.SetString(L"$&");
		if (nReplaceTarget == 1) {	// 選択始点へ挿入
			memRepKey2 = memClip;
			memRepKey2 += memMatchStr;
		}else if (nReplaceTarget == 2) { // 選択終点へ挿入
			memRepKey2 = memMatchStr;
			memRepKey2 += memClip;
		}else {
			memRepKey2 = memClip;
		}
		// 正規表現オプションの設定2006.04.01 かろと
		int nFlag = (m_view.m_curSearchOption.bLoHiCase ? Bregexp::optCaseSensitive : Bregexp::optNothing);
		nFlag |= (bConsecutiveAll ? Bregexp::optNothing : Bregexp::optGlobal);	// 2007.01.16 ryoji
		regexp.Compile(m_view.m_strCurSearchKey.c_str(), memRepKey2.GetStringPtr(), nFlag);
	}

	//$$ 単位混在
	LayoutPoint	ptOld;						// 検索後の選択範囲
	/*LogicInt*/int		lineCnt = 0;		// 置換前の行数
	/*LayoutInt*/int	linDif = (0);		// 置換後の行調整
	/*LayoutInt*/int	colDif = (0);		// 置換後の桁調整
	/*LayoutInt*/int	linPrev = (0);		// 前回の検索行(矩形) @@@2001.12.31 YAZAKI warning退治
	/*LogicInt*/int		linOldLen = (0);	// 検査後の行の長さ
	/*LayoutInt*/int	linNext;			// 次回の検索行(矩形)

	int nLoopCnt = -1;

	// テキストが選択されているか
	while (
		(!bFastMode && m_view.GetSelectionInfo().IsTextSelected())
		|| (bFastMode && selectLogic.IsValid())
	) {
		// キャンセルされたか
		if (bCancel) {
			break;
		}

		// 処理中のユーザー操作を可能にする
		if (!::BlockingHook(hwndCancel)) {
			m_view.SetDrawSwitch(bDrawSwitchOld);
			::EnableWindow(m_view.GetHwnd(), TRUE);
			::EnableWindow(::GetParent(m_view.GetHwnd()), TRUE);
			::EnableWindow(::GetParent(::GetParent(m_view.GetHwnd())), TRUE);
			return;// -1;
		}

		++nLoopCnt;
		// 128 ごとに表示。
		if ((nLoopCnt & 0x7F) == 0) {
			// 時間ごとに進歩状況描画だと時間取得分遅くなると思うが、そちらの方が自然だと思うので・・・。
			// と思ったけど、逆にこちらの方が自然ではないので、やめる。
		
			if (bFastMode) {
				int nDiff = nAllLineNumOrg - (Int)docLineMgr.GetLineCount();
				if (0 <= nDiff) {
					nNewPos = (nDiff + (Int)selectLogic.GetFrom().GetY2()) >> nShiftCount;
				}else {
					nNewPos = ::MulDiv((Int)selectLogic.GetFrom().GetY(), nAllLineNum, (Int)layoutMgr.GetLineCount());
				}
			}else {
				int nDiff = nAllLineNumOrg - (Int)layoutMgr.GetLineCount();
				if (0 <= nDiff) {
					nNewPos = (nDiff + (Int)GetSelect().GetFrom().GetY2()) >> nShiftCount;
				}else {
					nNewPos = ::MulDiv((Int)GetSelect().GetFrom().GetY(), nAllLineNum, (Int)layoutMgr.GetLineCount());
				}
			}
			if (nOldPos != nNewPos) {
				Progress_SetPos(hwndProgress, nNewPos +1);
				Progress_SetPos(hwndProgress, nNewPos);
				nOldPos = nNewPos;
			}
			_itot(nReplaceNum, szLabel, 10);
			::SendMessage(hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel);
		}

		// From Here 2001.12.03 hor
		// 検索後の位置を確認
		if (bSelectedArea) {
			// 矩形選択
			// o レイアウト座標をチェックしながら置換する
			// o 折り返しがあると変になるかも・・・
			//
			if (bBeginBoxSelect) {
				// 検索時の行数を記憶
				lineCnt = (Int)layoutMgr.GetLineCount();
				// 検索後の範囲終端
				ptOld = GetSelect().GetTo();
				// 前回の検索行と違う？
				if (ptOld.y != linPrev) {
					colDif = (0);
				}
				linPrev = (Int)ptOld.GetY2();
				// 行は範囲内？
				if (
					(rangeA.GetTo().y + linDif == ptOld.y && rangeA.GetTo().GetX2() + colDif < ptOld.x)
					|| (rangeA.GetTo().y + linDif <  ptOld.y)
				) {
					break;
				}
				// 桁は範囲内？
				if (!(rangeA.GetFrom().x <= GetSelect().GetFrom().x && ptOld.GetX2() <= rangeA.GetTo().GetX2() + colDif)) {
					if (ptOld.x < rangeA.GetTo().GetX2() + colDif) {
						linNext = (Int)GetSelect().GetTo().GetY2();
					}else {
						linNext = (Int)GetSelect().GetTo().GetY2() + 1;
					}
					// 次の検索開始位置へシフト
					caret.SetCaretLayoutPos(LayoutPoint(rangeA.GetFrom().x, LayoutInt(linNext)));
					// 2004.05.30 Moca 現在の検索文字列を使って検索する
					Command_SEARCH_NEXT(false, bDisplayUpdate, true, 0, nullptr);
					colDif = (0);
					continue;
				}
			}else {
				// 普通の選択
				// o 物理座標をチェックしながら置換する
				//
			
				// 検索時の行数を記憶
				lineCnt = docLineMgr.GetLineCount();

				// 検索後の範囲終端
				LogicPoint ptOldTmp;
				if (bFastMode) {
					ptOldTmp = selectLogic.GetTo();
				}else {
					layoutMgr.LayoutToLogic(
						GetSelect().GetTo(),
						&ptOldTmp
					);
				}
				ptOld.x = (LayoutInt)ptOldTmp.x; //$$ レイアウト型に無理やりロジック型を代入。気持ち悪い
				ptOld.y = (LayoutInt)ptOldTmp.y;

				// 置換前の行の長さ(改行は１文字と数える)を保存しておいて、置換前後で行位置が変わった場合に使用
				linOldLen = docLineMgr.GetLine(ptOldTmp.GetY2())->GetLengthWithoutEOL() + LogicInt(1);

				// 行は範囲内？
				// 2007.01.19 ryoji 条件追加: 選択終点が行頭(ptColLineP.x == 0)になっている場合は前の行の行末までを選択範囲とみなす
				// （選択始点が行頭ならその行頭は選択範囲に含み、終点が行頭ならその行頭は選択範囲に含まない、とする）
				// 論理的に少し変と指摘されるかもしれないが、実用上はそのようにしたほうが望ましいケースが多いと思われる。
				// ※行選択で行末までを選択範囲にしたつもりでも、UI上は次の行の行頭にカーソルが行く
				// ※終点の行頭を「^」にマッチさせたかったら１文字以上選択してね、ということで．．．
				// $$ 単位混在しまくりだけど、大丈夫？？
				if (
					(
						ptColLineP.y + linDif == (Int)ptOld.y
						&& (
							ptColLineP.x + colDif < (Int)ptOld.x
							|| ptColLineP.x == 0
						)
					)
					|| ptColLineP.y + linDif < (Int)ptOld.y
				) {
					break;
				}
			}
		}

		LayoutPoint ptTmp(0, 0);
		LogicPoint  ptTmpLogic(0, 0);

		if (bPaste || !bRegularExp) {
			// 正規表現時は 後方参照($&)で実現するので、正規表現は除外
			if (nReplaceTarget == 1) {	// 挿入位置セット
				if (bFastMode) {
					ptTmpLogic.x = selectLogic.GetTo().x - selectLogic.GetFrom().x;
					ptTmpLogic.y = selectLogic.GetTo().y - selectLogic.GetFrom().y;
					selectLogic.SetTo(selectLogic.GetFrom());
				}else {
					ptTmp.x = GetSelect().GetTo().x - GetSelect().GetFrom().x;
					ptTmp.y = GetSelect().GetTo().y - GetSelect().GetFrom().y;
					GetSelect().Clear(-1);
				}
			}else if (nReplaceTarget == 2) {	// 追加位置セット
				// 正規表現を除外したので、「検索後の文字が改行やったら次の行の先頭へ移動」の処理を削除
				if (bFastMode) {
					caret.MoveCursorFastMode(selectLogic.GetTo());
					selectLogic.SetFrom(selectLogic.GetTo());
				}else {
					caret.MoveCursor(GetSelect().GetTo(), false);
					GetSelect().Clear(-1);
				}
		    }else {
				// 位置指定ないので、何もしない
			}
		}
		// 行削除 選択範囲を行全体に拡大。カーソル位置を行頭へ(正規表現でも実行)
		if (nReplaceTarget == 3) {
			if (bFastMode) {
				const LogicInt y = selectLogic.GetFrom().y;
				selectLogic.SetFrom(LogicPoint(LogicXInt(0), y)); // 行頭
				selectLogic.SetTo(LogicPoint(LogicXInt(0), y + LogicInt(1))); // 次行の行頭
				if (docLineMgr.GetLineCount() == y + LogicInt(1)) {
					const DocLine* pLine = docLineMgr.GetLine(y);
					if (pLine->GetEol() == EolType::None) {
						// EOFは最終データ行にぶら下がりなので、選択終端は行末
						selectLogic.SetTo(LogicPoint(pLine->GetLengthWithEOL(), y)); // 対象行の行末
					}
				}
				caret.MoveCursorFastMode(selectLogic.GetFrom());
			}else {
				LogicPoint lineHome;
				layoutMgr.LayoutToLogic(GetSelect().GetFrom(), &lineHome);
				lineHome.x = LogicXInt(0); // 行頭
				LayoutRange selectFix;
				layoutMgr.LogicToLayout(lineHome, selectFix.GetFromPointer());
				lineHome.y++; // 次行の行頭
				layoutMgr.LogicToLayout(lineHome, selectFix.GetToPointer());
				caret.GetAdjustCursorPos(selectFix.GetToPointer());
				m_view.GetSelectionInfo().SetSelectArea(selectFix);
				caret.MoveCursor(selectFix.GetFrom(), false);
			}
		}


		// コマンドコードによる処理振り分け
		// テキストを貼り付け
		if (bPaste) {
			if (!bColumnSelect) {
				/* 本当は Command_INSTEXT を使うべきなんでしょうが、無駄な処理を避けるために直接たたく。
				** →m_nSelectXXXが-1の時に m_view.ReplaceData_CEditViewを直接たたくと動作不良となるため
				**   直接たたくのやめた。2003.05.18 by かろと
				*/
				Command_INSTEXT(false, szREPLACEKEY, nReplaceKey, true, bLineSelect);
			}else {
				Command_PASTEBOX(szREPLACEKEY, nReplaceKey);
				// 2013.06.11 再描画しないように
				// 再描画を行わないとどんな結果が起きているのか分からずみっともないので・・・。
				// m_view.AdjustScrollBars(); // 2007.07.22 ryoji
				// m_view.Redraw();
			}
			++nReplaceNum;
		}else if (nReplaceTarget == 3) {
			Command_INSTEXT( false, L"", LogicInt(0), true, false, bFastMode, bFastMode ? &selectLogic : nullptr );
			++nReplaceNum;
		}else if (bRegularExp) { // 検索／置換  1==正規表現
			// 2002/01/19 novice 正規表現による文字列置換
			// 物理行、物理行長、物理行での検索マッチ位置
			const DocLine* pDocLine;
			const wchar_t* pLine;
			LogicInt nLogicLineNum;
			LogicInt nIdx;
			LogicInt nLen;
			if (bFastMode) {
				pDocLine = docLineMgr.GetLine(selectLogic.GetFrom().GetY2());
				pLine = pDocLine->GetPtr();
				nLogicLineNum = selectLogic.GetFrom().GetY2();
				nIdx = selectLogic.GetFrom().GetX2();
				nLen = pDocLine->GetLengthWithEOL();
			}else {
				const Layout* pLayout = layoutMgr.SearchLineByLayoutY(GetSelect().GetFrom().GetY2());
				pDocLine = pLayout->GetDocLineRef();
				pLine = pDocLine->GetPtr();
				nLogicLineNum = pLayout->GetLogicLineNo();
				nIdx = m_view.LineColumnToIndex(pLayout, GetSelect().GetFrom().GetX2()) + pLayout->GetLogicOffset();
				nLen = pDocLine->GetLengthWithEOL();
			}
			LogicInt colDiff = LogicInt(0);
			if (!bConsecutiveAll) {	// 一括置換
				// 2007.01.16 ryoji
				// 選択範囲置換の場合は行内の選択範囲末尾まで置換範囲を縮め，
				// その位置を記憶する．
				if (bSelectedArea) {
					if (bBeginBoxSelect) {	// 矩形選択
						LogicPoint ptWork;
						layoutMgr.LayoutToLogic(
							LayoutPoint(rangeA.GetTo().x, ptOld.y),
							&ptWork
						);
						ptColLineP.x = ptWork.x;
						if (nLen - pDocLine->GetEol().GetLen() > ptColLineP.x + colDif)
							nLen = ptColLineP.GetX2() + LogicInt(colDif);
					}else {	// 通常の選択
						if (ptColLineP.y + linDif == (Int)ptOld.y) { //$$ 単位混在
							if (nLen - pDocLine->GetEol().GetLen() > ptColLineP.x + colDif)
								nLen = ptColLineP.GetX2() + LogicInt(colDif);
						}
					}
				}

				if (pDocLine->GetLengthWithoutEOL() < nLen)
					ptOld.x = (LayoutInt)(Int)pDocLine->GetLengthWithoutEOL() + 1; //$$ 単位混在
				else
					ptOld.x = (LayoutInt)(Int)nLen; //$$ 単位混在
			}

			if (int nReplace = regexp.Replace(pLine, nLen, nIdx)) {
				nReplaceNum += nReplace;
				if (!bConsecutiveAll) { // 2006.04.01 かろと	// 2007.01.16 ryoji
					// 行単位での置換処理
					// 選択範囲を物理行末までにのばす
					if (bFastMode) {
						selectLogic.SetTo(LogicPoint(nLen, nLogicLineNum));
					}else {
						layoutMgr.LogicToLayout(LogicPoint(nLen, nLogicLineNum), GetSelect().GetToPointer());
					}
				}else {
				    // From Here Jun. 6, 2005 かろと
				    // 物理行末までINSTEXTする方法は、キャレット位置を調整する必要があり、
				    // キャレット位置の計算が複雑になる。（置換後に改行がある場合に不具合発生）
				    // そこで、INSTEXTする文字列長を調整する方法に変更する（実はこっちの方がわかりやすい）
				    LogicInt matchLen = regexp.GetMatchLen();
				    LogicInt nIdxTo = nIdx + matchLen;		// 検索文字列の末尾
				    if (matchLen == 0) {
					    // ０文字マッチの時(無限置換にならないように１文字進める)
					    if (nIdxTo < nLen) {
						    // 2005-09-02 D.S.Koba GetSizeOfChar
						    nIdxTo += LogicInt(NativeW::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1);
					    }
					    // 無限置換しないように、１文字増やしたので１文字選択に変更
					    // 選択始点・終点への挿入の場合も０文字マッチ時は動作は同じになるので
						if (bFastMode) {
							selectLogic.SetTo(LogicPoint(nIdxTo, nLogicLineNum));
						}else {
							layoutMgr.LogicToLayout(LogicPoint(nIdxTo, nLogicLineNum), GetSelect().GetToPointer());	// 2007.01.19 ryoji 行位置も取得する
						}
				    }
				    // 行末から検索文字列末尾までの文字数
					colDiff =  nLen - nIdxTo;
					ptOld.x = (LayoutInt)(Int)nIdxTo;	// 2007.01.19 ryoji 追加  // $$ 単位混在
				    // Oct. 22, 2005 Karoto
				    // \rを置換するとその後ろの\nが消えてしまう問題の対応
				    if (colDiff < pDocLine->GetEol().GetLen()) {
					    // 改行にかかっていたら、行全体をINSTEXTする。
					    colDiff = LogicInt(0);
						if (bFastMode) {
							selectLogic.SetTo(LogicPoint(nLen, nLogicLineNum));
						}else {
							layoutMgr.LogicToLayout(LogicPoint(nLen, nLogicLineNum), GetSelect().GetToPointer());	// 2007.01.19 ryoji 追加
						}
						ptOld.x = (LayoutInt)(Int)pDocLine->GetLengthWithoutEOL() + 1;	// 2007.01.19 ryoji 追加 //$$ 単位混在
				    }
				}
				// 置換後文字列への書き換え(行末から検索文字列末尾までの文字を除く)
				Command_INSTEXT(false, regexp.GetString(), regexp.GetStringLen() - colDiff, true, false, bFastMode, bFastMode ? &selectLogic : nullptr);
				// To Here Jun. 6, 2005 かろと
			}
		}else {
			/* 本当は元コードを使うべきなんでしょうが、無駄な処理を避けるために直接たたく。
			** →m_nSelectXXXが-1の時に m_view.ReplaceData_CEditViewを直接たたくと動作不良となるため直接たたくのやめた。2003.05.18 かろと
			*/
			Command_INSTEXT(false, szREPLACEKEY, nReplaceKey, true, false, bFastMode, bFastMode ? &selectLogic : nullptr);
			++nReplaceNum;
		}

		// 挿入後の位置調整
		if (nReplaceTarget == 1) {
			if (bFastMode) {
				caret.SetCaretLogicPos(caret.GetCaretLogicPos() + ptTmpLogic);
			}else {
				caret.SetCaretLayoutPos(caret.GetCaretLayoutPos() + ptTmp);
				if (!bBeginBoxSelect) {
					LogicPoint p;
					layoutMgr.LayoutToLogic(
						caret.GetCaretLayoutPos(),
						&p
					);
					caret.SetCaretLogicPos(p);
				}
			}
		}

		if (!bFastMode && 50 <= nReplaceNum && !(bSelectedArea || bPaste)) {
			bFastMode = true;
			nAllLineNum = (Int)docLineMgr.GetLineCount();
			nAllLineNumOrg = nAllLineNumLogicOrg;
			for (nShiftCount=0; 300<nAllLineNum; ++nShiftCount) {
				nAllLineNum /= 2;
			}
			Progress_SetRange( hwndProgress, 0, nAllLineNum + 1 );
			int nDiff = nAllLineNumOrg - (Int)docLineMgr.GetLineCount();
			if (0 <= nDiff) {
				nNewPos = (nDiff + (Int)selectLogic.GetFrom().GetY2()) >> nShiftCount;
			}else {
				nNewPos = ::MulDiv((Int)selectLogic.GetFrom().GetY(), nAllLineNum, (Int)docLineMgr.GetLineCount());
			}
			Progress_SetPos( hwndProgress, nNewPos +1 );
			Progress_SetPos( hwndProgress, nNewPos );
		}
		// 最後に置換した位置を記憶
		if (bFastMode) {
			ptLastLogic = caret.GetCaretLogicPos();
		}else {
			ptLast = caret.GetCaretLayoutPos();
		}

		// 置換後の位置を確認
		if (bSelectedArea) {
			// 検索→置換の行補正値取得
			if (bBeginBoxSelect) {
				colDif += (Int)(ptLast.GetX2() - ptOld.GetX2());
				linDif += (Int)(layoutMgr.GetLineCount() - lineCnt);
			}else {
				// 置換前の検索文字列の最終位置は ptOld
				// 置換後のカーソル位置
				LogicPoint ptTmp2 = caret.GetCaretLogicPos();
				int linDif_thistime = docLineMgr.GetLineCount() - lineCnt;	// 今回置換での行数変化
				linDif += linDif_thistime;
				if (ptColLineP.y + linDif == ptTmp2.y) {
					// 最終行で置換した時、又は、置換の結果、選択エリア最終行まで到達した時
					// 最終行なので、置換前後の文字数の増減で桁位置を調整する
					colDif += (Int)ptTmp2.GetX2() - (Int)ptOld.GetX2(); //$$ 単位混在

					// 但し、以下の場合は置換前後で行が異なってしまうので、行の長さで補正する必要がある
					// １）最終行直前で行連結が起こり、行が減っている場合（行連結なので、桁位置は置換後のカーソル桁位置分増加する）
					// 　　ptTmp2.x-ptOld.xだと、\r\n → "" 置換で行連結した場合に、桁位置が負になり失敗する（負とは前行の後ろの方になることなので補正する）
					// 　　今回置換での行数の変化(linDif_thistime)で、最終行が行連結されたかどうかを見ることにする
					// ２）改行を置換した（ptTmp2.y != ptOld.y）場合、改行を置換すると置換後の桁位置が次行の桁位置になっているため
					//     ptTmp2.x-ptOld.xだと、負の数となり、\r\n → \n や \n → "abc" などで桁位置がずれる
					//     これも前行の長さで調整する必要がある
					if (linDif_thistime < 0 || ptTmp2.y != (Int)ptOld.y) { //$$ 単位混在
						colDif += linOldLen;
					}
				}
			}
		}
		// To Here 2001.12.03 hor

		// 次を検索
		// 2004.05.30 Moca 現在の検索文字列を使って検索する
		Command_SEARCH_NEXT(false, bDisplayUpdate, true, 0, NULL, bFastMode ? &selectLogic : nullptr);
	}

	if (bFastMode) {
		if (0 < nReplaceNum) {
			// LayoutMgrの更新(変更有の場合)
			layoutMgr._DoLayout();
			GetEditWindow().ClearViewCaretPosInfo();
			if (GetDocument().m_nTextWrapMethodCur == TextWrappingMethod::NoWrapping) {
				layoutMgr.CalculateTextWidth();
			}
		}
		layoutMgr.LogicToLayout(ptLastLogic, &ptLast);
		caret.MoveCursor(ptLast, true);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();	// 2009.07.25 ryoji
	}
	//>> 2002/03/26 Azumaiya

	_itot(nReplaceNum, szLabel, 10);
	::SendMessage(hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel);

	if (!dlgCancel.IsCanceled()) {
		nNewPos = nAllLineNum;
		Progress_SetPos(hwndProgress, nNewPos + 1);
		Progress_SetPos(hwndProgress, nNewPos);
	}
	dlgCancel.CloseDialog(0);
	::EnableWindow(m_view.GetHwnd(), TRUE);
	::EnableWindow(::GetParent(m_view.GetHwnd()), TRUE);
	::EnableWindow(::GetParent(::GetParent(m_view.GetHwnd())), TRUE);

	// From Here 2001.12.03 hor

	// テキスト選択解除
	m_view.GetSelectionInfo().DisableSelectArea(false);

	// カーソル・選択範囲復元
	if (
		!bSelectedArea			// ファイル全体置換
		|| dlgCancel.IsCanceled()
	) {		// キャンセルされた
		// 最後に置換した文字列の右へ
		if (!bFastMode) {
			caret.MoveCursor(ptLast, true);
		}
	}else {
		if (bBeginBoxSelect) {
			// 矩形選択
			m_view.GetSelectionInfo().SetBoxSelect(bBeginBoxSelect);
			rangeA.GetToPointer()->y += linDif;
			if (rangeA.GetTo().y < 0) rangeA.SetToY(LayoutInt(0));
		}else {
			// 普通の選択
			ptColLineP.x += colDif;
			if (ptColLineP.x < 0) ptColLineP.x = 0;
			ptColLineP.y += linDif;
			if (ptColLineP.y < 0) ptColLineP.y = 0;
			layoutMgr.LogicToLayout(
				ptColLineP,
				rangeA.GetToPointer()
			);
		}
		if (rangeA.GetFrom().y<rangeA.GetTo().y || rangeA.GetFrom().x<rangeA.GetTo().x) {
			m_view.GetSelectionInfo().SetSelectArea(rangeA);	// 2009.07.25 ryoji
		}
		caret.MoveCursor(rangeA.GetTo(), true);
		caret.m_nCaretPosX_Prev = caret.GetCaretLayoutPos().GetX2();	// 2009.07.25 ryoji
	}
	// To Here 2001.12.03 hor

	dlgReplace.m_bCanceled = dlgCancel.IsCanceled();
	dlgReplace.m_nReplaceCnt = nReplaceNum;
	m_view.SetDrawSwitch(bDrawSwitchOld);
	ActivateFrameWindow(GetMainWindow());
}


// 検索マークの切替え	// 2001.12.03 hor クリア を 切替え に変更
void ViewCommander::Command_SEARCH_CLEARMARK(void)
{
// From Here 2001.12.03 hor

	// 検索マークのセット

	if (m_view.GetSelectionInfo().IsTextSelected()) {

		// 検索文字列取得
		NativeW	memCurText;
		m_view.GetCurrentTextForSearch(memCurText, false);
		auto& csSearch = GetDllShareData().common.search;

		m_view.m_strCurSearchKey = memCurText.GetStringPtr();
		if (m_view.m_nCurSearchKeySequence < csSearch.nSearchKeySequence) {
			m_view.m_curSearchOption = csSearch.searchOption;
		}
		m_view.m_curSearchOption.bRegularExp = false;	// 正規表現使わない
		m_view.m_curSearchOption.bWordOnly = false;		// 単語で検索しない

		// 共有データへ登録
		if (memCurText.GetStringLength() < _MAX_PATH) {
			SearchKeywordManager().AddToSearchKeys(memCurText.GetStringPtr());
			csSearch.searchOption = m_view.m_curSearchOption;
		}
		m_view.m_nCurSearchKeySequence = csSearch.nSearchKeySequence;
		m_view.m_bCurSearchUpdate = true;

		m_view.ChangeCurRegexp(false); // 2002.11.11 Moca 正規表現で検索した後，色分けができていなかった

		// 再描画
		m_view.RedrawAll();
		return;
	}
// To Here 2001.12.03 hor

	// 検索マークのクリア

	m_view.m_bCurSrchKeyMark = false;	// 検索文字列のマーク
	// フォーカス移動時の再描画
	m_view.RedrawAll();
	return;
}

// Jun. 16, 2000 genta
// 対括弧の検索
void ViewCommander::Command_BRACKETPAIR(void)
{
	LayoutPoint ptColLine;
	//int nLine, nCol;

	int mode = 3;
	/*
	bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる
	bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
	bit2(out) : 見つかった位置         0:後ろ      1:前
	*/
	if (m_view.SearchBracket(GetCaret().GetCaretLayoutPos(), &ptColLine, &mode)) {	// 02/09/18 ai
		// 2005.06.24 Moca
		// 2006.07.09 genta 表示更新漏れ：新規関数にて対応
		m_view.MoveCursorSelecting(ptColLine, m_view.GetSelectionInfo().m_bSelectingLock);
	}else {
		// 失敗した場合は nCol/nLineには有効な値が入っていない.
		// 何もしない
	}
}

