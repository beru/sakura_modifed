/*!	@file
	@brief 検索ダイアログボックス
*/

#include "StdAfx.h"
#include "dlg/DlgFind.h"
#include "view/EditView.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

// 検索 CDlgFind.cpp
const DWORD p_helpids[] = {	//11800
	IDC_BUTTON_SEARCHNEXT,			HIDC_FIND_BUTTON_SEARCHNEXT,		// 次を検索
	IDC_BUTTON_SEARCHPREV,			HIDC_FIND_BUTTON_SEARCHPREV,		// 前を検索
	IDCANCEL,						HIDCANCEL_FIND,						// キャンセル
	IDC_BUTTON_HELP,				HIDC_FIND_BUTTON_HELP,				// ヘルプ
	IDC_CHK_WORD,					HIDC_FIND_CHK_WORD,					// 単語単位
	IDC_CHK_LOHICASE,				HIDC_FIND_CHK_LOHICASE,				// 大文字小文字
	IDC_CHK_REGULAREXP,				HIDC_FIND_CHK_REGULAREXP,			// 正規表現
	IDC_CHECK_NOTIFYNOTFOUND,		HIDC_FIND_CHECK_NOTIFYNOTFOUND,		// 見つからないときに通知
	IDC_CHECK_bAutoCloseDlgFind,	HIDC_FIND_CHECK_bAutoCloseDlgFind,	// 自動的に閉じる
	IDC_COMBO_TEXT,					HIDC_FIND_COMBO_TEXT,				// 検索文字列
	IDC_STATIC_JRE32VER,			HIDC_FIND_STATIC_JRE32VER,			// 正規表現バージョン
	IDC_BUTTON_SETMARK,				HIDC_FIND_BUTTON_SETMARK,			// 検索該当行をマーク
	IDC_CHECK_SEARCHALL,			HIDC_FIND_CHECK_SEARCHALL,			// 先頭（末尾）から再検索
//	IDC_STATIC,						-1,
	0, 0
};

DlgFind::DlgFind()
{
	searchOption.Reset();
	return;
}


/*!
	コンボボックスのドロップダウンメッセージを捕捉する
*/
BOOL DlgFind::OnCbnDropDown(HWND hwndCtl, int wID)
{
	switch (wID) {
	case IDC_COMBO_TEXT:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			auto& keywords = pShareData->searchKeywords.searchKeys;
			size_t nSize = keywords.size();
			for (size_t i=0; i<nSize; ++i) {
				Combo_AddString(hwndCtl, keywords[i]);
			}
		}
		break;
	}
	return Dialog::OnCbnDropDown( hwndCtl, wID );
}


// モードレスダイアログの表示
HWND DlgFind::DoModeless(
	HINSTANCE hInstance,
	HWND hwndParent,
	LPARAM lParam
	)
{
	auto& csSearch = pShareData->common.search;
	searchOption = csSearch.searchOption;		// 検索オプション
	bNotifyNotFound = csSearch.bNotifyNotFound;	// 検索／置換  見つからないときメッセージを表示
	ptEscCaretPos_PHY = ((EditView*)lParam)->GetCaret().GetCaretLogicPos();	// 検索開始時のカーソル位置退避
	((EditView*)lParam)->bSearch = TRUE;							// 検索開始位置の登録有無
	return Dialog::DoModeless(hInstance, hwndParent, IDD_FIND, lParam, SW_SHOW);
}

// モードレス時：検索対象となるビューの変更
void DlgFind::ChangeView(LPARAM pcEditView)
{
	lParam = pcEditView;
	return;
}


BOOL DlgFind::OnInitDialog(
	HWND hwnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	BOOL bRet = Dialog::OnInitDialog(hwnd, wParam, lParam);
	comboDel = ComboBoxItemDeleter();
	comboDel.pRecent = &recentSearch;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_TEXT), &comboDel);

	// フォント設定
	HFONT hFontOld = (HFONT)::SendMessage(GetItemHwnd(IDC_COMBO_TEXT), WM_GETFONT, 0, 0);
	HFONT hFont = SetMainFont(GetItemHwnd(IDC_COMBO_TEXT));
	fontText.SetFont(hFontOld, hFont, GetItemHwnd(IDC_COMBO_TEXT));
	return bRet;
}


BOOL DlgFind::OnDestroy()
{
	fontText.ReleaseOnDestroy();
	return Dialog::OnDestroy();
}


// ダイアログデータの設定
void DlgFind::SetData(void)
{
//	MYTRACE(_T("DlgFind::SetData()"));

	/*****************************
	*           初期化           *
	*****************************/
	// コンボボックスのユーザー インターフェイスを拡張インターフェースにする
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_TEXT), TRUE);


	/*****************************
	*         データ設定         *
	*****************************/
	// 検索文字列
	SetCombosList();

	// 英大文字と英小文字を区別する
	CheckButton(IDC_CHK_LOHICASE, searchOption.bLoHiCase);

	// 単語単位で検索
	CheckButton(IDC_CHK_WORD, searchOption.bWordOnly);

	// 検索／置換  見つからないときメッセージを表示
	CheckButton(IDC_CHECK_NOTIFYNOTFOUND, bNotifyNotFound);

	// 正規表現ライブラリの差し替えに伴う処理の見直し
	// 処理フロー及び判定条件の見直し。必ず正規表現のチェックと
	// 無関係にCheckRegexpVersionを通過するようにした。
	if (1
		&& CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, false)
		&& searchOption.bRegularExp
	) {
		// 英大文字と英小文字を区別する
		CheckButton(IDC_CHK_REGULAREXP, true);
// 正規表現がONでも、大文字小文字を区別する／しないを選択できるように。
//		CheckButton(IDC_CHK_LOHICASE, true);
//		EnableItem(IDC_CHK_LOHICASE, FALSE);

		// 単語単位で探す
		EnableItem(IDC_CHK_WORD, false);
	}else {
		CheckButton(IDC_CHK_REGULAREXP, false);
	}

	// 検索ダイアログを自動的に閉じる
	CheckButton(IDC_CHECK_bAutoCloseDlgFind, pShareData->common.search.bAutoCloseDlgFind);

	// 先頭（末尾）から再検索
	CheckButton(IDC_CHECK_SEARCHALL, pShareData->common.search.bSearchAll);

	return;
}


// 検索文字列リストの設定
void DlgFind::SetCombosList(void)
{
	// 検索文字列
	HWND hwndCombo = GetItemHwnd(IDC_COMBO_TEXT);
	while (Combo_GetCount(hwndCombo) > 0) {
		Combo_DeleteString(hwndCombo, 0);
	}
	int nBufferSize = ::GetWindowTextLength(GetItemHwnd(IDC_COMBO_TEXT)) + 1;
	std::vector<TCHAR> vText(nBufferSize);
	Combo_GetText(hwndCombo, &vText[0], nBufferSize);
	if (auto_strcmp(to_wchar(&vText[0]), strText.c_str()) != 0) {
		SetItemText(IDC_COMBO_TEXT, strText.c_str());
	}
}


// ダイアログデータの取得
int DlgFind::GetData(void)
{
//	MYTRACE(_T("DlgFind::GetData()"));

	// 英大文字と英小文字を区別する
	searchOption.bLoHiCase = IsButtonChecked(IDC_CHK_LOHICASE);

	// 単語単位で検索
	searchOption.bWordOnly = IsButtonChecked(IDC_CHK_WORD);

	// 一致する単語のみ検索する
	// 正規表現
	searchOption.bRegularExp = IsButtonChecked(IDC_CHK_REGULAREXP);

	// 検索／置換  見つからないときメッセージを表示
	bNotifyNotFound = IsButtonChecked(IDC_CHECK_NOTIFYNOTFOUND);

	pShareData->common.search.bNotifyNotFound = bNotifyNotFound;	// 検索／置換  見つからないときメッセージを表示

	// 検索文字列
	int nBufferSize = ::GetWindowTextLength(GetItemHwnd(IDC_COMBO_TEXT)) + 1;
	std::vector<TCHAR> vText(nBufferSize);
	GetItemText(IDC_COMBO_TEXT, &vText[0], nBufferSize);
	strText = to_wchar(&vText[0]);

	// 検索ダイアログを自動的に閉じる
	pShareData->common.search.bAutoCloseDlgFind = IsButtonChecked(IDC_CHECK_bAutoCloseDlgFind);

	// 先頭（末尾）から再検索
	pShareData->common.search.bSearchAll = IsButtonChecked(IDC_CHECK_SEARCHALL);

	if (0 < strText.length()) {
		// 正規表現？
		// 正規表現ライブラリの差し替えに伴う処理の見直し
		int nFlag = 0x00;
		nFlag |= searchOption.bLoHiCase ? 0x01 : 0x00;
		if (searchOption.bRegularExp
			&& !CheckRegexpSyntax(strText.c_str(), GetHwnd(), true, nFlag)
		) {
			return -1;
		}
		// 正規表現ライブラリ差し替え

		// 検索文字列
		if (strText.size() < _MAX_PATH) {
			SearchKeywordManager().AddToSearchKeys(strText.c_str());
			pShareData->common.search.searchOption = searchOption;		// 検索オプション
		}
		EditView* pEditView = (EditView*)lParam;
		if (1
			&& pEditView->strCurSearchKey == strText
			&& pEditView->curSearchOption == searchOption
		) {
		}else {
			pEditView->strCurSearchKey = strText;
			pEditView->curSearchOption = searchOption;
			pEditView->bCurSearchUpdate = true;
		}
		pEditView->nCurSearchKeySequence = GetDllShareData().common.search.nSearchKeySequence;
		if (!bModal) {
			// ダイアログデータの設定
			//SetData();
			SetCombosList();		// コンボのみの初期化
		}
		return 1;
	}else {
		return 0;
	}
}


BOOL DlgFind::OnBnClicked(int wID)
{
	int nRet;
	EditView*	pEditView = (EditView*)lParam;
	switch (wID) {
	case IDC_BUTTON_HELP:
		//「検索」のヘルプ
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_SEARCH_DIALOG));
		break;
	case IDC_CHK_REGULAREXP:	// 正規表現
//		MYTRACE(_T("IDC_CHK_REGULAREXP ::IsDlgButtonChecked(GetHwnd(), IDC_CHK_REGULAREXP) = %d\n"), ::IsDlgButtonChecked(GetHwnd(), IDC_CHK_REGULAREXP));
		if (IsButtonChecked(IDC_CHK_REGULAREXP)) {
			if (!CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, true)) {
				CheckButton(IDC_CHK_REGULAREXP, false);
			}else {
				// 単語単位で検索
				EnableItem(IDC_CHK_WORD, false);
			}
		}else {
			// 単語単位で検索
			EnableItem(IDC_CHK_WORD, true);
		}
		break;
	case IDC_BUTTON_SEARCHPREV:	// 上検索
		// ダイアログデータの取得
		nRet = GetData();
		if (0 < nRet) {
			if (bModal) {		// モーダルダイアログか
				CloseDialog(1);
			}else {
				// 前を検索
				pEditView->GetCommander().HandleCommand(F_SEARCH_PREV, true, (LPARAM)GetHwnd(), 0, 0, 0);

				// 0文字幅マッチでキャレットを表示するため
				pEditView->Redraw();	// 前回0文字幅マッチの消去にも必要

				// 検索開始位置を登録
				if (pEditView->bSearch) {
					// 検索開始時のカーソル位置登録条件変更
					pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
					pEditView->bSearch = false;
				}

				// 検索ダイアログを自動的に閉じる
				if (pShareData->common.search.bAutoCloseDlgFind) {
					CloseDialog(0);
				}
			}
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGFIND1));	// 検索条件を指定してください。
		}
		return TRUE;
	case IDC_BUTTON_SEARCHNEXT:		// 下検索
		// ダイアログデータの取得
		nRet = GetData();
		if (0 < nRet) {
			if (bModal) {		// モーダルダイアログか
				CloseDialog(2);
			}else {
				// 次を検索
				pEditView->GetCommander().HandleCommand(F_SEARCH_NEXT, true, (LPARAM)GetHwnd(), 0, 0, 0);

				// 再描画 0文字幅マッチでキャレットを表示するため
				pEditView->Redraw();	// 前回0文字幅マッチの消去にも必要

				// 検索開始位置を登録
				if (pEditView->bSearch) {
					// 検索開始時のカーソル位置登録条件変更
					pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
					pEditView->bSearch = false;
				}

				// 検索ダイアログを自動的に閉じる
				if (pShareData->common.search.bAutoCloseDlgFind) {
					CloseDialog(0);
				}
			}
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGFIND1));	// 検索条件を指定してください。
		}
		return TRUE;
	case IDC_BUTTON_SETMARK:
		if (0 < GetData()) {
			if (bModal) {		// モーダルダイアログか
				CloseDialog(2);
			}else {
				pEditView->GetCommander().HandleCommand(F_BOOKMARK_PATTERN, false, 0, 0, 0, 0);
				// 検索ダイアログを自動的に閉じる
				if (pShareData->common.search.bAutoCloseDlgFind) {
					CloseDialog(0);
				}else {
					::SendMessage(GetHwnd(), WM_NEXTDLGCTL, (WPARAM)GetItemHwnd(IDC_COMBO_TEXT), TRUE);
				}
			}
		}
		return TRUE;
	case IDCANCEL:
		CloseDialog(0);
		return TRUE;
	}
	return FALSE;
}

BOOL DlgFind::OnActivate(WPARAM wParam, LPARAM lParam)
{
	// 0文字幅マッチ描画のON/OFF
	EditView* pEditView = (EditView*)(this->lParam);
	Range rangeSel = pEditView->GetSelectionInfo().select;
	if (rangeSel.IsValid() && rangeSel.IsLineOne() && rangeSel.IsOne())
		pEditView->InvalidateRect(NULL);	// アクティブ化／非アクティブ化が完了してから再描画

	return Dialog::OnActivate(wParam, lParam);
}

LPVOID DlgFind::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

