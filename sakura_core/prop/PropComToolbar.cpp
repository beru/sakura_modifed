/*!	@file
	@brief 共通設定ダイアログボックス、「ツールバー」ページ
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "uiparts/MenuDrawer.h"
#include "uiparts/ImageListMgr.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids[] = {	//11000
	IDC_BUTTON_DELETE,				HIDC_BUTTON_DELETE_TOOLBAR,				// ツールバーから機能削除
	IDC_BUTTON_INSERTSEPARATOR,		HIDC_BUTTON_INSERTSEPARATOR_TOOLBAR,	// セパレータ挿入
	IDC_BUTTON_INSERT,				HIDC_BUTTON_INSERT_TOOLBAR,				// ツールバーへ機能挿入
	IDC_BUTTON_ADD,					HIDC_BUTTON_ADD_TOOLBAR,				// ツールバーへ機能追加
	IDC_BUTTON_UP,					HIDC_BUTTON_UP_TOOLBAR,					// ツールバーの機能を上へ移動
	IDC_BUTTON_DOWN,				HIDC_BUTTON_DOWN_TOOLBAR,				// ツールバーの機能を下へ移動
	IDC_CHECK_TOOLBARISFLAT,		HIDC_CHECK_TOOLBARISFLAT,				// フラットなボタン
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND_TOOLBAR,			// 機能の種別
	IDC_LIST_FUNC,					HIDC_LIST_FUNC_TOOLBAR,					// 機能一覧
	IDC_LIST_RES,					HIDC_LIST_RES_TOOLBAR,					// ツールバー一覧
	IDC_BUTTON_INSERTWRAP,			HIDC_BUTTON_INSERTWRAP,					// ツールバー折返
	IDC_LABEL_MENUFUNCKIND,			(DWORD)-1,
	IDC_LABEL_MENUFUNC,				(DWORD)-1,
	IDC_LABEL_TOOLBAR,				(DWORD)-1,
//	IDC_STATIC,						-1,
	0, 0
};

/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK PropToolbar::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropToolbar::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}

/*!
	Owner Draw List Boxに指定の値を挿入する (Windows XPの問題回避用)
	
	Windows XP + manifestの時にLB_INSERTSTRINGが値0を受け付けないので
	とりあえず0以外の値を入れてから0に設定し直して回避する。
	1回目の挿入は0でなければ何でもいいはず。
	
	@param hWnd [in] リストボックスのウィンドウハンドル
	@param index [in] 挿入位置
	@param value [in] 挿入する値
	@return 挿入位置。エラーの時はLB_ERRまたはLB_ERRSPACE
*/
int Listbox_INSERTDATA(
	HWND hWnd,				// handle to destination window 
	int index,				// item index
	int value
	)
{
	int nIndex1 = List_InsertItemData(hWnd, index, 1);
	if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
		TopErrorMessage(NULL, LS(STR_PROPCOMTOOL_ERR01), index, nIndex1);
		return nIndex1;
	}else if (List_SetItemData(hWnd, nIndex1, value) == LB_ERR) {
		TopErrorMessage(NULL, LS(STR_PROPCOMTOOL_ERR02), nIndex1);
		return LB_ERR;
	}
	return nIndex1;
}

/*!
	Owner Draw List Boxに指定の値を追加する (Windows XPの問題回避用)
	
	Windows XP + manifestの時にLB_ADDSTRINGが値0を受け付けないので
	とりあえず0以外の値を入れてから0に設定し直して回避する。
	1回目の挿入は0でなければ何でもいいはず。
	
	@param hWnd [in] リストボックスのウィンドウハンドル
	@param index [in] 挿入位置
	@param value [in] 挿入する値
	@return 挿入位置。エラーの時はLB_ERRまたはLB_ERRSPACE
*/
int Listbox_ADDDATA(
	HWND hWnd,              // handle to destination window 
	int value
	)
{
	int nIndex1 = List_AddItemData(hWnd, 1);
	if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
		TopErrorMessage(NULL, LS(STR_PROPCOMTOOL_ERR03), nIndex1);
		return nIndex1;
	}else if (List_SetItemData(hWnd, nIndex1, value) == LB_ERR) {
		TopErrorMessage(NULL, LS(STR_PROPCOMTOOL_ERR04), nIndex1);
		return LB_ERR;
	}
	return nIndex1;
}


static int nToolBarListBoxTopMargin = 0;


// Toolbar メッセージ処理
INT_PTR PropToolbar::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
	)
{
	WORD				wNotifyCode;
	WORD				wID;
	HWND				hwndCtl;
	NMHDR*				pNMHDR;
	int					idCtrl;
	static HWND			hwndCombo;
	static HWND			hwndFuncList;
	static HWND			hwndResList;
	LPDRAWITEMSTRUCT	pDis;
	int					nIndex1;
	int					nIndex2;
	int					i;
	int					j;
	static int			nListItemHeight;
	LRESULT				lResult;

	switch (uMsg) {
	case WM_INITDIALOG:
		// コントロールのハンドルを取得
		hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_FUNCKIND);
		hwndFuncList = ::GetDlgItem(hwndDlg, IDC_LIST_FUNC);
		hwndResList = ::GetDlgItem(hwndDlg, IDC_LIST_RES);

		{
			TextWidthCalc calc(hwndResList);
			int nFontHeight = calc.GetTextHeight();
			nListItemHeight = pIcons->GetCy() + 2;
			if (nListItemHeight < nFontHeight) {
				nListItemHeight = nFontHeight;
				nToolBarListBoxTopMargin = 0;
			}else {
				nToolBarListBoxTopMargin = (nListItemHeight - (nFontHeight + 1)) / 2;
			}
		}
		/* ダイアログデータの設定 Toolbar */
		SetData( hwndDlg );
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		// キー選択時の処理
		::SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_COMBO_FUNCKIND, CBN_SELCHANGE), (LPARAM)hwndCombo);

		::SetTimer(hwndDlg, 1, 300, NULL);

		return TRUE;

	case WM_DRAWITEM:
		idCtrl = (UINT) wParam;	// コントロールのID
		pDis = (LPDRAWITEMSTRUCT) lParam;	// 項目描画情報
		switch (idCtrl) {
		case IDC_LIST_RES:	// ツールバーボタン結果リスト
		case IDC_LIST_FUNC:	// ボタン一覧リスト
			DrawToolBarItemList(pDis);	// ツールバーボタンリストのアイテム描画
			return TRUE;
		}
		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		switch (pNMHDR->code) {
		case PSN_HELP:
			OnHelp(hwndDlg, IDD_PROP_TOOLBAR);
			return TRUE;
		case PSN_KILLACTIVE:
			// ダイアログデータの取得 Toolbar
			GetData(hwndDlg);
			return TRUE;
		case PSN_SETACTIVE:
			nPageNum = ID_PROPCOM_PAGENUM_TOOLBAR;
			return TRUE;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// 通知コード
		wID = LOWORD(wParam);			// 項目ID､ コントロールID､ またはアクセラレータID
		hwndCtl = (HWND) lParam;		// コントロールのハンドル

		if (hwndResList == hwndCtl) {
			switch (wNotifyCode) {
			case LBN_SELCHANGE:
				return TRUE;
			}
		}else
		if (hwndCombo == hwndCtl) {
			switch (wNotifyCode) {
			case CBN_SELCHANGE:
				nIndex2 = Combo_GetCurSel(hwndCombo);
				if (nIndex2 == CB_ERR) {
					return TRUE;
				}
				List_ResetContent(hwndFuncList);
				// 機能一覧に文字列をセット (リストボックス)
				size_t nNum = lookup.GetItemCount(nIndex2);
				for (size_t i=0; i<nNum; ++i) {
					nIndex1 = lookup.Pos2FuncCode(nIndex2, i);
					int nbarNo = pMenuDrawer->FindToolbarNoFromCommandId(nIndex1);
					if (nbarNo >= 0) {
						// ツールバーボタンの情報をセット (リストボックス)
						lResult = ::Listbox_ADDDATA(hwndFuncList, (LPARAM)nbarNo);
						if (lResult == LB_ERR || lResult == LB_ERRSPACE) {
							break;
						}
						lResult = List_SetItemHeight(hwndFuncList, lResult, nListItemHeight);
					}

				}
				return TRUE;
			}
		}else {
			switch (wNotifyCode) {
			// ボタン／チェックボックスがクリックされた
			case BN_CLICKED:
				switch (wID) {
				case IDC_BUTTON_INSERTSEPARATOR:
					nIndex1 = List_GetCurSel(hwndResList);
					if (nIndex1 == LB_ERR) {
						nIndex1 = 0;
					}
					nIndex1 = ::Listbox_INSERTDATA(hwndResList, nIndex1, 0);
					if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
						break;
					}
					List_SetCurSel(hwndResList, nIndex1);
					break;

        // 折返ボタンが押されたら、右のリストに「ツールバー折返」を追加する。
				case IDC_BUTTON_INSERTWRAP:
					nIndex1 = List_GetCurSel(hwndResList);
					if (nIndex1 == LB_ERR) {
						nIndex1 = 0;
					}
					nIndex1 = ::Listbox_INSERTDATA(hwndResList, nIndex1, MenuDrawer::TOOLBAR_BUTTON_F_TOOLBARWRAP);
					if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
						break;
					}
					List_SetCurSel(hwndResList, nIndex1);
					break;

				case IDC_BUTTON_DELETE:
					nIndex1 = List_GetCurSel(hwndResList);
					if (nIndex1 == LB_ERR) {
						break;
					}
					i = List_DeleteString(hwndResList, nIndex1);
					if (i == LB_ERR) {
						break;
					}
					if (nIndex1 >= i) {
						if (i == 0) {
							i = List_SetCurSel(hwndResList, 0);
						}else {
							i = List_SetCurSel(hwndResList, i - 1);
						}
					}else {
						i = List_SetCurSel(hwndResList, nIndex1);
					}
					break;

				case IDC_BUTTON_INSERT:
					nIndex1 = List_GetCurSel(hwndResList);
					if (nIndex1 == LB_ERR) {
//						break;
						nIndex1 = 0;
					}
					nIndex2 = List_GetCurSel(hwndFuncList);
					if (nIndex2 == LB_ERR) {
						break;
					}
					i = List_GetItemData(hwndFuncList, nIndex2);
					nIndex1 = ::Listbox_INSERTDATA(hwndResList, nIndex1, i);
					if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
						break;
					}
					List_SetCurSel(hwndResList, nIndex1 + 1);
					break;

				case IDC_BUTTON_ADD:
					nIndex1 = List_GetCount(hwndResList);
					nIndex2 = List_GetCurSel(hwndFuncList);
					if (nIndex2 == LB_ERR) {
						break;
					}
					i = List_GetItemData(hwndFuncList, nIndex2);
					//	ここでは i != 0 だとは思うけど、一応保険です。
					nIndex1 = ::Listbox_INSERTDATA(hwndResList, nIndex1, i);
					if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
						TopErrorMessage(NULL, LS(STR_PROPCOMTOOL_ERR05), nIndex1);
						break;
					}
					List_SetCurSel(hwndResList, nIndex1);
					break;

				case IDC_BUTTON_UP:
					nIndex1 = List_GetCurSel(hwndResList);
					if (nIndex1 == LB_ERR || 0 >= nIndex1) {
						break;
					}
					i = List_GetItemData(hwndResList, nIndex1);

					j = List_DeleteString(hwndResList, nIndex1);
					if (j == LB_ERR) {
						break;
					}
					nIndex1 = ::Listbox_INSERTDATA(hwndResList, nIndex1 - 1, i);
					if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
						TopErrorMessage(NULL, LS(STR_PROPCOMTOOL_ERR05), nIndex1);
						break;
					}
					List_SetCurSel(hwndResList, nIndex1);
					break;

				case IDC_BUTTON_DOWN:
					i = List_GetCount(hwndResList);
					nIndex1 = List_GetCurSel(hwndResList);
					if (nIndex1 == LB_ERR || nIndex1 + 1 >= i) {
						break;
					}
					i = List_GetItemData(hwndResList, nIndex1);

					j = List_DeleteString(hwndResList, nIndex1);
					if (j == LB_ERR) {
						break;
					}
					nIndex1 = ::Listbox_INSERTDATA(hwndResList, nIndex1 + 1, i);
					if (nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE) {
						TopErrorMessage(NULL, LS(STR_PROPCOMTOOL_ERR05), nIndex1);
						break;
					}
					List_SetCurSel(hwndResList, nIndex1);
					break;
				}
				break;
			}
		}
		break;

	case WM_TIMER:
		nIndex1 = List_GetCurSel(hwndResList);
		nIndex2 = List_GetCurSel(hwndFuncList);
		i = List_GetCount(hwndResList);
		if (nIndex1 == LB_ERR) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DELETE), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_UP), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DOWN), FALSE);
		}else {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DELETE), TRUE);
			if (nIndex1 <= 0) {
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_UP), FALSE);
			}else {
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_UP), TRUE);
			}
			if (nIndex1 + 1 >= i) {
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DOWN), FALSE);
			}else {
				::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_DOWN), TRUE);
			}
		}
		if (nIndex1 == LB_ERR || nIndex2 == LB_ERR) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_INSERT), FALSE);
		}else {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_INSERT), TRUE);
		}
		if (nIndex2 == LB_ERR) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_ADD), FALSE);
		}else {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_BUTTON_ADD), TRUE);
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

	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);
		return TRUE;

	}
	return FALSE;
}

// ダイアログデータの設定 Toolbar
void PropToolbar::SetData(HWND hwndDlg)
{
	// 機能種別一覧に文字列をセット(コンボボックス)
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_FUNCKIND);
	lookup.SetCategory2Combo(hwndCombo);
	
	// 種別の先頭の項目を選択(コンボボックス)
	Combo_SetCurSel(hwndCombo, 0);
	::PostMessage(hwndCombo, WM_COMMAND, MAKELONG(IDC_COMBO_FUNCKIND, CBN_SELCHANGE), (LPARAM)hwndCombo);

  // コントロールのハンドルを取得
	HWND hwndResList = ::GetDlgItem(hwndDlg, IDC_LIST_RES);
	int nFontHeight = TextWidthCalc(hwndResList).GetTextHeight();
	int nListItemHeight = std::max(pIcons->GetCy() + 2, nFontHeight);
	auto& csToolBar = common.toolBar;
	// ツールバーボタンの情報をセット(リストボックス)
	for (int i=0; i<csToolBar.nToolBarButtonNum; ++i) {
		LRESULT lResult = ::Listbox_ADDDATA(hwndResList, (LPARAM)csToolBar.nToolBarButtonIdxArr[i]);
		if (lResult == LB_ERR || lResult == LB_ERRSPACE) {
			break;
		}
		lResult = List_SetItemHeight(hwndResList, lResult, nListItemHeight);
	}
	// ツールバーの先頭の項目を選択(リストボックス)
	List_SetCurSel(hwndResList, 0);	// ここをコメントアウトすると先頭項目が選択されなくなる
	// フラットツールバーにする／しない 
	::CheckDlgButton(hwndDlg, IDC_CHECK_TOOLBARISFLAT, csToolBar.bToolBarIsFlat);
}

// ダイアログデータの取得 Toolbar
int PropToolbar::GetData(HWND hwndDlg)
{
	HWND hwndResList = ::GetDlgItem(hwndDlg, IDC_LIST_RES);
	auto& csToolBar = common.toolBar;

	// ツールバーボタンの数
	csToolBar.nToolBarButtonNum = List_GetCount(hwndResList);

	// ツールバーボタンの情報を取得
	int k = 0;
	for (int i=0; i<csToolBar.nToolBarButtonNum; ++i) {
		int j = List_GetItemData(hwndResList, i);
		if (j != LB_ERR) {
			csToolBar.nToolBarButtonIdxArr[k] = j;
			++k;
		}
	}
	csToolBar.nToolBarButtonNum = k;

	// フラットツールバーにする／しない
	csToolBar.bToolBarIsFlat = DlgButton_IsChecked(hwndDlg, IDC_CHECK_TOOLBARISFLAT);
	return TRUE;
}

/* ツールバーボタンリストのアイテム描画 */
void PropToolbar::DrawToolBarItemList(DRAWITEMSTRUCT* pDis)
{
	TBBUTTON	tbb;

	HBRUSH hBrush = ::GetSysColorBrush(COLOR_WINDOW);
	::FillRect(pDis->hDC, &pDis->rcItem, hBrush);

	RECT rc  = pDis->rcItem;
	RECT rc0 = pDis->rcItem;
	rc0.left += 2;
	RECT rc1 = rc0;
	RECT rc2 = rc0;

	if ((int)pDis->itemID < 0) {
	}else {

		tbb = pMenuDrawer->getButton(pDis->itemData);

		// ボタンとセパレータとで処理を分ける
		wchar_t	szLabel[256];
		if (tbb.fsStyle & TBSTYLE_SEP) {
			// テキストだけ表示する
			if (tbb.idCommand == F_SEPARATOR) {
				auto_strncpy( szLabel, LSW(STR_PROPCOMTOOL_ITEM1), _countof(szLabel) - 1 );
				szLabel[_countof(szLabel) - 1] = L'\0';
			}else if (tbb.idCommand == F_MENU_NOT_USED_FIRST) {
				// ツールバー折返
				auto_strncpy( szLabel, LSW(STR_PROPCOMTOOL_ITEM2), _countof(szLabel) - 1 );
				szLabel[_countof(szLabel) - 1] = L'\0';
			}else {
				auto_strncpy( szLabel, LSW(STR_PROPCOMTOOL_ITEM3), _countof(szLabel) - 1 );
				szLabel[_countof(szLabel) - 1] = L'\0';
			}
		}else {
			// アイコンとテキストを表示する
			pIcons->Draw(tbb.iBitmap, pDis->hDC, rc.left + 2, rc.top + 2, ILD_NORMAL);
			lookup.Funccode2Name(tbb.idCommand, szLabel, _countof(szLabel));
		}

		// アイテムが選択されている
		if (pDis->itemState & ODS_SELECTED) {
			hBrush = ::GetSysColorBrush(COLOR_HIGHLIGHT);
			::SetTextColor(pDis->hDC, ::GetSysColor(COLOR_HIGHLIGHTTEXT));
		}else {
			hBrush = ::GetSysColorBrush(COLOR_WINDOW);
			::SetTextColor(pDis->hDC, ::GetSysColor(COLOR_WINDOWTEXT));
		}
		rc1.left += pIcons->GetCx() + 2;
		rc1.top++;
		rc1.right--;
		rc1.bottom--;
		::FillRect(pDis->hDC, &rc1, hBrush);

		::SetBkMode(pDis->hDC, TRANSPARENT);
		TextOutW_AnyBuild( pDis->hDC, rc1.left + 4, rc1.top + nToolBarListBoxTopMargin, szLabel, wcslen( szLabel ) );
	}

	// アイテムにフォーカスがある
	if (pDis->itemState & ODS_FOCUS) {
		::DrawFocusRect(pDis->hDC, &rc2);
	}
	return;
}

