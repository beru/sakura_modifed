// マウスイベントの処理

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

	if (bHokan) {
		editWnd.hokanMgr.Hide();
		bHokan = false;
	}

	if (nISearchMode > 0) {
		ISearchExit();
	}
	if (nAutoScrollMode) {
		AutoScrollExit();
	}
	if (bMiniMap) {
		::SetFocus(GetHwnd());
		::SetCapture(GetHwnd());
		bMiniMapMouseDown = true;
		OnMOUSEMOVE(fwKeys, _xPos, _yPos);
		return;
	}

	NativeW	memCurText;
	const wchar_t*	pLine;
	size_t			nLineLen;

	Range range;

	size_t		nIdx;
	int			nWork;
	int			nFuncID = 0;				// マウス左クリックに対応する機能コード

	if (pEditDoc->layoutMgr.GetLineCount() == 0) {
		return;
	}
	auto& caret = GetCaret();
	if (!caret.ExistCaretFocus()) { // フォーカスがないとき
		return;
	}

	// 辞書Tipが起動されている
	if (dwTipTimer == 0) {
		// 辞書Tipを消す
		tipWnd.Hide();
		dwTipTimer = ::GetTickCount();	// 辞書Tip起動タイマー
	}else {
		dwTipTimer = ::GetTickCount();		// 辞書Tip起動タイマー
	}

	// トリプルクリックであることを示す
	bool tripleClickMode = CheckTripleClick(ptMouse);
	if (tripleClickMode) {
		// マウス左トリプルクリックに対応する機能コードはcommon.pKeyNameArr[5]に入っている
		nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::TripleClick].nFuncCodeArr[GetCtrlKeyState()];
		if (nFuncID == 0) {
			tripleClickMode = false;	// 割り当て機能無しの時はトリプルクリック OFF
		}
	}else {
		dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
	}

	// 現在のマウスカーソル位置→レイアウト位置
	Point ptNew;
	auto& textArea = GetTextArea();
	textArea.ClientToLayout(ptMouse, &ptNew);

	// マウスダウン時の座標を覚えて利用する
	mouseDownPos = ptMouse;

	// OLEによるドラッグ & ドロップを使う
	// トリプルクリック時はドラッグを開始しない
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
					// カーソル移動。
					if (ptMouse.y >= textArea.GetAreaTop() && ptMouse.y < textArea.GetAreaBottom()) {
						if (ptMouse.x >= textArea.GetAreaLeft() && ptMouse.x < textArea.GetAreaRight()
						) {
							caret.MoveCursorToClientPoint(ptMouse);
						}else if (ptMouse.x < textArea.GetAreaLeft()) {
							caret.MoveCursorToClientPoint(Point(textArea.GetDocumentLeftClientPointX(), ptMouse.y));
						}
					}
					return;
				}
				// 選択範囲のデータを取得
				if (GetSelectedData(&memCurText, false, NULL, false, GetDllShareData().common.edit.bAddCRLFWhenCopy)) {
					DWORD dwEffects;
					DWORD dwEffectsSrc = (!pEditDoc->IsEditable()) ?
											DROPEFFECT_COPY: DROPEFFECT_COPY | DROPEFFECT_MOVE;
					int nOpe = pEditDoc->docEditor.opeBuf.GetCurrentPointer();
					editWnd.SetDragSourceView(this);
					DataObject data(memCurText.GetStringPtr(), memCurText.GetStringLength(), GetSelectionInfo().IsBoxSelecting());
					dwEffects = data.DragDrop(TRUE, dwEffectsSrc);
					editWnd.SetDragSourceView(nullptr);
					if (pEditDoc->docEditor.opeBuf.GetCurrentPointer() == nOpe) {	// ドキュメント変更なしか？
						editWnd.SetActivePane(nMyIndex);
						if ((dwEffectsSrc & dwEffects) == DROPEFFECT_MOVE) {
							// 移動範囲を削除する
							// ドロップ先が移動を処理したが自ドキュメントにここまで変更が無い
							// →ドロップ先は外部のウィンドウである
							if (!commander.GetOpeBlk()) {
								commander.SetOpeBlk(new OpeBlk);
							}
							commander.GetOpeBlk()->AddRef();

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

	// ALTキーが押されている、かつトリプルクリックでない
	if (GetKeyState_Alt() && !tripleClickMode) {
		if (GetSelectionInfo().IsTextSelected()) {	// テキストが選択されているか
			// 現在の選択範囲を非選択状態に戻す
			GetSelectionInfo().DisableSelectArea(true);
		}
		if (ptMouse.y >= textArea.GetAreaTop()  && ptMouse.y < textArea.GetAreaBottom()) {
			if (ptMouse.x >= textArea.GetAreaLeft() && ptMouse.x < textArea.GetAreaRight()) {
				caret.MoveCursorToClientPoint(ptMouse);
			}else if (ptMouse.x < textArea.GetAreaLeft()) {
				caret.MoveCursorToClientPoint(Point(textArea.GetDocumentLeftClientPointX(), ptMouse.y));
			}else {
				return;
			}
		}
		GetSelectionInfo().ptMouseRollPosOld = ptMouse;	// マウス範囲選択前回位置(XY座標)

		// 範囲選択開始 & マウスキャプチャー
		GetSelectionInfo().SelectBeginBox();

		::SetCapture(GetHwnd());
		caret.HideCaret_(GetHwnd());
		// 現在のカーソル位置から選択を開始する
		GetSelectionInfo().BeginSelectArea();
		caret.underLine.CaretUnderLineOFF(true);
		caret.underLine.UnderLineLock();
		if (ptMouse.x < textArea.GetAreaLeft()) {
			// カーソル下移動
			GetCommander().Command_Down(true, false);
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
		GetSelectionInfo().ptMouseRollPosOld = ptMouse;	// マウス範囲選択前回位置(XY座標)
		
		// 範囲選択開始 & マウスキャプチャー
		GetSelectionInfo().SelectBeginNazo();
		::SetCapture(GetHwnd());
		caret.HideCaret_(GetHwnd());

		Point ptNewCaret = caret.GetCaretLayoutPos();
		bool bSetPtNewCaret = false;
		if (tripleClickMode) {
			// 1行選択でない場合は選択文字列を解除
			// トリプルクリックが1行選択でなくてもクアドラプルクリックを有効とする
			if (nFuncID != F_SELECTLINE) {
				OnLBUTTONUP(fwKeys, ptMouse.x, ptMouse.y);	// ここで左ボタンアップしたことにする

				if (GetSelectionInfo().IsTextSelected())		// テキストが選択されているか
					GetSelectionInfo().DisableSelectArea(true);	// 現在の選択範囲を非選択状態に戻す
			}

			// 単語の途中で折り返されていると下の行が選択されてしまうことへの対処
			if (nFuncID != F_SELECTLINE) {
				caret.MoveCursorToClientPoint(ptMouse);	// カーソル移動
			}else {
				caret.MoveCursorToClientPoint(ptMouse, true, &ptNewCaret);	// カーソル移動
				bSetPtNewCaret = true;
			}

			// コマンドコードによる処理振り分け
			// マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
			::SendMessage(::GetParent(hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE), (LPARAM)NULL);

			// 1行選択でない場合はここで抜ける（他の選択コマンドの時問題となるかも）
			if (nFuncID != F_SELECTLINE)
				return;
			ptNewCaret = caret.GetCaretLayoutPos();

			// 選択するものが無い（[EOF]のみの行）時は通常クリックと同じ処理
			if (1
				&& !GetSelectionInfo().IsTextSelected()
				&& caret.GetCaretLogicPos().y >= (int)pEditDoc->docLineMgr.GetLineCount()
			) {
				GetSelectionInfo().BeginSelectArea();				// 現在のカーソル位置から選択を開始する
				GetSelectionInfo().bBeginLineSelect = false;		// 行単位選択中 OFF
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
					caret.MoveCursorToClientPoint(ptMouse, true, &ptNewCaret);
				}else if (ptMouse.x < textArea.GetAreaLeft()) {
					caret.MoveCursorToClientPoint(Point(textArea.GetDocumentLeftClientPointX(), ptMouse.y), true, &ptNewCaret);
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
					caret.MoveCursorToClientPoint(ptMouse, true, &ptNewCaret);
				}else if (ptMouse.x < textArea.GetAreaLeft()) {
					caret.MoveCursorToClientPoint(Point(textArea.GetDocumentLeftClientPointX(), ptMouse.y), true, &ptNewCaret);
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
		// CTRLキーが押されている、かつトリプルクリックでない
		if (GetKeyState_Control() && !tripleClickMode) {
			GetSelectionInfo().bBeginWordSelect = true;		// 単語単位選択中
			if (!GetSelectionInfo().IsTextSelected()) {
				// 現在位置の単語選択
				if (GetCommander().Command_SelectWord(&ptNewCaret)) {
					bSelectWord = true;
					GetSelectionInfo().selectBgn = GetSelectionInfo().select;
				}
			}else {

				// 選択領域描画
				GetSelectionInfo().DrawSelectArea();

				// 指定された桁に対応する行のデータ内の位置を調べる
				const Layout* pLayout;
				pLine = pEditDoc->layoutMgr.GetLineStr(
					GetSelectionInfo().select.GetFrom().y,
					&nLineLen,
					&pLayout
				);
				if (pLine) {
					nIdx = LineColumnToIndex(pLayout, GetSelectionInfo().select.GetFrom().x);
					// 現在位置の単語の範囲を調べる
					bool bWhareResult = pEditDoc->layoutMgr.WhereCurrentWord(
						GetSelectionInfo().select.GetFrom().y,
						nIdx,
						&range,
						NULL,
						NULL
					);
					if (bWhareResult) {
						nWork = IsCurrentPositionSelected(
							range.GetFrom()	// カーソル位置
						);
						if (nWork == -1 || nWork == 0) {
							GetSelectionInfo().select.SetFrom(range.GetFrom());
							if (nWorkRel == 1) {
								GetSelectionInfo().selectBgn = range;
							}
						}
					}
				}
				pLine = pEditDoc->layoutMgr.GetLineStr(GetSelectionInfo().select.GetTo().y, &nLineLen, &pLayout);
				if (pLine) {
					nIdx = LineColumnToIndex(pLayout, GetSelectionInfo().select.GetTo().x);
					// 現在位置の単語の範囲を調べる
					if (pEditDoc->layoutMgr.WhereCurrentWord(
						GetSelectionInfo().select.GetTo().y, nIdx, &range, NULL, NULL)
					) {
						nWork = IsCurrentPositionSelected(range.GetFrom());
						if (nWork == -1 || nWork == 0) {
							GetSelectionInfo().select.SetTo(range.GetFrom());
						}
						if (IsCurrentPositionSelected(range.GetTo()) == 1) {
							GetSelectionInfo().select.SetTo(range.GetTo());
						}
						if (nWorkRel == -1 || nWorkRel == 0) {
							GetSelectionInfo().selectBgn = range;
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
		// シフトキーを押している場合は行頭クリックとして扱う
		if (ptMouse.x < textArea.GetAreaLeft() && !GetKeyState_Shift()) {
			// 現在のカーソル位置から選択を開始する
			GetSelectionInfo().bBeginLineSelect = true;

			const Layout* pLayout = pEditDoc->layoutMgr.SearchLineByLayoutY(ptNewCaret.y);
			if (pLayout) {
				Point ptCaret;
				const Layout* pNext = pLayout->GetNextLayout();
				if (pNext) {
					ptCaret.x = pNext->GetIndent();
				}else {
					ptCaret.x = 0;
				}
				ptCaret.y = ptNewCaret.y + 1;	// 改行無しEOF行でも MoveCursor() が有効な座標に調整してくれる
				caret.GetAdjustCursorPos(&ptCaret);
				GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptCaret);
				caret.MoveCursor(ptCaret, true);
				caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
			}else {
				// 現在のカーソル位置によって選択範囲を変更
				if (bSetPtNewCaret) {
					GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptNewCaret);
					caret.MoveCursor(ptNewCaret, true, 1000);
					caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
				}
			}

			//	行番号をクリックした場合にはGetSelectionInfo().ChangeSelectAreaByCurrentCursor()にて
			//	GetSelectionInfo().select.GetTo().x/GetSelectionInfo().select.GetTo().yに-1が設定されるが、上の
			//	GetCommander().Command_GoLineEnd(), Command_Right()によって行選択が行われる。
			//	しかしキャレットが末尾にある場合にはキャレットが移動しないので
			//	GetSelectionInfo().select.GetTo().x/GetSelectionInfo().select.GetTo().yが-1のまま残ってしまい、それが
			//	原点に設定されるためにおかしくなっていた。
			//	なので、範囲選択が行われていない場合は起点末尾の設定を行わないようにする
			if (GetSelectionInfo().IsTextSelected()) {
				GetSelectionInfo().selectBgn.SetTo(GetSelectionInfo().select.GetTo());
			}
		}else {
			// URLがクリックされたら選択するか
			if (GetDllShareData().common.edit.bSelectClickedURL) {

				Range cUrlRange;	// URL範囲
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
					*/
					Range rangeB;
					pEditDoc->layoutMgr.LogicToLayout(cUrlRange, &rangeB);
					/*
					pEditDoc->layoutMgr.LogicToLayout(Point(nUrlIdxBgn          , nUrlLine), rangeB.GetFromPointer());
					pEditDoc->layoutMgr.LogicToLayout(Point(nUrlIdxBgn + nUrlLen, nUrlLine), rangeB.GetToPointer());
					*/

					GetSelectionInfo().selectBgn = rangeB;
					GetSelectionInfo().select = rangeB;

					// 選択領域描画
					GetSelectionInfo().DrawSelectArea();
				}
			}
			if (bSetPtNewCaret && !bSelectWord) {
				// 現在のカーソル位置によって選択範囲を変更
				caret.MoveCursor(ptNewCaret, true, 1000);
				caret.nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;
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

	@note	dwTripleClickCheckが0でない時にチェックモードと判定するが、PCを
			連続稼動している場合49.7日毎にカウンタが0になる為、わずかな可能性
			であるがトリプルクリックが判定できない時がある。
			行番号表示エリアのトリプルクリックは通常クリックとして扱う。
*/
bool EditView::CheckTripleClick(Point ptMouse)
{

	// トリプルクリックチェック有効でない（時刻がセットされていない）
	if (!dwTripleClickCheck)
		return false;

	bool result = false;

	// 前回クリックとのクリック位置のずれを算出
	Point dpos(GetSelectionInfo().ptMouseRollPosOld.x - ptMouse.x,
				   GetSelectionInfo().ptMouseRollPosOld.y - ptMouse.y);

	if (dpos.x < 0)
		dpos.x = -dpos.x;	// 絶対値化

	if (dpos.y < 0)
		dpos.y = -dpos.y;	// 絶対値化

	// 行番号表示エリアでない、かつクリックプレスからダブルクリック時間以内、
	// かつダブルクリックの許容ずれピクセル以下のずれの時トリプルクリックとする
	if ((ptMouse.x >= GetTextArea().GetAreaLeft())&&
		(::GetTickCount() - dwTripleClickCheck <= GetDoubleClickTime())&&
		(dpos.x <= GetSystemMetrics(SM_CXDOUBLECLK)) &&
		(dpos.y <= GetSystemMetrics(SM_CYDOUBLECLK))
	) {
		result = true;
	}else {
		dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
	}
	
	return result;
}

// マウス右ボタン押下
void EditView::OnRBUTTONDOWN(WPARAM fwKeys, int xPos , int yPos)
{
	if (nAutoScrollMode) {
		AutoScrollExit();
	}
	if (bMiniMap) {
		return;
	}
	// 現在のマウスカーソル位置→レイアウト位置

	Point ptNew;
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
	// Shift,Ctrl,Altキーが押されていたか
	nIdx = GetCtrlKeyState();
	// マウス右クリックに対応する機能コードはcommon.pKeyNameArr[1]に入っている
	nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::RightClick].nFuncCodeArr[nIdx];
	if (nFuncID != 0) {
		// コマンドコードによる処理振り分け
		// マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage(::GetParent(hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE), (LPARAM)NULL);
	}
//	// 右クリックメニュー
//	GetCommander().Command_Menu_RButton();
	return;
}


/*!
	@brief マウス中ボタンを押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
*/
void EditView::OnMBUTTONDOWN(WPARAM fwKeys, int xPos , int yPos)
{
	int nIdx = GetCtrlKeyState();
	if (GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::CenterClick].nFuncCodeArr[nIdx] == F_AUTOSCROLL) {
		if (nAutoScrollMode) {
			AutoScrollExit();
			return;
		}else {
			nAutoScrollMode = 1;
			autoScrollMousePos = Point(xPos, yPos);
			::SetCapture(GetHwnd());
		}
	}
}


/*!
	@brief マウス中ボタンを開放したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
*/
void EditView::OnMBUTTONUP(WPARAM fwKeys, int xPos , int yPos)
{
	int		nIdx;
	int		nFuncID;

	// ホイール操作によるページスクロールあり
	if (GetDllShareData().common.general.nPageScrollByWheel == (int)MouseFunctionType::CenterClick &&
	    editWnd.IsPageScrollByWheel()
	) {
		editWnd.SetPageScrollByWheel(FALSE);
		return;
	}

	// ホイール操作によるページスクロールあり
	if (GetDllShareData().common.general.nHorizontalScrollByWheel == (int)MouseFunctionType::CenterClick &&
	    editWnd.IsHScrollByWheel()
	) {
		editWnd.SetHScrollByWheel(FALSE);
		return;
	}

	// Shift,Ctrl,Altキーが押されていたか
	nIdx = GetCtrlKeyState();
	// マウス左サイドボタンに対応する機能コードはcommon.pKeyNameArr[2]に入っている
	nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::CenterClick].nFuncCodeArr[nIdx];
	if (nFuncID == F_AUTOSCROLL) {
		if (nAutoScrollMode == 1) {
			bAutoScrollDragMode = false;
			AutoScrollEnter();
			return;
		}else if (nAutoScrollMode == 2 && bAutoScrollDragMode) {
			AutoScrollExit();
			return;
		}
	}else if (nFuncID != 0) {
		// コマンドコードによる処理振り分け
		// マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage(::GetParent(hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE),  (LPARAM)NULL);
	}
	if (nAutoScrollMode) {
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
	bAutoScrollVertical = GetTextArea().nViewRowNum < (int)pEditDoc->layoutMgr.GetLineCount() + 2;
	bAutoScrollHorizontal = GetTextArea().nViewColNum < (int)GetRightEdgeForScrollBar();
	if (bMiniMap) {
		bAutoScrollHorizontal = false;
	}
	if (!bAutoScrollHorizontal && !bAutoScrollVertical) {
		nAutoScrollMode = 0;
		return;
	}
	nAutoScrollMode = 2;
	autoScrollWnd.Create(G_AppInstance(), GetHwnd(), bAutoScrollVertical, bAutoScrollHorizontal, autoScrollMousePos, this);
	::SetTimer(GetHwnd(), 2, 200, AutoScrollTimerProc);
	HCURSOR hCursor;
	hCursor = ::LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_CURSOR_AUTOSCROLL_CENTER));
	::SetCursor(hCursor);
}

void EditView::AutoScrollExit()
{
	if (nAutoScrollMode) {
		::ReleaseCapture();
	}
	if (nAutoScrollMode == 2) {
		KillTimer(GetHwnd(), 2);
		autoScrollWnd.Close();
	}
	nAutoScrollMode = 0;
}

void EditView::AutoScrollMove(Point& point)
{
	const Point relPos = point - autoScrollMousePos;
	int idcX, idcY;
	if (!bAutoScrollHorizontal || abs(relPos.x) < 16) {
		idcX = 0;
	}else if (relPos.x < 0) {
		idcX = 1;
	}else {
		idcX = 2;
	}
	if (!bAutoScrollVertical || abs(relPos.y) < 16) {
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
		if (!bAutoScrollVertical) {
			cursor = IDC_CURSOR_AUTOSCROLL_HORIZONTAL;
		}else if (!bAutoScrollHorizontal) {
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
	
	const Point relPos = cursorPos - autoScrollMousePos;
	Point scrollPos = relPos / 8;
	if (bAutoScrollHorizontal) {
		if (scrollPos.x < 0) {
			scrollPos.x += 1;
		}else if (scrollPos.x > 0) {
			scrollPos.x -= 1;
		}
		SyncScrollH(ScrollAtH(GetTextArea().GetViewLeftCol() + scrollPos.x));
	}
	if (bAutoScrollVertical) {
		if (scrollPos.y < 0) {
			scrollPos.y += 1;
		}else if (scrollPos.y > 0) {
			scrollPos.y -= 1;
		}
		SyncScrollV(ScrollAtV(GetTextArea().GetViewTopLine() + scrollPos.y));
	}
}

/*!
	@brief マウスサイドボタン1を押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
*/
void EditView::OnXLBUTTONDOWN(WPARAM fwKeys, int xPos , int yPos)
{
	if (nAutoScrollMode) {
		AutoScrollExit();
	}
}


/*!
	@brief マウスサイドボタン1を開放したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
*/
void EditView::OnXLBUTTONUP(WPARAM fwKeys, int xPos , int yPos)
{
	int		nIdx;
	int		nFuncID;

	// ホイール操作によるページスクロールあり
	if (GetDllShareData().common.general.nPageScrollByWheel == (int)MouseFunctionType::LeftSideClick &&
	    editWnd.IsPageScrollByWheel()
	) {
		editWnd.SetPageScrollByWheel(FALSE);
		return;
	}

	// ホイール操作によるページスクロールあり
	if (GetDllShareData().common.general.nHorizontalScrollByWheel == (int)MouseFunctionType::LeftSideClick &&
	    editWnd.IsHScrollByWheel()
	) {
		editWnd.SetHScrollByWheel(FALSE);
		return;
	}

	// Shift,Ctrl,Altキーが押されていたか
	nIdx = GetCtrlKeyState();
	// マウスサイドボタン1に対応する機能コードはcommon.pKeyNameArr[3]に入っている
	nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::LeftSideClick].nFuncCodeArr[nIdx];
	if (nFuncID != 0) {
		// コマンドコードによる処理振り分け
		// マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage(::GetParent(hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE),  (LPARAM)NULL);
	}

	return;
}


/*!
	@brief マウスサイドボタン2を押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
*/
void EditView::OnXRBUTTONDOWN(WPARAM fwKeys, int xPos , int yPos)
{
	if (nAutoScrollMode) {
		AutoScrollExit();
	}
}


/*!
	@brief マウスサイドボタン2を開放したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
*/
void EditView::OnXRBUTTONUP(WPARAM fwKeys, int xPos , int yPos)
{
	int		nIdx;
	int		nFuncID;

	// ホイール操作によるページスクロールあり
	if (GetDllShareData().common.general.nPageScrollByWheel == (int)MouseFunctionType::RightSideClick &&
	    editWnd.IsPageScrollByWheel()
	) {
		// ホイール操作によるページスクロールありをOFF
		editWnd.SetPageScrollByWheel(FALSE);
		return;
	}

	// ホイール操作によるページスクロールあり
	if (GetDllShareData().common.general.nHorizontalScrollByWheel == (int)MouseFunctionType::RightSideClick &&
	    editWnd.IsHScrollByWheel()
	) {
		// ホイール操作による横スクロールありをOFF
		editWnd.SetHScrollByWheel(FALSE);
		return;
	}

	// Shift,Ctrl,Altキーが押されていたか
	nIdx = GetCtrlKeyState();
	// マウスサイドボタン2に対応する機能コードはcommon.pKeyNameArr[4]に入っている
	nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::RightSideClick].nFuncCodeArr[nIdx];
	if (nFuncID != 0) {
		// コマンドコードによる処理振り分け
		// マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage(::GetParent(hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE),  (LPARAM)NULL);
	}

	return;
}

// マウス移動のメッセージ処理
void EditView::OnMOUSEMOVE(WPARAM fwKeys, int xPos_, int yPos_)
{
	Point ptMouse(xPos_, yPos_);

	if (mousePousePos != ptMouse) {
		mousePousePos = ptMouse;
		if (nMousePouse < 0) {
			nMousePouse = 0;
		}
	}

	Range selectionOld    = GetSelectionInfo().select;

	// オートスクロール
	if (nAutoScrollMode == 1) {
		if (::GetSystemMetrics(SM_CXDOUBLECLK) < abs(ptMouse.x - autoScrollMousePos.x) ||
		    ::GetSystemMetrics(SM_CYDOUBLECLK) < abs(ptMouse.y - autoScrollMousePos.y)
		) {
			bAutoScrollDragMode = true;
			AutoScrollEnter();
		}
		return;
	}else if (nAutoScrollMode == 2) {
		AutoScrollMove(ptMouse);
		return;
	}

	TextArea& textArea = GetTextArea();
	auto& caret = GetCaret();
	if (bMiniMap) {
		POINT po;
		::GetCursorPos(&po);
		// 辞書Tipが起動されている
		if (dwTipTimer == 0) {
			if ((poTipCurPos.x != po.x || poTipCurPos.y != po.y )) {
				tipWnd.Hide();
				dwTipTimer = ::GetTickCount();
			}
		}else {
			dwTipTimer = ::GetTickCount();
		}
		if (bMiniMapMouseDown) {
			Point ptNew;
			textArea.ClientToLayout( ptMouse, &ptNew );
			// ミニマップの上下スクロール
			if (ptNew.y < 0) {
				ptNew.y = 0;
			}
			int nScrollRow = 0;
			int nScrollMargin = 15;
			nScrollMargin  = t_min(nScrollMargin,  (int)textArea.nViewRowNum / 2);
			if ((int)pEditDoc->layoutMgr.GetLineCount() > textArea.nViewRowNum
				&& ptNew.y > textArea.GetViewTopLine() + textArea.nViewRowNum - nScrollMargin
			) {
				nScrollRow = (textArea.GetViewTopLine() + textArea.nViewRowNum - nScrollMargin) - ptNew.y;
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
				ptNew.y = 0;
			}
			EditView& view = editWnd.GetActiveView();
			ptNew.x = 0;
			Point ptNewLogic;
			view.GetCaret().GetAdjustCursorPos(&ptNew);
			ptNewLogic = GetDocument().layoutMgr.LayoutToLogic(ptNew);
			ptNew = GetDocument().layoutMgr.LogicToLayout(ptNewLogic, ptNew.y);
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
			view.GetCaret().nCaretPosX_Prev = caret.GetCaretLayoutPos().x;
		}
		::SetCursor(::LoadCursor(NULL, IDC_ARROW));
		GetSelectionInfo().ptMouseRollPosOld = ptMouse; // マウス範囲選択前回位置(XY座標)
		return;
	}

	if (!GetSelectionInfo().IsMouseSelecting()) {
		// マウスによる範囲選択中でない場合
		POINT		po;
		::GetCursorPos(&po);
		// 補完ウィンドウが表示されていない
		if (!bHokan) {
			// 辞書Tipが起動されている
			if (dwTipTimer == 0) {
				if ((poTipCurPos.x != po.x || poTipCurPos.y != po.y)) {
					// 辞書Tipを消す
					tipWnd.Hide();
					dwTipTimer = ::GetTickCount();	// 辞書Tip起動タイマー
				}
			}else {
				dwTipTimer = ::GetTickCount();		// 辞書Tip起動タイマー
			}
		}
		// 現在のマウスカーソル位置→レイアウト位置
		Point ptNew;
		textArea.ClientToLayout(ptMouse, &ptNew);

		Range	cUrlRange;	// URL範囲
		auto& csEdit = GetDllShareData().common.edit;
		// 選択テキストのドラッグ中か
		if (bDragMode) {
			if (csEdit.bUseOLE_DragDrop) {	// OLEによるドラッグ & ドロップを使う
				// 座標指定によるカーソル移動
				caret.MoveCursorToClientPoint(ptMouse);
			}
		}else {
			// 行選択エリア?
			if (ptMouse.x < textArea.GetAreaLeft() || ptMouse.y < textArea.GetAreaTop()) {
				// 矢印カーソル
				if (ptMouse.y >= textArea.GetAreaTop()) {
					::SetCursor(::LoadCursor(G_AppInstance(), MAKEINTRESOURCE(IDC_CURSOR_RVARROW)));
				}else {
					::SetCursor(::LoadCursor(NULL, IDC_ARROW));
				}
			}else if (1
				&& csEdit.bUseOLE_DragDrop		// OLEによるドラッグ & ドロップを使う
				&& csEdit.bUseOLE_DropSource		// OLEによるドラッグ元にするか
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
				SetHandCursor();		// Hand Cursorを設定
			}else {
				if (nISearchMode > 0) {
					if (nISearchDirection == SearchDirection::Forward) {
						::SetCursor(::LoadCursor(G_AppInstance(), MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
					}else {
						::SetCursor(::LoadCursor(G_AppInstance(), MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
					}
				// アイビーム
				}else if (0 <= nMousePouse) {
					::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
				}
			}
		}
		return;
	}
	// 以下、マウスでの選択中(ドラッグ中)

	if (0 <= nMousePouse) {
		::SetCursor(::LoadCursor(NULL, IDC_IBEAM));
	}

	// ドラッグ開始位置から移動していない場合はMOVEとみなさない
	// 遊びは 2px固定とする
	Point ptMouseMove = ptMouse - mouseDownPos;
	if (mouseDownPos.x != -INT_MAX && abs(ptMouseMove.x) <= 2 && abs(ptMouseMove.y) <= 2) {
		return;
	}
	// 一度移動したら戻ってきたときも、移動とみなすように設定
	mouseDownPos.Set(-INT_MAX, -INT_MAX);
	
	Point ptNewCursor(-1, -1);
	if (GetSelectionInfo().IsBoxSelecting()) {	// 矩形範囲選択中
		// 座標指定によるカーソル移動
		caret.MoveCursorToClientPoint(ptMouse, true, &ptNewCursor);
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptNewCursor);
		caret.MoveCursorToClientPoint(ptMouse);
		// 現在のカーソル位置によって選択範囲を変更
		GetSelectionInfo().ptMouseRollPosOld = ptMouse; // マウス範囲選択前回位置(XY座標)
	}else {
		// 座標指定によるカーソル移動
		if ((ptMouse.x < textArea.GetAreaLeft() || dwTripleClickCheck)&& GetSelectionInfo().bBeginLineSelect) {	// 行単位選択中
			// 上方向の行選択時もマウスカーソルの位置の行が選択されるようにする
			Point nNewPos(0, ptMouse.y);

			// 1行の高さ
			int nLineHeight = GetTextMetrics().GetHankakuHeight() + pTypeData->nLineSpace;

			// 選択開始行以下へのドラッグ時は1行下にカーソルを移動する
			if (textArea.GetViewTopLine() + (ptMouse.y - textArea.GetAreaTop()) / nLineHeight >= GetSelectionInfo().selectBgn.GetTo().y) {
				nNewPos.y += nLineHeight;
			}

			// カーソルを移動
			nNewPos.x = textArea.GetAreaLeft() - textArea.GetViewLeftCol() * (int)(GetTextMetrics().GetHankakuWidth() + pTypeData->nColumnSpace);
			caret.MoveCursorToClientPoint(nNewPos, false, &ptNewCursor);

			// 2.5クリックによる行単位のドラッグ
			if (dwTripleClickCheck) {
				// 選択開始行以上にドラッグした
				if (ptNewCursor.GetY() <= GetSelectionInfo().selectBgn.GetTo().y) {
					// GetCommander().Command_GoLineTop(true, 0x09);		// 改行単位の行頭へ移動
					size_t nLineLen;
					const Layout*	pLayout;
					const wchar_t*	pLine = pEditDoc->layoutMgr.GetLineStr(ptNewCursor.y, &nLineLen, &pLayout);
					ptNewCursor.x = 0;
					if (pLine) {
						while (pLayout->GetLogicOffset()) {
							ptNewCursor.y--;
							pLayout = pLayout->GetPrevLayout();
						}
					}
				}else {
					Point ptCaret;
					Point ptCaretPrevLog(0, caret.GetCaretLogicPos().y);

					// 選択開始行より下にカーソルがある時は1行前と物理行番号の違いをチェックする
					// 選択開始行にカーソルがある時はチェック不要
					if (ptNewCursor.GetY() > GetSelectionInfo().selectBgn.GetTo().y) {
						// 1行前の物理行を取得する
						ptCaretPrevLog - pEditDoc->layoutMgr.LayoutToLogic(Point(0, ptNewCursor.GetY() - 1));
					}

					Point ptNewCursorLogic = pEditDoc->layoutMgr.LayoutToLogic(ptNewCursor);
					// 前の行と同じ物理行
					if (ptCaretPrevLog.y == ptNewCursorLogic.y) {
						// 1行先の物理行からレイアウト行を求める
						ptCaret = pEditDoc->layoutMgr.LogicToLayout(Point(0, caret.GetCaretLogicPos().y + 1));
						// カーソルを次の物理行頭へ移動する
						ptNewCursor = ptCaret;
					}
				}
			}
		}else {
			caret.MoveCursorToClientPoint(ptMouse, true, &ptNewCursor);
		}
		GetSelectionInfo().ptMouseRollPosOld = ptMouse; // マウス範囲選択前回位置(XY座標)

		// CTRLキーが押されていたか
//		if (GetKeyState_Control()) {
		if (!GetSelectionInfo().bBeginWordSelect) {
			// 現在のカーソル位置によって選択範囲を変更
			GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptNewCursor);
			caret.MoveCursor(ptNewCursor, true, 1000);
		}else {
			Range select;
			// 現在のカーソル位置によって選択範囲を変更(テストのみ)
			GetSelectionInfo().ChangeSelectAreaByCurrentCursorTEST(
				caret.GetCaretLayoutPos(),
				&select
			);
			// 選択範囲に変更なし
			if (selectionOld == select) {
				GetSelectionInfo().ChangeSelectAreaByCurrentCursor(
					caret.GetCaretLayoutPos()
				);
				caret.MoveCursor(ptNewCursor, true, 1000);
				return;
			}
			size_t nLineLen;
			const Layout* pLayout;
			if (pEditDoc->layoutMgr.GetLineStr(caret.GetCaretLayoutPos().y, &nLineLen, &pLayout)) {
				size_t nIdx = LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().x);
				Range range;
				// 現在位置の単語の範囲を調べる
				bool bResult = pEditDoc->layoutMgr.WhereCurrentWord(
					caret.GetCaretLayoutPos().y,
					nIdx,
					&range,
					nullptr,
					nullptr
				);
				if (bResult) {
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
					GetSelectionInfo().ChangeSelectAreaByCurrentCursor(caret.GetCaretLayoutPos());
				}
			}else {
				// 現在のカーソル位置によって選択範囲を変更
				GetSelectionInfo().ChangeSelectAreaByCurrentCursor(caret.GetCaretLayoutPos());
			}
			caret.MoveCursor(ptNewCursor, true, 1000);
		}
	}
	return;
}
//dwTipTimerm_dwTipTimerm_dwTipTimer


#ifndef SPI_GETWHEELSCROLLCHARS
#define SPI_GETWHEELSCROLLCHARS 0x006C
#endif


/* マウスホイールのメッセージ処理 */
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
		bool bHorizontal = false;
		bool bKeyPageScroll = false;
		if (nCmdFuncID == F_0) {
			// 通常スクロールの時だけ適用
			bHorizontal = IsSpecialScrollMode(GetDllShareData().common.general.nHorizontalScrollByWheel);
			bKeyPageScroll = IsSpecialScrollMode(GetDllShareData().common.general.nPageScrollByWheel);
		}

		// ホイールスクロールにキー割り当て
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
					if (nRollLineNum != -1 && bMiniMap) {
						nRollLineNum *= 10;
					}
				}
			}
			if (!bGetParam) {
				if (ReadRegistry(HKEY_CURRENT_USER, _T("Control Panel\\desktop"), _T("WheelScrollLines"), szValStr, uDataLen)) {
					nRollLineNum = ::_ttoi(szValStr);
					if (nRollLineNum != -1 && bMiniMap) {
						nRollLineNum *= 10;
					}
				}
			}
		}

		if (nRollLineNum == -1 || bKeyPageScroll) {
			// 「1画面分スクロールする」
			if (bHorizontal) {
				nRollLineNum = GetTextArea().nViewColNum - 1;	// 表示域の桁数
			}else {
				nRollLineNum = GetTextArea().nViewRowNum - 1;	// 表示域の行数
			}
		}else {
			if (nRollLineNum > 30) {
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
				editWnd.SetHScrollByWheel(TRUE);
			}
			// ホイール操作によるページスクロールあり
			editWnd.SetPageScrollByWheel(TRUE);
		}else {
			if (bHorizontal) {
				// ホイール操作による横スクロールあり
				editWnd.SetHScrollByWheel(TRUE);
			}
		}

		if (0
			|| nFuncID != eWheelScroll
			|| (zDelta < 0 && 0 < nWheelDelta)
			|| (0 < zDelta && nWheelDelta < 0)
		) {
			nWheelDelta = 0;
			eWheelScroll = nFuncID;
		}
		nWheelDelta += zDelta;

		// APIのスクロール量に従う
		int nRollNum = abs(nWheelDelta) * nRollLineNum / 120;
		// 次回持越しの変化量(上記式Deltaのあまり。スクロール方向とzDeltaは符号が反対)
		nWheelDelta = (abs(nWheelDelta) - nRollNum * 120 / nRollLineNum) * ((nScrollCode == SB_LINEUP) ? 1 : -1);

		if (bExecCmd) {
			if (nFuncID != F_0) {
				// スクロール変化量分コマンド実行(zDeltaが120あたりで1回)
				for (int i=0; i<nRollNum; ++i) {
					::PostMessage(::GetParent(hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE),  (LPARAM)NULL);
				}
			}
			return bHorizontalMsg ? TRUE: 0;
		}

		const bool bSmooth = !! GetDllShareData().common.general.nRepeatedScroll_Smooth;
		const int nRollActions = bSmooth ? nRollNum : 1;
		const int nCount = ((nScrollCode == SB_LINEUP) ? -1 : 1) * (bSmooth ? 1 : nRollNum);

		for (i=0; i<nRollActions; ++i) {
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
		GetCaret().ShowCaret_(GetHwnd());

		GetSelectionInfo().SelectEnd();

		// マウスクリック座標をリセット
		mouseDownPos.Set(-INT_MAX, -INT_MAX);

		GetCaret().underLine.UnderLineUnLock();
		if (GetSelectionInfo().select.IsOne()) {
			// 現在の選択範囲を非選択状態に戻す
			GetSelectionInfo().DisableSelectArea(true);
		}
	}
	if (bMiniMapMouseDown) {
		bMiniMapMouseDown = false;
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
	delete[] pszFile;
	return 0;
}


// マウス左ボタンダブルクリック
void EditView::OnLBUTTONDBLCLK(WPARAM fwKeys, int _xPos , int _yPos)
{
	Point ptMouse(_xPos, _yPos);

	Range		cUrlRange;	// URL範囲
	std::wstring	wstrURL;
	const wchar_t*	pszMailTo = L"mailto:";

	if (! dwTripleClickCheck) {
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
			if (IsMailAddress(wstrURL.c_str(), wstrURL.length(), nullptr)) {
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
				// UNCパスだと1分以上無応答になることがあるのでスレッド化
				WaitCursor waitCursor(GetHwnd());	// カーソルを砂時計にする

				unsigned int nThreadId;
				LPCTSTR szUrl = to_tchar(wstrOPEN.c_str());
				LPTSTR pszUrlDup = new TCHAR[_tcslen(szUrl) + 1];
				_tcscpy(pszUrlDup, szUrl);
				HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ShellExecuteProc, (LPVOID)pszUrlDup, 0, &nThreadId);
				if (hThread != INVALID_HANDLE_VALUE) {
					// ユーザーのURL起動指示に反応した目印としてちょっとの時間だけ砂時計カーソルを表示しておく
					// ShellExecute は即座にエラー終了することがちょくちょくあるので WaitForSingleObject ではなく Sleep を使用（ex.存在しないパスの起動）
					// 【補足】いずれの API でも待ちを長め（2〜3秒）にするとなぜか Web ブラウザ未起動からの起動が重くなる模様（PCタイプ, XP/Vista, IE/FireFox に関係なく）
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
		// 外部コマンドの出力からTagjumpできるように
		if (1
			&& (EditApp::getInstance().pGrepAgent->bGrepMode || AppMode::getInstance().IsDebugMode())
			&& GetDllShareData().common.search.bGTJW_DoubleClick
		) {
			// タグジャンプ機能
			if (GetCommander().Command_TagJump()) {
				// タグジャンプ失敗時は通常の処理を実行する
				return;
			}
		}
	}

	// Shift,Ctrl,Altキーが押されていたか
	int	nIdx = GetCtrlKeyState();

	// マウス左クリックに対応する機能コードはcommon.pKeyNameArr[?]に入っている
	EFunctionCode nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[
		(int)(dwTripleClickCheck ? MouseFunctionType::QuadrapleClick : MouseFunctionType::DoubleClick)
	].nFuncCodeArr[nIdx];
	if (dwTripleClickCheck) {
		// 非選択状態にした後左クリックしたことにする
		// すべて選択の場合は、3.5クリック時の選択状態保持とドラッグ開始時の
		// 範囲変更のため。
		// クアドラプルクリック機能が割り当てられていない場合は、ダブルクリック
		// として処理するため。
		if (GetSelectionInfo().IsTextSelected())		// テキストが選択されているか
			GetSelectionInfo().DisableSelectArea(true);		// 現在の選択範囲を非選択状態に戻す

		if (!nFuncID) {
			dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
			nFuncID = GetDllShareData().common.keyBind.pKeyNameArr[(int)MouseFunctionType::DoubleClick].nFuncCodeArr[nIdx];
			OnLBUTTONDOWN(fwKeys, ptMouse.x , ptMouse.y);	// カーソルをクリック位置へ移動する
		}
	}

	if (nFuncID != 0) {
		// コマンドコードによる処理振り分け
		// マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::SendMessage(::GetParent(hwndParent), WM_COMMAND, MAKELONG(nFuncID, CMD_FROM_MOUSE),  (LPARAM)NULL);
	}

	if (dwTripleClickCheck) {
		dwTripleClickCheck = 0;	// トリプルクリックチェック OFF（次回は通常クリック）
		return;
	}

	dwTripleClickCheck = ::GetTickCount();

	// ダブルクリック位置として記憶
	GetSelectionInfo().ptMouseRollPosOld = ptMouse;	// マウス範囲選択前回位置(XY座標)

	/* 機能コードの判定
		ダブルクリックからのドラッグでは単語単位の範囲選択(エディタの一般的動作)になるが
		この動作は、ダブルクリック＝単語選択を前提としたもの。
		キー割り当ての変更により、ダブルクリック≠単語選択のときには GetSelectionInfo().bBeginWordSelect = true
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
	GetCaret().HideCaret_(GetHwnd());
	if (GetSelectionInfo().IsTextSelected()) {
		// 常時選択範囲の範囲
		GetSelectionInfo().selectBgn.SetTo(GetSelectionInfo().select.GetTo());
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
	if (!pEditDoc->IsEditable()) return E_UNEXPECTED;


	if (!pDataObject || !pdwEffect)
		return E_INVALIDARG;

	cfDragData = GetAvailableClipFormat(pDataObject);
	if (cfDragData == 0)
		return E_INVALIDARG;
	else if (cfDragData == CF_HDROP) {
		// 右ボタンで入ってきたときだけファイルをビューで取り扱う
		if (!(MK_RBUTTON & dwKeyState))
			return E_INVALIDARG;
	}
	
	// 自分をアクティブペインにする
	editWnd.SetActivePane(nMyIndex);

	// 現在のカーソル位置を記憶する
	ptCaretPos_DragEnter = GetCaret().GetCaretLayoutPos();
	nCaretPosX_Prev_DragEnter = GetCaret().nCaretPosX_Prev;

	// ドラッグデータは矩形か
	bDragBoxData = IsDataAvailable(pDataObject, (CLIPFORMAT)::RegisterClipboardFormat(_T("MSDEVColumnSelect")));

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

	*pdwEffect = TranslateDropEffect(cfDragData, dwKeyState, pt, *pdwEffect);

	EditView* pDragSourceView = editWnd.GetDragSourceView();

	// ドラッグ元が他ビューで、このビューのカーソルがドラッグ元の選択範囲内の場合は禁止マークにする
	// ※自ビューのときは禁止マークにしない（他アプリでも多くはそうなっている模様）
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

	// DragEnter時のカーソル位置を復元
	// ※範囲選択中のときに選択範囲とカーソルが分離すると変だから
	GetCaret().MoveCursor(ptCaretPos_DragEnter, false);
	GetCaret().nCaretPosX_Prev = nCaretPosX_Prev_DragEnter;
	RedrawAll();	// ルーラー、アンダーライン、カーソル位置表示更新

	// 非アクティブ時は表示状態を非アクティブに戻す
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

	Range selectBgn_Old;
	Range select_Old;

	// 選択テキストのドラッグ中か
	_SetDragMode(FALSE);

	// 非アクティブ時は表示状態を非アクティブに戻す
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
	EditView* pDragSourceView = editWnd.GetDragSourceView();
	bMove = (*pdwEffect == DROPEFFECT_MOVE) && pDragSourceView;
	bBoxData = bDragBoxData;

	auto& caret = GetCaret();
	// カーソルが選択範囲内にあるときはコピー／移動しない
	if (pDragSourceView &&
		!pDragSourceView->IsCurrentPositionSelected(caret.GetCaretLayoutPos())
	) {
		// DragEnter時のカーソル位置を復元
		// Note. ドラッグ元が他ビューでもマウス移動が速いと稀にここにくる可能性がありそう
		*pdwEffect = DROPEFFECT_NONE;
		caret.MoveCursor(ptCaretPos_DragEnter, false);
		caret.nCaretPosX_Prev = nCaretPosX_Prev_DragEnter;
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
	if (!commander.GetOpeBlk()) {
		commander.SetOpeBlk(new OpeBlk);
	}
	commander.GetOpeBlk()->AddRef();

	// 移動の場合、位置関係を算出
	if (bMove) {
		if (bBoxData) {
			// 2点を対角とする矩形を求める
			TwoPointToRect(
				&rcSel,
				pDragSourceView->GetSelectionInfo().select.GetFrom(),	// 範囲選択開始
				pDragSourceView->GetSelectionInfo().select.GetTo()		// 範囲選択終了
			);
			++rcSel.bottom;
			if (caret.GetCaretLayoutPos().GetY() >= rcSel.bottom) {
				bMoveToPrev = false;
			}else if (caret.GetCaretLayoutPos().GetY() + rcSel.bottom - rcSel.top < rcSel.top) {
				bMoveToPrev = true;
			}else if (caret.GetCaretLayoutPos().x < rcSel.left) {
				bMoveToPrev = true;
			}else {
				bMoveToPrev = false;
			}
		}else {
			if (pDragSourceView->GetSelectionInfo().select.GetFrom().y > caret.GetCaretLayoutPos().GetY()) {
				bMoveToPrev = true;
			}else if (pDragSourceView->GetSelectionInfo().select.GetFrom().y == caret.GetCaretLayoutPos().GetY()) {
				if (pDragSourceView->GetSelectionInfo().select.GetFrom().x > caret.GetCaretLayoutPos().x) {
					bMoveToPrev = true;
				}else {
					bMoveToPrev = false;
				}
			}else {
				bMoveToPrev = false;
			}
		}
	}

	Point ptCaretPos_Old = caret.GetCaretLayoutPos();
	if (!bMove) {
		// コピーモード
		// 現在の選択範囲を非選択状態に戻す
		GetSelectionInfo().DisableSelectArea(true);
	}else {
		bBeginBoxSelect_Old = pDragSourceView->GetSelectionInfo().IsBoxSelecting();
		selectBgn_Old = pDragSourceView->GetSelectionInfo().selectBgn;
		select_Old = pDragSourceView->GetSelectionInfo().select;
		if (bMoveToPrev) {
			// 移動モード & 前に移動
			// 選択エリアを削除
			if (this != pDragSourceView) {
				pDragSourceView->GetSelectionInfo().DisableSelectArea(true);
				GetSelectionInfo().DisableSelectArea(true);
				GetSelectionInfo().SetBoxSelect(bBeginBoxSelect_Old);
				GetSelectionInfo().selectBgn = selectBgn_Old;
				GetSelectionInfo().select = select_Old;
			}
			DeleteData(true);
			caret.MoveCursor(ptCaretPos_Old, true);
		}else {
			// 現在の選択範囲を非選択状態に戻す
			pDragSourceView->GetSelectionInfo().DisableSelectArea(true);
			if (this != pDragSourceView)
				GetSelectionInfo().DisableSelectArea(true);
		}
	}
	if (!bBoxData) {	// 矩形データ
		// 挿入前のキャレット位置を記憶する
		// （キャレットが行終端より右の場合は埋め込まれる空白分だけ桁位置をシフト）
		Point ptCaretLogicPos_Old = caret.GetCaretLogicPos();
		const Layout* pLayout;
		size_t nLineLen;
		Point ptCaretLayoutPos_Old = caret.GetCaretLayoutPos();
		if (pEditDoc->layoutMgr.GetLineStr(ptCaretLayoutPos_Old.y, &nLineLen, &pLayout)) {
			size_t nLineAllColLen;
			LineColumnToIndex2(pLayout, ptCaretLayoutPos_Old.x, &nLineAllColLen);
			if (nLineAllColLen > 0) {	// 行終端より右の場合には nLineAllColLen に行全体の表示桁数が入っている
				ptCaretLogicPos_Old.SetX(
					ptCaretLogicPos_Old.x
					+ (ptCaretLayoutPos_Old.x - nLineAllColLen)
				);
			}
		}

		GetCommander().Command_InsText(true, memBuf.GetStringPtr(), memBuf.GetStringLength(), false);

		// 挿入前のキャレット位置から挿入後のキャレット位置までを選択範囲にする
		Point ptSelectFrom = pEditDoc->layoutMgr.LogicToLayout(ptCaretLogicPos_Old);
		GetSelectionInfo().SetSelectArea(Range(ptSelectFrom, caret.GetCaretLayoutPos()));
	}else {
		// 貼り付け（クリップボードから貼り付け）
		GetCommander().Command_PasteBox(memBuf.GetStringPtr(), memBuf.GetStringLength());
		AdjustScrollBars();
		Redraw();
	}
	if (bMove) {
		if (bMoveToPrev) {
		}else {
			// 移動モード & 後ろに移動

			// 現在の選択範囲を記憶する
			Range selLogic;
			pEditDoc->layoutMgr.LayoutToLogic(
				GetSelectionInfo().select,
				&selLogic
			);

			// 以前の選択範囲を記憶する
			Range delLogic;
			pEditDoc->layoutMgr.LayoutToLogic(
				select_Old,
				&delLogic
			);

			// 現在の行数を記憶する
			size_t nLines_Old = pEditDoc->docLineMgr.GetLineCount();

			// 以前の選択範囲を選択する
			GetSelectionInfo().SetBoxSelect(bBeginBoxSelect_Old);
			GetSelectionInfo().selectBgn = selectBgn_Old;
			GetSelectionInfo().select = select_Old;

			// 選択エリアを削除
			DeleteData(true);

			// 削除前の選択範囲を復元する
			if (!bBoxData) {
				// 削除された範囲を考慮して選択範囲を調整する
				if (selLogic.GetFrom().y == delLogic.GetTo().y) {	// 選択開始が削除末尾と同一行
					selLogic.SetFromX(
						selLogic.GetFrom().x
						- (delLogic.GetTo().x - delLogic.GetFrom().x)
					);
				}
				if (selLogic.GetTo().y == delLogic.GetTo().y) {	// 選択終了が削除末尾と同一行
					selLogic.SetToX(
						selLogic.GetTo().x
						- (delLogic.GetTo().x - delLogic.GetFrom().x)
					);
				}
				// Note.
				// (delLogic.GetTo().y - delLogic.GetFrom().y) だと実際の削除行数と同じになる
				// こともあるが、（削除行数−１）になることもある．
				// 例）フリーカーソルでの行番号クリック時の１行選択
				size_t nLines = pEditDoc->docLineMgr.GetLineCount();
				selLogic.SetFromY(selLogic.GetFrom().y - (nLines_Old - nLines));
				selLogic.SetToY(selLogic.GetTo().y - (nLines_Old - nLines));

				// 調整後の選択範囲を設定する
				Range select;
				pEditDoc->layoutMgr.LogicToLayout(
					selLogic,
					&select
				);
				GetSelectionInfo().SetSelectArea(select);
				ptCaretPos_Old = GetSelectionInfo().select.GetTo();
			}

			// キャレットを移動する
			caret.MoveCursor(ptCaretPos_Old, true);
			caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

			// 削除位置から移動先へのカーソル移動をUndo操作に追加する
			Point ptBefore = pEditDoc->layoutMgr.LayoutToLogic(GetSelectionInfo().select.GetFrom());
			commander.GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					delLogic.GetFrom(),
					caret.GetCaretLogicPos()
				)
			);
		}
	}
	GetSelectionInfo().DrawSelectArea();

	// アンドゥバッファの処理
	SetUndoBuffer();

	::GlobalUnlock(hData);
	if ((GMEM_LOCKCOUNT & ::GlobalFlags(hData)) == 0) {
		::GlobalFree(hData);
	}

	return S_OK;
}


/** 独自ドロップファイルメッセージをポストする */
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

/** 独自ドロップファイルメッセージ処理 */
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
		::SendMessage(editWnd.GetHwnd(), WM_DROPFILES, (WPARAM)hDrop, 0);
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
			memBuf.AppendString(szPath);
			if (nFiles > 1) {
				memBuf.AppendString(pEditDoc->docEditor.GetNewLineCode().GetValue2());
			}
		}
		::DragFinish(hDrop);

		// 選択範囲の選択解除
		if (GetSelectionInfo().IsTextSelected()) {
			GetSelectionInfo().DisableSelectArea(true);
		}

		// 挿入前のキャレット位置を記憶する
		// （キャレットが行終端より右の場合は埋め込まれる空白分だけ桁位置をシフト）
		Point ptCaretLogicPos_Old = GetCaret().GetCaretLogicPos();
		const Layout* pLayout;
		size_t nLineLen;
		Point ptCaretLayoutPos_Old = GetCaret().GetCaretLayoutPos();
		if (pEditDoc->layoutMgr.GetLineStr(ptCaretLayoutPos_Old.y, &nLineLen, &pLayout)) {
			size_t nLineAllColLen;
			LineColumnToIndex2(pLayout, ptCaretLayoutPos_Old.x, &nLineAllColLen);
			if (nLineAllColLen > 0) {	// 行終端より右の場合には nLineAllColLen に行全体の表示桁数が入っている
				ptCaretLogicPos_Old.SetX(
					ptCaretLogicPos_Old.x
					+ (ptCaretLayoutPos_Old.x - nLineAllColLen)
				);
			}
		}

		// テキスト挿入
		GetCommander().HandleCommand(F_INSTEXT_W, true, (LPARAM)memBuf.GetStringPtr(), memBuf.GetStringLength(), TRUE, 0);

		// 挿入前のキャレット位置から挿入後のキャレット位置までを選択範囲にする
		Point ptSelectFrom = pEditDoc->layoutMgr.LogicToLayout(ptCaretLogicPos_Old);
		GetSelectionInfo().SetSelectArea(Range(ptSelectFrom, GetCaret().GetCaretLayoutPos()));
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
	else if (IsDataAvailable(pDataObject, CF_HDROP))
		cf = CF_HDROP;

	return cf;
}

DWORD EditView::TranslateDropEffect(CLIPFORMAT cf, DWORD dwKeyState, POINTL pt, DWORD dwEffect)
{
	if (cf == CF_HDROP)
		return DROPEFFECT_LINK;

	EditView* pDragSourceView = editWnd.GetDragSourceView();

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
	return (this == editWnd.GetDragSourceView());
}


