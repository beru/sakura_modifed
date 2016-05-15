/*!	@file
	@brief 共通設定ダイアログボックス、「ウィンドウ」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, genta, MIK, asa-o
	Copyright (C) 2002, YAZAKI, genta, Moca, aroka
	Copyright (C) 2003, MIK, KEITA, genta
	Copyright (C) 2004, Moca
	Copyright (C) 2006, ryoji, fon
	Copyright (C) 2007, genta

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "dlg/DlgWinSize.h"	//	2004.05.13 Moca
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"
#include "_main/Process.h"

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//11200
	IDC_CHECK_DispFUNCKEYWND,		HIDC_CHECK_DispFUNCKEYWND,		// ファンクションキー表示
	IDC_CHECK_DispSTATUSBAR,		HIDC_CHECK_DispSTATUSBAR,		// ステータスバー表示
	IDC_CHECK_DispTOOLBAR,			HIDC_CHECK_DispTOOLBAR,			// ツールバー表示
	IDC_CHECK_bScrollBarHorz,		HIDC_CHECK_bScrollBarHorz,		// 水平スクロールバー
	IDC_CHECK_bMenuIcon,			HIDC_CHECK_bMenuIcon,			// アイコン付きメニュー
	IDC_CHECK_SplitterWndVScroll,	HIDC_CHECK_SplitterWndVScroll,	// 垂直スクロールの同期	//Jul. 05, 2001 JEPRO 追加
	IDC_CHECK_SplitterWndHScroll,	HIDC_CHECK_SplitterWndHScroll,	// 水平スクロールの同期	//Jul. 05, 2001 JEPRO 追加
	IDC_EDIT_nRulerBottomSpace,		HIDC_EDIT_nRulerBottomSpace,	// ルーラーの高さ
	IDC_EDIT_nRulerHeight,			HIDC_EDIT_nRulerHeight,			// ルーラーとテキストの間隔
	IDC_EDIT_nLineNumberRightSpace,	HIDC_EDIT_nLineNumberRightSpace,// 行番号とテキストの隙間
	IDC_RADIO_FUNCKEYWND_PLACE1,	HIDC_RADIO_FUNCKEYWND_PLACE1,	// ファンクションキー表示位置
	IDC_RADIO_FUNCKEYWND_PLACE2,	HIDC_RADIO_FUNCKEYWND_PLACE2,	// ファンクションキー表示位置
	IDC_EDIT_FUNCKEYWND_GROUPNUM,	HIDC_EDIT_FUNCKEYWND_GROUPNUM,	// ファンクションキーのグループボタン数
	IDC_SPIN_nRulerBottomSpace,		HIDC_EDIT_nRulerBottomSpace,
	IDC_SPIN_nRulerHeight,			HIDC_EDIT_nRulerHeight,
	IDC_SPIN_nLineNumberRightSpace,	HIDC_EDIT_nLineNumberRightSpace,
	IDC_SPIN_FUNCKEYWND_GROUPNUM,	HIDC_EDIT_FUNCKEYWND_GROUPNUM,
	IDC_WINCAPTION_ACTIVE,			HIDC_WINCAPTION_ACTIVE,			// アクティブ時	//@@@ 2003.06.15 MIK
	IDC_WINCAPTION_INACTIVE,		HIDC_WINCAPTION_INACTIVE,		// 非アクティブ時	//@@@ 2003.06.15 MIK
	IDC_BUTTON_WINSIZE,				HIDC_BUTTON_WINSIZE,			// 位置と大きさの設定	// 2006.08.06 ryoji
	IDC_COMBO_LANGUAGE,				HIDC_COMBO_LANGUAGE,			// 言語選択
	//	Feb. 11, 2007 genta TAB関連は「タブバー」シートへ移動
//	IDC_STATIC,						-1,
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
INT_PTR CALLBACK PropWin::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropWin::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
	}
//	To Here Jun. 2, 2001 genta


// メッセージ処理
INT_PTR PropWin::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,	// message
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
// From Here Sept. 9, 2000 JEPRO
	WORD		wNotifyCode;
	WORD		wID;
// To Here Sept. 9, 2000

	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	int			nVal;	// Sept.21, 2000 JEPRO スピン要素を加えたので復活させた

	switch (uMsg) {
	case WM_INITDIALOG:
		// ダイアログデータの設定 Window
		SetData(hwndDlg);
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// ユーザーがエディット コントロールに入力できるテキストの長さを制限する
		// ルーラー高さ
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_nRulerHeight), 2);
		// ルーラーとテキストの隙間
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_nRulerBottomSpace), 2);

		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch (idCtrl) {
		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_WIN);
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE(_T("Window PSN_KILLACTIVE\n"));
				// ダイアログデータの取得 Window
				GetData(hwndDlg);
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				nPageNum = ID_PROPCOM_PAGENUM_WIN;
				return TRUE;
			}
			break;
		case IDC_SPIN_nRulerHeight:
			// ルーラ－の高さ
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nRulerHeight, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < IDC_SPIN_nRulerHeight_MIN) {
				nVal = IDC_SPIN_nRulerHeight_MIN;
			}
			if (nVal > IDC_SPIN_nRulerHeight_MAX) {
				nVal = IDC_SPIN_nRulerHeight_MAX;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_nRulerHeight, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_nRulerBottomSpace:
			// ルーラーとテキストの隙間
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nRulerBottomSpace, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 0) {
				nVal = 0;
			}
			if (nVal > 32) {
				nVal = 32;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_nRulerBottomSpace, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_nLineNumberRightSpace:
			// ルーラーとテキストの隙間
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nLineNumberRightSpace, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 0) {
				nVal = 0;
			}
			if (nVal > 32) {
				nVal = 32;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_nLineNumberRightSpace, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_FUNCKEYWND_GROUPNUM:
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 12) {
				nVal = 12;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM, nVal, FALSE);
			return TRUE;
		}
		break;
//****	To Here Sept. 21, 2000
//	From Here Sept. 9, 2000 JEPRO
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// 通知コード
		wID			= LOWORD(wParam);	// 項目ID､ コントロールID､ またはアクセラレータID
		switch (wNotifyCode) {
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			//	ファンクションキーを表示する時だけその位置指定をEnableに設定
			case IDC_CHECK_DispFUNCKEYWND:
				EnableWinPropInput(hwndDlg);
				break;

			// From Here 2004.05.13 Moca 「位置と大きさの設定」ボタン
			//	ウィンドウ設定ダイアログにて起動時のウィンドウ状態指定
			case IDC_BUTTON_WINSIZE:
				{
					auto& csWindow = common.window;
					DlgWinSize dlgWinSize;
					RECT rc;
					rc.right  = csWindow.nWinSizeCX;
					rc.bottom = csWindow.nWinSizeCY;
					rc.top    = csWindow.nWinPosX;
					rc.left   = csWindow.nWinPosY;
					dlgWinSize.DoModal(
						::GetModuleHandle(NULL),
						hwndDlg,
						csWindow.eSaveWindowSize,
						csWindow.eSaveWindowPos,
						csWindow.nWinSizeType,
						rc
					);
					csWindow.nWinSizeCX = rc.right;
					csWindow.nWinSizeCY = rc.bottom;
					csWindow.nWinPosX = rc.top;
					csWindow.nWinPosY = rc.left;
				}
				break;
			// To Here 2004.05.13 Moca
			}
			break;
		}
		break;
//	To Here Sept. 9, 2000

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

// ダイアログデータの設定
void PropWin::SetData(HWND hwndDlg)
{
//	BOOL	bRet;
	auto& csWindow = common.window;

	// 次回ウィンドウを開いたときツールバーを表示する
	::CheckDlgButton(hwndDlg, IDC_CHECK_DispTOOLBAR, csWindow.bDispToolBar);

	// 次回ウィンドウを開いたときファンクションキーを表示する
	::CheckDlgButton(hwndDlg, IDC_CHECK_DispFUNCKEYWND, csWindow.bDispFuncKeyWnd);

	// ファンクションキー表示位置／0:上 1:下
	if (csWindow.nFuncKeyWnd_Place == 0) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1, TRUE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2, FALSE);
	}else {
		::CheckDlgButton(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1, FALSE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2, TRUE);
	}
	// 2002/11/04 Moca ファンクションキーのグループボタン数
	::SetDlgItemInt(hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM, csWindow.nFuncKeyWnd_GroupNum, FALSE);

	// From Here@@@ 2003.06.13 MIK
	//	Feb. 12, 2007 genta TAB関連は「タブバー」シートへ移動

	// To Here@@@ 2003.06.13 MIK
	//	Feb. 11, 2007 genta TAB関連は「タブバー」シートへ移動

	// 次回ウィンドウを開いたときステータスバーを表示する
	::CheckDlgButton(hwndDlg, IDC_CHECK_DispSTATUSBAR, csWindow.bDispStatusBar);

	// ルーラー高さ
	::SetDlgItemInt(hwndDlg, IDC_EDIT_nRulerHeight, csWindow.nRulerHeight, FALSE);
	// ルーラーとテキストの隙間
	::SetDlgItemInt(hwndDlg, IDC_EDIT_nRulerBottomSpace, csWindow.nRulerBottomSpace, FALSE);
	//	Sep. 18. 2002 genta 行番号とテキストの隙間
	::SetDlgItemInt(hwndDlg, IDC_EDIT_nLineNumberRightSpace, csWindow.nLineNumRightSpace, FALSE);

	// ルーラーのタイプ//	del 2008/7/4 Uchi
//	if (csWindow.nRulerType == 0) {
//		::CheckDlgButton(hwndDlg, IDC_RADIO_nRulerType_0, TRUE);
//		::CheckDlgButton(hwndDlg, IDC_RADIO_nRulerType_1, FALSE);
//	}else {
//		::CheckDlgButton(hwndDlg, IDC_RADIO_nRulerType_0, FALSE);
//		::CheckDlgButton(hwndDlg, IDC_RADIO_nRulerType_1, TRUE);
//	}

	// 水平スクロールバー
	::CheckDlgButton(hwndDlg, IDC_CHECK_bScrollBarHorz, csWindow.bScrollBarHorz);

	// アイコン付きメニュー
	::CheckDlgButton(hwndDlg, IDC_CHECK_bMenuIcon, csWindow.bMenuIcon);

	//	2001/06/20 Start by asa-o:	スクロールの同期
	::CheckDlgButton(hwndDlg, IDC_CHECK_SplitterWndVScroll, csWindow.bSplitterWndVScroll);
	::CheckDlgButton(hwndDlg, IDC_CHECK_SplitterWndHScroll, csWindow.bSplitterWndHScroll);
	//	2001/06/20 End

	//	Apr. 05, 2003 genta ウィンドウキャプションのカスタマイズ
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_WINCAPTION_ACTIVE  ), _countof(csWindow.szWindowCaptionActive  ) - 1);	//@@@ 2003.06.13 MIK
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_WINCAPTION_INACTIVE), _countof(csWindow.szWindowCaptionInactive) - 1);	//@@@ 2003.06.13 MIK
	::DlgItem_SetText(hwndDlg, IDC_WINCAPTION_ACTIVE, csWindow.szWindowCaptionActive);
	::DlgItem_SetText(hwndDlg, IDC_WINCAPTION_INACTIVE, csWindow.szWindowCaptionInactive);

	//	Fronm Here Sept. 9, 2000 JEPRO
	//	ファンクションキーを表示する時だけその位置指定をEnableに設定
	EnableWinPropInput(hwndDlg);
	//	To Here Sept. 9, 2000

	// 言語選択
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_LANGUAGE);
	Combo_ResetContent(hwndCombo);
	int nSelPos = 0;
	UINT uiIndex = 0;
	for (uiIndex=0; uiIndex<SelectLang::psLangInfoList.size(); ++uiIndex) {
		SelectLang::SelLangInfo* psLangInfo = SelectLang::psLangInfoList.at(uiIndex);
		Combo_InsertString(hwndCombo, uiIndex, psLangInfo->szLangName);
		if (_tcscmp(csWindow.szLanguageDll, psLangInfo->szDllName) == 0) {
			nSelPos = uiIndex;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	return;
}


// ダイアログデータの取得
int PropWin::GetData(HWND hwndDlg)
{
	auto& csWindow = common.window;

	// 次回ウィンドウを開いたときツールバーを表示する
	csWindow.bDispToolBar = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispTOOLBAR);

	// 次回ウィンドウを開いたときファンクションキーを表示する
	csWindow.bDispFuncKeyWnd = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispFUNCKEYWND);

	// ファンクションキー表示位置／0:上 1:下
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1)) {
		csWindow.nFuncKeyWnd_Place = 0;
	}
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2)) {
		csWindow.nFuncKeyWnd_Place = 1;
	}

	// 2002/11/04 Moca ファンクションキーのグループボタン数
	csWindow.nFuncKeyWnd_GroupNum = ::GetDlgItemInt(hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM, NULL, FALSE);
	if (csWindow.nFuncKeyWnd_GroupNum < 1) {
		csWindow.nFuncKeyWnd_GroupNum = 1;
	}
	if (csWindow.nFuncKeyWnd_GroupNum > 12) {
		csWindow.nFuncKeyWnd_GroupNum = 12;
	}

	// From Here@@@ 2003.06.13 MIK
	//	Feb. 12, 2007 genta TAB関連は「タブバー」シートへ移動
	// To Here@@@ 2003.06.13 MIK

	// 次回ウィンドウを開いたときステータスバーを表示する 
	csWindow.bDispStatusBar = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispSTATUSBAR);

	// ルーラーのタイプ//	del 2008/7/4 Uchi
//	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_nRulerType_0)) {
//		csWindow.nRulerType = 0;
//	}
//	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_nRulerType_1)) {
//		csWindow.nRulerType = 1;
//	}

	// ルーラー高さ
	csWindow.nRulerHeight = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nRulerHeight, NULL, FALSE);
	if (csWindow.nRulerHeight < IDC_SPIN_nRulerHeight_MIN) {
		csWindow.nRulerHeight = IDC_SPIN_nRulerHeight_MIN;
	}
	if (csWindow.nRulerHeight > IDC_SPIN_nRulerHeight_MAX) {
		csWindow.nRulerHeight = IDC_SPIN_nRulerHeight_MAX;
	}
	// ルーラーとテキストの隙間
	csWindow.nRulerBottomSpace = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nRulerBottomSpace, NULL, FALSE);
	if (csWindow.nRulerBottomSpace < 0) {
		csWindow.nRulerBottomSpace = 0;
	}
	if (csWindow.nRulerBottomSpace > 32) {
		csWindow.nRulerBottomSpace = 32;
	}

	//	Sep. 18. 2002 genta 行番号とテキストの隙間
	csWindow.nLineNumRightSpace = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nLineNumberRightSpace, NULL, FALSE);
	if (csWindow.nLineNumRightSpace < 0) {
		csWindow.nLineNumRightSpace = 0;
	}
	if (csWindow.nLineNumRightSpace > 32) {
		csWindow.nLineNumRightSpace = 32;
	}

	// 水平スクロールバー
	csWindow.bScrollBarHorz = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bScrollBarHorz);

	// アイコン付きメニュー
	csWindow.bMenuIcon = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bMenuIcon);

	//	2001/06/20 Start by asa-o:	スクロールの同期
	csWindow.bSplitterWndVScroll = DlgButton_IsChecked(hwndDlg, IDC_CHECK_SplitterWndVScroll);
	csWindow.bSplitterWndHScroll = DlgButton_IsChecked(hwndDlg, IDC_CHECK_SplitterWndHScroll);
	//	2001/06/20 End

	//	Apr. 05, 2003 genta ウィンドウキャプションのカスタマイズ
	::DlgItem_GetText(hwndDlg, IDC_WINCAPTION_ACTIVE, csWindow.szWindowCaptionActive,
		_countof(csWindow.szWindowCaptionActive));
	::DlgItem_GetText(hwndDlg, IDC_WINCAPTION_INACTIVE, csWindow.szWindowCaptionInactive,
		_countof(csWindow.szWindowCaptionInactive));

	// 言語選択
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_LANGUAGE);
	int nSelPos = Combo_GetCurSel(hwndCombo);
	SelectLang::SelLangInfo *psLangInfo = SelectLang::psLangInfoList.at(nSelPos);
	if (_tcscmp(csWindow.szLanguageDll, psLangInfo->szDllName) != 0) {
		_tcsncpy(csWindow.szLanguageDll, psLangInfo->szDllName, _countof(csWindow.szLanguageDll));
	}
	return TRUE;
}


//	From Here Sept. 9, 2000 JEPRO
//	チェック状態に応じてダイアログボックス要素のEnable/Disableを
//	適切に設定する
void PropWin::EnableWinPropInput(HWND hwndDlg)
{
	//	ファクションキーを表示するかどうか
	if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispFUNCKEYWND)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM), TRUE);	// IDC_GROUP_FUNCKEYWND_POSITION->IDC_EDIT_FUNCKEYWND_GROUPNUM 2008/7/4 Uchi
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM), FALSE);	// IDC_GROUP_FUNCKEYWND_POSITION->IDC_EDIT_FUNCKEYWND_GROUPNUM 2008/7/4 Uchi
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2), FALSE);
	}
}
//	To Here Sept. 9, 2000

