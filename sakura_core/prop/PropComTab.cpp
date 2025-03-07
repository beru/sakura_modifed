/*!	@file
	@brief 共通設定ダイアログボックス、「タブバー」ページ
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "PropertyManager.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {
	IDC_CHECK_DispTabWnd,			HIDC_CHECK_DispTabWnd,			// タブウィンドウ表示
	IDC_CHECK_GroupMultiTabWnd,		HIDC_CHECK_GroupMultiTabWnd,	// ウィンドウをまとめてグループ化する
	IDC_CHECK_RetainEmptyWindow,	HIDC_CHECK_RetainEmptyWindow,	// 最後のファイルを閉じたとき(無題)文書を残す
	IDC_CHECK_CloseOneWin,			HIDC_CHECK_CloseOneWin,			// ウィンドウの閉じるボタンは現在のファイルのみ閉じる
	IDC_CHECK_OpenNewWin,			HIDC_CHECK_OpenNewWin,			// 外部から起動するときは新しいウィンドウで開く
	IDC_CHECK_DispTabIcon,			HIDC_CHECK_DispTabIcon,			// アイコン表示
	IDC_CHECK_SameTabWidth,			HIDC_CHECK_SameTabWidth,		// 等幅
	IDC_CHECK_DispTabClose,			HIDC_CHECK_DispTabClose,		// タブを閉じるボタン表示
	IDC_BUTTON_TABFONT,				HIDC_BUTTON_TABFONT,			// タブフォント
	IDC_CHECK_SortTabList,			HIDC_CHECK_SortTabList,			// タブ一覧ソート
	IDC_CHECK_TAB_MULTILINE,		HIDC_CHECK_TAB_MULTILINE,		// タブ多段
	IDC_COMBO_TAB_POSITION,			HIDC_COMBO_TAB_POSITION,		// タブ表示位置
	IDC_TABWND_CAPTION,				HIDC_TABWND_CAPTION,			// タブウィンドウキャプション
	IDC_CHECK_ChgWndByWheel,		HIDC_CHECK_ChgWndByWheel,		// マウスホイールでウィンドウ切り替え
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
			case PSN_SETACTIVE:
				nPageNum = ID_PROPCOM_PAGENUM_TAB;
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
					auto& csTabBar = common.tabBar;
					LOGFONT   lf = csTabBar.lf;
					INT nPointSize = csTabBar.nPointSize;

					if (MySelectFont(&lf, &nPointSize, hwndDlg, false)) {
						csTabBar.lf = lf;
						csTabBar.nPointSize = nPointSize;
						// タブ フォント表示
						HFONT hFont = SetFontLabel(hwndDlg, IDC_STATIC_TABFONT, csTabBar.lf, csTabBar.nPointSize);
						if (hTabFont) {
							::DeleteObject(hTabFont);
						}
						hTabFont = hFont;
					}
					break;
				}
			}
		}
		break;
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);
		}
		return TRUE;
		// NOTREACHED
		//break;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);
		return TRUE;

	case WM_DESTROY:
		// タブ フォント破棄
		if (hTabFont) {
			::DeleteObject(hTabFont);
			hTabFont = NULL;
		}
		return TRUE;
	}
	return FALSE;
}


// ダイアログデータの設定
void PropTab::SetData(HWND hwndDlg)
{
	auto& csTabBar = common.tabBar;

	CheckDlgButtonBool(hwndDlg, IDC_CHECK_DispTabWnd, csTabBar.bDispTabWnd);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_SameTabWidth, csTabBar.bSameTabWidth);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_DispTabIcon, csTabBar.bDispTabIcon);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_SortTabList, csTabBar.bSortTabList);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_TAB_MULTILINE, csTabBar.bTabMultiLine );
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_GroupMultiTabWnd, !csTabBar.bDispTabWndMultiWin);
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_TABWND_CAPTION), _countof(csTabBar.szTabWndCaption) - 1);
	::DlgItem_SetText(hwndDlg, IDC_TABWND_CAPTION, csTabBar.szTabWndCaption);

	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_CHECK_DispTabClose);
	Combo_ResetContent(hwndCombo);
	int nSelPos = 0;
	for (size_t i=0; i<_countof(DispTabCloseArr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(DispTabCloseArr[i].nNameId));
		if (DispTabCloseArr[i].nMethod == common.tabBar.dispTabClose) {
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAB_POSITION);
	Combo_ResetContent(hwndCombo);
	nSelPos = 0;
	for (size_t i=0; i<_countof(TabPosArr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(TabPosArr[i].nNameId));
		if (TabPosArr[i].nMethod == common.tabBar.eTabPosition) {
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	CheckDlgButtonBool(hwndDlg, IDC_CHECK_RetainEmptyWindow, csTabBar.bTab_RetainEmptyWin);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_CloseOneWin, csTabBar.bTab_CloseOneWin);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_ChgWndByWheel, csTabBar.bChgWndByWheel);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_OpenNewWin, csTabBar.bNewWindow);

	// タブ フォント
	hTabFont = SetFontLabel(hwndDlg, IDC_STATIC_TABFONT, csTabBar.lf, csTabBar.nPointSize);

	EnableTabPropInput(hwndDlg);
}

// ダイアログデータの取得
int PropTab::GetData(HWND hwndDlg)
{
	auto& csTabBar = common.tabBar;
	csTabBar.bDispTabWnd = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispTabWnd);
	csTabBar.bSameTabWidth = DlgButton_IsChecked(hwndDlg, IDC_CHECK_SameTabWidth);
	csTabBar.bDispTabIcon = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispTabIcon);
	csTabBar.bSortTabList = DlgButton_IsChecked(hwndDlg, IDC_CHECK_SortTabList);
	csTabBar.bDispTabWndMultiWin = !DlgButton_IsChecked(hwndDlg, IDC_CHECK_GroupMultiTabWnd);
	::DlgItem_GetText(hwndDlg, IDC_TABWND_CAPTION, csTabBar.szTabWndCaption, _countof(csTabBar.szTabWndCaption));

	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_CHECK_DispTabClose);
	int nSelPos = Combo_GetCurSel(hwndCombo);
	csTabBar.dispTabClose = DispTabCloseArr[nSelPos].nMethod;

	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAB_POSITION);
	nSelPos = Combo_GetCurSel(hwndCombo);
	csTabBar.eTabPosition = TabPosArr[nSelPos].nMethod;

	csTabBar.bTab_RetainEmptyWin = DlgButton_IsChecked(hwndDlg, IDC_CHECK_RetainEmptyWindow);
	csTabBar.bTab_CloseOneWin = DlgButton_IsChecked(hwndDlg, IDC_CHECK_CloseOneWin);
	csTabBar.bChgWndByWheel = DlgButton_IsChecked(hwndDlg, IDC_CHECK_ChgWndByWheel);
	csTabBar.bNewWindow = DlgButton_IsChecked(hwndDlg, IDC_CHECK_OpenNewWin);

	return TRUE;
}

/*! 「タブバー」シート上のアイテムの有効・無効を適切に設定する */
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
	DlgItem_Enable(hwndDlg, IDC_CHECK_OpenNewWin,			bGroupTabWin);
	DlgItem_Enable(hwndDlg, IDC_CHECK_DispTabIcon,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_SameTabWidth,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_DispTabClose,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TextTabClose,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TextTabCaption,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_BUTTON_TABFONT,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_STATIC_TABFONT,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_SortTabList,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_TAB_MULTILINE,		bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TAB_POSITION,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_COMBO_TAB_POSITION,			bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_TABWND_CAPTION,				bTabWnd);
	DlgItem_Enable(hwndDlg, IDC_CHECK_ChgWndByWheel,		bTabWnd);
}

