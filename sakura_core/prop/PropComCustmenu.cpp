/*!	@file
	共通設定ダイアログボックス、「カスタムメニュー」ページ
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "dlg/DlgInput1.h"
#include "env/ShareData.h"
#include "util/shell.h"
#include "util/window.h"
#include "typeprop/ImpExpManager.h"
#include "sakura_rc.h"
#include "sakura.hh"

using namespace std;

// 特別機能
struct SpecialFunc	{
	EFunctionCode	nFunc;			// Function
	int			 	nNameId;		// 名前
};
extern const	SpecialFunc	gSpecialFuncs[];
extern const int gSpecialFuncsCount;

static	int 	nSpecialFuncsNum;		// 特別機能のコンボボックス内での番号

// Popup Help
static const DWORD p_helpids[] = {	//10100
	IDC_BUTTON_DELETE,				HIDC_BUTTON_DELETE,				// メニューから機能削除
	IDC_BUTTON_INSERTSEPARATOR,		HIDC_BUTTON_INSERTSEPARATOR,	// セパレータ挿入
	IDC_BUTTON_INSERT,				HIDC_BUTTON_INSERT,				// メニューへ機能挿入
	IDC_BUTTON_ADD,					HIDC_BUTTON_ADD,				// メニューへ機能追加
	IDC_BUTTON_UP,					HIDC_BUTTON_UP,					// メニューの機能を上へ移動
	IDC_BUTTON_DOWN,				HIDC_BUTTON_DOWN,				// メニューの機能を下へ移動
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT,				// インポート
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT,				// エクスポート
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND,			// 機能の種別
	IDC_COMBO_MENU,					HIDC_COMBO_MENU,				// メニューの種別
	IDC_EDIT_MENUNAME,				HIDC_EDIT_MENUNAME,				// メニュー名
	IDC_BUTTON_MENUNAME,			HIDC_BUTTON_MENUNAME,			// メニュー名設定
	IDC_LIST_FUNC,					HIDC_LIST_FUNC,					// 機能一覧
	IDC_LIST_RES,					HIDC_LIST_RES,					// メニュー一覧
	IDC_CHECK_SUBMENU,				HIDC_CHECK_SUBMENU,				// サブメニューとして表示
//	IDC_LABEL_MENUFUNCKIND,			-1,
//	IDC_LABEL_MENUCHOICE,			-1,
//	IDC_LABEL_MENUFUNC,				-1,
//	IDC_LABEL_MENU,					-1,
//	IDC_LABEL_MENUKEYCHANGE,		-1,
//	IDC_STATIC,						-1,
	0, 0
};

static
bool SetSpecialFuncName(EFunctionCode code, wchar_t* ptr)
{
	if (F_SPECIAL_FIRST <= code && code <= F_SPECIAL_LAST) {
		for (int k=0; k<gSpecialFuncsCount; ++k) {
			if (gSpecialFuncs[k].nFunc == code) {
				auto_strcpy(ptr, LSW(gSpecialFuncs[k].nNameId));
				return true;
			}
		}
	}
	return false;
}

/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK PropCustmenu::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropCustmenu::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}

// Custom menu メッセージ処理
INT_PTR PropCustmenu::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
	)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	static HWND	hwndCOMBO_FUNCKIND;
	static HWND	hwndLIST_FUNC;
	static HWND	hwndCOMBO_MENU;
	static HWND	hwndLIST_RES;

	int			i;

	int			nIdx1;
	int			nIdx2;
	int			nNum2;
	int			nIdx3;
	int			nIdx4;
	wchar_t		szLabel[300];
	wchar_t		szLabel2[300];

	DlgInput1	dlgInput1;
	auto& csCustomMenu = common.customMenu;

	switch (uMsg) {
	case WM_INITDIALOG:
		// ダイアログデータの設定 Custom menu
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// コントロールのハンドルを取得
		hwndCOMBO_FUNCKIND = ::GetDlgItem(hwndDlg, IDC_COMBO_FUNCKIND);
		hwndLIST_FUNC = ::GetDlgItem(hwndDlg, IDC_LIST_FUNC);
		hwndCOMBO_MENU = ::GetDlgItem(hwndDlg, IDC_COMBO_MENU);
		hwndLIST_RES = ::GetDlgItem(hwndDlg, IDC_LIST_RES);

		// キー選択時の処理
		::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_COMBO_FUNCKIND, CBN_SELCHANGE), (LPARAM)hwndCOMBO_FUNCKIND);
		::SetTimer(hwndDlg, 1, 300, NULL);
		return TRUE;

	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
		switch (pNMHDR->code) {
		case PSN_HELP:
			OnHelp(hwndDlg, IDD_PROP_CUSTMENU);
			return TRUE;
		case PSN_KILLACTIVE:
//			MYTRACE(_T("Custom menu PSN_KILLACTIVE\n"));
			// ダイアログデータの取得 Custom menu
			GetData(hwndDlg);
			return TRUE;
		case PSN_SETACTIVE:
			nPageNum = ID_PROPCOM_PAGENUM_CUSTMENU;

			// 表示を更新する（マクロ設定画面でのマクロ名変更を反映）
			nIdx1 = Combo_GetCurSel(hwndCOMBO_MENU);
			nIdx2 = List_GetCurSel(hwndLIST_RES);
			nIdx3 = Combo_GetCurSel(hwndCOMBO_FUNCKIND);
			nIdx4 = List_GetCurSel(hwndLIST_FUNC);
			if (nIdx1 != CB_ERR) {
				::SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_COMBO_MENU, CBN_SELCHANGE), (LPARAM)hwndCOMBO_MENU);
				if (nIdx2 != LB_ERR) {
					List_SetCurSel(hwndLIST_RES, nIdx2);
				}
			}
			if (nIdx3 != CB_ERR) {
				::SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_COMBO_FUNCKIND, CBN_SELCHANGE), (LPARAM)hwndCOMBO_FUNCKIND);
				if (nIdx4 != LB_ERR) {
					List_SetCurSel(hwndLIST_FUNC, nIdx4);
				}
			}
			return TRUE;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// 通知コード
		wID = LOWORD(wParam);			// 項目ID､ コントロールID､ またはアクセラレータID
		hwndCtl = (HWND) lParam;		// コントロールのハンドル

		switch (wNotifyCode) {
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			case IDC_BUTTON_IMPORT:	// インポート
				// カスタムメニュー設定をインポートする
				Import(hwndDlg);
				return TRUE;
			case IDC_BUTTON_EXPORT:	// エクスポート
				// カスタムメニュー設定をエクスポートする
				Export(hwndDlg);
				return TRUE;
			case IDC_BUTTON_MENUNAME:
				wchar_t buf[MAX_CUSTOM_MENU_NAME_LEN + 1];
				//	メニュー文字列の設定
				nIdx1 = Combo_GetCurSel(hwndCOMBO_MENU);
				if (nIdx1 == CB_ERR) {
					break;
				}
				::DlgItem_GetText(hwndDlg, IDC_EDIT_MENUNAME,
					csCustomMenu.szCustMenuNameArr[nIdx1], MAX_CUSTOM_MENU_NAME_LEN);
				//	Combo Boxも変更 削除＆再登録
				Combo_DeleteString(hwndCOMBO_MENU, nIdx1);
				Combo_InsertString(hwndCOMBO_MENU, nIdx1,
					lookup.Custmenu2Name(nIdx1, buf, _countof(buf)));
				// 削除すると選択が解除されるので，元に戻す
				Combo_SetCurSel(hwndCOMBO_MENU, nIdx1);
				return TRUE;
			}
			break;	// BN_CLICKED
		}

		if (hwndCOMBO_MENU == hwndCtl) {
			switch (wNotifyCode) {
			case CBN_SELCHANGE:
				nIdx1 = Combo_GetCurSel(hwndCOMBO_MENU);
				if (nIdx1 == CB_ERR) {
					break;
				}
				SetDataMenuList(hwndDlg, nIdx1);
				break;	// CBN_SELCHANGE
			}
		}else
		if (hwndLIST_RES == hwndCtl) {
			switch (wNotifyCode) {
			case LBN_DBLCLK:
				nIdx1 = Combo_GetCurSel(hwndCOMBO_MENU);
				if (nIdx1 == CB_ERR) {
					break;
				}
				nIdx2 = List_GetCurSel(hwndLIST_RES);
				if (nIdx2 == LB_ERR) {
					break;
				}

				if (csCustomMenu.nCustMenuItemFuncArr[nIdx1][nIdx2] == 0) {
					break;
				}

//			idListBox = (int) LOWORD(wParam);	// identifier of list box
//			hwndListBox = (HWND) lParam;		// handle of list box
				TCHAR szKey[2];
				auto_sprintf_s(szKey, _T("%hc"), csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2]);
				{
					BOOL bDlgInputResult = dlgInput1.DoModal(
						G_AppInstance(),
						hwndDlg,
						LS(STR_PROPCOMCUSTMENU_AC1),
						LS(STR_PROPCOMCUSTMENU_AC2),
						1,
						szKey
					);
					if (!bDlgInputResult) {
						return TRUE;
					}
				}
				if (!lookup.Funccode2Name(csCustomMenu.nCustMenuItemFuncArr[nIdx1][nIdx2], szLabel, 255)) {
					SetSpecialFuncName(csCustomMenu.nCustMenuItemFuncArr[nIdx1][nIdx2], szLabel);
				}

				{
					KEYCODE keycode[3]={0}; _tctomb(szKey, keycode);
					csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2] = keycode[0];
				}
				if (csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2]) {
					auto_sprintf_s(szLabel2, LTEXT("%ts(%hc)"),
						szLabel,
						csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2]
					);
				}else {
					auto_sprintf_s(szLabel2, LTEXT("%ls"), szLabel);
				}

				List_InsertString(hwndLIST_RES, nIdx2, szLabel2);
				List_DeleteString(hwndLIST_RES, nIdx2 + 1);

				break;
			case LBN_SELCHANGE:
				nIdx1 = Combo_GetCurSel(hwndCOMBO_MENU);
				if (nIdx1 == CB_ERR) {
					break;
				}

				if (csCustomMenu.nCustMenuItemNumArr[nIdx1] >= MAX_CUSTOM_MENU_ITEMS) {
					break;
				}

				nIdx2 = List_GetCurSel(hwndLIST_RES);
				if (nIdx2 == LB_ERR) {
					break;
				}

				// キー
				if ('\0' == csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2] ||
					' '  == csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2]
				) {
				}else {
				}
				break;	// LBN_SELCHANGE
			}
		}else if (hwndCOMBO_FUNCKIND == hwndCtl) {
			switch (wNotifyCode) {
			case CBN_SELCHANGE:
				nIdx3 = Combo_GetCurSel(hwndCOMBO_FUNCKIND);

				if (nIdx3 == nSpecialFuncsNum) {
					// 機能一覧に特殊機能をセット
					List_ResetContent(hwndLIST_FUNC);
					for (i=0; i<gSpecialFuncsCount; ++i) {
						List_AddString(hwndLIST_FUNC, LS(gSpecialFuncs[i].nNameId));
					}
				}else if (nIdx3 != CB_ERR) {
					lookup.SetListItem(hwndLIST_FUNC, nIdx3);
				}
				return TRUE;
			}
		}else {
			EFunctionCode	eFuncCode = F_0;
			switch (wNotifyCode) {
			// ボタン／チェックボックスがクリックされた
			case BN_CLICKED:
				switch (wID) {
				case IDC_BUTTON_INSERTSEPARATOR:
					nIdx1 = Combo_GetCurSel(hwndCOMBO_MENU);
					if (nIdx1 == CB_ERR) {
						break;
					}

					if (MAX_CUSTOM_MENU_ITEMS <= csCustomMenu.nCustMenuItemNumArr[nIdx1]) {
						break;
					}

					nIdx2 = List_GetCurSel(hwndLIST_RES);
					if (nIdx2 == LB_ERR) {
						nIdx2 = 0;
					}
					nIdx2 = List_InsertString(hwndLIST_RES, nIdx2, LSW(STR_PROPCOMCUSTMENU_SEP));
					if (nIdx2 == LB_ERR || nIdx2 == LB_ERRSPACE) {
						break;
					}
					List_SetCurSel(hwndLIST_RES, nIdx2);

					for (i=csCustomMenu.nCustMenuItemNumArr[nIdx1]; i>nIdx2; --i) {
						csCustomMenu.nCustMenuItemFuncArr[nIdx1][i] = csCustomMenu.nCustMenuItemFuncArr[nIdx1][i - 1];
						csCustomMenu.nCustMenuItemKeyArr[nIdx1][i] = csCustomMenu.nCustMenuItemKeyArr[nIdx1][i - 1];
					}
					csCustomMenu.nCustMenuItemFuncArr[nIdx1][nIdx2] = F_0;
					csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2] = '\0';
					csCustomMenu.nCustMenuItemNumArr[nIdx1]++;

//					::SetWindowText(hwndEDIT_KEY, L"");
					break;
				case IDC_BUTTON_DELETE:
					nIdx1 = Combo_GetCurSel(hwndCOMBO_MENU);
					if (nIdx1 == CB_ERR) {
						break;
					}

					if (csCustomMenu.nCustMenuItemNumArr[nIdx1] == 0) {
						break;
					}

					nIdx2 = List_GetCurSel(hwndLIST_RES);
					if (nIdx2 == LB_ERR) {
						break;
					}
					nNum2 = List_DeleteString(hwndLIST_RES, nIdx2);
					if (nNum2 == LB_ERR) {
						break;
					}

					for (i=nIdx2; i<csCustomMenu.nCustMenuItemNumArr[nIdx1]; ++i) {
						csCustomMenu.nCustMenuItemFuncArr[nIdx1][i] = csCustomMenu.nCustMenuItemFuncArr[nIdx1][i + 1];
						csCustomMenu.nCustMenuItemKeyArr[nIdx1][i] = csCustomMenu.nCustMenuItemKeyArr[nIdx1][i + 1];
					}
					csCustomMenu.nCustMenuItemNumArr[nIdx1]--;

					if (nNum2 > 0) {
						if (nNum2 <= nIdx2) {
							nIdx2 = nNum2 - 1;
						}
						nIdx2 = List_SetCurSel(hwndLIST_RES, nIdx2);

					}else {
					}
					break;

				case IDC_BUTTON_INSERT:
					nIdx1 = Combo_GetCurSel(hwndCOMBO_MENU);
					if (nIdx1 == CB_ERR) {
						break;
					}

					if (MAX_CUSTOM_MENU_ITEMS <= csCustomMenu.nCustMenuItemNumArr[nIdx1]) {
						break;
					}

					nIdx2 = List_GetCurSel(hwndLIST_RES);
					if (nIdx2 == LB_ERR) {
						nIdx2 = 0;
					}
					nIdx3 = Combo_GetCurSel(hwndCOMBO_FUNCKIND);
					if (nIdx3 == CB_ERR) {
						break;
					}
					nIdx4 = List_GetCurSel(hwndLIST_FUNC);
					if (nIdx4 == LB_ERR) {
						break;
					}
					List_GetText(hwndLIST_FUNC, nIdx4, szLabel);

					for (i=csCustomMenu.nCustMenuItemNumArr[nIdx1]; i>nIdx2; --i) {
						csCustomMenu.nCustMenuItemFuncArr[nIdx1][i] = csCustomMenu.nCustMenuItemFuncArr[nIdx1][i - 1];
						csCustomMenu.nCustMenuItemKeyArr[nIdx1][i] = csCustomMenu.nCustMenuItemKeyArr[nIdx1][i - 1];
					}
					if (nIdx3 == nSpecialFuncsNum) {
						// 特殊機能
						eFuncCode = gSpecialFuncs[nIdx4].nFunc;
					}else {
						eFuncCode = lookup.Pos2FuncCode(nIdx3, nIdx4);
					}
					csCustomMenu.nCustMenuItemFuncArr[nIdx1][nIdx2] = eFuncCode;
					csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2] = '\0';
					csCustomMenu.nCustMenuItemNumArr[nIdx1]++;

					nIdx2 = List_InsertString(hwndLIST_RES, nIdx2, szLabel);
					if (nIdx2 == LB_ERR || nIdx2 == LB_ERRSPACE) {
						break;
					}
					List_SetCurSel(hwndLIST_RES, nIdx2);
					break;
					
				case IDC_BUTTON_ADD:
					nIdx1 = Combo_GetCurSel(hwndCOMBO_MENU);
					if (nIdx1 == CB_ERR) {
						break;
					}

					if (MAX_CUSTOM_MENU_ITEMS <= csCustomMenu.nCustMenuItemNumArr[nIdx1]) {
						break;
					}

					nIdx2 = List_GetCurSel(hwndLIST_RES);
					if (nIdx2 == LB_ERR) {
						nIdx2 = 0;
					}
					nNum2 = List_GetCount(hwndLIST_RES);
					if (nNum2 == LB_ERR) {
						nIdx2 = 0;
					}
					nIdx3 = Combo_GetCurSel(hwndCOMBO_FUNCKIND);
					if (nIdx3 == CB_ERR) {
						break;
					}
					nIdx4 = List_GetCurSel(hwndLIST_FUNC);
					if (nIdx4 == LB_ERR) {
						break;
					}

					List_GetText(hwndLIST_FUNC, nIdx4, szLabel);
					eFuncCode = F_DISABLE;
					if (nIdx3 == nSpecialFuncsNum) {
						// 特殊機能
						if (0 <= nIdx4 && nIdx4 < gSpecialFuncsCount) {
							eFuncCode = gSpecialFuncs[nIdx4].nFunc;
						}
					}else {
						eFuncCode = lookup.Pos2FuncCode(nIdx3, nIdx4);
					}
					if (eFuncCode == F_DISABLE) {
						break;
					}
					csCustomMenu.nCustMenuItemFuncArr[nIdx1][nNum2] = eFuncCode;
					csCustomMenu.nCustMenuItemKeyArr[nIdx1][nNum2] = '\0';
					csCustomMenu.nCustMenuItemNumArr[nIdx1]++;

					nIdx2 = List_AddString(hwndLIST_RES, szLabel);
					if (nIdx2 == LB_ERR || nIdx2 == LB_ERRSPACE) {
						break;
					}
					List_SetCurSel(hwndLIST_RES, nIdx2);

					break;

				case IDC_BUTTON_UP:
					nIdx1 = Combo_GetCurSel(hwndCOMBO_MENU);
					if (nIdx1 == CB_ERR) {
						break;
					}
					nIdx2 = List_GetCurSel(hwndLIST_RES);
					if (nIdx2 == LB_ERR) {
						break;
					}
					if (nIdx2 == 0) {
						break;
					}

					{
						EFunctionCode nFunc = csCustomMenu.nCustMenuItemFuncArr[nIdx1][nIdx2 - 1];
						KEYCODE key = csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2 - 1];
						csCustomMenu.nCustMenuItemFuncArr[nIdx1][nIdx2 - 1] = csCustomMenu.nCustMenuItemFuncArr[nIdx1][nIdx2];
						csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2 - 1]  = csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2];
						csCustomMenu.nCustMenuItemFuncArr[nIdx1][nIdx2] =	nFunc;
						csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2]  = key;
					}

					List_GetText(hwndLIST_RES, nIdx2, szLabel);
					List_DeleteString(hwndLIST_RES, nIdx2);
					List_InsertString(hwndLIST_RES, nIdx2 - 1, szLabel);
					List_SetCurSel(hwndLIST_RES, nIdx2 - 1);
					break;

				case IDC_BUTTON_DOWN:
					nIdx1 = Combo_GetCurSel(hwndCOMBO_MENU);
					if (nIdx1 == CB_ERR) {
						break;
					}
					nIdx2 = List_GetCurSel(hwndLIST_RES);
					if (nIdx2 == LB_ERR) {
						break;
					}
					nNum2 = List_GetCount(hwndLIST_RES);
					if (nNum2 == LB_ERR) {
						break;
					}
					if (nNum2 - 1 <= nIdx2) {
						break;
					}

					{
						EFunctionCode nFunc = csCustomMenu.nCustMenuItemFuncArr[nIdx1][nIdx2 + 1];
						KEYCODE key = csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2 + 1];
						csCustomMenu.nCustMenuItemFuncArr[nIdx1][nIdx2 + 1] = csCustomMenu.nCustMenuItemFuncArr[nIdx1][nIdx2];
						csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2 + 1]  = csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2];
						csCustomMenu.nCustMenuItemFuncArr[nIdx1][nIdx2] =	nFunc;
						csCustomMenu.nCustMenuItemKeyArr[nIdx1][nIdx2]  = key;
					}
					List_GetText(hwndLIST_RES, nIdx2, szLabel);
					List_DeleteString(hwndLIST_RES, nIdx2);
					List_InsertString(hwndLIST_RES, nIdx2 + 1, szLabel);
					List_SetCurSel(hwndLIST_RES, nIdx2 + 1);
					break;
				case IDC_CHECK_SUBMENU:
					nIdx1 = Combo_GetCurSel(hwndCOMBO_MENU);
					if (nIdx1 == CB_ERR) {
						break;
					}
					csCustomMenu.bCustMenuPopupArr[nIdx1] = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_SUBMENU);
					break;
				}
				break;
			}
		}
		break;

	case WM_TIMER:
		nIdx1 = Combo_GetCurSel(hwndCOMBO_MENU);
		nIdx2 = List_GetCurSel(hwndLIST_RES);
		nIdx3 = Combo_GetCurSel(hwndCOMBO_FUNCKIND);
		nIdx4 = List_GetCurSel(hwndLIST_FUNC);
		i = List_GetCount(hwndLIST_RES);
		if (nIdx2 == LB_ERR) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DELETE), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_UP), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DOWN), FALSE);
		}else {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DELETE), TRUE);
			if (nIdx2 <= 0) {
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_UP), FALSE);
			}else {
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_UP), TRUE);
			}
			if (nIdx2 + 1 >= i) {
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DOWN), FALSE);
			}else {
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DOWN), TRUE);
			}
		}
		if (nIdx2 == LB_ERR || nIdx4 == LB_ERR) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_INSERT), FALSE);
		}else {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_INSERT), TRUE);
		}
		if (nIdx4 == LB_ERR) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_ADD), FALSE);
		}else {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_ADD), TRUE);
		}
		if (MAX_CUSTOM_MENU_ITEMS <= csCustomMenu.nCustMenuItemNumArr[nIdx1]) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_INSERTSEPARATOR), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_INSERT), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_ADD), FALSE);
		}
		if (nIdx3 != CB_ERR && nIdx4 != LB_ERR &&
		 	lookup.Pos2FuncCode(nIdx3, nIdx4) == 0 &&
			!(nIdx3 == nSpecialFuncsNum && 0 <= nIdx4 && nIdx4 < gSpecialFuncsCount)
		) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_INSERT), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_ADD), FALSE);
		}
		break;
	case WM_DESTROY:
		::KillTimer(hwndDlg, 1);
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

	}
	return FALSE;
}


// ダイアログデータの設定 Custom menu
void PropCustmenu::SetData(HWND hwndDlg)
{
	// 機能種別一覧に文字列をセット（コンボボックス）
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_FUNCKIND);
	lookup.SetCategory2Combo(hwndCombo);
	// 特別機能追加
	nSpecialFuncsNum = Combo_AddString(hwndCombo, LS(STR_SPECIAL_FUNC));

	// 種別の先頭の項目を選択（コンボボックス）
	Combo_SetCurSel(hwndCombo, 0);

	// メニュー一覧に文字列をセット（コンボボックス）
	HWND hwndCOMBO_MENU = ::GetDlgItem(hwndDlg, IDC_COMBO_MENU);
	wchar_t buf[MAX_CUSTOM_MENU_NAME_LEN + 1];
	for (int i=0; i<MAX_CUSTOM_MENU; ++i) {
		Combo_AddString(hwndCOMBO_MENU, lookup.Custmenu2Name(i, buf, _countof(buf)));
	}
	// メニュー一覧の先頭の項目を選択（コンボボックス）
	Combo_SetCurSel(hwndCOMBO_MENU, 0);
	SetDataMenuList(hwndDlg, 0);

//	// カスタムメニューの先頭の項目を選択（リストボックス）
	HWND hwndLIST_RES = ::GetDlgItem(hwndDlg, IDC_LIST_RES);
	List_SetCurSel(hwndLIST_RES, 0);
}

void PropCustmenu::SetDataMenuList(HWND hwndDlg, int nIdx)
{
	wchar_t szLabel[300];
	wchar_t szLabel2[300];
	auto& csCustomMenu = common.customMenu;

	// メニュー項目一覧に文字列をセット（リストボックス）
	HWND hwndLIST_RES = ::GetDlgItem(hwndDlg, IDC_LIST_RES);
//	hwndEDIT_KEY = ::GetDlgItem(hwndDlg, IDC_EDIT_KEY);
	List_ResetContent(hwndLIST_RES);
	for (int i=0; i<csCustomMenu.nCustMenuItemNumArr[nIdx]; ++i) {
		if (csCustomMenu.nCustMenuItemFuncArr[nIdx][i] == 0) {
			auto_strcpy(szLabel, LSW(STR_PROPCOMCUSTMENU_SEP));
		}else {
			EFunctionCode code = csCustomMenu.nCustMenuItemFuncArr[nIdx][i];
			if (!lookup.Funccode2Name(code, szLabel, 256)) {
				SetSpecialFuncName(code, szLabel);
			}
		}
		// キー 
		if (csCustomMenu.nCustMenuItemKeyArr[nIdx][i] == '\0') {
			auto_strcpy(szLabel2, szLabel);
		}else {
			auto_sprintf_s(szLabel2, LTEXT("%ls(%hc)"),
				szLabel,
				csCustomMenu.nCustMenuItemKeyArr[nIdx][i]
			);
		}
		::List_AddString(hwndLIST_RES, szLabel2);
	}
	
	// メニュー名を設定
	::DlgItem_SetText(hwndDlg, IDC_EDIT_MENUNAME, csCustomMenu.szCustMenuNameArr[nIdx]);

	CheckDlgButtonBool(hwndDlg, IDC_CHECK_SUBMENU, csCustomMenu.bCustMenuPopupArr[nIdx]);
	return;
}


// ダイアログデータの取得 Custom menu
int PropCustmenu::GetData(HWND hwndDlg)
{
	return TRUE;
}


// カスタムメニュー設定をインポートする
void PropCustmenu::Import(HWND hwndDlg)
{
	ImpExpCustMenu	impExpCustMenu(common);
	// インポート
	if (!impExpCustMenu.ImportUI(G_AppInstance(), hwndDlg)) {
		// インポートをしていない
		return;
	}
	// 画面更新
	HWND	hwndCtrl = ::GetDlgItem(hwndDlg, IDC_COMBO_MENU);
	::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_COMBO_MENU, CBN_SELCHANGE), (LPARAM)hwndCtrl);
}

// カスタムメニュー設定をエクスポートする
void PropCustmenu::Export(HWND hwndDlg)
{
	ImpExpCustMenu	impExpCustMenu(common);
	// エクスポート
	if (!impExpCustMenu.ExportUI(G_AppInstance(), hwndDlg)) {
		// エクスポートをしていない
		return;
	}
}

