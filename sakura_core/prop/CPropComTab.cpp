/*!	@file
	@brief 共通設定ダイアログボックス、「タブバー」ページ

	@author Norio Nakatani
	@date 2007.02.11 genta 共通設定に新規タブを追加
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2007, genta
	Copyright (C) 2001, MIK, genta
	Copyright (C) 2002, YAZAKI, MIK
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, genta, ryoji
	Copyright (C) 2012, Moca
	Copyright (C) 2013, Uchi

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
#include "prop/CPropCommon.h"
#include "CPropertyManager.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {
	IDC_CHECK_DispTabWnd,			HIDC_CHECK_DispTabWnd,			// タブウィンドウ表示	//@@@ 2003.05.31 MIK
	IDC_CHECK_GroupMultiTabWnd,		HIDC_CHECK_GroupMultiTabWnd,	// ウィンドウをまとめてグループ化する
	IDC_CHECK_RetainEmptyWindow,	HIDC_CHECK_RetainEmptyWindow,	// 最後のファイルを閉じたとき(無題)文書を残す	// 2007.02.13 ryoji
	IDC_CHECK_CloseOneWin,			HIDC_CHECK_CloseOneWin,			// ウィンドウの閉じるボタンは現在のファイルのみ閉じる	// 2007.02.13 ryoji
	IDC_CHECK_OpenNewWin,			HIDC_CHECK_OpenNewWin,			// 外部から起動するときは新しいウィンドウで開く 2009.06.19
	IDC_CHECK_DispTabIcon,			HIDC_CHECK_DispTabIcon,			// アイコン表示	// 2006.08.06 ryoji
	IDC_CHECK_SameTabWidth,			HIDC_CHECK_SameTabWidth,		// 等幅	// 2006.08.06 ryoji
	IDC_CHECK_DispTabClose,			HIDC_CHECK_DispTabClose,		// タブを閉じるボタン表示	// 2012.04.14 syat
	IDC_BUTTON_TABFONT,				HIDC_BUTTON_TABFONT,			// タブフォント
	IDC_CHECK_SortTabList,			HIDC_CHECK_SortTabList,			// タブ一覧ソート	// 2006.08.06 ryoji
	IDC_CHECK_TAB_MULTILINE,		HIDC_CHECK_TAB_MULTILINE,		// タブ多段
	IDC_COMBO_TAB_POSITION,			HIDC_COMBO_TAB_POSITION,		// タブ表示位置
	IDC_TABWND_CAPTION,				HIDC_TABWND_CAPTION,			// タブウィンドウキャプション	//@@@ 2003.06.15 MIK
	IDC_CHECK_ChgWndByWheel,		HIDC_CHECK_ChgWndByWheel,		// マウスホイールでウィンドウ切り替え 2007.04.03 ryoji
	0, 0
};

TYPE_NAME_ID<DispTabCloseType> DispTabCloseArr[] = {
	{ DispTabCloseType::No,		STR_PROPCOMTAB_DISP_NO },
	{ DispTabCloseType::Always,	STR_PROPCOMTAB_DISP_ALLWAYS },
	{ DispTabCloseType::Auto,	STR_PROPCOMTAB_DISP_AUTO },
};

TYPE_NAME_ID<TabPosition> TabPosArr[] = {
	{ TabPosition::Top, STR_PROPCOMTAB_TAB_POS_TOP },
	{ TabPosition::Bottom, STR_PROPCOMTAB_TAB_POS_BOTTOM },
#if 0
	{ TabPosition::Left, STR_PROPCOMTAB_TAB_POS_LEFT },
	{ TabPosition::Right, STR_PROPCOMTAB_TAB_POS_RIGHT },
#endif
};

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg[in] ダイアログボックスのWindow Handle
	@param uMsg[in] メッセージ
	@param wParam[in] パラメータ1
	@param lParam[in] パラメータ2
*/
INT_PTR CALLBACK PropTab::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropTab::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}
//	To Here Jun. 2, 2001 genta

// メッセージ処理
INT_PTR PropTab::DispatchEvent(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	NMHDR*		pNMHDR;
//	int			idCtrl;

	switch (uMsg) {
	case WM_INITDIALOG:
		// ダイアログデータの設定 Tab
		SetData(hwndDlg);
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// ユーザーがエディット コントロールに入力できるテキストの長さを制限する
		return TRUE;
		
	case WM_NOTIFY:
//		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
//		switch (idCtrl) {
//		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_TAB);
				return TRUE;
			case PSN_KILLACTIVE:
				// ダイアログデータの取得 Tab
				GetData(hwndDlg);
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = ID_PROPCOM_PAGENUM_TAB;
				return TRUE;
			}
//			break;	// default
//		}
		break;	// WM_NOTIFY

	case WM_COMMAND:
		{
			WORD wNotifyCode = HIWORD(wParam);	// 通知コード
			WORD wID = LOWORD(wParam);	// 項目ID､ コントロールID､ またはアクセラレータID
			if (wNotifyCode == BN_CLICKED) {
				switch (wID) {
				case IDC_CHECK_DispTabWnd:
				case IDC_CHECK_GroupMultiTabWnd:
					EnableTabPropInput(hwndDlg);
					break;
				case IDC_BUTTON_TABFONT:
					auto& csTabBar = m_common.m_tabBar;
					LOGFONT   lf = csTabBar.m_lf;
					INT nPointSize = csTabBar.m_nPointSize;

					if (MySelectFont(&lf, &nPointSize, hwndDlg, false)) {
						csTabBar.m_lf = lf;
						csTabBar.m_nPointSize = nPointSize;
						// タブ フォント表示	// 2013/4/24 Uchi
						HFONT hFont = SetFontLabel(hwndDlg, IDC_STATIC_TABFONT, csTabBar.m_lf, csTabBar.m_nPointSize);
						if (m_hTabFont) {
							::DeleteObject(m_hTabFont);
						}
						m_hTabFont = hFont;
					}
					break;
				}
			}
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

	case WM_DESTROY:
		// タブ フォント破棄	// 2013/4/24 Uchi
		if (m_hTabFont) {
			::DeleteObject(m_hTabFont);
			m_hTabFont = NULL;
		}
		return TRUE;
	}
	return FALSE;
}


// ダイアログデータの設定
void PropTab::SetData(HWND hwndDlg)
{
	auto& csTabBar = m_common.m_tabBar;

	//	Feb. 11, 2007 genta「ウィンドウ」シートより移動
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_DispTabWnd, csTabBar.m_bDispTabWnd);	//@@@ 2003.05.31 MIK
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_SameTabWidth, csTabBar.m_bSameTabWidth);	//@@@ 2006.01.28 ryoji
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_DispTabIcon, csTabBar.m_bDispTabIcon);	//@@@ 2006.01.28 ryoji
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_SortTabList, csTabBar.m_bSortTabList);			//@@@ 2006.03.23 fon
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_TAB_MULTILINE, csTabBar.m_bTabMultiLine );
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_GroupMultiTabWnd, !csTabBar.m_bDispTabWndMultiWin); //@@@ 2003.05.31 MIK
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_TABWND_CAPTION), _countof(csTabBar.m_szTabWndCaption) - 1);
	::DlgItem_SetText(hwndDlg, IDC_TABWND_CAPTION, csTabBar.m_szTabWndCaption);

	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_CHECK_DispTabClose);
	Combo_ResetContent(hwndCombo);
	int nSelPos = 0;
	for (int i=0; i<_countof(DispTabCloseArr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(DispTabCloseArr[i].nNameId));
		if (DispTabCloseArr[i].nMethod == m_common.m_tabBar.m_dispTabClose) {
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAB_POSITION);
	Combo_ResetContent(hwndCombo);
	nSelPos = 0;
	for (int i=0; i<_countof(TabPosArr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(TabPosArr[i].nNameId));
		if (TabPosArr[i].nMethod == m_common.m_tabBar.m_eTabPosition) {
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	//	Feb. 11, 2007 genta 新規作成
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_RetainEmptyWindow, csTabBar.m_bTab_RetainEmptyWin);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_CloseOneWin, csTabBar.m_bTab_CloseOneWin);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_ChgWndByWheel, csTabBar.m_bChgWndByWheel);	// 2007.04.03 ryoji
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_OpenNewWin, csTabBar.m_bNewWindow); // 2009.06.17

	// タブ フォント	// 2013/4/24 Uchi
	m_hTabFont = SetFontLabel(hwndDlg, IDC_STATIC_TABFONT, csTabBar.m_lf, csTabBar.m_nPointSize);

	EnableTabPropInput(hwndDlg);
}

// ダイアログデータの取得
int PropTab::GetData(HWND hwndDlg)
{
	auto& csTabBar = m_common.m_tabBar;
	//	Feb. 11, 2007 genta「ウィンドウ」シートより移動
	csTabBar.m_bDispTabWnd = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispTabWnd);
	csTabBar.m_bSameTabWidth = DlgButton_IsChecked(hwndDlg, IDC_CHECK_SameTabWidth);		// 2006.01.28 ryoji
	csTabBar.m_bDispTabIcon = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispTabIcon);		// 2006.01.28 ryoji
	csTabBar.m_bSortTabList = DlgButton_IsChecked(hwndDlg, IDC_CHECK_SortTabList);		// 2006.03.23 fon
	csTabBar.m_bDispTabWndMultiWin = !DlgButton_IsChecked(hwndDlg, IDC_CHECK_GroupMultiTabWnd);
	::DlgItem_GetText(hwndDlg, IDC_TABWND_CAPTION, csTabBar.m_szTabWndCaption, _countof(csTabBar.m_szTabWndCaption));

	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_CHECK_DispTabClose);
	int nSelPos = Combo_GetCurSel(hwndCombo);
	csTabBar.m_dispTabClose = DispTabCloseArr[nSelPos].nMethod;

	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAB_POSITION);
	nSelPos = Combo_GetCurSel(hwndCombo);
	csTabBar.m_eTabPosition = TabPosArr[nSelPos].nMethod;

	//	Feb. 11, 2007 genta 新規作成
	csTabBar.m_bTab_RetainEmptyWin = DlgButton_IsChecked(hwndDlg, IDC_CHECK_RetainEmptyWindow);
	csTabBar.m_bTab_CloseOneWin = DlgButton_IsChecked(hwndDlg, IDC_CHECK_CloseOneWin);
	csTabBar.m_bChgWndByWheel = DlgButton_IsChecked(hwndDlg, IDC_CHECK_ChgWndByWheel);	// 2007.04.03 ryoji
	csTabBar.m_bNewWindow = DlgButton_IsChecked(hwndDlg, IDC_CHECK_OpenNewWin);  // 2009.06.17

	return TRUE;
}

/*! 「タブバー」シート上のアイテムの有効・無効を適切に設定する

	@date 2007.02.12 genta 新規作成
*/
void PropTab::EnableTabPropInput(HWND hwndDlg)
{
	
	bool bTabWnd = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispTabWnd);
	bool bGroupTabWin = false;
	if (bTabWnd) {
		bGroupTabWin = DlgButton_IsChecked(hwndDlg, IDC_CHECK_GroupMultiTabWnd);
	}
	DlgItem_Enable(hwndDlg, IDC_CHECK_GroupMultiTabWnd,		bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_RetainEmptyWindow,	bGroupTabWin);
	DlgItem_Enable(hwndDlg, IDC_CHECK_CloseOneWin,			bGroupTabWin);
	DlgItem_Enable(hwndDlg, IDC_CHECK_OpenNewWin,			bGroupTabWin);	// 2009.06.17
	DlgItem_Enable(hwndDlg, IDC_CHECK_DispTabIcon,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_SameTabWidth,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_DispTabClose,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TextTabClose,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TextTabCaption,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_BUTTON_TABFONT,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_STATIC_TABFONT,				bTabWnd);	// 2013/4/24 Uchi
	DlgItem_Enable(hwndDlg, IDC_CHECK_SortTabList,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_TAB_MULTILINE,		bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TAB_POSITION,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_COMBO_TAB_POSITION,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TABWND_CAPTION,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_ChgWndByWheel,		bTabWnd);	// 2007.04.03 ryoji
}

