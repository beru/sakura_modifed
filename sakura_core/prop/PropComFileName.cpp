/*!	@file
	共通設定ダイアログボックス、「ファイル名表示」ページ
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {	//13400
	IDC_CHECK_SHORTPATH,	HIDC_CHECK_FNAME_SHORTPATH,
	IDC_EDIT_SHORTMAXWIDTH,	HIDC_EDIT_FNAME_SHORTMAXWIDTH,
	IDC_LIST_FNAME,			HIDC_LIST_FNAME, 		// ファイル名置換リスト
	IDC_EDIT_FNAME_FROM,	HIDC_EDIT_FNAME_FROM,	// 置換前
	IDC_EDIT_FNAME_TO,		HIDC_EDIT_FNAME_TO,		// 置換後
	IDC_BUTTON_FNAME_INS,	HIDC_BUTTON_FNAME_INS,	// 挿入
	IDC_BUTTON_FNAME_ADD,	HIDC_BUTTON_FNAME_ADD,	// 追加
	IDC_BUTTON_FNAME_UPD,	HIDC_BUTTON_FNAME_UPD,	// 更新
	IDC_BUTTON_FNAME_DEL,	HIDC_BUTTON_FNAME_DEL,	// 削除
	IDC_BUTTON_FNAME_TOP,	HIDC_BUTTON_FNAME_TOP,	// 先頭
	IDC_BUTTON_FNAME_UP,	HIDC_BUTTON_FNAME_UP,	// 上へ
	IDC_BUTTON_FNAME_DOWN,	HIDC_BUTTON_FNAME_DOWN,	// 下へ
	IDC_BUTTON_FNAME_LAST,	HIDC_BUTTON_FNAME_LAST,	// 最終
//	IDC_CHECK_FNAME,		HIDC_CHECK_FNAME,	// ファイル名を簡易表示する
	0, 0 // 
};


INT_PTR CALLBACK PropFileName::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropFileName::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}

INT_PTR PropFileName::DispatchEvent(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	HWND hListView;
	int nIndex;
	TCHAR szFrom[_MAX_PATH];
	TCHAR szTo[_MAX_PATH];

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			RECT rc;
			LV_COLUMN	col;
			hListView = GetDlgItem(hwndDlg, IDC_LIST_FNAME);

			::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
			::GetWindowRect(hListView, &rc);
			col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			col.fmt      = LVCFMT_LEFT;
			col.cx       = (rc.right - rc.left) * 60 / 100;
			col.pszText  = const_cast<TCHAR*>(LS(STR_PROPCOMFNM_LIST1));
			col.iSubItem = 0;
			ListView_InsertColumn(hListView, 0, &col);
			col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			col.fmt      = LVCFMT_LEFT;
			col.cx       = (rc.right - rc.left) * 35 / 100;
			col.pszText  = const_cast<TCHAR*>(LS(STR_PROPCOMFNM_LIST2));
			col.iSubItem = 1;
			ListView_InsertColumn(hListView, 1, &col);

			// ダイアログを開いたときにリストが選択されていてもフィールドが空の場合があった
			nLastPos_FILENAME = -1;

			// ダイアログデータの設定
			SetData(hwndDlg);

			// エディット コントロールに入力できるテキストの長さを制限する
			EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_SHORTMAXWIDTH ), 4 );
			EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_FNAME_FROM), _MAX_PATH - 1);
			EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_FNAME_TO),   _MAX_PATH - 1);
		}
		return TRUE;

	case WM_NOTIFY:
		{
			NMHDR* pNMHDR = (NMHDR*)lParam;
			int idCtrl = (int)wParam;

			switch (idCtrl) {
			case IDC_LIST_FNAME:
				switch (pNMHDR->code) {
				case LVN_ITEMCHANGED:
					hListView = GetDlgItem(hwndDlg, IDC_LIST_FNAME);
					nIndex = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
					// 未選択
					if (nIndex == -1) {
						::DlgItem_SetText(hwndDlg, IDC_EDIT_FNAME_FROM, _T(""));
						::DlgItem_SetText(hwndDlg, IDC_EDIT_FNAME_TO, _T(""));
					}else if (nIndex != nLastPos_FILENAME) {
						GetListViewItem_FILENAME(hListView, nIndex, szFrom, szTo);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_FNAME_FROM, szFrom);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_FNAME_TO, szTo);
					}else {
						// nIndex == nLastPos_FILENAMEのとき
						// リスト→エディットボックスにデータをコピーすると[更新]がうまくうまく動作しない
					}
					nLastPos_FILENAME = nIndex;
					break;
				}
				break;
			default:
				switch (pNMHDR->code) {
				case PSN_HELP:
					OnHelp(hwndDlg, IDD_PROP_FNAME);
					return TRUE;
				case PSN_KILLACTIVE:
					// ダイアログデータの取得
					GetData(hwndDlg);
					return TRUE;
	//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
				case PSN_SETACTIVE:
					nPageNum = ID_PROPCOM_PAGENUM_FILENAME;
					return TRUE;
				}
				break;
			}
		}
		break;

	case WM_COMMAND:
		{
			WORD	wNotifyCode = HIWORD(wParam);	// 通知コード
			WORD	wID = LOWORD(wParam);			// 項目ID､ コントロールID､ またはアクセラレータID
			int		nCount;

			switch (wNotifyCode) {
			// ボタンがクリックされた
			case BN_CLICKED:
				hListView = GetDlgItem(hwndDlg, IDC_LIST_FNAME);
				nIndex = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
				switch (wID) {
				case IDC_BUTTON_FNAME_INS:	// 挿入
					// 選択中のキーを探す
					nCount = ListView_GetItemCount(hListView);
					if (nIndex == -1) {
						// 選択中でなければ最後に追加
						nIndex = nCount;
					}
					::DlgItem_GetText(hwndDlg, IDC_EDIT_FNAME_FROM, szFrom, _MAX_PATH);
					::DlgItem_GetText(hwndDlg, IDC_EDIT_FNAME_TO,   szTo,   _MAX_PATH);

					if (SetListViewItem_FILENAME(hListView, nIndex, szFrom, szTo, true) != -1) {
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_ADD:	// 追加
					nCount = ListView_GetItemCount(hListView);

					::DlgItem_GetText(hwndDlg, IDC_EDIT_FNAME_FROM, szFrom, _MAX_PATH);
					::DlgItem_GetText(hwndDlg, IDC_EDIT_FNAME_TO,   szTo,   _MAX_PATH);
					
					if (SetListViewItem_FILENAME(hListView, nCount, szFrom, szTo, true) != -1) {
						return TRUE;
					}
					break;

				case IDC_BUTTON_FNAME_UPD:	// 更新
					if (nIndex != -1) {
						::DlgItem_GetText(hwndDlg, IDC_EDIT_FNAME_FROM, szFrom, _MAX_PATH);
						::DlgItem_GetText(hwndDlg, IDC_EDIT_FNAME_TO,   szTo,   _MAX_PATH);
						if (SetListViewItem_FILENAME(hListView, nIndex, szFrom, szTo, false) != -1) {
							return TRUE;
						}
					}else {
						// 未選択でリストにひとつも項目がない場合は追加しておく
						if (ListView_GetItemCount(hListView) == 0) {
							if (SetListViewItem_FILENAME(hListView, 0, szFrom, szTo, true) != -1) {
								return TRUE;
							}
						}
					}
					break;
				case IDC_BUTTON_FNAME_DEL:	// 削除
					if (nIndex != -1) {
						ListView_DeleteItem(hListView, nIndex);	// 古いキーを削除
						ListView_SetItemState(hListView, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_TOP:	// 先頭
					if (MoveListViewItem_FILENAME(hListView, nIndex, 0) != -1) {
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_UP: 	// 上へ
					if (MoveListViewItem_FILENAME(hListView, nIndex, nIndex - 1) != -1) {
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_DOWN:	// 下へ
					if (MoveListViewItem_FILENAME(hListView, nIndex, nIndex + 1) != -1) {
						return TRUE;
					}
					break;
				case IDC_BUTTON_FNAME_LAST:	// 最終
					nCount = ListView_GetItemCount(hListView);
					if (MoveListViewItem_FILENAME(hListView, nIndex, nCount - 1) != -1) {
						return TRUE;
					}
					break;
				// default:
				}
				break;
			// default:
			}
		}

		break;	// WM_COMMAND
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);
		}
		return TRUE;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);
		return TRUE;

	}
	return FALSE;
}


/*!
	ダイアログ上のコントロールにデータを設定する

	@param hwndDlg ダイアログボックスのウィンドウハンドル
*/
void PropFileName::SetData(HWND hwndDlg)
{
	::CheckDlgButtonBool( hwndDlg, IDC_CHECK_SHORTPATH, common.fileName.bTransformShortPath );
	::SetDlgItemInt( hwndDlg, IDC_EDIT_SHORTMAXWIDTH, common.fileName.nTransformShortMaxWidth, FALSE );

	// ファイル名置換リスト
	HWND hListView = ::GetDlgItem(hwndDlg, IDC_LIST_FNAME);
	ListView_DeleteAllItems(hListView); // リストを空にする

	auto& csFileName = common.fileName;
	// リストにデータをセット
	int nIndex = 0;
	for (int i=0; i<csFileName.nTransformFileNameArrNum; ++i) {
		if ('\0' == csFileName.szTransformFileNameFrom[i][0]) {
			continue;
		}

		LVITEM lvItem = {0};
		lvItem.mask     = LVIF_TEXT;
		lvItem.iItem    = nIndex;
		lvItem.iSubItem = 0;
		lvItem.pszText  = csFileName.szTransformFileNameFrom[i];
		ListView_InsertItem(hListView, &lvItem);

		::ZeroMemory(&lvItem, sizeof_raw(lvItem));
		lvItem.mask     = LVIF_TEXT;
		lvItem.iItem    = nIndex;
		lvItem.iSubItem = 1;
		lvItem.pszText  = csFileName.szTransformFileNameTo[i];
		ListView_SetItem(hListView, &lvItem);

		++nIndex;
	}

	// 一番上を選択しておく
	if (nIndex != 0) {
		ListView_SetItemState(hListView, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}
	//	リストビューの行選択を可能にする．
	DWORD dwStyle;
	dwStyle = ListView_GetExtendedListViewStyle(hListView);
	dwStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle(hListView, dwStyle);

	return;
}

/*!
	ダイアログ上のコントロールからデータを取得してメモリに格納する

	@param hwndDlg ダイアログボックスのウィンドウハンドル
*/

int PropFileName::GetData(HWND hwndDlg)
{
	auto& csFileName = common.fileName;

	common.fileName.bTransformShortPath = ::IsDlgButtonCheckedBool( hwndDlg, IDC_CHECK_SHORTPATH );
	common.fileName.nTransformShortMaxWidth = ::GetDlgItemInt( hwndDlg, IDC_EDIT_SHORTMAXWIDTH, NULL, FALSE );

	// ファイル名置換リスト
	HWND hListView = ::GetDlgItem(hwndDlg, IDC_LIST_FNAME);
	csFileName.nTransformFileNameArrNum = ListView_GetItemCount(hListView);

	for (int nIndex=0, nCount=0; nIndex<MAX_TRANSFORM_FILENAME; ++nIndex) {
		if (nIndex < csFileName.nTransformFileNameArrNum) {
			ListView_GetItemText(hListView, nIndex, 0, csFileName.szTransformFileNameFrom[nCount], _MAX_PATH);

			// 置換前文字列がNULLだったら捨てる
			if (L'\0' == csFileName.szTransformFileNameFrom[nCount][0]) {
				csFileName.szTransformFileNameTo[nIndex][0] = L'\0';
			}else {
				ListView_GetItemText(hListView, nIndex, 1, csFileName.szTransformFileNameTo[nCount], _MAX_PATH);
				++nCount;
			}
		}else {
			csFileName.szTransformFileNameFrom[nIndex][0] = L'\0';
			csFileName.szTransformFileNameTo[nIndex][0] = L'\0';
		}
	}

	return TRUE;
}


int PropFileName::SetListViewItem_FILENAME(
	HWND hListView,
	int nIndex,
	LPTSTR szFrom,
	LPTSTR szTo,
	bool bInsMode
	)
{
	if (szFrom[0] == _T('\0') || nIndex == -1) {
		return -1;
	}

	int nCount = ListView_GetItemCount(hListView);

	// これ以上追加できない
	if (bInsMode && MAX_TRANSFORM_FILENAME <= nCount) {
		ErrorMessage(GetParent(hListView), LS(STR_PROPCOMFNM_ERR_REG));
		return -1;
	}

	LV_ITEM	item = {0};
	item.mask     = LVIF_TEXT;
	item.iItem    = nIndex;
	item.iSubItem = 0;
	item.pszText  = szFrom;
	if (bInsMode) {
		ListView_InsertItem(hListView, &item);
	}else {
		ListView_SetItem(hListView, &item);
	}

	::ZeroMemory(&item, sizeof_raw(item));
	item.mask     = LVIF_TEXT;
	item.iItem    = nIndex;
	item.iSubItem = 1;
	item.pszText  = szTo;
	ListView_SetItem(hListView, &item);

	ListView_SetItemState(hListView, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	return nIndex;
}


void PropFileName::GetListViewItem_FILENAME(
	HWND hListView,
	int nIndex,
	LPTSTR szFrom,
	LPTSTR szTo
	)
{
	ListView_GetItemText(hListView, nIndex, 0, szFrom, _MAX_PATH);
	ListView_GetItemText(hListView, nIndex, 1, szTo, _MAX_PATH);
}


int PropFileName::MoveListViewItem_FILENAME(
	HWND hListView,
	int nIndex,
	int nIndex2
	)
{
	TCHAR szFrom[_MAX_PATH];
	TCHAR szTo[_MAX_PATH];
	int nCount = ListView_GetItemCount(hListView);

	//	2004.03.24 dskoba
	if (nIndex > nCount - 1) {
		nIndex = nCount - 1;
	}
	if (nIndex2 > nCount - 1) {
		nIndex2 = nCount - 1;
	}
	if (nIndex < 0) {
		nIndex = 0;
	}
	if (nIndex2 < 0) {
		nIndex2 = 0;
	}
	
	if (nIndex == nIndex2) {
		return -1;
	}

	GetListViewItem_FILENAME(hListView, nIndex, szFrom, szTo);
	ListView_DeleteItem(hListView, nIndex);	// 古いキーを削除
	SetListViewItem_FILENAME(hListView, nIndex2, szFrom, szTo, true);
	return nIndex2;
}

