/*!	@file
	@brief マウスイベントの処理

	@author Norio Nakatani
	@date	1998/03/13 作成
	@date   2008/04/13 CEditView.cppから分離
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, genta, JEPRO, MIK
	Copyright (C) 2001, genta, GAE, MIK, hor, asa-o, Stonee, Misaka, novice, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, Moca, minfu, KK, novice, ai, Azumaiya, genta
	Copyright (C) 2003, MIK, ai, ryoji, Moca, wmlhq, genta
	Copyright (C) 2004, genta, Moca, novice, naoh, isearch, fotomo
	Copyright (C) 2005, genta, MIK, novice, aroka, D.S.Koba, かろと, Moca
	Copyright (C) 2006, Moca, aroka, ryoji, fon, genta
	Copyright (C) 2007, ryoji, じゅうじ, maru

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <process.h> // _beginthreadex
#include <limits.h>
#include "EditView.h"
#include "_main/AppMode.h"
#include "EditApp.h"
#include "GrepAgent.h" // use EditApp.h
#include "window/EditWnd.h"
#include "_os/DropTarget.h" // DataObject
#include "_os/Clipboard.h"
#include "OpeBlk.h"
#include "doc/layout/Layout.h"
#include "cmd/ViewCommander_inline.h"
#include "uiparts/WaitCursor.h"
#include "uiparts/HandCursor.h"
#include "util/input.h"
#include "util/os.h"
#include "sakura_rc.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      マウスイベント                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// マウス左ボタン押下
void EditView::OnLBUTTONDOWN(WPARAM fwKeys, int _xPos , int _yPos)
{
	Point ptMouse(_xPos, _yPos);

	if (m_bHokan) {
		m_pEditWnd->m_hokanMgr.Hide();
		m_bHokan = FALSE;
	}

	// isearch 2004.10.22 isearchをキャンセルする
	if (m_nISearchMode > 0) {
		ISearchExit();
	}
	if (m_nAutoScrollMode) {
		AutoScrollExit();
	}
	if (m_bMiniMap) {
		::SetFocus(GetHwnd());
		::SetCapture(GetHwnd());
		m_bMiniMapMouseDown = true;
		OnMOUSEMOVE(fwKeys, _xPos, _yPos);
		return;
	}

	NativeW	memCurText;
	const wchar_t*	pLine;
	LogicInt		nLineLen;

	LayoutRange range;

	LogicInt	nIdx;
	int			nWork;
	int			nFuncID = 0;				// 2007.12.02 nasukoji	マウス左クリックに対応する機能コード

	if (m_pEditDoc->m_layoutMgr.GetLineCount() == 0) {
		return;
	}
	if (!GetCaret().ExistCaretFocus()) { // フォーカスがないとき
		return;
	}

	// 辞書Tipが起動されている
	if (m_dwTipTimer == 0) {
		// 辞書Tipを消す
		m_tipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	// 辞書Tip起動タイマー
	}else {
		m_dwTipTimer = ::GetTickCount();		// 辞書Tip起動タイマー
	}

	// 2007.10.02 nasukoji	トリプルクリックであることを示す
	// 2007.12.02 nasukoji	トリプルクリックをチェック
	bool tripleClickMode = CheckTripleClick(ptMouse);
	if (tripleClickMode) {
		// マウス左トリプルクリックに対応する機能コードはm_common.m_pKeyNameArr[5]に入っている
		nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::TripleClick].nFuncCodeArr[GetCtrlKeyState()];
		if (nFuncID == 0) {
			tripleClickMode = false;	// 割り当て機能無しの時はトリプルクリック OFF
		}
	}else {
		m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
	}

	// 現在のマウスカーソル位置→レイアウト位置
	LayoutPoint ptNew;
	auto& textArea = GetTextArea();
	textArea.ClientToLayout(ptMouse, &ptNew);

	// 2010.07.15 Moca マウスダウン時の座標を覚えて利用する
	m_mouseDownPos = ptMouse;

	// OLEによるドラッグ & ドロップを使う
	// 2007.12.02 nasukoji	トリプルクリック時はドラッグを開始しない
	if (!tripleClickMode && GetDllShareData().common.edit.bUseOLE_DragDrop) {
		if (GetDllShareData().common.edit.bUseOLE_DropSource) {		// OLEによるドラッグ元にするか
			// 行選択エリアをドラッグした
			if (ptMouse.x < textArea.GetAreaLeft() - GetTextMetrics().GetHankakuDx()) {
				goto normal_action;
			}
			// 指定カーソル位置が選択エリア内にあるか
			if (IsCurrentPositionSelected(ptNew) == 0) {
				POINT ptWk = {ptMouse.x, ptMouse.y};
				::ClientToScreen(GetHwnd(), &ptWk);
				if (!::DragDetect(GetHwnd(), ptWk)) {
					// ドラッグ開始条件を満たさなかったのでクリック位置にカーソル移動する
					if (GetSelectionInfo().IsTextSelected()) {	// テキストが選択されているか
						// 現在の選択範囲を非選択状態に戻す
						GetSelectionInfo().DisableSelectArea(true);
					}
//@@@ 2002.01.08 YAZAKI フリーカーソルOFFで複数行選択し、行の後ろをクリックするとそこにキャレットが置かれてしまうバグ修正
					// カーソル移動。
					if (ptMouse.y >= textArea.GetAreaTop() && ptMouse.y < textArea.GetAreaBottom()) {
						if (ptMouse.x >= textArea.GetAreaLeft() && ptMouse.x < textArea.GetAreaRight()
						) {
							GetCaret().MoveCursorToClientPoint(ptMouse);
						}else if (ptMouse.x < textArea.GetAreaLeft()) {
							GetCaret().MoveCursorToClientPoint(Point(textArea.GetDocumentLeftClientPointX(), ptMouse.y));
						}
					}
					return;
				}
				// 選択範囲のデータを取得
				if (GetSelectedData(&memCurText, false, NULL, false, GetDllShareData().common.edit.bAddCRLFWhenCopy)) {
					DWORD dwEffects;
					DWORD dwEffectsSrc = (!m_pEditDoc->IsEditable()) ?
											DROPEFFECT_COPY: DROPEFFECT_COPY | DROPEFFECT_MOVE;
					int nOpe = m_pEditDoc->m_docEditor.m_opeBuf.GetCurrentPointer();
					m_pEditWnd->SetDragSourceView(this);
					DataObject data(memCurText.GetStringPtr(), memCurText.GetStringLength(), GetSelectionInfo().IsBoxSelecting());
					dwEffects = data.DragDrop(TRUE, dwEffectsSrc);
					m_pEditWnd->SetDragSourceView(NULL);
					if (m_pEditDoc->m_docEditor.m_opeBuf.GetCurrentPointer() == nOpe) {	// ドキュメント変更なしか？	// 2007.12.09 ryoji
						m_pEditWnd->SetActivePane(m_nMyIndex);
						if ((dwEffectsSrc & dwEffects) == DROPEFFECT_MOVE) {
							// 移動範囲を削除する
							// ドロップ先が移動を処理したが自ドキュメントにここまで変更が無い
							// →ドロップ先は外部のウィンドウである
							if (!m_commander.GetOpeBlk()) {
								m_commander.SetOpeBlk(new OpeBlk);
							}
							m_commander.GetOpeBlk()->AddRef();

							// 選択範囲を削除
							DeleteData(true);

							// アンドゥバッファの処理
							SetUndoBuffer();
						}
					}
				}
				return;
			}
		}
	}

normal_action:;

	// ALTキーが押されている、かつトリプルクリックでない		// 2007.11.15 nasukoji	トリプルクリック対応
	if (GetKeyState_Alt() && !tripleClickMode) {
		if (GetSelectionInfo().IsTextSelected()) {	// テキストが選択されているか
			// 現在の選択範囲を非選択状態に戻す
			GetSelectionInfo().DisableSelectArea(true);
		}
		if (ptMouse.y >= textArea.GetAreaTop()  && ptMouse.y < textArea.GetAreaBottom()) {
			if (ptMouse.x >= textArea.GetAreaLeft() && ptMouse.x < textArea.GetAreaRight()) {
				GetCaret().MoveCursorToClientPoint(ptMouse);
			}else if (ptMouse.x < textArea.GetAreaLeft()) {
				GetCaret().MoveCursorToClientPoint(Point(textArea.GetDocumentLeftClientPointX(), ptMouse.y));
			}else {
				return;
			}
		}
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse;	// マウス範囲選択前回位置(XY座標)

		// 範囲選択開始 & マウスキャプチャー
		GetSelectionInfo().SelectBeginBox();

		::SetCapture(GetHwnd());
		GetCaret().HideCaret_(GetHwnd()); // 2002/07/22 novice
		// 現在のカーソル位置から選択を開始する
		GetSelectionInfo().BeginSelectArea();
		GetCaret().m_underLine.CaretUnderLineOFF(true);
		GetCaret().m_underLine.UnderLineLock();
		if (ptMouse.x < textArea.GetAreaLeft()) {
			// カーソル下移動
			GetCommander().Command_DOWN(true, false);
		}
	}else {
		// カーソル移動
		if (ptMouse.y >= textArea.GetAreaTop() && ptMouse.y < textArea.GetAreaBottom()) {
			if (ptMouse.x >= textArea.GetAreaLeft() && ptMouse.x < textArea.GetAreaRight()) {
			}else if (ptMouse.x < textArea.GetAreaLeft()) {
			}else {
				return;
			}
		}else if (ptMouse.y < textArea.GetAreaTop()) {
			//	ルーラクリック
			return;
		}else {
			return;
		}

		// マウスのキャプチャなど
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse;	// マウス範囲選択前回位置(XY座標)
		
		// 範囲選択開始 & マウスキャプチャー
		GetSelectionInfo().SelectBeginNazo();
		::SetCapture(GetHwnd());
		GetCaret().HideCaret_(GetHwnd()); // 2002/07/22 novice

		LayoutPoint ptNewCaret = GetCaret().GetCaretLayoutPos();
		bool bSetPtNewCaret = false;
		if (tripleClickMode) {		// 2007.11.15 nasukoji	トリプルクリックを処理する
			// 1行選択でない場合は選択文字列を解除
			// トリプルクリックが1行選択でなくてもクアドラプルクリックを有効とする
			if (nFuncID != F_SELECTLINE) {
				OnLBUTTONUP(fwKeys, ptMouse.x, ptMouse.y);	// ここで左ボタンアップしたことにする

				if (GetSelectionInfo().IsTextSelected())		// テキストが選択されているか
					GetSelectionInfo().DisableSelectArea(true);	// 現在の選択範囲を非選択状態に戻す
			}

			// 単語の途中で折り返されていると下の行が選択されてしまうことへの対処
			if (nFuncID != F_SELECTLINE) {
				GetCaret().MoveCursorToClientPoint(ptMouse);	// カーソル移動
			}else {
				GetCaret().MoveCursorToClientPoint(ptMouse, true, &ptNewCaret);	// カーソル移動
				bSetPtNewCaret = true;
			}

			// コマンドコードによる処理振り分け
			// マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
			::SendMessage(::GetParent(m_hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE), (LPARAM)NULL);

			// 1行選択でない場合はここで抜ける（他の選択コマンドの時問題となるかも）
			if (nFuncID != F_SELECTLINE)
				return;
			ptNewCaret = GetCaret().GetCaretLayoutPos();

			// 選択するものが無い（[EOF]のみの行）時は通常クリックと同じ処理
			if (1
				&& !GetSelectionInfo().IsTextSelected()
				&& GetCaret().GetCaretLogicPos().y >= m_pEditDoc->m_docLineMgr.GetLineCount()
			) {
				GetSelectionInfo().BeginSelectArea();				// 現在のカーソル位置から選択を開始する
				GetSelectionInfo().m_bBeginLineSelect = false;		// 行単位選択中 OFF
			}
		// 選択開始処理
		// SHIFTキーが押されていたか
		}else if (GetKeyState_Shift()) {
			if (GetSelectionInfo().IsTextSelected()) {		// テキストが選択されているか
				if (GetSelectionInfo().IsBoxSelecting()) {	// 矩形範囲選択中
					// 現在の選択範囲を非選択状態に戻す
					GetSelectionInfo().DisableSelectArea(true);

					// 現在のカーソル位置から選択を開始する
					GetSelectionInfo().BeginSelectArea();
				}else {
				}
			}else {
				// 現在のカーソル位置から選択を開始する
				GetSelectionInfo().BeginSelectArea();
			}

			// カーソル移動
			if (ptMouse.y >= textArea.GetAreaTop() && ptMouse.y < textArea.GetAreaBottom()) {
				if (ptMouse.x >= textArea.GetAreaLeft() && ptMouse.x < textArea.GetAreaRight()) {
					GetCaret().MoveCursorToClientPoint(ptMouse, true, &ptNewCaret);
				}else if (ptMouse.x < textArea.GetAreaLeft()) {
					GetCaret().MoveCursorToClientPoint(Point(textArea.GetDocumentLeftClientPointX(), ptMouse.y), true, &ptNewCaret);
				}
				bSetPtNewCaret = true;
			}
		}else {
			if (GetSelectionInfo().IsTextSelected()) {	// テキストが選択されているか
				// 現在の選択範囲を非選択状態に戻す
				GetSelectionInfo().DisableSelectArea(true);
			}
			// カーソル移動
			if (ptMouse.y >= textArea.GetAreaTop() && ptMouse.y < textArea.GetAreaBottom()) {
				if (ptMouse.x >= textArea.GetAreaLeft() && ptMouse.x < textArea.GetAreaRight()) {
					GetCaret().MoveCursorToClientPoint(ptMouse, true, &ptNewCaret);
				}else if (ptMouse.x < textArea.GetAreaLeft()) {
					GetCaret().MoveCursorToClientPoint(Point(textArea.GetDocumentLeftClientPointX(), ptMouse.y), true, &ptNewCaret);
				}
				bSetPtNewCaret = true;
			}
			// 現在のカーソル位置から選択を開始する
			GetSelectionInfo().BeginSelectArea(&ptNewCaret);
		}


		/******* この時点で必ず true == GetSelectionInfo().IsTextSelected() の状態になる ****:*/
		if (!GetSelectionInfo().IsTextSelected()) {
			WarningMessage(GetHwnd(), LS(STR_VIEW_MOUSE_BUG));
			return;
		}

		int	nWorkRel;
		nWorkRel = IsCurrentPositionSelected(
			ptNewCaret	// カーソル位置
		);


		// 現在のカーソル位置によって選択範囲を変更
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptNewCaret);

		bool bSelectWord = false;
		// CTRLキーが押されている、かつトリプルクリックでない		// 2007.11.15 nasukoji	トリプルクリック対応
		if (GetKeyState_Control() && !tripleClickMode) {
			GetSelectionInfo().m_bBeginWordSelect = true;		// 単語単位選択中
			if (!GetSelectionInfo().IsTextSelected()) {
				// 現在位置の単語選択
				if (GetCommander().Command_SELECTWORD(&ptNewCaret)) {
					bSelectWord = true;
					GetSelectionInfo().m_selectBgn = GetSelectionInfo().m_select;
				}
			}else {

				// 選択領域描画
				GetSelectionInfo().DrawSelectArea();

				// 指定された桁に対応する行のデータ内の位置を調べる
				const Layout* pLayout;
				pLine = m_pEditDoc->m_layoutMgr.GetLineStr(
					GetSelectionInfo().m_select.GetFrom().GetY2(),
					&nLineLen,
					&pLayout
				);
				if (pLine) {
					nIdx = LineColumnToIndex(pLayout, GetSelectionInfo().m_select.GetFrom().GetX2());
					// 現在位置の単語の範囲を調べる
					bool bWhareResult = m_pEditDoc->m_layoutMgr.WhereCurrentWord(
						GetSelectionInfo().m_select.GetFrom().GetY2(),
						nIdx,
						&range,
						NULL,
						NULL
					);
					if (bWhareResult) {
						// 指定された行のデータ内の位置に対応する桁の位置を調べる。
						// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
						/*
						pLine            = m_pEditDoc->m_layoutMgr.GetLineStr(range.GetFrom().GetY2(), &nLineLen, &pLayout);
						range.SetFromX(LineIndexToColumn(pLayout, range.GetFrom().x));
						pLine            = m_pEditDoc->m_layoutMgr.GetLineStr(range.GetTo().GetY2(), &nLineLen, &pLayout);
						range.SetToX(LineIndexToColumn(pLayout, range.GetTo().x));
						*/

						nWork = IsCurrentPositionSelected(
							range.GetFrom()	// カーソル位置
						);
						if (nWork == -1 || nWork == 0) {
							GetSelectionInfo().m_select.SetFrom(range.GetFrom());
							if (nWorkRel == 1) {
								GetSelectionInfo().m_selectBgn = range;
							}
						}
					}
				}
				pLine = m_pEditDoc->m_layoutMgr.GetLineStr(GetSelectionInfo().m_select.GetTo().GetY2(), &nLineLen, &pLayout);
				if (pLine) {
					nIdx = LineColumnToIndex(pLayout, GetSelectionInfo().m_select.GetTo().GetX2());
					// 現在位置の単語の範囲を調べる
					if (m_pEditDoc->m_layoutMgr.WhereCurrentWord(
						GetSelectionInfo().m_select.GetTo().GetY2(), nIdx, &range, NULL, NULL)
					) {
						// 指定された行のデータ内の位置に対応する桁の位置を調べる
						// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
						/*
						pLine = m_pEditDoc->m_layoutMgr.GetLineStr(range.GetFrom().GetY2(), &nLineLen, &pLayout);
						range.SetFromX(LineIndexToColumn(pLayout, range.GetFrom().x));
						pLine = m_pEditDoc->m_layoutMgr.GetLineStr(range.GetTo().GetY2(), &nLineLen, &pLayout);
						range.SetToX(LineIndexToColumn(pLayout, range.GetTo().x));
						*/

						nWork = IsCurrentPositionSelected(range.GetFrom());
						if (nWork == -1 || nWork == 0) {
							GetSelectionInfo().m_select.SetTo(range.GetFrom());
						}
						if (IsCurrentPositionSelected(range.GetTo()) == 1) {
							GetSelectionInfo().m_select.SetTo(range.GetTo());
						}
						if (nWorkRel == -1 || nWorkRel == 0) {
							GetSelectionInfo().m_selectBgn=range;
						}
					}
				}

				if (0 < nWorkRel) {

				}
				// 選択領域描画
				GetSelectionInfo().DrawSelectArea();
			}
		}
		// 行番号エリアをクリックした
		// 2008.05.22 nasukoji	シフトキーを押している場合は行頭クリックとして扱う
		if (ptMouse.x < textArea.GetAreaLeft() && !GetKeyState_Shift()) {
			// 現在のカーソル位置から選択を開始する
			GetSelectionInfo().m_bBeginLineSelect = true;

			// 2009.02.22 ryoji 
			// Command_GOLINEEND()/Command_RIGHT()ではなく次のレイアウトを調べて移動選択する方法に変更
			// ※Command_GOLINEEND()/Command_RIGHT()は[折り返し末尾文字の右へ移動]＋[次行の先頭文字の右に移動]の仕様だとＮＧ
			const Layout* pLayout = m_pEditDoc->m_layoutMgr.SearchLineByLayoutY(ptNewCaret.GetY2());
			if (pLayout) {
				LayoutPoint ptCaret;
				const Layout* pNext = pLayout->GetNextLayout();
				if (pNext) {
					ptCaret.x = pNext->GetIndent();
				}else {
					ptCaret.x = LayoutInt(0);
				}
				ptCaret.y = ptNewCaret.GetY2() + 1;	// 改行無しEOF行でも MoveCursor() が有効な座標に調整してくれる
				GetCaret().GetAdjustCursorPos(&ptCaret);
				GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptCaret);
				GetCaret().MoveCursor(ptCaret, true);
				GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			}else {
				// 現在のカーソル位置によって選択範囲を変更
				if (bSetPtNewCaret) {
					GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptNewCaret);
					GetCaret().MoveCursor(ptNewCaret, true, 1000);
					GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
				}
			}

			//	Apr. 14, 2003 genta
			//	行番号の下をクリックしてドラッグを開始するとおかしくなるのを修正
			//	行番号をクリックした場合にはGetSelectionInfo().ChangeSelectAreaByCurrentCursor()にて
			//	GetSelectionInfo().m_select.GetTo().x/GetSelectionInfo().m_select.GetTo().yに-1が設定されるが、上の
			//	GetCommander().Command_GOLINEEND(), Command_RIGHT()によって行選択が行われる。
			//	しかしキャレットが末尾にある場合にはキャレットが移動しないので
			//	GetSelectionInfo().m_select.GetTo().x/GetSelectionInfo().m_select.GetTo().yが-1のまま残ってしまい、それが
			//	原点に設定されるためにおかしくなっていた。
			//	なので、範囲選択が行われていない場合は起点末尾の設定を行わないようにする
			if (GetSelectionInfo().IsTextSelected()) {
				GetSelectionInfo().m_selectBgn.SetTo(GetSelectionInfo().m_select.GetTo());
			}
		}else {
			// URLがクリックされたら選択するか
			if (GetDllShareData().common.edit.bSelectClickedURL) {

				LogicRange cUrlRange;	// URL範囲
				// カーソル位置にURLが有る場合のその範囲を調べる
				bool bIsUrl = IsCurrentPositionURL(
					ptNewCaret,	// カーソル位置
					&cUrlRange,						// URL範囲
					NULL							// URL受け取り先
				);
				if (bIsUrl) {
					// 現在の選択範囲を非選択状態に戻す
					GetSelectionInfo().DisableSelectArea(true);

					/*
					  カーソル位置変換
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					  →レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						2002/04/08 YAZAKI 少しでもわかりやすく。
					*/
					LayoutRange rangeB;
					m_pEditDoc->m_layoutMgr.LogicToLayout(cUrlRange, &rangeB);
					/*
					m_pEditDoc->m_layoutMgr.LogicToLayout(LogicPoint(nUrlIdxBgn          , nUrlLine), rangeB.GetFromPointer());
					m_pEditDoc->m_layoutMgr.LogicToLayout(LogicPoint(nUrlIdxBgn + nUrlLen, nUrlLine), rangeB.GetToPointer());
					*/

					GetSelectionInfo().m_selectBgn = rangeB;
					GetSelectionInfo().m_select = rangeB;

					// 選択領域描画
					GetSelectionInfo().DrawSelectArea();
				}
			}
			if (bSetPtNewCaret && !bSelectWord) {
				// 現在のカーソル位置によって選択範囲を変更
				GetCaret().MoveCursor(ptNewCaret, true, 1000);
				GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			}
		}
	}
}


/*!	トリプルクリックのチェック
	@brief トリプルクリックを判定する
	
	2回目のクリックから3回目のクリックまでの時間がダブルクリック時間以内で、
	かつその時のクリック位置のずれがシステムメトリック（X:SM_CXDOUBLECLK,
	Y:SM_CYDOUBLECLK）の値（ピクセル）以下の時トリプルクリックとする。
	
	@param[in] xPos		マウスクリックX座標
	@param[in] yPos		マウスクリックY座標
	@return		トリプルクリックの時はTRUEを返す
	トリプルクリックでない時はFALSEを返す

	@note	m_dwTripleClickCheckが0でない時にチェックモードと判定するが、PCを
			連続稼動している場合49.7日毎にカウンタが0になる為、わずかな可能性
			であるがトリプルクリックが判定できない時がある。
			行番号表示エリアのトリプルクリックは通常クリックとして扱う。
	
	@date 2007.11.15 nasukoji	新規作成
*/
bool EditView::CheckTripleClick(Point ptMouse)
{

	// トリプルクリックチェック有効でない（時刻がセットされていない）
	if (!m_dwTripleClickCheck)
		return false;

	bool result = false;

	// 前回クリックとのクリック位置のずれを算出
	Point dpos(GetSelectionInfo().m_ptMouseRollPosOld.x - ptMouse.x,
				   GetSelectionInfo().m_ptMouseRollPosOld.y - ptMouse.y);

	if (dpos.x < 0)
		dpos.x = -dpos.x;	// 絶対値化

	if (dpos.y < 0)
		dpos.y = -dpos.y;	// 絶対値化

	// 行番号表示エリアでない、かつクリックプレスからダブルクリック時間以内、
	// かつダブルクリックの許容ずれピクセル以下のずれの時トリプルクリックとする
	//	2007.10.12 genta/dskoba システムのダブルクリック速度，ずれ許容量を取得
	if ((ptMouse.x >= GetTextArea().GetAreaLeft())&&
		(::GetTickCount() - m_dwTripleClickCheck <= GetDoubleClickTime())&&
		(dpos.x <= GetSystemMetrics(SM_CXDOUBLECLK)) &&
		(dpos.y <= GetSystemMetrics(SM_CYDOUBLECLK))
	) {
		result = true;
	}else {
		m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
	}
	
	return result;
}

// マウス右ボタン押下
void EditView::OnRBUTTONDOWN(WPARAM fwKeys, int xPos , int yPos)
{
	if (m_nAutoScrollMode) {
		AutoScrollExit();
	}
	if (m_bMiniMap) {
		return;
	}
	// 現在のマウスカーソル位置→レイアウト位置

	LayoutPoint ptNew;
	GetTextArea().ClientToLayout(Point(xPos, yPos), &ptNew);
	/*
	ptNew.x = GetTextArea().GetViewLeftCol() + (xPos - GetTextArea().GetAreaLeft()) / GetTextMetrics().GetHankakuDx();
	ptNew.y = GetTextArea().GetViewTopLine() + (yPos - GetTextArea().GetAreaTop()) / GetTextMetrics().GetHankakuDy();
	*/
	// 指定カーソル位置が選択エリア内にあるか
	if (IsCurrentPositionSelected(
			ptNew		// カーソル位置
		) == 0
	) {
		return;
	}
	OnLBUTTONDOWN(fwKeys, xPos , yPos);
	return;
}

// マウス右ボタン開放
void EditView::OnRBUTTONUP(WPARAM fwKeys, int xPos , int yPos)
{
	if (GetSelectionInfo().IsMouseSelecting()) {	// 範囲選択中
		// マウス左ボタン開放のメッセージ処理
		OnLBUTTONUP(fwKeys, xPos, yPos);
	}

	int		nIdx;
	int		nFuncID;
// novice 2004/10/10
	// Shift,Ctrl,Altキーが押されていたか
	nIdx = GetCtrlKeyState();
	// マウス右クリックに対応する機能コードはm_common.m_pKeyNameArr[1]に入っている
	nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::RightClick].nFuncCodeArr[nIdx];
	if (nFuncID != 0) {
		// コマンドコードによる処理振り分け
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage(::GetParent(m_hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE), (LPARAM)NULL);
	}
//	// 右クリックメニュー
//	GetCommander().Command_MENU_RBUTTON();
	return;
}


// novice 2004/10/11 マウス中ボタン対応
/*!
	@brief マウス中ボタンを押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.11 novice 新規作成
	@date 2008.10.06 nasukoji	マウス中ボタン押下中のホイール操作対応
	@date 2009.01.17 nasukoji	ボタンUPでコマンドを起動するように変更
*/
void EditView::OnMBUTTONDOWN(WPARAM fwKeys, int xPos , int yPos)
{
	int nIdx = GetCtrlKeyState();
	if (GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::CenterClick].nFuncCodeArr[nIdx] == F_AUTOSCROLL) {
		if (m_nAutoScrollMode) {
			AutoScrollExit();
			return;
		}else {
			m_nAutoScrollMode = 1;
			m_autoScrollMousePos = Point(xPos, yPos);
			::SetCapture(GetHwnd());
		}
	}
}


/*!
	@brief マウス中ボタンを開放したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	
	@date 2009.01.17 nasukoji	新規作成（ボタンUPでコマンドを起動するように変更）
*/
void EditView::OnMBUTTONUP(WPARAM fwKeys, int xPos , int yPos)
{
	int		nIdx;
	int		nFuncID;

	// ホイール操作によるページスクロールあり
	if (GetDllShareData().common.general.nPageScrollByWheel == (int)MouseFunctionType::CenterClick &&
	    m_pEditWnd->IsPageScrollByWheel()
	) {
		m_pEditWnd->SetPageScrollByWheel(FALSE);
		return;
	}

	// ホイール操作によるページスクロールあり
	if (GetDllShareData().common.general.nHorizontalScrollByWheel == (int)MouseFunctionType::CenterClick &&
	    m_pEditWnd->IsHScrollByWheel()
	) {
		m_pEditWnd->SetHScrollByWheel(FALSE);
		return;
	}

	// Shift,Ctrl,Altキーが押されていたか
	nIdx = GetCtrlKeyState();
	// マウス左サイドボタンに対応する機能コードはm_common.m_pKeyNameArr[2]に入っている
	nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::CenterClick].nFuncCodeArr[nIdx];
	if (nFuncID == F_AUTOSCROLL) {
		if (m_nAutoScrollMode == 1) {
			m_bAutoScrollDragMode = false;
			AutoScrollEnter();
			return;
		}else if (m_nAutoScrollMode == 2 && m_bAutoScrollDragMode) {
			AutoScrollExit();
			return;
		}
	}else if (nFuncID != 0) {
		// コマンドコードによる処理振り分け
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage(::GetParent(m_hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE),  (LPARAM)NULL);
	}
	if (m_nAutoScrollMode) {
		AutoScrollExit();
	}
}

void CALLBACK AutoScrollTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	EditView*	pEditView;
	pEditView = (EditView*)::GetWindowLongPtr(hwnd, 0);
	if (pEditView) {
		pEditView->AutoScrollOnTimer();
	}
}

void EditView::AutoScrollEnter()
{
	m_bAutoScrollVertical = GetTextArea().m_nViewRowNum < m_pEditDoc->m_layoutMgr.GetLineCount() + 2;
	m_bAutoScrollHorizontal = GetTextArea().m_nViewColNum < GetRightEdgeForScrollBar();
	if (m_bMiniMap) {
		m_bAutoScrollHorizontal = false;
	}
	if (!m_bAutoScrollHorizontal && !m_bAutoScrollVertical) {
		m_nAutoScrollMode = 0;
		return;
	}
	m_nAutoScrollMode = 2;
	m_autoScrollWnd.Create(G_AppInstance(), GetHwnd(), m_bAutoScrollVertical, m_bAutoScrollHorizontal, m_autoScrollMousePos, this);
	::SetTimer(GetHwnd(), 2, 200, AutoScrollTimerProc);
	HCURSOR hCursor;
	hCursor = ::LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_CURSOR_AUTOSCROLL_CENTER));
	::SetCursor(hCursor);
}

void EditView::AutoScrollExit()
{
	if (m_nAutoScrollMode) {
		::ReleaseCapture();
	}
	if (m_nAutoScrollMode == 2) {
		KillTimer(GetHwnd(), 2);
		m_autoScrollWnd.Close();
	}
	m_nAutoScrollMode = 0;
}

void EditView::AutoScrollMove(Point& point)
{
	const Point relPos = point - m_autoScrollMousePos;
	int idcX, idcY;
	if (!m_bAutoScrollHorizontal || abs(relPos.x) < 16) {
		idcX = 0;
	}else if (relPos.x < 0) {
		idcX = 1;
	}else {
		idcX = 2;
	}
	if (!m_bAutoScrollVertical || abs(relPos.y) < 16) {
		idcY = 0;
	}else if (relPos.y < 0) {
		idcY = 1;
	}else {
		idcY = 2;
	}
	const int idcs[3][3] = {
		{IDC_CURSOR_AUTOSCROLL_CENTER, IDC_CURSOR_AUTOSCROLL_UP,       IDC_CURSOR_AUTOSCROLL_DOWN},
		{IDC_CURSOR_AUTOSCROLL_LEFT,   IDC_CURSOR_AUTOSCROLL_UP_LEFT,  IDC_CURSOR_AUTOSCROLL_DOWN_LEFT},
		{IDC_CURSOR_AUTOSCROLL_RIGHT,  IDC_CURSOR_AUTOSCROLL_UP_RIGHT, IDC_CURSOR_AUTOSCROLL_DOWN_RIGHT}};
	int cursor = idcs[idcX][idcY];
	if (cursor == IDC_CURSOR_AUTOSCROLL_CENTER) {
		if (!m_bAutoScrollVertical) {
			cursor = IDC_CURSOR_AUTOSCROLL_HORIZONTAL;
		}else if (!m_bAutoScrollHorizontal) {
			cursor = IDC_CURSOR_AUTOSCROLL_VERTICAL;
		}
	}
	const HCURSOR hCursor = ::LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(cursor));
	::SetCursor(hCursor);
}

void EditView::AutoScrollOnTimer()
{
	Point cursorPos;
	::GetCursorPos(&cursorPos);
	::ScreenToClient(GetHwnd(), &cursorPos);
	
	const Point relPos = cursorPos - m_autoScrollMousePos;
	Point scrollPos = relPos / 8;
	if (m_bAutoScrollHorizontal) {
		if (scrollPos.x < 0) {
			scrollPos.x += 1;
		}else if (scrollPos.x > 0) {
			scrollPos.x -= 1;
		}
		SyncScrollH(ScrollAtH(GetTextArea().GetViewLeftCol() + scrollPos.x));
	}
	if (m_bAutoScrollVertical) {
		if (scrollPos.y < 0) {
			scrollPos.y += 1;
		}else if (scrollPos.y > 0) {
			scrollPos.y -= 1;
		}
		SyncScrollV(ScrollAtV(GetTextArea().GetViewTopLine() + scrollPos.y));
	}
}

// novice 2004/10/10 マウスサイドボタン対応
/*!
	@brief マウスサイドボタン1を押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.10 novice 新規作成
	@date 2004.10.11 novice マウス中ボタン対応のため変更
	@date 2009.01.17 nasukoji	ボタンUPでコマンドを起動するように変更
*/
void EditView::OnXLBUTTONDOWN(WPARAM fwKeys, int xPos , int yPos)
{
	if (m_nAutoScrollMode) {
		AutoScrollExit();
	}
}


/*!
	@brief マウスサイドボタン1を開放したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標

	@date 2009.01.17 nasukoji	新規作成（ボタンUPでコマンドを起動するように変更）
*/
void EditView::OnXLBUTTONUP(WPARAM fwKeys, int xPos , int yPos)
{
	int		nIdx;
	int		nFuncID;

	// ホイール操作によるページスクロールあり
	if (GetDllShareData().common.general.nPageScrollByWheel == (int)MouseFunctionType::LeftSideClick &&
	    m_pEditWnd->IsPageScrollByWheel()
	) {
		m_pEditWnd->SetPageScrollByWheel(FALSE);
		return;
	}

	// ホイール操作によるページスクロールあり
	if (GetDllShareData().common.general.nHorizontalScrollByWheel == (int)MouseFunctionType::LeftSideClick &&
	    m_pEditWnd->IsHScrollByWheel()
	) {
		m_pEditWnd->SetHScrollByWheel(FALSE);
		return;
	}

	// Shift,Ctrl,Altキーが押されていたか
	nIdx = GetCtrlKeyState();
	// マウスサイドボタン1に対応する機能コードはm_common.m_pKeyNameArr[3]に入っている
	nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::LeftSideClick].nFuncCodeArr[nIdx];
	if (nFuncID != 0) {
		// コマンドコードによる処理振り分け
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage(::GetParent(m_hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE),  (LPARAM)NULL);
	}

	return;
}


/*!
	@brief マウスサイドボタン2を押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.10 novice 新規作成
	@date 2004.10.11 novice マウス中ボタン対応のため変更
	@date 2009.01.17 nasukoji	ボタンUPでコマンドを起動するように変更
*/
void EditView::OnXRBUTTONDOWN(WPARAM fwKeys, int xPos , int yPos)
{
	if (m_nAutoScrollMode) {
		AutoScrollExit();
	}
}


/*!
	@brief マウスサイドボタン2を開放したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標

	@date 2009.01.17 nasukoji	新規作成（ボタンUPでコマンドを起動するように変更）
*/
void EditView::OnXRBUTTONUP(WPARAM fwKeys, int xPos , int yPos)
{
	int		nIdx;
	int		nFuncID;

	// ホイール操作によるページスクロールあり
	if (GetDllShareData().common.general.nPageScrollByWheel == (int)MouseFunctionType::RightSideClick &&
	    m_pEditWnd->IsPageScrollByWheel()
	) {
		// ホイール操作によるページスクロールありをOFF
		m_pEditWnd->SetPageScrollByWheel(FALSE);
		return;
	}

	// ホイール操作によるページスクロールあり
	if (GetDllShareData().common.general.nHorizontalScrollByWheel == (int)MouseFunctionType::RightSideClick &&
	    m_pEditWnd->IsHScrollByWheel()
	) {
		// ホイール操作による横スクロールありをOFF
		m_pEditWnd->SetHScrollByWheel(FALSE);
		return;
	}

	// Shift,Ctrl,Altキーが押されていたか
	nIdx = GetCtrlKeyState();
	// マウスサイドボタン2に対応する機能コードはm_common.m_pKeyNameArr[4]に入っている
	nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::RightSideClick].nFuncCodeArr[nIdx];
	if (nFuncID != 0) {
		// コマンドコードによる処理振り分け
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage(::GetParent(m_hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE),  (LPARAM)NULL);
	}

	return;
}

// マウス移動のメッセージ処理
void EditView::OnMOUSEMOVE(WPARAM fwKeys, int xPos_, int yPos_)
{
	Point ptMouse(xPos_, yPos_);

	if (m_mousePousePos != ptMouse) {
		m_mousePousePos = ptMouse;
		if (m_nMousePouse < 0) {
			m_nMousePouse = 0;
		}
	}

	LayoutRange selectionOld    = GetSelectionInfo().m_select;

	// オートスクロール
	if (m_nAutoScrollMode == 1) {
		if (::GetSystemMetrics(SM_CXDOUBLECLK) < abs(ptMouse.x - m_autoScrollMousePos.x) ||
		    ::GetSystemMetrics(SM_CYDOUBLECLK) < abs(ptMouse.y - m_autoScrollMousePos.y)
		) {
			m_bAutoScrollDragMode = true;
			AutoScrollEnter();
		}
		return;
	}else if (m_nAutoScrollMode == 2) {
		AutoScrollMove(ptMouse);
		return;
	}

	TextArea& textArea = GetTextArea();
	if (m_bMiniMap) {
		POINT po;
		::GetCursorPos(&po);
		// 辞書Tipが起動されている
		if (m_dwTipTimer == 0) {
			if ((m_poTipCurPos.x != po.x || m_poTipCurPos.y != po.y )) {
				m_tipWnd.Hide();
				m_dwTipTimer = ::GetTickCount();
			}
		}else {
			m_dwTipTimer = ::GetTickCount();
		}
		if (m_bMiniMapMouseDown) {
			LayoutPoint ptNew;
			textArea.ClientToLayout( ptMouse, &ptNew );
			// ミニマップの上下スクロール
			if (ptNew.y < 0) {
				ptNew.y = LayoutYInt(0);
			}
			LayoutYInt nScrollRow = LayoutYInt(0);
			LayoutYInt nScrollMargin = LayoutYInt(15);
			nScrollMargin  = t_min(nScrollMargin,  (textArea.m_nViewRowNum) / 2);
			if (m_pEditDoc->m_layoutMgr.GetLineCount() > textArea.m_nViewRowNum
				&& ptNew.y > textArea.GetViewTopLine() + textArea.m_nViewRowNum - nScrollMargin
			) {
				nScrollRow = (textArea.GetViewTopLine() + textArea.m_nViewRowNum - nScrollMargin) - ptNew.y;
			}else if (0 < textArea.GetViewTopLine() && ptNew.y < textArea.GetViewTopLine() + nScrollMargin) {
				nScrollRow = textArea.GetViewTopLine() + nScrollMargin - ptNew.y;
				if (0 > textArea.GetViewTopLine() - nScrollRow) {
					nScrollRow = textArea.GetViewTopLine();
				}
			}
			if (nScrollRow != 0) {
				ScrollAtV( textArea.GetViewTopLine() - nScrollRow );
			}

			textArea.ClientToLayout( ptMouse, &ptNew );
			if (ptNew.y < 0) {
				ptNew.y = LayoutYInt(0);
			}
			EditView& view = m_pEditWnd->GetActiveView();
			ptNew.x = 0;
			LogicPoint ptNewLogic;
			view.GetCaret().GetAdjustCursorPos(&ptNew);
			GetDocument().m_layoutMgr.LayoutToLogic(ptNew, &ptNewLogic);
			GetDocument().m_layoutMgr.LogicToLayout(ptNewLogic, &ptNew, ptNew.y);
			if (GetKeyState_Shift()) {
				if (view.GetSelectionInfo().IsTextSelected()) {
					if (view.GetSelectionInfo().IsBoxSelecting()) {
						view.GetSelectionInfo().DisableSelectArea(true);
						view.GetSelectionInfo().BeginSelectArea();
					}
				}else {
					view.GetSelectionInfo().BeginSelectArea();
				}
				view.GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptNew);
			}else {
				if (view.GetSelectionInfo().IsTextSelected()) {
					view.GetSelectionInfo().DisableSelectArea(true);
				}
			}
			view.GetCaret().MoveCursor(ptNew, true);
			view.GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse; // マウス範囲選択前回位置(XY座標)
		return;
	}

	if (!GetSelectionInfo().IsMouseSelecting()) {
		// マウスによる範囲選択中でない場合
		POINT		po;
		::GetCursorPos(&po);
		//	2001/06/18 asa-o: 補完ウィンドウが表示されていない
		if (!m_bHokan) {
			// 辞書Tipが起動されている
			if (m_dwTipTimer == 0) {
				if ((m_poTipCurPos.x != po.x || m_poTipCurPos.y != po.y)) {
					// 辞書Tipを消す
					m_tipWnd.Hide();
					m_dwTipTimer = ::GetTickCount();	// 辞書Tip起動タイマー
				}
			}else {
				m_dwTipTimer = ::GetTickCount();		// 辞書Tip起動タイマー
			}
		}
		// 現在のマウスカーソル位置→レイアウト位置
		LayoutPoint ptNew;
		textArea.ClientToLayout(ptMouse, &ptNew);

		LogicRange	cUrlRange;	// URL範囲

		// 選択テキストのドラッグ中か
		if (m_bDragMode) {
			if (GetDllShareData().common.edit.bUseOLE_DragDrop) {	// OLEによるドラッグ & ドロップを使う
				// 座標指定によるカーソル移動
				GetCaret().MoveCursorToClientPoint(ptMouse);
			}
		}else {
			// 行選択エリア?
			if (ptMouse.x < textArea.GetAreaLeft() || ptMouse.y < textArea.GetAreaTop()) {	//	2002/2/10 aroka
				// 矢印カーソル
				if (ptMouse.y >= textArea.GetAreaTop())
					::SetCursor(::LoadCursor(G_AppInstance(), MAKEINTRESOURCE(IDC_CURSOR_RVARROW)));
				else
					::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			}else if (1
				&& GetDllShareData().common.edit.bUseOLE_DragDrop		// OLEによるドラッグ & ドロップを使う
				&& GetDllShareData().common.edit.bUseOLE_DropSource		// OLEによるドラッグ元にするか
				&& IsCurrentPositionSelected(ptNew) == 0				// 指定カーソル位置が選択エリア内にあるか
			) {
				// 矢印カーソル
				::SetCursor(::LoadCursor(NULL, IDC_ARROW));
			// カーソル位置にURLが有る場合
			}else if (
				IsCurrentPositionURL(
					ptNew,			// カーソル位置
					&cUrlRange,		// URL範囲
					NULL			// URL受け取り先
				)
			) {
				// 手カーソル
				SetHandCursor();		// Hand Cursorを設定 2013/1/29 Uchi
			}else {
				// migemo isearch 2004.10.22
				if (m_nISearchMode > 0) {
					if (m_nISearchDirection == SearchDirection::Forward) {
						::SetCursor(::LoadCursor(G_AppInstance(), MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
					}else {
						::SetCursor(::LoadCursor(G_AppInstance(), MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
					}
				// アイビーム
				}else if (0 <= m_nMousePouse) {
					::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
				}
			}
		}
		return;
	}
	// 以下、マウスでの選択中(ドラッグ中)

	if (0 <= m_nMousePouse) {
		::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
	}

	// 2010.07.15 Moca ドラッグ開始位置から移動していない場合はMOVEとみなさない
	// 遊びは 2px固定とする
	Point ptMouseMove = ptMouse - m_mouseDownPos;
	if (m_mouseDownPos.x != -INT_MAX && abs(ptMouseMove.x) <= 2 && abs(ptMouseMove.y) <= 2) {
		return;
	}
	// 一度移動したら戻ってきたときも、移動とみなすように設定
	m_mouseDownPos.Set(-INT_MAX, -INT_MAX);
	
	LayoutPoint ptNewCursor(LayoutInt(-1), LayoutInt(-1));
	if (GetSelectionInfo().IsBoxSelecting()) {	// 矩形範囲選択中
		// 座標指定によるカーソル移動
		GetCaret().MoveCursorToClientPoint(ptMouse, true, &ptNewCursor);
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptNewCursor);
		GetCaret().MoveCursorToClientPoint(ptMouse);
		// 現在のカーソル位置によって選択範囲を変更
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse; // マウス範囲選択前回位置(XY座標)
	}else {
		// 座標指定によるカーソル移動
		if ((ptMouse.x < textArea.GetAreaLeft() || m_dwTripleClickCheck)&& GetSelectionInfo().m_bBeginLineSelect) {	// 行単位選択中
			// 2007.11.15 nasukoji	上方向の行選択時もマウスカーソルの位置の行が選択されるようにする
			Point nNewPos(0, ptMouse.y);

			// 1行の高さ
			int nLineHeight = GetTextMetrics().GetHankakuHeight() + m_pTypeData->nLineSpace;

			// 選択開始行以下へのドラッグ時は1行下にカーソルを移動する
			if (textArea.GetViewTopLine() + (ptMouse.y - textArea.GetAreaTop()) / nLineHeight >= GetSelectionInfo().m_selectBgn.GetTo().y)
				nNewPos.y += nLineHeight;

			// カーソルを移動
			nNewPos.x = textArea.GetAreaLeft() - Int(textArea.GetViewLeftCol()) * (GetTextMetrics().GetHankakuWidth() + m_pTypeData->nColumnSpace);
			GetCaret().MoveCursorToClientPoint(nNewPos, false, &ptNewCursor);

			// 2.5クリックによる行単位のドラッグ
			if (m_dwTripleClickCheck) {
				// 選択開始行以上にドラッグした
				if (ptNewCursor.GetY() <= GetSelectionInfo().m_selectBgn.GetTo().y) {
					// GetCommander().Command_GOLINETOP(true, 0x09);		// 改行単位の行頭へ移動
					LogicInt nLineLen;
					const Layout*	pLayout;
					const wchar_t*	pLine = m_pEditDoc->m_layoutMgr.GetLineStr(ptNewCursor.GetY2(), &nLineLen, &pLayout);
					ptNewCursor.x = LayoutInt(0);
					if (pLine) {
						while (pLayout->GetLogicOffset()) {
							ptNewCursor.y--;
							pLayout = pLayout->GetPrevLayout();
						}
					}
				}else {
					LayoutPoint ptCaret;

					LogicPoint ptCaretPrevLog(0, GetCaret().GetCaretLogicPos().y);

					// 選択開始行より下にカーソルがある時は1行前と物理行番号の違いをチェックする
					// 選択開始行にカーソルがある時はチェック不要
					if (ptNewCursor.GetY() > GetSelectionInfo().m_selectBgn.GetTo().y) {
						// 1行前の物理行を取得する
						m_pEditDoc->m_layoutMgr.LayoutToLogic(LayoutPoint(LayoutInt(0), ptNewCursor.GetY() - 1), &ptCaretPrevLog);
					}

					LogicPoint ptNewCursorLogic;
					m_pEditDoc->m_layoutMgr.LayoutToLogic(ptNewCursor, &ptNewCursorLogic);
					// 前の行と同じ物理行
					if (ptCaretPrevLog.y == ptNewCursorLogic.y) {
						// 1行先の物理行からレイアウト行を求める
						m_pEditDoc->m_layoutMgr.LogicToLayout(LogicPoint(0, GetCaret().GetCaretLogicPos().y + 1), &ptCaret);

						// カーソルを次の物理行頭へ移動する
						ptNewCursor = ptCaret;
					}
				}
			}
		}else {
			GetCaret().MoveCursorToClientPoint(ptMouse, true, &ptNewCursor);
		}
		GetSelectionInfo().m_ptMouseRollPosOld = ptMouse; // マウス範囲選択前回位置(XY座標)

		// CTRLキーが押されていたか
//		if (GetKeyState_Control()) {
		if (!GetSelectionInfo().m_bBeginWordSelect) {
			// 現在のカーソル位置によって選択範囲を変更
			GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptNewCursor);
			GetCaret().MoveCursor(ptNewCursor, true, 1000);
		}else {
			LayoutRange select;
			
			// 現在のカーソル位置によって選択範囲を変更(テストのみ)
			GetSelectionInfo().ChangeSelectAreaByCurrentCursorTEST(
				GetCaret().GetCaretLayoutPos(),
				&select
			);
			// 選択範囲に変更なし
			if (selectionOld == select) {
				GetSelectionInfo().ChangeSelectAreaByCurrentCursor(
					GetCaret().GetCaretLayoutPos()
				);
				GetCaret().MoveCursor(ptNewCursor, true, 1000);
				return;
			}
			LogicInt nLineLen;
			const Layout* pLayout;
			if (m_pEditDoc->m_layoutMgr.GetLineStr(GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout)) {
				LogicInt	nIdx = LineColumnToIndex(pLayout, GetCaret().GetCaretLayoutPos().GetX2());
				LayoutRange range;

				// 現在位置の単語の範囲を調べる
				bool bResult = m_pEditDoc->m_layoutMgr.WhereCurrentWord(
					GetCaret().GetCaretLayoutPos().GetY2(),
					nIdx,
					&range,
					NULL,
					NULL
				);
				if (bResult) {
					// 指定された行のデータ内の位置に対応する桁の位置を調べる
					// 2007.10.15 kobake 既にレイアウト単位なので変換は不要
					/*
					pLine     = m_pEditDoc->m_layoutMgr.GetLineStr(range.GetFrom().GetY2(), &nLineLen, &pLayout);
					range.SetFromX(LineIndexToColumn(pLayout, range.GetFrom().x));
					pLine     = m_pEditDoc->m_layoutMgr.GetLineStr(range.GetTo().GetY2(), &nLineLen, &pLayout);
					range.SetToX(LineIndexToColumn(pLayout, range.GetTo().x));
					*/
					int nWorkF = IsCurrentPositionSelectedTEST(
						range.GetFrom(), // カーソル位置
						select
					);
					int nWorkT = IsCurrentPositionSelectedTEST(
						range.GetTo(),	// カーソル位置
						select
					);
					if (nWorkF == -1) {
						// 始点が前方に移動。現在のカーソル位置によって選択範囲を変更
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor(range.GetFrom());
					}else if (nWorkT == 1) {
						// 終点が後方に移動。現在のカーソル位置によって選択範囲を変更
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor(range.GetTo());
					}else if (selectionOld.GetFrom() == select.GetFrom()) {
						// 始点が無変更＝前方に縮小された
						// 現在のカーソル位置によって選択範囲を変更
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor(range.GetTo());
					}else if (selectionOld.GetTo() == select.GetTo()) {
						// 終点が無変更＝後方に縮小された
						// 現在のカーソル位置によって選択範囲を変更
						GetSelectionInfo().ChangeSelectAreaByCurrentCursor(range.GetFrom());
					}
				}else {
					// 現在のカーソル位置によって選択範囲を変更
					GetSelectionInfo().ChangeSelectAreaByCurrentCursor(GetCaret().GetCaretLayoutPos());
				}
			}else {
				// 現在のカーソル位置によって選択範囲を変更
				GetSelectionInfo().ChangeSelectAreaByCurrentCursor(GetCaret().GetCaretLayoutPos());
			}
			GetCaret().MoveCursor(ptNewCursor, true, 1000);
		}
	}
	return;
}
//m_dwTipTimerm_dwTipTimerm_dwTipTimer


#ifndef SPI_GETWHEELSCROLLCHARS
#define SPI_GETWHEELSCROLLCHARS 0x006C
#endif


/* マウスホイールのメッセージ処理
	2009.01.17 nasukoji	ホイールスクロールを利用したページスクロール・横スクロール対応
	2011.11.16 Moca スクロール変化量への対応
	2013.09.10 Moca スペシャルスクロールの不具合の修正
*/
LRESULT EditView::OnMOUSEWHEEL2(
	WPARAM wParam,
	LPARAM lParam,
	bool bHorizontalMsg,
	EFunctionCode nCmdFuncID
	)
{
//	WORD	fwKeys;
	short	zDelta;
//	short	xPos;
//	short	yPos;
	int		i;
	int		nScrollCode;
	int		nRollLineNum;

//	fwKeys = LOWORD(wParam);			// key flags
	zDelta = (short) HIWORD(wParam);	// wheel rotation
//	xPos = (short) LOWORD(lParam);		// horizontal position of pointer
//	yPos = (short) HIWORD(lParam);		// vertical position of pointer
//	MYTRACE(_T("EditView::DispatchEvent() WM_MOUSEWHEEL fwKeys=%xh zDelta=%d xPos=%d yPos=%d \n"), fwKeys, zDelta, xPos, yPos);

	if (bHorizontalMsg) {
		if (0 < zDelta) {
			nScrollCode = SB_LINEDOWN; // 右
		}else {
			nScrollCode = SB_LINEUP; // 左
		}
		zDelta *= -1; // 反対にする
	}else {
		if (0 < zDelta) {
			nScrollCode = SB_LINEUP;
		}else {
			nScrollCode = SB_LINEDOWN;
		}
	}

	{
		// 2009.01.17 nasukoji	キー/マウスボタン + ホイールスクロールで横スクロールする
		bool bHorizontal = false;
		bool bKeyPageScroll = false;
		if (nCmdFuncID == F_0) {
			// 通常スクロールの時だけ適用
			bHorizontal = IsSpecialScrollMode(GetDllShareData().common.general.nHorizontalScrollByWheel);
			bKeyPageScroll = IsSpecialScrollMode(GetDllShareData().common.general.nPageScrollByWheel);
		}

		// 2013.05.30 Moca ホイールスクロールにキー割り当て
		int nIdx = GetCtrlKeyState();
		EFunctionCode nFuncID = nCmdFuncID;
		if (nFuncID != F_0) {
		}else if (bHorizontalMsg) {
			if (nScrollCode == SB_LINEUP) {
				nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::WheelLeft].nFuncCodeArr[nIdx];
			}else {
				nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::WheelRight].nFuncCodeArr[nIdx];
			}
		}else {
			if (nScrollCode == SB_LINEUP) {
				nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::WheelUp].nFuncCodeArr[nIdx];
			}else {
				nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::WheelDown].nFuncCodeArr[nIdx];
			}
		}
		bool bExecCmd = false;
		{
			if (nFuncID < F_WHEEL_FIRST || F_WHEEL_LAST < nFuncID) {
				bExecCmd = true;
			}
			if (nFuncID == F_WHEELLEFT || nFuncID == F_WHEELRIGHT
				|| nFuncID == F_WHEELPAGELEFT ||  nFuncID == F_WHEELPAGERIGHT
			) {
				bHorizontal = true;
			}
			if (nFuncID == F_WHEELPAGEUP || nFuncID == F_WHEELPAGEDOWN
				|| nFuncID == F_WHEELPAGELEFT ||  nFuncID == F_WHEELPAGERIGHT
			) {
				bKeyPageScroll = true;
			}
			if (nFuncID == F_WHEELUP || nFuncID == F_WHEELLEFT
				|| nFuncID == F_WHEELPAGEUP || nFuncID == F_WHEELPAGELEFT
			) {
				if (nScrollCode != SB_LINEUP) {
					zDelta *= -1;
					nScrollCode = SB_LINEUP;
				}
			}else if (nFuncID == F_WHEELDOWN || nFuncID == F_WHEELRIGHT
				|| nFuncID == F_WHEELPAGEDOWN || nFuncID == F_WHEELPAGERIGHT
			) {
				if (nScrollCode != SB_LINEDOWN) {
					zDelta *= -1;
					nScrollCode = SB_LINEDOWN;
				}
			}
		}

		// マウスホイールによるスクロール行数をレジストリから取得
		nRollLineNum = 3;

		// レジストリの存在チェック
		// 2006.06.03 Moca ReadRegistry に書き換え
		unsigned int uDataLen;	// size of value data
		TCHAR szValStr[256];
		uDataLen = _countof(szValStr) - 1;
		if (!bExecCmd) {
			bool bGetParam = false;
			if (bHorizontal) {
				int nScrollChars = 3;
				if (::SystemParametersInfo(SPI_GETWHEELSCROLLCHARS, 0, &nScrollChars, 0)) {
					bGetParam = true;
					nRollLineNum = nScrollChars;
					if (nRollLineNum != -1 && m_bMiniMap) {
						nRollLineNum *= 10;
					}
				}
			}
			if (!bGetParam) {
				if (ReadRegistry(HKEY_CURRENT_USER, _T("Control Panel\\desktop"), _T("WheelScrollLines"), szValStr, uDataLen)) {
					nRollLineNum = ::_ttoi(szValStr);
					if (nRollLineNum != -1 && m_bMiniMap) {
						nRollLineNum *= 10;
					}
				}
			}
		}

		if (nRollLineNum == -1 || bKeyPageScroll) {
			// 「1画面分スクロールする」
			if (bHorizontal) {
				nRollLineNum = (Int)GetTextArea().m_nViewColNum - 1;	// 表示域の桁数
			}else {
				nRollLineNum = (Int)GetTextArea().m_nViewRowNum - 1;	// 表示域の行数
			}
		}else {
			if (nRollLineNum > 30) {	//@@@ YAZAKI 2001.12.31 10→30へ。
				nRollLineNum = 30;
			}
		}
		if (nRollLineNum < 1 || bExecCmd) {
			nRollLineNum = 1;
		}

		// スクロール操作の種類(通常方法のページスクロールはNORMAL扱い)
		if (bKeyPageScroll) {
			if (bHorizontal) {
				// ホイール操作による横スクロールあり
				m_pEditWnd->SetHScrollByWheel(TRUE);
			}
			// ホイール操作によるページスクロールあり
			m_pEditWnd->SetPageScrollByWheel(TRUE);
		}else {
			if (bHorizontal) {
				// ホイール操作による横スクロールあり
				m_pEditWnd->SetHScrollByWheel(TRUE);
			}
		}

		if (0
			|| nFuncID != m_eWheelScroll
			|| (zDelta < 0 && 0 < m_nWheelDelta)
			|| (0 < zDelta && m_nWheelDelta < 0)
		) {
			m_nWheelDelta = 0;
			m_eWheelScroll = nFuncID;
		}
		m_nWheelDelta += zDelta;

		// 2011.05.18 APIのスクロール量に従う
		int nRollNum = abs(m_nWheelDelta) * nRollLineNum / 120;
		// 次回持越しの変化量(上記式Deltaのあまり。スクロール方向とzDeltaは符号が反対)
		m_nWheelDelta = (abs(m_nWheelDelta) - nRollNum * 120 / nRollLineNum) * ((nScrollCode == SB_LINEUP) ? 1 : -1);

		if (bExecCmd) {
			if (nFuncID != F_0) {
				// スクロール変化量分コマンド実行(zDeltaが120あたりで1回)
				for (int i=0; i<nRollNum; ++i) {
					::PostMessage(::GetParent(m_hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE),  (LPARAM)NULL);
				}
			}
			return bHorizontalMsg ? TRUE: 0;
		}

		const bool bSmooth = !! GetDllShareData().common.general.nRepeatedScroll_Smooth;
		const int nRollActions = bSmooth ? nRollNum : 1;
		const LayoutInt nCount = LayoutInt(((nScrollCode == SB_LINEUP) ? -1 : 1) * (bSmooth ? 1 : nRollNum));

		for (i=0; i<nRollActions; ++i) {
			//	Sep. 11, 2004 genta 同期スクロール行数
			if (bHorizontal) {
				SyncScrollH(ScrollAtH(GetTextArea().GetViewLeftCol() + nCount));
			}else {
				SyncScrollV(ScrollAtV(GetTextArea().GetViewTopLine() + nCount));
			}
		}
	}
	return bHorizontalMsg ? TRUE: 0;
}


// 垂直マウススクロール
LRESULT EditView::OnMOUSEWHEEL(WPARAM wParam, LPARAM lParam)
{
	return OnMOUSEWHEEL2(wParam, lParam, false, F_0);
}

/*! 水平マウススクロール
	@note http://msdn.microsoft.com/en-us/library/ms997498.aspx
	Best Practices for Supporting Microsoft Mouse and Keyboard Devices
	によると、WM_MOUSEHWHEELを処理した場合はTRUEを返す必要があるそうです。
	MSDNのWM_MOUSEHWHEEL Messageのページは間違っているので注意。
*/
LRESULT EditView::OnMOUSEHWHEEL(WPARAM wParam, LPARAM lParam)
{
	return OnMOUSEWHEEL2(wParam, lParam, true, F_0);
}

/*!
	@brief キー・マウスボタン状態よりスクロールモードを判定する

	マウスホイール時、行スクロールすべきかページスクロール・横スクロール
	すべきかを判定する。
	現在のキーまたはマウス状態が引数で指定された組み合わせに合致する場合
	trueを返す。

	@param nSelect	[in] キー・マウスボタンの組み合わせ指定番号

	@return ページスクロールまたは横スクロールすべき状態の時trueを返す
	        通常の行スクロールすべき状態の時falseを返す

	@date 2009.01.17 nasukoji	新規作成
*/
bool EditView::IsSpecialScrollMode(int nSelect)
{
	bool bSpecialScrollMode;

	switch (nSelect) {
	case 0:		// 指定の組み合わせなし
		bSpecialScrollMode = false;
		break;

	case MouseFunctionType::CenterClick:		// マウス中ボタン
		bSpecialScrollMode = ((::GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0);
		break;

	case MouseFunctionType::LeftSideClick:	// マウスサイドボタン1
		bSpecialScrollMode = ((::GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0);
		break;

	case MouseFunctionType::RightSideClick:	// マウスサイドボタン2
		bSpecialScrollMode = ((::GetAsyncKeyState(VK_XBUTTON2) & 0x8000) != 0);
		break;

	case VK_CONTROL:	// Controlキー
		bSpecialScrollMode = GetKeyState_Control();
		break;

	case VK_SHIFT:		// Shiftキー
		bSpecialScrollMode = GetKeyState_Shift();
		break;

	default:	// 上記以外（ここには来ない）
		bSpecialScrollMode = false;
		break;
	}

	return bSpecialScrollMode;
}


// マウス左ボタン開放のメッセージ処理
void EditView::OnLBUTTONUP(WPARAM fwKeys, int xPos , int yPos)
{
//	MYTRACE(_T("OnLBUTTONUP()\n"));

	// 範囲選択終了 & マウスキャプチャーおわり
	if (GetSelectionInfo().IsMouseSelecting()) {	// 範囲選択中
		// マウス キャプチャを解放
		::ReleaseCapture();
		GetCaret().ShowCaret_(GetHwnd()); // 2002/07/22 novice

		GetSelectionInfo().SelectEnd();

		// 20100715 Moca マウスクリック座標をリセット
		m_mouseDownPos.Set(-INT_MAX, -INT_MAX);

		GetCaret().m_underLine.UnderLineUnLock();
		if (GetSelectionInfo().m_select.IsOne()) {
			// 現在の選択範囲を非選択状態に戻す
			GetSelectionInfo().DisableSelectArea(true);
		}
	}
	if (m_bMiniMapMouseDown) {
		m_bMiniMapMouseDown = false;
		::ReleaseCapture();
	}
	return;
}

// ShellExecuteを呼び出すプロシージャ
//   呼び出し前に lpParameter を new しておくこと
static unsigned __stdcall ShellExecuteProc(LPVOID lpParameter)
{
	LPTSTR pszFile = (LPTSTR)lpParameter;
	::ShellExecute(NULL, _T("open"), pszFile, NULL, NULL, SW_SHOW);
	delete []pszFile;
	return 0;
}


// マウス左ボタンダブルクリック
// 2007.01.18 kobake IsCurrentPositionURL仕様変更に伴い、処理の書き換え
void EditView::OnLBUTTONDBLCLK(WPARAM fwKeys, int _xPos , int _yPos)
{
	Point ptMouse(_xPos, _yPos);

	LogicRange		cUrlRange;	// URL範囲
	std::wstring	wstrURL;
	const wchar_t*	pszMailTo = L"mailto:";

	// 2007.10.06 nasukoji	クアドラプルクリック時はチェックしない
	if (! m_dwTripleClickCheck) {
		// カーソル位置にURLが有る場合のその範囲を調べる
		if (
			IsCurrentPositionURL(
				GetCaret().GetCaretLayoutPos(),	// カーソル位置
				&cUrlRange,				// URL範囲
				&wstrURL				// URL受け取り先
			)
		) {
			std::wstring wstrOPEN;

			// URLを開く
		 	// 現在位置がメールアドレスならば、NULL以外と、その長さを返す
			if (IsMailAddress(wstrURL.c_str(), wstrURL.length(), NULL)) {
				wstrOPEN = pszMailTo + wstrURL;
			}else {
				if (wcsnicmp(wstrURL.c_str(), L"ttp://", 6) == 0) {	// 抑止URL
					wstrOPEN = L"h" + wstrURL;
				}else if (wcsnicmp(wstrURL.c_str(), L"tp://", 5) == 0) {	// 抑止URL
					wstrOPEN = L"ht" + wstrURL;
				}else {
					wstrOPEN = wstrURL;
				}
			}
			{
				// URLを開く
				// 2009.05.21 syat UNCパスだと1分以上無応答になることがあるのでスレッド化
				WaitCursor waitCursor(GetHwnd());	// カーソルを砂時計にする

				unsigned int nThreadId;
				LPCTSTR szUrl = to_tchar(wstrOPEN.c_str());
				LPTSTR pszUrlDup = new TCHAR[_tcslen(szUrl) + 1];
				_tcscpy(pszUrlDup, szUrl);
				HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ShellExecuteProc, (LPVOID)pszUrlDup, 0, &nThreadId);
				if (hThread != INVALID_HANDLE_VALUE) {
					// ユーザーのURL起動指示に反応した目印としてちょっとの時間だけ砂時計カーソルを表示しておく
					// ShellExecute は即座にエラー終了することがちょくちょくあるので WaitForSingleObject ではなく Sleep を使用（ex.存在しないパスの起動）
					// 【補足】いずれの API でも待ちを長め（2～3秒）にするとなぜか Web ブラウザ未起動からの起動が重くなる模様（PCタイプ, XP/Vista, IE/FireFox に関係なく）
					::Sleep(200);
					::CloseHandle(hThread);
				}else {
					// スレッド作成失敗
					delete[] pszUrlDup;
				}
			}
			return;
		}

		// GREP出力モードまたはデバッグモード かつ マウス左ボタンダブルクリックでタグジャンプ の場合
		//	2004.09.20 naoh 外部コマンドの出力からTagjumpできるように
		if (1
			&& (EditApp::getInstance().m_pGrepAgent->m_bGrepMode || AppMode::getInstance().IsDebugMode())
			&& GetDllShareData().common.search.bGTJW_DoubleClick
		) {
			// タグジャンプ機能
			if (GetCommander().Command_TAGJUMP()) {
				// 2013.05.27 タグジャンプ失敗時は通常の処理を実行する
				return;
			}
		}
	}

// novice 2004/10/10
	// Shift,Ctrl,Altキーが押されていたか
	int	nIdx = GetCtrlKeyState();

	// マウス左クリックに対応する機能コードはm_common.m_pKeyNameArr[?]に入っている 2007.11.15 nasukoji
	EFunctionCode nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[
		(int)(m_dwTripleClickCheck ? MouseFunctionType::QuadrapleClick : MouseFunctionType::DoubleClick)
	].nFuncCodeArr[nIdx];
	if (m_dwTripleClickCheck) {
		// 非選択状態にした後左クリックしたことにする
		// すべて選択の場合は、3.5クリック時の選択状態保持とドラッグ開始時の
		// 範囲変更のため。
		// クアドラプルクリック機能が割り当てられていない場合は、ダブルクリック
		// として処理するため。
		if (GetSelectionInfo().IsTextSelected())		// テキストが選択されているか
			GetSelectionInfo().DisableSelectArea(true);		// 現在の選択範囲を非選択状態に戻す

		if (!nFuncID) {
			m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
			nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::DoubleClick].nFuncCodeArr[nIdx];
			OnLBUTTONDOWN(fwKeys, ptMouse.x , ptMouse.y);	// カーソルをクリック位置へ移動する
		}
	}

	if (nFuncID != 0) {
		// コマンドコードによる処理振り分け
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::SendMessage(::GetParent(m_hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE),  (LPARAM)NULL);
	}

	// 2007.10.06 nasukoji	クアドラプルクリック時もここで抜ける
	if (m_dwTripleClickCheck) {
		m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF（次回は通常クリック）
		return;
	}

	// 2007.11.06 nasukoji	ダブルクリックが単語選択でなくてもトリプルクリックを有効とする
	// 2007.10.02 nasukoji	トリプルクリックチェック用に時刻を取得
	m_dwTripleClickCheck = ::GetTickCount();

	// ダブルクリック位置として記憶
	GetSelectionInfo().m_ptMouseRollPosOld = ptMouse;	// マウス範囲選択前回位置(XY座標)

	/*	2007.07.09 maru 機能コードの判定を追加
		ダブルクリックからのドラッグでは単語単位の範囲選択(エディタの一般的動作)になるが
		この動作は、ダブルクリック＝単語選択を前提としたもの。
		キー割り当ての変更により、ダブルクリック≠単語選択のときには GetSelectionInfo().m_bBeginWordSelect = true
		にすると、処理の内容によっては表示がおかしくなるので、ここで抜けるようにする。
	*/
	if (F_SELECTWORD != nFuncID) return;

	// 範囲選択開始 & マウスキャプチャー
	GetSelectionInfo().SelectBeginWord();

	if (GetDllShareData().common.view.bFontIs_FixedPitch) {	// 現在のフォントは固定幅フォントである
		// ALTキーが押されていたか
		if (GetKeyState_Alt()) {
			GetSelectionInfo().SetBoxSelect(true);	// 矩形範囲選択中
		}
	}
	::SetCapture(GetHwnd());
	GetCaret().HideCaret_(GetHwnd()); // 2002/07/22 novice
	if (GetSelectionInfo().IsTextSelected()) {
		// 常時選択範囲の範囲
		GetSelectionInfo().m_selectBgn.SetTo(GetSelectionInfo().m_select.GetTo());
	}else {
		// 現在のカーソル位置から選択を開始する
		GetSelectionInfo().BeginSelectArea();
	}

	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           D&D                               //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

STDMETHODIMP EditView::DragEnter(
	LPDATAOBJECT pDataObject,
	DWORD dwKeyState,
	POINTL pt,
	LPDWORD pdwEffect
	)
{
	DEBUG_TRACE(_T("EditView::DragEnter()\n"));
	//「OLEによるドラッグ & ドロップを使う」オプションが無効の場合にはドロップを受け付けない
	if (!GetDllShareData().common.edit.bUseOLE_DragDrop) return E_UNEXPECTED;

	// 編集禁止の場合はドロップを受け付けない
	if (!m_pEditDoc->IsEditable()) return E_UNEXPECTED;


	if (!pDataObject || !pdwEffect)
		return E_INVALIDARG;

	m_cfDragData = GetAvailableClipFormat(pDataObject);
	if (m_cfDragData == 0)
		return E_INVALIDARG;
	else if (m_cfDragData == CF_HDROP) {
		// 右ボタンで入ってきたときだけファイルをビューで取り扱う
		if (!(MK_RBUTTON & dwKeyState))
			return E_INVALIDARG;
	}
	
	// 自分をアクティブペインにする
	m_pEditWnd->SetActivePane(m_nMyIndex);

	// 現在のカーソル位置を記憶する	// 2007.12.09 ryoji
	m_ptCaretPos_DragEnter = GetCaret().GetCaretLayoutPos();
	m_nCaretPosX_Prev_DragEnter = GetCaret().m_nCaretPosX_Prev;

	// ドラッグデータは矩形か
	m_bDragBoxData = IsDataAvailable(pDataObject, (CLIPFORMAT)::RegisterClipboardFormat(_T("MSDEVColumnSelect")));

	// 選択テキストのドラッグ中か
	_SetDragMode(TRUE);

	DragOver(dwKeyState, pt, pdwEffect);
	return S_OK;
}

STDMETHODIMP EditView::DragOver(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
	DEBUG_TRACE(_T("EditView::DragOver()\n"));

	// マウス移動のメッセージ処理
	::ScreenToClient(GetHwnd(), (LPPOINT)&pt);
	OnMOUSEMOVE(dwKeyState, pt.x , pt.y);

	if (!pdwEffect)
		return E_INVALIDARG;

	*pdwEffect = TranslateDropEffect(m_cfDragData, dwKeyState, pt, *pdwEffect);

	EditView* pDragSourceView = m_pEditWnd->GetDragSourceView();

	// ドラッグ元が他ビューで、このビューのカーソルがドラッグ元の選択範囲内の場合は禁止マークにする
	// ※自ビューのときは禁止マークにしない（他アプリでも多くはそうなっている模様）	// 2009.06.09 ryoji
	if (pDragSourceView && !IsDragSource() &&
		!pDragSourceView->IsCurrentPositionSelected(GetCaret().GetCaretLayoutPos())
	) {
		*pdwEffect = DROPEFFECT_NONE;
	}

	return S_OK;
}

STDMETHODIMP EditView::DragLeave(void)
{
	DEBUG_TRACE(_T("EditView::DragLeave()\n"));
	// 選択テキストのドラッグ中か
	_SetDragMode(FALSE);

	// DragEnter時のカーソル位置を復元	// 2007.12.09 ryoji
	// ※範囲選択中のときに選択範囲とカーソルが分離すると変だから
	GetCaret().MoveCursor(m_ptCaretPos_DragEnter, false);
	GetCaret().m_nCaretPosX_Prev = m_nCaretPosX_Prev_DragEnter;
	RedrawAll();	// ルーラー、アンダーライン、カーソル位置表示更新

	// 非アクティブ時は表示状態を非アクティブに戻す	// 2007.12.09 ryoji
	if (!::GetActiveWindow())
		OnKillFocus();

	return S_OK;
}

STDMETHODIMP EditView::Drop(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
	DEBUG_TRACE(_T("EditView::Drop()\n"));
	BOOL		bBoxData;
	BOOL		bMove;
	bool		bMoveToPrev = false;
	RECT		rcSel;
	NativeW		memBuf;
	bool		bBeginBoxSelect_Old = false;

	LayoutRange selectBgn_Old;
	LayoutRange select_Old;

	// 選択テキストのドラッグ中か
	_SetDragMode(FALSE);

	// 非アクティブ時は表示状態を非アクティブに戻す	// 2007.12.09 ryoji
	if (!::GetActiveWindow())
		OnKillFocus();

	if (!pDataObject || !pdwEffect)
		return E_INVALIDARG;

	CLIPFORMAT cf;
	cf = GetAvailableClipFormat(pDataObject);
	if (cf == 0)
		return E_INVALIDARG;

	*pdwEffect = TranslateDropEffect(cf, dwKeyState, pt, *pdwEffect);
	if (*pdwEffect == DROPEFFECT_NONE)
		return E_INVALIDARG;

	// ファイルドロップは PostMyDropFiles() で処理する
	if (cf == CF_HDROP)
		return PostMyDropFiles(pDataObject);

	// 外部からのドロップは以後の処理ではコピーと同様に扱う
	EditView* pDragSourceView = m_pEditWnd->GetDragSourceView();
	bMove = (*pdwEffect == DROPEFFECT_MOVE) && pDragSourceView;
	bBoxData = m_bDragBoxData;

	// カーソルが選択範囲内にあるときはコピー／移動しない	// 2009.06.09 ryoji
	if (pDragSourceView &&
		!pDragSourceView->IsCurrentPositionSelected(GetCaret().GetCaretLayoutPos())
	) {
		// DragEnter時のカーソル位置を復元
		// Note. ドラッグ元が他ビューでもマウス移動が速いと稀にここにくる可能性がありそう
		*pdwEffect = DROPEFFECT_NONE;
		GetCaret().MoveCursor(m_ptCaretPos_DragEnter, false);
		GetCaret().m_nCaretPosX_Prev = m_nCaretPosX_Prev_DragEnter;
		if (!IsDragSource())	// ドラッグ元の場合はここでは再描画不要（DragDrop後処理のSetActivePaneで再描画される）
			RedrawAll();	// ←主に以後の非アクティブ化に伴うアンダーライン消しのために一度更新して整合をとる
		return S_OK;
	}

	// ドロップデータの取得
	HGLOBAL hData = GetGlobalData(pDataObject, cf);
	if (!hData)
		return E_INVALIDARG;
	LPVOID pData = ::GlobalLock(hData);
	SIZE_T nSize = ::GlobalSize(hData);
	if (cf == Clipboard::GetSakuraFormat()) {
		if (nSize > sizeof(int)) {
			wchar_t* pszData = (wchar_t*)((BYTE*)pData + sizeof(int));
			memBuf.SetString(pszData, t_min((SIZE_T)*(int*)pData, nSize / sizeof(wchar_t)));	// 途中のNUL文字も含める
		}
	}else if (cf == CF_UNICODETEXT) {
		memBuf.SetString((wchar_t*)pData, wcsnlen((wchar_t*)pData, nSize / sizeof(wchar_t)));
	}else {
		memBuf.SetStringOld((char*)pData, strnlen((char*)pData, nSize / sizeof(char)));
	}

	// アンドゥバッファの準備
	if (!m_commander.GetOpeBlk()) {
		m_commander.SetOpeBlk(new OpeBlk);
	}
	m_commander.GetOpeBlk()->AddRef();

	// 移動の場合、位置関係を算出
	if (bMove) {
		if (bBoxData) {
			// 2点を対角とする矩形を求める
			TwoPointToRect(
				&rcSel,
				pDragSourceView->GetSelectionInfo().m_select.GetFrom(),	// 範囲選択開始
				pDragSourceView->GetSelectionInfo().m_select.GetTo()		// 範囲選択終了
			);
			++rcSel.bottom;
			if (GetCaret().GetCaretLayoutPos().GetY() >= rcSel.bottom) {
				bMoveToPrev = false;
			}else if (GetCaret().GetCaretLayoutPos().GetY() + rcSel.bottom - rcSel.top < rcSel.top) {
				bMoveToPrev = true;
			}else if (GetCaret().GetCaretLayoutPos().GetX2() < rcSel.left) {
				bMoveToPrev = true;
			}else {
				bMoveToPrev = false;
			}
		}else {
			if (pDragSourceView->GetSelectionInfo().m_select.GetFrom().y > GetCaret().GetCaretLayoutPos().GetY()) {
				bMoveToPrev = true;
			}else if (pDragSourceView->GetSelectionInfo().m_select.GetFrom().y == GetCaret().GetCaretLayoutPos().GetY()) {
				if (pDragSourceView->GetSelectionInfo().m_select.GetFrom().x > GetCaret().GetCaretLayoutPos().GetX2()) {
					bMoveToPrev = true;
				}else {
					bMoveToPrev = false;
				}
			}else {
				bMoveToPrev = false;
			}
		}
	}

	LayoutPoint ptCaretPos_Old = GetCaret().GetCaretLayoutPos();
	if (!bMove) {
		// コピーモード
		// 現在の選択範囲を非選択状態に戻す
		GetSelectionInfo().DisableSelectArea(true);
	}else {
		bBeginBoxSelect_Old = pDragSourceView->GetSelectionInfo().IsBoxSelecting();
		selectBgn_Old = pDragSourceView->GetSelectionInfo().m_selectBgn;
		select_Old = pDragSourceView->GetSelectionInfo().m_select;
		if (bMoveToPrev) {
			// 移動モード & 前に移動
			// 選択エリアを削除
			if (this != pDragSourceView) {
				pDragSourceView->GetSelectionInfo().DisableSelectArea(true);
				GetSelectionInfo().DisableSelectArea(true);
				GetSelectionInfo().SetBoxSelect(bBeginBoxSelect_Old);
				GetSelectionInfo().m_selectBgn = selectBgn_Old;
				GetSelectionInfo().m_select = select_Old;
			}
			DeleteData(true);
			GetCaret().MoveCursor(ptCaretPos_Old, true);
		}else {
			// 現在の選択範囲を非選択状態に戻す
			pDragSourceView->GetSelectionInfo().DisableSelectArea(true);
			if (this != pDragSourceView)
				GetSelectionInfo().DisableSelectArea(true);
		}
	}
	if (!bBoxData) {	// 矩形データ
		//	2004,05.14 Moca 引数に文字列長を追加

		// 挿入前のキャレット位置を記憶する
		// （キャレットが行終端より右の場合は埋め込まれる空白分だけ桁位置をシフト）
		LogicPoint ptCaretLogicPos_Old = GetCaret().GetCaretLogicPos();
		const Layout* pLayout;
		LogicInt nLineLen;
		LayoutPoint ptCaretLayoutPos_Old = GetCaret().GetCaretLayoutPos();
		if (m_pEditDoc->m_layoutMgr.GetLineStr(ptCaretLayoutPos_Old.GetY2(), &nLineLen, &pLayout)) {
			LayoutInt nLineAllColLen;
			LineColumnToIndex2(pLayout, ptCaretLayoutPos_Old.GetX2(), &nLineAllColLen);
			if (nLineAllColLen > LayoutInt(0)) {	// 行終端より右の場合には nLineAllColLen に行全体の表示桁数が入っている
				ptCaretLogicPos_Old.SetX(
					ptCaretLogicPos_Old.GetX2()
					+ (Int)(ptCaretLayoutPos_Old.GetX2() - nLineAllColLen)
				);
			}
		}

		GetCommander().Command_INSTEXT(true, memBuf.GetStringPtr(), memBuf.GetStringLength(), FALSE);

		// 挿入前のキャレット位置から挿入後のキャレット位置までを選択範囲にする
		LayoutPoint ptSelectFrom;
		m_pEditDoc->m_layoutMgr.LogicToLayout(
			ptCaretLogicPos_Old,
			&ptSelectFrom
		);
		GetSelectionInfo().SetSelectArea(LayoutRange(ptSelectFrom, GetCaret().GetCaretLayoutPos()));	// 2009.07.25 ryoji
	}else {
		// 2004.07.12 Moca クリップボードを書き換えないように
		// bBoxSelected == TRUE
		// GetSelectionInfo().IsBoxSelecting() == FALSE
		// 貼り付け（クリップボードから貼り付け）
		GetCommander().Command_PASTEBOX(memBuf.GetStringPtr(), memBuf.GetStringLength());
		AdjustScrollBars(); // 2007.07.22 ryoji
		Redraw();
	}
	if (bMove) {
		if (bMoveToPrev) {
		}else {
			// 移動モード & 後ろに移動

			// 現在の選択範囲を記憶する	// 2008.03.26 ryoji
			LogicRange selLogic;
			m_pEditDoc->m_layoutMgr.LayoutToLogic(
				GetSelectionInfo().m_select,
				&selLogic
			);

			// 以前の選択範囲を記憶する	// 2008.03.26 ryoji
			LogicRange delLogic;
			m_pEditDoc->m_layoutMgr.LayoutToLogic(
				select_Old,
				&delLogic
			);

			// 現在の行数を記憶する	// 2008.03.26 ryoji
			int nLines_Old = m_pEditDoc->m_docLineMgr.GetLineCount();

			// 以前の選択範囲を選択する
			GetSelectionInfo().SetBoxSelect(bBeginBoxSelect_Old);
			GetSelectionInfo().m_selectBgn = selectBgn_Old;
			GetSelectionInfo().m_select = select_Old;

			// 選択エリアを削除
			DeleteData(true);

			// 削除前の選択範囲を復元する	// 2008.03.26 ryoji
			if (!bBoxData) {
				// 削除された範囲を考慮して選択範囲を調整する
				if (selLogic.GetFrom().GetY2() == delLogic.GetTo().GetY2()) {	// 選択開始が削除末尾と同一行
					selLogic.SetFromX(
						selLogic.GetFrom().GetX2()
						- (delLogic.GetTo().GetX2() - delLogic.GetFrom().GetX2())
					);
				}
				if (selLogic.GetTo().GetY2() == delLogic.GetTo().GetY2()) {	// 選択終了が削除末尾と同一行
					selLogic.SetToX(
						selLogic.GetTo().GetX2()
						- (delLogic.GetTo().GetX2() - delLogic.GetFrom().GetX2())
					);
				}
				// Note.
				// (delLogic.GetTo().GetY2() - delLogic.GetFrom().GetY2()) だと実際の削除行数と同じになる
				// こともあるが、（削除行数－１）になることもある．
				// 例）フリーカーソルでの行番号クリック時の１行選択
				int nLines = m_pEditDoc->m_docLineMgr.GetLineCount();
				selLogic.SetFromY(selLogic.GetFrom().GetY2() - (nLines_Old - nLines));
				selLogic.SetToY(selLogic.GetTo().GetY2() - (nLines_Old - nLines));

				// 調整後の選択範囲を設定する
				LayoutRange select;
				m_pEditDoc->m_layoutMgr.LogicToLayout(
					selLogic,
					&select
				);
				GetSelectionInfo().SetSelectArea(select);	// 2009.07.25 ryoji
				ptCaretPos_Old = GetSelectionInfo().m_select.GetTo();
			}

			// キャレットを移動する
			GetCaret().MoveCursor(ptCaretPos_Old, true);
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			// 削除位置から移動先へのカーソル移動をUndo操作に追加する	// 2008.03.26 ryoji
			LogicPoint ptBefore;
			m_pEditDoc->m_layoutMgr.LayoutToLogic(
				GetSelectionInfo().m_select.GetFrom(),
				&ptBefore
			);
			m_commander.GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					delLogic.GetFrom(),
					GetCaret().GetCaretLogicPos()
				)
			);
		}
	}
	GetSelectionInfo().DrawSelectArea();

	// アンドゥバッファの処理
	SetUndoBuffer();

	::GlobalUnlock(hData);
	// 2004.07.12 fotomo/もか メモリーリークの修正
	if ((GMEM_LOCKCOUNT & ::GlobalFlags(hData)) == 0) {
		::GlobalFree(hData);
	}

	return S_OK;
}


/** 独自ドロップファイルメッセージをポストする
	@date 2008.06.20 ryoji 新規作成
*/
STDMETHODIMP EditView::PostMyDropFiles(LPDATAOBJECT pDataObject)
{
	HGLOBAL hData = GetGlobalData(pDataObject, CF_HDROP);
	if (!hData)
		return E_INVALIDARG;
	LPVOID pData = ::GlobalLock(hData);
	SIZE_T nSize = ::GlobalSize(hData);

	// ドロップデータをコピーしてあとで独自のドロップファイル処理を行う
	HGLOBAL hDrop = ::GlobalAlloc(GHND | GMEM_DDESHARE, nSize);
	memcpy_raw(::GlobalLock(hDrop), pData, nSize);
	::GlobalUnlock(hDrop);
	::PostMessage(
		GetHwnd(),
		MYWM_DROPFILES,
		(WPARAM)hDrop,
		0
	);

	::GlobalUnlock(hData);
	if ((GMEM_LOCKCOUNT & ::GlobalFlags(hData)) == 0) {
		::GlobalFree(hData);
	}

	return S_OK;
}

/** 独自ドロップファイルメッセージ処理
	@date 2008.06.20 ryoji 新規作成
*/
void EditView::OnMyDropFiles(HDROP hDrop)
{
	// 普通にメニュー操作ができるように入力状態をフォアグランドウィンドウにアタッチする
	int nTid2 = ::GetWindowThreadProcessId(::GetForegroundWindow(), NULL);
	int nTid1 = ::GetCurrentThreadId();
	if (nTid1 != nTid2) ::AttachThreadInput(nTid1, nTid2, TRUE);
	
	// ダミーの STATIC を作ってフォーカスを当てる（エディタが前面に出ないように）
	HWND hwnd = ::CreateWindow(_T("STATIC"), _T(""), 0, 0, 0, 0, 0, NULL, NULL, G_AppInstance(), NULL);
	::SetFocus(hwnd);

	// メニューを作成する
	POINT pt;
	::GetCursorPos(&pt);
	RECT rcWork;
	GetMonitorWorkRect(pt, &rcWork);	// モニタのワークエリア
	HMENU hMenu = ::CreatePopupMenu();
	::InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, 100, LS(STR_VIEW_MOUSE_MENU_PATH));
	::InsertMenu(hMenu, 1, MF_BYPOSITION | MF_STRING, 101, LS(STR_VIEW_MOUSE_MENU_FILE));
	::InsertMenu(hMenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// セパレータ
	::InsertMenu(hMenu, 3, MF_BYPOSITION | MF_STRING, 110, LS(STR_VIEW_MOUSE_MENU_OPEN));
	::InsertMenu(hMenu, 4, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);	// セパレータ
	::InsertMenu(hMenu, 5, MF_BYPOSITION | MF_STRING, IDCANCEL, LS(STR_VIEW_MOUSE_MENU_CANCEL));
	int nId = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
									(pt.x > rcWork.left)? pt.x: rcWork.left,
									(pt.y < rcWork.bottom)? pt.y: rcWork.bottom,
								0, hwnd, NULL);
	::DestroyMenu(hMenu);

	::DestroyWindow(hwnd);

	// 入力状態をデタッチする
	if (nTid1 != nTid2) ::AttachThreadInput(nTid1, nTid2, FALSE);

	// 選択されたメニューに対応する処理を実行する
	switch (nId) {
	case 110:	// ファイルを開く
		// 通常のドロップファイル処理を行う
		::SendMessage(m_pEditWnd->GetHwnd(), WM_DROPFILES, (WPARAM)hDrop, 0);
		break;

	case 100:	// パス名を貼り付ける
	case 101:	// ファイル名を貼り付ける
		NativeW memBuf;
		UINT nFiles;
		TCHAR szPath[_MAX_PATH];
		TCHAR szExt[_MAX_EXT];
		TCHAR szWork[_MAX_PATH];

		nFiles = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		for (UINT i=0; i<nFiles; ++i) {
			::DragQueryFile(hDrop, i, szPath, sizeof(szPath)/sizeof(TCHAR));
			if (!::GetLongFileName(szPath, szWork))
				continue;
			if (nId == 100) {	// パス名
				::lstrcpy(szPath, szWork);
			}else if (nId == 101) {	// ファイル名
				_tsplitpath(szWork, NULL, NULL, szPath, szExt);
				::lstrcat(szPath, szExt);
			}
#ifdef _UNICODE
			memBuf.AppendString(szPath);
#else
			memBuf.AppendStringOld(szPath);
#endif
			if (nFiles > 1) {
				memBuf.AppendString(m_pEditDoc->m_docEditor.GetNewLineCode().GetValue2());
			}
		}
		::DragFinish(hDrop);

		// 選択範囲の選択解除
		if (GetSelectionInfo().IsTextSelected()) {
			GetSelectionInfo().DisableSelectArea(true);
		}

		// 挿入前のキャレット位置を記憶する
		// （キャレットが行終端より右の場合は埋め込まれる空白分だけ桁位置をシフト）
		LogicPoint ptCaretLogicPos_Old = GetCaret().GetCaretLogicPos();
		const Layout* pLayout;
		LogicInt nLineLen;
		LayoutPoint ptCaretLayoutPos_Old = GetCaret().GetCaretLayoutPos();
		if (m_pEditDoc->m_layoutMgr.GetLineStr(ptCaretLayoutPos_Old.GetY2(), &nLineLen, &pLayout)) {
			LayoutInt nLineAllColLen;
			LineColumnToIndex2(pLayout, ptCaretLayoutPos_Old.GetX2(), &nLineAllColLen);
			if (nLineAllColLen > LayoutInt(0)) {	// 行終端より右の場合には nLineAllColLen に行全体の表示桁数が入っている
				ptCaretLogicPos_Old.SetX(
					ptCaretLogicPos_Old.GetX2()
					+ (Int)(ptCaretLayoutPos_Old.GetX2() - nLineAllColLen)
				);
			}
		}

		// テキスト挿入
		GetCommander().HandleCommand(F_INSTEXT_W, true, (LPARAM)memBuf.GetStringPtr(), memBuf.GetStringLength(), TRUE, 0);

		// 挿入前のキャレット位置から挿入後のキャレット位置までを選択範囲にする
		LayoutPoint ptSelectFrom;
		m_pEditDoc->m_layoutMgr.LogicToLayout(
			ptCaretLogicPos_Old,
			&ptSelectFrom
		);
		GetSelectionInfo().SetSelectArea(LayoutRange(ptSelectFrom, GetCaret().GetCaretLayoutPos()));	// 2009.07.25 ryoji
		GetSelectionInfo().DrawSelectArea();
		break;
	}

	// メモリ解放
	::GlobalFree(hDrop);
}

CLIPFORMAT EditView::GetAvailableClipFormat(LPDATAOBJECT pDataObject)
{
	CLIPFORMAT cf = 0;
	CLIPFORMAT cfSAKURAClip = Clipboard::GetSakuraFormat();

	if (IsDataAvailable(pDataObject, cfSAKURAClip))
		cf = cfSAKURAClip;
	else if (IsDataAvailable(pDataObject, CF_UNICODETEXT))
		cf = CF_UNICODETEXT;
	else if (IsDataAvailable(pDataObject, CF_TEXT))
		cf = CF_TEXT;
	else if (IsDataAvailable(pDataObject, CF_HDROP))	// 2008.06.20 ryoji
		cf = CF_HDROP;

	return cf;
}

DWORD EditView::TranslateDropEffect(CLIPFORMAT cf, DWORD dwKeyState, POINTL pt, DWORD dwEffect)
{
	if (cf == CF_HDROP)	// 2008.06.20 ryoji
		return DROPEFFECT_LINK;

	EditView* pDragSourceView = m_pEditWnd->GetDragSourceView();

	// 2008.06.21 ryoji
	// Win 98/Me 環境では外部からのドラッグ時に GetKeyState() ではキー状態を正しく取得できないため、
	// Drag & Drop インターフェースで渡される dwKeyState を用いて判定する。
#if 1
	// ドラッグ元が外部ウィンドウかどうかによって受け方を変える
	// ※汎用テキストエディタではこちらが主流っぽい
	if (pDragSourceView) {
#else
	// ドラッグ元が移動を許すかどうかによって受け方を変える
	// ※MS 製品（MS Office, Visual Studioなど）ではこちらが主流っぽい
	if (dwEffect & DROPEFFECT_MOVE) {
#endif
		dwEffect &= (dwKeyState & MK_CONTROL)? DROPEFFECT_COPY: DROPEFFECT_MOVE;
	}else {
		dwEffect &= (dwKeyState & MK_SHIFT)? DROPEFFECT_MOVE: DROPEFFECT_COPY;
	}
	return dwEffect;
}

bool EditView::IsDragSource(void)
{
	return (this == m_pEditWnd->GetDragSourceView());
}


