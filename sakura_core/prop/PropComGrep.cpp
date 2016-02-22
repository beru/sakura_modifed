/*!	@file
	@brief 共通設定ダイアログボックス、「検索」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, genta
	Copyright (C) 2002, YAZAKI, MIK
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "extmodule/Bregexp.h"	// 2007.08/12 genta バージョン取得
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10500
	IDC_EDIT_REGEXPLIB,					HIDC_EDIT_REGEXPLIB,				// 正規表現ライブラリ選択	// 2007.09.02 genta
	IDC_LABEL_REGEXP,					HIDC_EDIT_REGEXPLIB,
	IDC_LABEL_REGEXP_VER,				HIDC_LABEL_REGEXPVER,				// 正規表現ライブラリバージョン	// 2007.09.02 genta
	IDC_CHECK_bCaretTextForSearch,		HIDC_CHECK_bCaretTextForSearch,		// カーソル位置の文字列をデフォルトの検索文字列にする	// 2006.08.23 ryoji
	IDC_CHECK_INHERIT_KEY_OTHER_VIEW,	HIDC_CHECK_INHERIT_KEY_OTHER_VIEW,	// 次・前検索で他のビューの検索条件を引き継ぐ	// 2011.12.18 Moca
	IDC_CHECK_bGrepExitConfirm,			HIDC_CHECK_bGrepExitConfirm,		// GREPの保存確認
	IDC_CHECK_GTJW_RETURN,				HIDC_CHECK_GTJW_RETURN,				// タグジャンプ（エンターキー）
	IDC_CHECK_GTJW_LDBLCLK,				HIDC_CHECK_GTJW_LDBLCLK,			// タグジャンプ（ダブルクリック）
	IDC_CHECK_GREPREALTIME,				HIDC_CHECK_GREPREALTIME,			// リアルタイムで表示する	// 2006.08.08 ryoji
	IDC_COMBO_TAGJUMP,					HIDC_COMBO_TAGJUMP,					// タグファイルの検索
	IDC_COMBO_KEYWORD_TAGJUMP,			HIDC_COMBO_KEYWORD_TAGJUMP,			// タグファイルの検索
//	IDC_STATIC,							-1,
	0, 0
};
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK PropGrep::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropGrep::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}
//	To Here Jun. 2, 2001 genta

// メッセージ処理
INT_PTR PropGrep::DispatchEvent(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
//	WORD		wNotifyCode;
//	WORD		wID;
//	HWND		hwndCtl;
	NMHDR*		pNMHDR;
//	int			nVal;
//    LPDRAWITEMSTRUCT pDis;

	switch (uMsg) {
	case WM_INITDIALOG:
		// ダイアログデータの設定 Grep
		SetData(hwndDlg);
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// ユーザーがエディット コントロールに入力できるテキストの長さを制限する

		return TRUE;
	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
//		switch (idCtrl) {
//		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_GREP);
				return TRUE;
			case PSN_KILLACTIVE:
				// ダイアログデータの取得 Grep
				GetData(hwndDlg);
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = ID_PROPCOM_PAGENUM_GREP;
				return TRUE;
			}
//			break;	// default
//		}
		break;	// WM_NOTIFY
	case WM_COMMAND:
		//	2007.08.12 genta 正規表現DLLの変更に応じてVersionを再取得する
		if (wParam == MAKEWPARAM(IDC_EDIT_REGEXPLIB, EN_KILLFOCUS)) {
			SetRegexpVersion(hwndDlg);
		}
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		// NOTREACHED
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}

struct TagJumpMode {
	DWORD nMethod;
	DWORD nNameID;
};

// ダイアログデータの設定
void PropGrep::SetData(HWND hwndDlg)
{
	auto& csSearch = m_common.search;
	// 2006.08.23 ryoji カーソル位置の文字列をデフォルトの検索文字列にする
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_bCaretTextForSearch, csSearch.bCaretTextForSearch);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_INHERIT_KEY_OTHER_VIEW, csSearch.bInheritKeyOtherView);

	// Grepモードで保存確認するか
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_bGrepExitConfirm, csSearch.bGrepExitConfirm);

	// Grep結果のリアルタイム表示
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_GREPREALTIME, csSearch.bGrepRealTimeView);	// 2006.08.08 ryoji ID修正

	
	// Grepモード: エンターキーでタグジャンプ
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_GTJW_RETURN, csSearch.bGTJW_Return);

	// Grepモード: ダブルクリックでタグジャンプ
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_GTJW_LDBLCLK, csSearch.bGTJW_DoubleClick);

	//	2007.08.12 genta 正規表現DLL
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_REGEXPLIB), _countof(csSearch.szRegexpLib) - 1);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_REGEXPLIB, csSearch.szRegexpLib);
	SetRegexpVersion(hwndDlg);
	
	TagJumpMode tagJumpMode1Arr[] ={
		{ 0, STR_TAGJUMP_0 },
		{ 1, STR_TAGJUMP_1 },
		//{ 2, STR_TAGJUMP_2 },
		{ 3, STR_TAGJUMP_3 }
	};
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAGJUMP);
	Combo_ResetContent(hwndCombo);
	int nSelPos = 0;
	for (int i=0; i<_countof(tagJumpMode1Arr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(tagJumpMode1Arr[i].nNameID));
		Combo_SetItemData(hwndCombo, i, tagJumpMode1Arr[i].nMethod);
		if (tagJumpMode1Arr[i].nMethod == m_common.search.nTagJumpMode) {
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	TagJumpMode tagJumpMode2Arr[] ={
		{ 0, STR_TAGJUMP_0 },
		{ 1, STR_TAGJUMP_1 },
		{ 2, STR_TAGJUMP_2 },
		{ 3, STR_TAGJUMP_3 }
	};
	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_KEYWORD_TAGJUMP);
	Combo_ResetContent(hwndCombo);
	nSelPos = 0;
	for (int i=0; i<_countof(tagJumpMode2Arr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(tagJumpMode2Arr[i].nNameID));
		Combo_SetItemData(hwndCombo, i, tagJumpMode2Arr[i].nMethod);
		if (tagJumpMode2Arr[i].nMethod == m_common.search.nTagJumpModeKeyword) {
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	return;
}


// ダイアログデータの取得
int PropGrep::GetData(HWND hwndDlg)
{
	auto& csSearch = m_common.search;

	// 2006.08.23 ryoji カーソル位置の文字列をデフォルトの検索文字列にする
	csSearch.bCaretTextForSearch = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bCaretTextForSearch);
	csSearch.bInheritKeyOtherView = DlgButton_IsChecked(hwndDlg, IDC_CHECK_INHERIT_KEY_OTHER_VIEW);

	// Grepモードで保存確認するか
	csSearch.bGrepExitConfirm = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bGrepExitConfirm);

	// Grep結果のリアルタイム表示
	csSearch.bGrepRealTimeView = DlgButton_IsChecked(hwndDlg, IDC_CHECK_GREPREALTIME);	// 2006.08.08 ryoji ID修正

	// Grepモード: エンターキーでタグジャンプ
	csSearch.bGTJW_Return = DlgButton_IsChecked(hwndDlg, IDC_CHECK_GTJW_RETURN);

	// Grepモード: ダブルクリックでタグジャンプ
	csSearch.bGTJW_DoubleClick = DlgButton_IsChecked(hwndDlg, IDC_CHECK_GTJW_LDBLCLK);

	//	2007.08.12 genta 正規表現DLL
	::DlgItem_GetText(hwndDlg, IDC_EDIT_REGEXPLIB, csSearch.szRegexpLib, _countof(csSearch.szRegexpLib));

	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAGJUMP);
	int nSelPos = Combo_GetCurSel(hwndCombo);
	m_common.search.nTagJumpMode = Combo_GetItemData(hwndCombo, nSelPos);
	
	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_KEYWORD_TAGJUMP);
	nSelPos = Combo_GetCurSel(hwndCombo);
	m_common.search.nTagJumpModeKeyword = Combo_GetItemData(hwndCombo, nSelPos);

	return TRUE;
}

void PropGrep::SetRegexpVersion(HWND hwndDlg)
{
	TCHAR regexp_dll[_MAX_PATH];
	
	::DlgItem_GetText(hwndDlg, IDC_EDIT_REGEXPLIB, regexp_dll, _countof(regexp_dll));
	Bregexp breg;
	if (breg.InitDll(regexp_dll) != InitDllResultType::Success) {
		::DlgItem_SetText(hwndDlg, IDC_LABEL_REGEXP_VER, LS(STR_PROPCOMGREP_DLL));
		return;
	}
	::DlgItem_SetText(hwndDlg, IDC_LABEL_REGEXP_VER, breg.GetVersionT());
}

