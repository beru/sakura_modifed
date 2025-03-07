/*!	@file
	@brief 置換ダイアログ
*/
#include "StdAfx.h"
#include "dlg/DlgReplace.h"
#include "view/EditView.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {	//11900
	IDC_BUTTON_SEARCHNEXT,			HIDC_REP_BUTTON_SEARCHNEXT,			// 下検索
	IDC_BUTTON_SEARCHPREV,			HIDC_REP_BUTTON_SEARCHPREV,			// 上検索
	IDC_BUTTON_REPALCE,				HIDC_REP_BUTTON_REPALCE,			// 置換
	IDC_BUTTON_REPALCEALL,			HIDC_REP_BUTTON_REPALCEALL,			// 全置換
	IDCANCEL,						HIDCANCEL_REP,						// キャンセル
	IDC_BUTTON_HELP,				HIDC_REP_BUTTON_HELP,				// ヘルプ
	IDC_CHK_PASTE,					HIDC_REP_CHK_PASTE,					// クリップボードから貼り付け
	IDC_CHK_WORD,					HIDC_REP_CHK_WORD,					// 単語単位
	IDC_CHK_LOHICASE,				HIDC_REP_CHK_LOHICASE,				// 大文字小文字
	IDC_CHK_REGULAREXP,				HIDC_REP_CHK_REGULAREXP,			// 正規表現
	IDC_CHECK_NOTIFYNOTFOUND,		HIDC_REP_CHECK_NOTIFYNOTFOUND,		// 見つからないときに通知
	IDC_CHECK_bAutoCloseDlgReplace,	HIDC_REP_CHECK_bAutoCloseDlgReplace,// 自動的に閉じる
	IDC_COMBO_TEXT,					HIDC_REP_COMBO_TEXT,				// 置換前
	IDC_COMBO_TEXT2,				HIDC_REP_COMBO_TEXT2,				// 置換後
	IDC_RADIO_REPLACE,				HIDC_REP_RADIO_REPLACE,				// 置換対象：置換
	IDC_RADIO_INSERT,				HIDC_REP_RADIO_INSERT,				// 置換対象：挿入
	IDC_RADIO_ADD,					HIDC_REP_RADIO_ADD,					// 置換対象：追加
	IDC_RADIO_LINEDELETE,			HIDC_REP_RADIO_LINEDELETE,			// 置換対象：行削除
	IDC_RADIO_SELECTEDAREA,			HIDC_REP_RADIO_SELECTEDAREA,		// 範囲：全体
	IDC_RADIO_ALLAREA,				HIDC_REP_RADIO_ALLAREA,				// 範囲：選択範囲
	IDC_STATIC_JRE32VER,			HIDC_REP_STATIC_JRE32VER,			// 正規表現バージョン
	IDC_BUTTON_SETMARK,				HIDC_REP_BUTTON_SETMARK,			// 検索該当行をマーク
	IDC_CHECK_SEARCHALL,			HIDC_REP_CHECK_SEARCHALL,			// 先頭（末尾）から再検索
	IDC_CHECK_CONSECUTIVEALL,		HIDC_REP_CHECK_CONSECUTIVEALL,		//「すべて置換」は置換の繰返し
//	IDC_STATIC,						-1,
	0, 0
};

DlgReplace::DlgReplace()
{
	searchOption.Reset();	// 検索オプション
	bConsecutiveAll = false;	//「すべて置換」は置換の繰返し
	bSelectedArea = false;	// 選択範囲内置換
	nReplaceTarget = 0;		// 置換対象
	bPaste = false;			// 貼り付ける？
	nReplaceCnt = 0;			// すべて置換の実行結果
	bCanceled = false;		// すべて置換を中断したか
}

/*!
	コンボボックスのドロップダウンメッセージを捕捉する
*/
BOOL DlgReplace::OnCbnDropDown(HWND hwndCtl, int wID)
{
	switch (wID) {
	case IDC_COMBO_TEXT:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			size_t nSize = pShareData->searchKeywords.searchKeys.size();
			for (size_t i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, pShareData->searchKeywords.searchKeys[i] );
			}
		}
		break;
	case IDC_COMBO_TEXT2:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			size_t nSize = pShareData->searchKeywords.replaceKeys.size();
			for (size_t i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, pShareData->searchKeywords.replaceKeys[i] );
			}
		}
		break;
	}
	return Dialog::OnCbnDropDown( hwndCtl, wID );
}

// モードレスダイアログの表示
HWND DlgReplace::DoModeless(
	HINSTANCE hInstance,
	HWND hwndParent,
	LPARAM lParam,
	bool bSelected
	)
{
	auto& csSearch = pShareData->common.search;
	searchOption = csSearch.searchOption;		// 検索オプション
	bConsecutiveAll = csSearch.bConsecutiveAll;	//「すべて置換」は置換の繰返し
	bSelectedArea = csSearch.bSelectedArea;		// 選択範囲内置換
	bNotifyNotFound = csSearch.bNotifyNotFound;	// 検索／置換  見つからないときメッセージを表示
	bSelected = bSelected;
	ptEscCaretPos_PHY = ((EditView*)lParam)->GetCaret().GetCaretLogicPos();	// 検索/置換開始時のカーソル位置退避
	((EditView*)lParam)->bSearch = true;			// 検索/置換開始位置の登録有無
	return Dialog::DoModeless(hInstance, hwndParent, IDD_REPLACE, lParam, SW_SHOW);
}

// モードレス時：置換・検索対象となるビューの変更
void DlgReplace::ChangeView(LPARAM pcEditView)
{
	lParam = pcEditView;
	return;
}


// ダイアログデータの設定
void DlgReplace::SetData(void)
{
	auto& csSearch = pShareData->common.search;

	// 検索文字列/置換後文字列リストの設定(関数化)
	SetCombosList();

	// 英大文字と英小文字を区別する
	CheckButton(IDC_CHK_LOHICASE, searchOption.bLoHiCase);

	// 単語単位で探す
	CheckButton(IDC_CHK_WORD, searchOption.bWordOnly);

	//「すべて置換」は置換の繰返し
	CheckButton(IDC_CHECK_CONSECUTIVEALL, bConsecutiveAll);

	// 正規表現ライブラリの差し替えに伴う処理の見直し
	// 処理フロー及び判定条件の見直し。必ず正規表現のチェックと
	// 無関係にCheckRegexpVersionを通過するようにした。
	if (CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, false)
		&& searchOption.bRegularExp
	) {
		// 英大文字と英小文字を区別する
		CheckButton(IDC_CHK_REGULAREXP, true);

		// 単語単位で探す
		EnableItem(IDC_CHK_WORD, false);
	}else {
		CheckButton(IDC_CHK_REGULAREXP, false);

		//「すべて置換」は置換の繰返し
		EnableItem(IDC_CHECK_CONSECUTIVEALL, false);
	}

	// 検索／置換  見つからないときメッセージを表示
	CheckButton(IDC_CHECK_NOTIFYNOTFOUND, bNotifyNotFound);

	// 置換 ダイアログを自動的に閉じる
	CheckButton(IDC_CHECK_bAutoCloseDlgReplace, csSearch.bAutoCloseDlgReplace);

	// 先頭（末尾）から再検索
	CheckButton(IDC_CHECK_SEARCHALL, csSearch.bSearchAll);

	// クリップボードから貼り付ける？
	CheckButton(IDC_CHK_PASTE, bPaste);
	// 置換対象
	if (nReplaceTarget == 0) {
		CheckButton(IDC_RADIO_REPLACE, true);
	}else if (nReplaceTarget == 1) {
		CheckButton(IDC_RADIO_INSERT, true);
	}else if (nReplaceTarget == 2) {
		CheckButton(IDC_RADIO_ADD, true);
	}else if (nReplaceTarget == 3) {
		CheckButton(IDC_RADIO_LINEDELETE, true);
		EnableItem(IDC_COMBO_TEXT2, false);
		EnableItem(IDC_CHK_PASTE, false);
	}

	return;
}


// 検索文字列/置換後文字列リストの設定
void DlgReplace::SetCombosList(void)
{
	// 検索文字列
	HWND hwndCombo = GetItemHwnd(IDC_COMBO_TEXT);
	while (Combo_GetCount(hwndCombo) > 0) {
		Combo_DeleteString(hwndCombo, 0);
	}
	int nBufferSize = ::GetWindowTextLength(hwndCombo) + 1;
	std::vector<TCHAR> vText;
	vText.resize(nBufferSize);
	Combo_GetText(hwndCombo, &vText[0], nBufferSize);
	if (auto_strcmp(to_wchar(&vText[0]), strText.c_str()) != 0) {
		SetItemText(IDC_COMBO_TEXT, strText.c_str());
	}

	// 置換後文字列
	hwndCombo = GetItemHwnd(IDC_COMBO_TEXT2);
	while (Combo_GetCount(hwndCombo) > 0) {
		Combo_DeleteString(hwndCombo, 0);
	}
	nBufferSize = ::GetWindowTextLength(hwndCombo) + 1;
	vText.resize(nBufferSize);
	Combo_GetText(hwndCombo, &vText[0], nBufferSize);
	if (auto_strcmp(to_wchar(&vText[0]), strText2.c_str()) != 0) {
		SetItemText(IDC_COMBO_TEXT2, strText2.c_str());
	}
}


// ダイアログデータの取得
// 0==条件未入力  0より大きい==正常   0より小さい==入力エラー
int DlgReplace::GetData(void)
{
	auto& csSearch = pShareData->common.search;

	// 英大文字と英小文字を区別する
	searchOption.bLoHiCase = IsButtonChecked(IDC_CHK_LOHICASE);

	// 単語単位で探す
	searchOption.bWordOnly = IsButtonChecked(IDC_CHK_WORD);

	//「すべて置換」は置換の繰返し
	bConsecutiveAll = IsButtonChecked(IDC_CHECK_CONSECUTIVEALL);

	// 正規表現
	searchOption.bRegularExp = IsButtonChecked(IDC_CHK_REGULAREXP);
	// 選択範囲内置換
	bSelectedArea = IsButtonChecked(IDC_RADIO_SELECTEDAREA);
	// 検索／置換  見つからないときメッセージを表示
	bNotifyNotFound = IsButtonChecked(IDC_CHECK_NOTIFYNOTFOUND);

	csSearch.bConsecutiveAll = bConsecutiveAll;	// 1==「すべて置換」は置換の繰返し
	csSearch.bSelectedArea = bSelectedArea;		// 選択範囲内置換
	csSearch.bNotifyNotFound = bNotifyNotFound;	// 検索／置換  見つからないときメッセージを表示

	// 検索文字列
	int nBufferSize = ::GetWindowTextLength(GetItemHwnd(IDC_COMBO_TEXT)) + 1;
	std::vector<TCHAR> vText(nBufferSize);
	GetItemText(IDC_COMBO_TEXT, &vText[0], nBufferSize);
	strText = to_wchar(&vText[0]);
	// 置換後文字列
	if (IsButtonChecked(IDC_RADIO_LINEDELETE )) {
		strText2 = L"";
	}else {
		nBufferSize = ::GetWindowTextLength(GetItemHwnd(IDC_COMBO_TEXT2)) + 1;
		vText.resize(nBufferSize);
		GetItemText(IDC_COMBO_TEXT2, &vText[0], nBufferSize);
		strText2 = to_wchar(&vText[0]);
	}

	// 置換 ダイアログを自動的に閉じる
	csSearch.bAutoCloseDlgReplace = IsButtonChecked(IDC_CHECK_bAutoCloseDlgReplace);

	// 先頭（末尾）から再検索
	csSearch.bSearchAll = IsButtonChecked(IDC_CHECK_SEARCHALL);

	if (0 < strText.size()) {
		// 正規表現？
		int nFlag = 0x00;
		nFlag |= searchOption.bLoHiCase ? 0x01 : 0x00;
		if (searchOption.bRegularExp
			&& !CheckRegexpSyntax(strText.c_str(), GetHwnd(), true, nFlag)
		) {
			return -1;
		}

		// 検索文字列
		if (strText.size() < _MAX_PATH) {
			SearchKeywordManager().AddToSearchKeys(strText.c_str());
			csSearch.searchOption = searchOption;		// 検索オプション
		}
		// viewに直接設定
		EditView* pEditView = (EditView*)lParam;
		if (pEditView->strCurSearchKey == strText && pEditView->curSearchOption == searchOption) {
		}else {
			pEditView->strCurSearchKey = strText;
			pEditView->curSearchOption = searchOption;
			pEditView->bCurSearchUpdate = true;
		}
		pEditView->nCurSearchKeySequence = GetDllShareData().common.search.nSearchKeySequence;

		// 置換後文字列
		if (strText2.size() < _MAX_PATH) {
			SearchKeywordManager().AddToReplaceKeys(strText2.c_str());
		}
		nReplaceKeySequence = GetDllShareData().common.search.nReplaceKeySequence;

		// クリップボードから貼り付ける？
		bPaste = IsButtonChecked(IDC_CHK_PASTE);
		EnableItem(IDC_COMBO_TEXT2, !bPaste);
		// 置換対象
		nReplaceTarget = 0;
		if (IsButtonChecked(IDC_RADIO_INSERT)) {
			nReplaceTarget = 1;
		}else if (IsButtonChecked(IDC_RADIO_ADD)) {
			nReplaceTarget = 2;
		}else if (IsButtonChecked(IDC_RADIO_LINEDELETE )) {
			nReplaceTarget = 3;
			bPaste = false;
			EnableItem(IDC_COMBO_TEXT2, false);
		}
		// 検索文字列/置換後文字列リストの設定
		if (!bModal) {
			SetCombosList();
		}
		return 1;
	}else {
		return 0;
	}
}


BOOL DlgReplace::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwndDlg);
	// コンボボックスのユーザー インターフェイスを拡張インターフェースにする
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_TEXT), TRUE);
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_TEXT2), TRUE);

	// テキスト選択中か
	if (bSelected) {
		EnableItem(IDC_BUTTON_SEARCHPREV, false);
		EnableItem(IDC_BUTTON_SEARCHNEXT, false);
		EnableItem(IDC_BUTTON_REPALCE, false);
		CheckButton(IDC_RADIO_SELECTEDAREA, true);
	}else {
		CheckButton(IDC_RADIO_ALLAREA, true);
	}

	comboDelText = ComboBoxItemDeleter();
	comboDelText.pRecent = &recentSearch;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_TEXT), &comboDelText);
	comboDelText2 = ComboBoxItemDeleter();
	comboDelText2.pRecent = &recentReplace;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_TEXT2), &comboDelText2);

	// フォント設定
	HFONT hFontOld = (HFONT)::SendMessage(GetItemHwnd(IDC_COMBO_TEXT), WM_GETFONT, 0, 0);
	HFONT hFont = SetMainFont(GetItemHwnd(IDC_COMBO_TEXT));
	fontText.SetFont(hFontOld, hFont, GetItemHwnd(IDC_COMBO_TEXT));

	hFontOld = (HFONT)::SendMessage(GetItemHwnd(IDC_COMBO_TEXT2), WM_GETFONT, 0, 0);
	hFont = SetMainFont(GetItemHwnd(IDC_COMBO_TEXT2));
	fontText2.SetFont(hFontOld, hFont, GetItemHwnd(IDC_COMBO_TEXT2));

	// 基底クラスメンバ
	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}


BOOL DlgReplace::OnDestroy()
{
	fontText.ReleaseOnDestroy();
	fontText2.ReleaseOnDestroy();
	return Dialog::OnDestroy();
}


BOOL DlgReplace::OnBnClicked(int wID)
{
	int nRet;
	EditView* pEditView = (EditView*)lParam;

	switch (wID) {
	case IDC_CHK_PASTE:
		// テキストの貼り付け
		if (1
			&& IsButtonChecked(IDC_CHK_PASTE)
			&& !pEditView->pEditDoc->docEditor.IsEnablePaste()
		) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_CLIPBOARD));
			CheckButton(IDC_CHK_PASTE, false);
		}
		EnableItem(IDC_COMBO_TEXT2, !IsButtonChecked(IDC_CHK_PASTE));
		return TRUE;
		// 置換対象
	case IDC_RADIO_REPLACE:
	case IDC_RADIO_INSERT:
	case IDC_RADIO_ADD:
	case IDC_RADIO_LINEDELETE:
		if (IsButtonChecked(IDC_RADIO_LINEDELETE )) {
			EnableItem(IDC_COMBO_TEXT2, false);
			EnableItem(IDC_CHK_PASTE, false);
		}else {
			EnableItem(IDC_COMBO_TEXT2, true);
			EnableItem(IDC_CHK_PASTE, true);
		}
		return TRUE;
	case IDC_RADIO_SELECTEDAREA:
		// 範囲範囲
		if (IsButtonChecked(IDC_RADIO_ALLAREA)) {
			EnableItem(IDC_BUTTON_SEARCHPREV, true);
			EnableItem(IDC_BUTTON_SEARCHNEXT, true);
			EnableItem(IDC_BUTTON_REPALCE, true);
		}else {
			EnableItem(IDC_BUTTON_SEARCHPREV, false);
			EnableItem(IDC_BUTTON_SEARCHNEXT, false);
			EnableItem(IDC_BUTTON_REPALCE, false);
		}
		return TRUE;
	case IDC_RADIO_ALLAREA:
		// ファイル全体
		if (IsButtonChecked(IDC_RADIO_ALLAREA)) {
			EnableItem(IDC_BUTTON_SEARCHPREV, true);
			EnableItem(IDC_BUTTON_SEARCHNEXT, true);
			EnableItem(IDC_BUTTON_REPALCE, true);
		}else {
			EnableItem(IDC_BUTTON_SEARCHPREV, false);
			EnableItem(IDC_BUTTON_SEARCHNEXT, false);
			EnableItem(IDC_BUTTON_REPALCE, false);
		}
		return TRUE;
	case IDC_BUTTON_HELP:
		//「置換」のヘルプ
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_REPLACE_DIALOG));
		return TRUE;
//	case IDC_CHK_LOHICASE:	// 大文字と小文字を区別する
//		MYTRACE(_T("IDC_CHK_LOHICASE\n"));
//		return TRUE;
//	case IDC_CHK_WORDONLY:	// 一致する単語のみ検索
//		MYTRACE(_T("IDC_CHK_WORDONLY\n"));
//		break;
	case IDC_CHK_REGULAREXP:	// 正規表現
//		MYTRACE(_T("IDC_CHK_REGULAREXP ::IsDlgButtonChecked(GetHwnd(), IDC_CHK_REGULAREXP) = %d\n"), ::IsDlgButtonChecked(GetHwnd(), IDC_CHK_REGULAREXP));
		if (IsButtonChecked(IDC_CHK_REGULAREXP)) {
			// 正規表現ライブラリの差し替えに伴う処理の見直し
			if (!CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, true)) {
				CheckButton(IDC_CHK_REGULAREXP, false);
			}else {
				// 単語単位で探す
				EnableItem(IDC_CHK_WORD, false);

				//「すべて置換」は置換の繰返し
				EnableItem(IDC_CHECK_CONSECUTIVEALL, true);
			}
		}else {
			// 単語単位で探す
			EnableItem(IDC_CHK_WORD, true);

			//「すべて置換」は置換の繰返し
			EnableItem(IDC_CHECK_CONSECUTIVEALL, false);
		}
		return TRUE;
//	case IDOK:			// 下検索
//		// ダイアログデータの取得
//		nRet = GetData();
//		if (0 < nRet) {
//			::EndDialog(hwndDlg, 2);
//		}else
//		if (nRet == 0) {
//			::EndDialog(hwndDlg, 0);
//		}
//		return TRUE;


	case IDC_BUTTON_SEARCHPREV:	// 上検索
		nRet = GetData();
		if (0 < nRet) {

			// 検索開始位置を登録
			if (pEditView->bSearch != FALSE) {
				pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
				pEditView->bSearch = FALSE;
			}

			// コマンドコードによる処理振り分け
			// 前を検索
			pEditView->GetCommander().HandleCommand(F_SEARCH_PREV, true, (LPARAM)GetHwnd(), 0, 0, 0);
			// 再描画（0文字幅マッチでキャレットを表示するため）
			pEditView->Redraw();	// 前回0文字幅マッチの消去にも必要
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_STR));
		}
		return TRUE;
	case IDC_BUTTON_SEARCHNEXT:	// 下検索
		nRet = GetData();
		if (0 < nRet) {

			// 検索開始位置を登録
			if (pEditView->bSearch) {
				pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
				pEditView->bSearch = false;
			}

			// コマンドコードによる処理振り分け
			// 次を検索
			pEditView->GetCommander().HandleCommand(F_SEARCH_NEXT, true, (LPARAM)GetHwnd(), 0, 0, 0);
			// 再描画（0文字幅マッチでキャレットを表示するため）
			pEditView->Redraw();	// 前回0文字幅マッチの消去にも必要
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_STR));
		}
		return TRUE;

	case IDC_BUTTON_SETMARK:	// 該当行マーク
		nRet = GetData();
		if (0 < nRet) {
			pEditView->GetCommander().HandleCommand(F_BOOKMARK_PATTERN, false, 0, 0, 0, 0);
			::SendMessage(GetHwnd(), WM_NEXTDLGCTL, (WPARAM)GetItemHwnd(IDC_COMBO_TEXT), TRUE);
		}
		return TRUE;

	case IDC_BUTTON_REPALCE:	// 置換
		nRet = GetData();
		if (0 < nRet) {

			// 置換開始位置を登録
			if (pEditView->bSearch) {
				pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
				pEditView->bSearch = false;
			}

			// 置換
			pEditView->GetCommander().HandleCommand(F_REPLACE, true, (LPARAM)GetHwnd(), 0, 0, 0);
			// 再描画
			pEditView->GetCommander().HandleCommand(F_REDRAW, true, 0, 0, 0, 0);
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_STR));
		}
		return TRUE;
	case IDC_BUTTON_REPALCEALL:	// すべて置換
		nRet = GetData();
		if (0 < nRet) {
			// 置換開始位置を登録
			if (pEditView->bSearch) {
				pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
				pEditView->bSearch = false;
			}

			pEditView->GetCommander().HandleCommand(F_REPLACE_ALL, true, 0, 0, 0, 0);
			pEditView->GetCommander().HandleCommand(F_REDRAW, true, 0, 0, 0, 0);

			// アクティブにする
			ActivateFrameWindow(GetHwnd());

			TopOkMessage(GetHwnd(), LS(STR_DLGREPLC_REPLACE), nReplaceCnt);

			if (!bCanceled) {
				if (bModal) {		// モーダルダイアログか
					// 置換ダイアログを閉じる
					::EndDialog(GetHwnd(), 0);
				}else {
					// 置換 ダイアログを自動的に閉じる
					if (pShareData->common.search.bAutoCloseDlgReplace) {
						::DestroyWindow(GetHwnd());
					}
				}
			}
			return TRUE;
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_REPSTR));
		}
		return TRUE;
//	case IDCANCEL:
//		::EndDialog(hwndDlg, 0);
//		return TRUE;
	}

	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);
}

BOOL DlgReplace::OnActivate(WPARAM wParam, LPARAM lParam)
{
	// 0文字幅マッチ描画のON/OFF
	EditView*	pEditView = (EditView*)(this->lParam);
	Range rangeSel = pEditView->GetSelectionInfo().select;
	if (rangeSel.IsValid() && rangeSel.IsLineOne() && rangeSel.IsOne())
		pEditView->InvalidateRect(NULL);	// アクティブ化／非アクティブ化が完了してから再描画

	return Dialog::OnActivate(wParam, lParam);
}

LPVOID DlgReplace::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


