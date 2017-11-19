// タイプ別設定 - キーワードヘルプ ページ

#include "StdAfx.h"
#include "PropTypes.h"
#include "env/ShareData.h"
#include "typeprop/ImpExpManager.h"
#include "dlg/DlgOpenFile.h"
#include "charset/CharPointer.h"
#include "io/TextStream.h"
#include "util/shell.h"
#include "util/module.h"
#include "sakura_rc.h"
#include "sakura.hh"

using namespace std;

static const DWORD p_helpids[] = {
	IDC_CHECK_KEYHELP,				HIDC_CHECK_KEYHELP,				// キーワードヘルプ機能を使う
	IDC_LIST_KEYHELP,				HIDC_LIST_KEYHELP,				// SysListView32
	IDC_BUTTON_KEYHELP_UPD,			HIDC_BUTTON_KEYHELP_UPD,		// 更新(&E)
	IDC_EDIT_KEYHELP,				HIDC_EDIT_KEYHELP,				// EDITTEXT
	IDC_BUTTON_KEYHELP_REF,			HIDC_BUTTON_KEYHELP_REF,		// 参照(&O)...
	IDC_BUTTON_KEYHELP_TOP,			HIDC_BUTTON_KEYHELP_TOP,		// 先頭(&T)
	IDC_BUTTON_KEYHELP_UP,			HIDC_BUTTON_KEYHELP_UP,			// 上へ(&U)
	IDC_BUTTON_KEYHELP_DOWN,		HIDC_BUTTON_KEYHELP_DOWN,		// 下へ(&G)
	IDC_BUTTON_KEYHELP_LAST,		HIDC_BUTTON_KEYHELP_LAST,		// 最終(&B)
	IDC_BUTTON_KEYHELP_INS,			HIDC_BUTTON_KEYHELP_INS,		// 挿入(&S)
	IDC_BUTTON_KEYHELP_DEL,			HIDC_BUTTON_KEYHELP_DEL,		// 削除(&D)
	IDC_CHECK_KEYHELP_ALLSEARCH,	HIDC_CHECK_KEYHELP_ALLSEARCH,	// 全辞書検索する(&A)
	IDC_CHECK_KEYHELP_KEYDISP,		HIDC_CHECK_KEYHELP_KEYDISP,		// キーワードも表示する(&W)
	IDC_CHECK_KEYHELP_PREFIX,		HIDC_CHECK_KEYHELP_PREFIX,		// 前方一致検索(&P)
	IDC_BUTTON_KEYHELP_IMPORT,		HIDC_BUTTON_KEYHELP_IMPORT,		// インポート
	IDC_BUTTON_KEYHELP_EXPORT,		HIDC_BUTTON_KEYHELP_EXPORT,		// エクスポート
	0, 0
};

static TCHAR* strcnv(TCHAR* str);
static TCHAR* GetFileName(const TCHAR* fullpath);

/*! キーワード辞書ファイル設定 メッセージ処理 */
INT_PTR PropTypesKeyHelp::DispatchEvent(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
)
{
	WORD	wNotifyCode;
	WORD	wID;
	NMHDR*	pNMHDR;
	int		nIndex, nIndex2;
	LV_ITEM	lvi;
	LV_COLUMN	col;
	RECT		rc;

	BOOL	bUse;						// 辞書を 使用する/しない
	TCHAR	szAbout[DICT_ABOUT_LEN];	// 辞書の説明(辞書ファイルの1行目から生成)
	TCHAR	szPath[_MAX_PATH];			// ファイルパス
	DWORD	dwStyle;

	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_KEYHELP);

	switch (uMsg) {
	case WM_INITDIALOG:
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
		// カラム追加
		::GetWindowRect(hwndList, &rc);
		// リストにチェックボックスを追加
		dwStyle = ListView_GetExtendedListViewStyle(hwndList);
		dwStyle |= LVS_EX_CHECKBOXES /*| LVS_EX_FULLROWSELECT*/ | LVS_EX_GRIDLINES;
		ListView_SetExtendedListViewStyle(hwndList, dwStyle);

		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 25 / 100;
		col.pszText  = const_cast<TCHAR*>(LS(STR_PROPTYPKEYHELP_DIC));	// 指定辞書ファイルの使用可否
		col.iSubItem = 0;
		ListView_InsertColumn(hwndList, 0, &col);
		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 55 / 100;
		col.pszText  = const_cast<TCHAR*>(LS(STR_PROPTYPKEYHELP_INFO));		// 指定辞書の１行目を取得
		col.iSubItem = 1;
		ListView_InsertColumn(hwndList, 1, &col);
		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 18 / 100;
		col.pszText  = const_cast<TCHAR*>(LS(STR_PROPTYPKEYHELP_PATH));				// 指定辞書ファイルパス
		col.iSubItem = 2;
		ListView_InsertColumn(hwndList, 2, &col);
		SetData(hwndDlg);	// ダイアログデータの設定 辞書ファイル一覧

		// リストがあれば先頭をフォーカスする
		if (ListView_GetItemCount(hwndList) > 0) {
			ListView_SetItemState(hwndList, 0, LVIS_SELECTED /*| LVIS_FOCUSED*/, LVIS_SELECTED /*| LVIS_FOCUSED*/);
		// リストがなければ初期値として用途を表示
		}else {
			::DlgItem_SetText(hwndDlg, IDC_LABEL_KEYHELP_ABOUT, LS(STR_PROPTYPKEYHELP_LINE1));
			::DlgItem_SetText(hwndDlg, IDC_EDIT_KEYHELP, LS(STR_PROPTYPKEYHELP_DICPATH));
		}

		// 初期状態を設定
		SendMessage(hwndDlg, WM_COMMAND, (WPARAM)MAKELONG(IDC_CHECK_KEYHELP, BN_CLICKED), 0);

		return TRUE;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// 通知コード
		wID	= LOWORD(wParam);			// 項目ID､ コントロールID､ またはアクセラレータID

		switch (wNotifyCode) {
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:

			switch (wID) {
			case IDC_CHECK_KEYHELP:	// キーワードヘルプ機能を使う
				if (!IsDlgButtonChecked(hwndDlg, IDC_CHECK_KEYHELP)) {
					//EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_KEYHELP), FALSE);			// キーワードヘルプ機能を使う(&K)
					EnableWindow(GetDlgItem(hwndDlg, IDC_FRAME_KEYHELP), FALSE);		  	// 辞書ファイル一覧(&L)
					EnableWindow(GetDlgItem(hwndDlg, IDC_LIST_KEYHELP), FALSE);         	// SysListView32
					EnableWindow(GetDlgItem(hwndDlg, IDC_LABEL_KEYHELP_TITLE), FALSE);  	// <辞書の説明>
					EnableWindow(GetDlgItem(hwndDlg, IDC_LABEL_KEYHELP_ABOUT), FALSE);  	// 辞書ファイルの概要
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_UPD), FALSE);   	// 更新(&E)
					EnableWindow(GetDlgItem(hwndDlg, IDC_LABEL_KEYHELP_KEYWORD), FALSE);	// 辞書ファイル
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_KEYHELP), FALSE);         	// EDITTEXT
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_REF), FALSE);   	// 参照(&O)...
					EnableWindow(GetDlgItem(hwndDlg, IDC_LABEL_KEYHELP_PRIOR), FALSE);  	// ↑優先度(高)
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_TOP), FALSE);   	// 先頭(&T)
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_UP), FALSE);    	// 上へ(&U)
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_DOWN), FALSE);  	// 下へ(&G)
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_LAST), FALSE);  	// 最終(&B)
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_INS), FALSE);   	// 挿入(&S)
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_DEL), FALSE);   	// 削除(&D)
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_KEYHELP_ALLSEARCH), FALSE);	// 全辞書検索する(&A)
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_KEYHELP_KEYDISP), FALSE);	// キーワードも表示する(&W)
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_KEYHELP_PREFIX), FALSE);		// 前方一致検索(&P)
				}else {
					//EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_KEYHELP), TRUE);			// キーワードヘルプ機能を使う(&K)
					EnableWindow(GetDlgItem(hwndDlg, IDC_FRAME_KEYHELP), TRUE);				// 辞書ファイル一覧(&L)
					EnableWindow(GetDlgItem(hwndDlg, IDC_LIST_KEYHELP), TRUE);				// SysListView32
					EnableWindow(GetDlgItem(hwndDlg, IDC_LABEL_KEYHELP_TITLE), TRUE);		// <辞書の説明>
					EnableWindow(GetDlgItem(hwndDlg, IDC_LABEL_KEYHELP_ABOUT), TRUE);  		// 辞書ファイルの概要
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_UPD), TRUE);   		// 更新(&E)
					EnableWindow(GetDlgItem(hwndDlg, IDC_LABEL_KEYHELP_KEYWORD), TRUE);		// 辞書ファイル
					EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_KEYHELP), TRUE);         		// EDITTEXT
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_REF), TRUE);   		// 参照(&O)...
					EnableWindow(GetDlgItem(hwndDlg, IDC_LABEL_KEYHELP_PRIOR), TRUE);  		// ↑優先度(高)
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_TOP), TRUE);   		// 先頭(&T)
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_UP), TRUE);    		// 上へ(&U)
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_DOWN), TRUE);  		// 下へ(&G)
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_LAST), TRUE);  		// 最終(&B)
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_INS), TRUE);   		// 挿入(&S)
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_KEYHELP_DEL), TRUE);   		// 削除(&D)
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_KEYHELP_ALLSEARCH), TRUE);	// 全辞書検索する(&A)
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_KEYHELP_KEYDISP), TRUE);		// キーワードも表示する(&W)
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_KEYHELP_PREFIX), TRUE);		// 前方一致検索(&P)
				}
				types.nKeyHelpNum = ListView_GetItemCount(hwndList);
				return TRUE;

			// 挿入・更新イベントを纏めて処理
			case IDC_BUTTON_KEYHELP_INS:	// 挿入
			case IDC_BUTTON_KEYHELP_UPD:	// 更新
				nIndex2 = ListView_GetItemCount(hwndList);
				// 選択中のキーを探す。
				nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);

				if (wID == IDC_BUTTON_KEYHELP_INS) {	// 挿入
					if (nIndex2 >= MAX_KEYHELP_FILE) {
						ErrorMessage(hwndDlg, LS(STR_PROPTYPKEYHELP_ERR_REG1));
						return FALSE;
					}
					if (nIndex == -1) {
						// 選択中でなければ最後にする。
						nIndex = nIndex2;
					}
				}else {								// 更新
					if (nIndex == -1) {
						ErrorMessage(hwndDlg, LS(STR_PROPTYPKEYHELP_SELECT));
						return FALSE;
					}
				}
				// 更新するキー情報を取得する。
				auto_memset(szPath, 0, _countof(szPath));
				::DlgItem_GetText(hwndDlg, IDC_EDIT_KEYHELP, szPath, _countof(szPath));
				if (szPath[0] == _T('\0')) {
					return FALSE;
				}
				// 重複検査
				nIndex2 = ListView_GetItemCount(hwndList);
				TCHAR szPath2[_MAX_PATH];
				for (int i=0; i<nIndex2; ++i) {
					auto_memset(szPath2, 0, _countof(szPath2));
					ListView_GetItemText(hwndList, i, 2, szPath2, _countof(szPath2));
					if (_tcscmp(szPath, szPath2) == 0) {
						if ((wID ==IDC_BUTTON_KEYHELP_UPD) && (i == nIndex)) {	// 更新時、変わっていなかったら何もしない
						}else {
							ErrorMessage(hwndDlg, LS(STR_PROPTYPKEYHELP_ERR_REG2));
							return FALSE;
						}
					}
				}

				// 指定したパスに辞書があるかチェックする
				{
					TextInputStream_AbsIni in(szPath);
					if (!in) {
						ErrorMessage(hwndDlg, LS(STR_PROPTYPKEYHELP_ERR_OPEN), szPath);
						return FALSE;
					}
					// 開けたなら1行目を取得してから閉じる -> szAbout
					std::wstring line = in.ReadLineW();
					_wcstotcs(szAbout, line.c_str(), _countof(szAbout));
					in.Close();
				}
				strcnv(szAbout);

				// ついでに辞書の説明を更新
				::DlgItem_SetText(hwndDlg, IDC_LABEL_KEYHELP_ABOUT, szAbout);	// 辞書ファイルの概要
				
				// 更新のときは行削除する。
				if (wID == IDC_BUTTON_KEYHELP_UPD) {	// 更新
					ListView_DeleteItem(hwndList, nIndex);
				}
				
				// ON/OFF ファイル名
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 0;
				lvi.pszText = GetFileName(szPath);	// ファイル名を表示
				ListView_InsertItem(hwndList, &lvi);
				// 辞書の説明
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 1;
				lvi.pszText  = szAbout;
				ListView_SetItem(hwndList, &lvi);
				// 辞書ファイルパス
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 2;
				lvi.pszText  = szPath;
				ListView_SetItem(hwndList, &lvi);
				
				// デフォルトでチェックON
				ListView_SetCheckState(hwndList, nIndex, TRUE);

				// 更新したキーを選択する。
				ListView_SetItemState(hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				GetData(hwndDlg);
				return TRUE;

			case IDC_BUTTON_KEYHELP_DEL:	// 削除
				// 選択中のキー番号を探す。
				nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
				if (nIndex == -1) {
					return FALSE;
				}
				// 削除する。
				ListView_DeleteItem(hwndList, nIndex);
				// リストがなくなったら初期値として用途を表示
				if (ListView_GetItemCount(hwndList) == 0) {
					::DlgItem_SetText(hwndDlg, IDC_LABEL_KEYHELP_ABOUT, LS(STR_PROPTYPKEYHELP_LINE1));
					::DlgItem_SetText(hwndDlg, IDC_EDIT_KEYHELP, LS(STR_PROPTYPKEYHELP_DICPATH));
				// リストの最後を削除した場合は、削除後のリストの最後を選択する。
				}else if (nIndex > ListView_GetItemCount(hwndList) - 1) {
					ListView_SetItemState(hwndList, ListView_GetItemCount(hwndList) - 1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				// 同じ位置のキーを選択状態にする。
				}else {
					ListView_SetItemState(hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				}
				GetData(hwndDlg);
				return TRUE;

			case IDC_BUTTON_KEYHELP_TOP:	// 先頭
				// 選択中のキーを探す。
				nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
				if (nIndex == -1) {
					return FALSE;
				}
				if (nIndex == 0) {
					return TRUE;	// すでに先頭にある。
				}
				nIndex2 = 0;
				bUse = ListView_GetCheckState(hwndList, nIndex);
				ListView_GetItemText(hwndList, nIndex, 1, szAbout, _countof(szAbout));
				ListView_GetItemText(hwndList, nIndex, 2, szPath, _countof(szPath));
				ListView_DeleteItem(hwndList, nIndex);	// 古いキーを削除
				// ON-OFF
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.pszText = GetFileName(szPath);	// ファイル名を表示
				ListView_InsertItem(hwndList, &lvi);
				// 辞書の説明
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szAbout;
				ListView_SetItem(hwndList, &lvi);
				// 辞書ファイルパス
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 2;
				lvi.pszText  = szPath;
				ListView_SetItem(hwndList, &lvi);
				ListView_SetCheckState(hwndList, nIndex2, bUse);
				// 移動したキーを選択状態にする。
				ListView_SetItemState(hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				GetData(hwndDlg);
				return TRUE;

			case IDC_BUTTON_KEYHELP_LAST:	// 最終
				nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
				if (nIndex == -1) {
					return FALSE;
				}
				nIndex2 = ListView_GetItemCount(hwndList);
				if (nIndex2 - 1 == nIndex) {
					return TRUE;	// すでに最終にある。
				}
				bUse = ListView_GetCheckState(hwndList, nIndex);
				ListView_GetItemText(hwndList, nIndex, 1, szAbout, _countof(szAbout));
				ListView_GetItemText(hwndList, nIndex, 2, szPath, _countof(szPath));
				// キーを追加する。
				// ON-OFF
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.pszText = GetFileName(szPath);	// ファイル名を表示
				ListView_InsertItem(hwndList, &lvi);
				// 辞書の説明
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szAbout;
				ListView_SetItem(hwndList, &lvi);
				// 辞書ファイルパス
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 2;
				lvi.pszText  = szPath;
				ListView_SetItem(hwndList, &lvi);
				ListView_SetCheckState(hwndList, nIndex2, bUse);
				// 移動したキーを選択状態にする。
				ListView_SetItemState(hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				ListView_DeleteItem(hwndList, nIndex);	// 古いキーを削除
				GetData(hwndDlg);
				return TRUE;

			case IDC_BUTTON_KEYHELP_UP:	// 上へ
				nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
				if (nIndex == -1) {
					return FALSE;
				}
				if (nIndex == 0) {
					return TRUE;	// すでに先頭にある。
				}
				nIndex2 = ListView_GetItemCount(hwndList);
				if (nIndex2 <= 1) {
					return TRUE;
				}
				nIndex2 = nIndex - 1;
				bUse = ListView_GetCheckState(hwndList, nIndex);
				ListView_GetItemText(hwndList, nIndex, 1, szAbout, _countof(szAbout));
				ListView_GetItemText(hwndList, nIndex, 2, szPath, _countof(szPath));
				ListView_DeleteItem(hwndList, nIndex);	// 古いキーを削除
				// キーを追加する。
				// ON-OFF
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.pszText = GetFileName(szPath);	// ファイル名を表示
				ListView_InsertItem(hwndList, &lvi);
				// 辞書の説明
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szAbout;
				ListView_SetItem(hwndList, &lvi);
				// 辞書ファイルパス
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 2;
				lvi.pszText  = szPath;
				ListView_SetItem(hwndList, &lvi);
				ListView_SetCheckState(hwndList, nIndex2, bUse);
				// 移動したキーを選択状態にする。
				ListView_SetItemState(hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				GetData(hwndDlg);
				return TRUE;

			case IDC_BUTTON_KEYHELP_DOWN:	// 下へ
				nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
				if (nIndex == -1) {
					return FALSE;
				}
				nIndex2 = ListView_GetItemCount(hwndList);
				if (nIndex2 - 1 == nIndex) {
					return TRUE;	// すでに最終にある。
				}
				if (nIndex2 <= 1) {
					return TRUE;
				}
				nIndex2 = nIndex + 2;
				bUse = ListView_GetCheckState(hwndList, nIndex);
				ListView_GetItemText(hwndList, nIndex, 1, szAbout, _countof(szAbout));
				ListView_GetItemText(hwndList, nIndex, 2, szPath, _countof(szPath));
				// キーを追加する。
				// ON-OFF
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.pszText = GetFileName(szPath);	// ファイル名を表示
				ListView_InsertItem(hwndList, &lvi);
				// 辞書の説明
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szAbout;
				ListView_SetItem(hwndList, &lvi);
				// 辞書ファイルパス
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 2;
				lvi.pszText  = szPath;
				ListView_SetItem(hwndList, &lvi);
				ListView_SetCheckState(hwndList, nIndex2, bUse);
				// 移動したキーを選択状態にする。
				ListView_SetItemState(hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				ListView_DeleteItem(hwndList, nIndex);	// 古いキーを削除
				GetData(hwndDlg);
				return TRUE;

			case IDC_BUTTON_KEYHELP_REF:	// キーワードヘルプ 辞書ファイルの「参照...」ボタン
				{
					DlgOpenFile	dlgOpenFile;
					// ファイルオープンダイアログの初期化
					// 相対パスは設定ファイルからのパスを優先
					TCHAR szWk[_MAX_PATH];
					::DlgItem_GetText(hwndDlg, IDC_EDIT_KEYHELP, szWk, _MAX_PATH);
					if (_IS_REL_PATH(szWk)) {
						GetInidirOrExedir(szPath, szWk);
					}else {
						::lstrcpy(szPath, szWk);
					}
					dlgOpenFile.Create(hInstance, hwndDlg, _T("*.khp"), szPath);
					if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
						::DlgItem_SetText(hwndDlg, IDC_EDIT_KEYHELP, szPath);
					}
				}
				return TRUE;

			case IDC_BUTTON_KEYHELP_IMPORT:	// インポート
				Import(hwndDlg);
				types.nKeyHelpNum = ListView_GetItemCount(hwndList);
				return TRUE;

			case IDC_BUTTON_KEYHELP_EXPORT:	// エクスポート
				Export(hwndDlg);
				return TRUE;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;

		switch (pNMHDR->code) {
		case PSN_HELP:
			OnHelp(hwndDlg, IDD_PROP_KEYHELP);
			return TRUE;

		case PSN_KILLACTIVE:
			// ダイアログデータの取得 辞書ファイルリスト
			GetData(hwndDlg);
			return TRUE;

		case PSN_SETACTIVE:
			nPageNum = ID_PROPTYPE_PAGENUM_KEYHELP;
			return TRUE;

		case LVN_ITEMCHANGED:	// リストの項目が変更された際の処理
			if (pNMHDR->hwndFrom == hwndList) {
				nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
				if (nIndex == -1) {	// 削除、範囲外でクリック時反映されないバグ修正
					nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_FOCUSED);
					return FALSE;
				}
				ListView_GetItemText(hwndList, nIndex, 1, szAbout, _countof(szAbout));
				ListView_GetItemText(hwndList, nIndex, 2, szPath, _countof(szPath));
				::DlgItem_SetText(hwndDlg, IDC_LABEL_KEYHELP_ABOUT, szAbout);	// 辞書の説明
				::DlgItem_SetText(hwndDlg, IDC_EDIT_KEYHELP, szPath);			// ファイルパス
			}
			break;
		}
		break;

	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;

	case WM_CONTEXTMENU:	// Context Menu
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;
	}
	return FALSE;
}

void CheckDlgButtonBOOL(HWND hwnd, int id, BOOL bState) {
	CheckDlgButton(hwnd, id, (bState ? BST_CHECKED : BST_UNCHECKED));
}

/*! ダイアログデータの設定 キーワード辞書ファイル設定 */
void PropTypesKeyHelp::SetData(HWND hwndDlg)
{
	// ユーザーがエディット コントロールに入力できるテキストの長さを制限する
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_KEYHELP), _countof2(types.keyHelpArr[0].szPath) - 1);

	// 使用する・使用しない
	CheckDlgButtonBOOL(hwndDlg, IDC_CHECK_KEYHELP, types.bUseKeywordHelp);
	CheckDlgButtonBOOL(hwndDlg, IDC_CHECK_KEYHELP_ALLSEARCH, types.bUseKeyHelpAllSearch);
	CheckDlgButtonBOOL(hwndDlg, IDC_CHECK_KEYHELP_KEYDISP, types.bUseKeyHelpKeyDisp);
	CheckDlgButtonBOOL(hwndDlg, IDC_CHECK_KEYHELP_PREFIX, types.bUseKeyHelpPrefix);

	// リスト
	HWND hwndWork = ::GetDlgItem(hwndDlg, IDC_LIST_KEYHELP);
	ListView_DeleteAllItems(hwndWork);  // リストを空にする
	// 行選択
	DWORD dwStyle = ListView_GetExtendedListViewStyle(hwndWork);
	dwStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle(hwndWork, dwStyle);
	// データ表示
	LV_ITEM	lvi;
	for (size_t i=0; i<MAX_KEYHELP_FILE; ++i) {
		if (types.keyHelpArr[i].szPath[0] == _T('\0')) {
			break;
		}
		// ON-OFF
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = (int)i;
		lvi.iSubItem = 0;
		lvi.pszText = GetFileName(types.keyHelpArr[i].szPath);
		ListView_InsertItem(hwndWork, &lvi);
		// 辞書の説明
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = (int)i;
		lvi.iSubItem = 1;
		lvi.pszText  = types.keyHelpArr[i].szAbout;
		ListView_SetItem(hwndWork, &lvi);
		// 辞書ファイルパス
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = (int)i;
		lvi.iSubItem = 2;
		lvi.pszText  = types.keyHelpArr[i].szPath;
		ListView_SetItem(hwndWork, &lvi);
		// ON/OFFを取得してチェックボックスにセット（とりあえず応急処置）
		if (types.keyHelpArr[i].bUse) {	// ON
			ListView_SetCheckState(hwndWork, i, TRUE);
		}else {
			ListView_SetCheckState(hwndWork, i, FALSE);
		}
	}
	ListView_SetItemState(hwndWork, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	return;
}

/*! ダイアログデータの取得 キーワード辞書ファイル設定 */
int PropTypesKeyHelp::GetData(HWND hwndDlg)
{
	TCHAR	szAbout[DICT_ABOUT_LEN];	// 辞書の説明(辞書ファイルの1行目から生成)
	TCHAR	szPath[_MAX_PATH];			// ファイルパス

	// 使用する・使用しない
	types.bUseKeywordHelp      = (IsDlgButtonChecked(hwndDlg, IDC_CHECK_KEYHELP) == BST_CHECKED);
	types.bUseKeyHelpAllSearch = (IsDlgButtonChecked(hwndDlg, IDC_CHECK_KEYHELP_ALLSEARCH) == BST_CHECKED);
	types.bUseKeyHelpKeyDisp   = (IsDlgButtonChecked(hwndDlg, IDC_CHECK_KEYHELP_KEYDISP) == BST_CHECKED);
	types.bUseKeyHelpPrefix    = (IsDlgButtonChecked(hwndDlg, IDC_CHECK_KEYHELP_PREFIX) == BST_CHECKED);

	// リストに登録されている情報を配列に取り込む
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_KEYHELP);
	int	nIndex = ListView_GetItemCount(hwndList);
	for (int i=0; i<MAX_KEYHELP_FILE; ++i) {
		if (i < nIndex) {
			bool		bUse = false;						// 辞書ON(1)/OFF(0)
			szAbout[0]	= _T('\0');
			szPath[0]	= _T('\0');
			// チェックボックス状態を取得してbUseにセット
			if (ListView_GetCheckState(hwndList, i))
				bUse = true;
			ListView_GetItemText(hwndList, i, 1, szAbout, _countof(szAbout));
			ListView_GetItemText(hwndList, i, 2, szPath, _countof(szPath));
			types.keyHelpArr[i].bUse = bUse;
			_tcscpy(types.keyHelpArr[i].szAbout, szAbout);
			_tcscpy(types.keyHelpArr[i].szPath, szPath);
		}else {	// 未登録部分はクリアする
			types.keyHelpArr[i].szPath[0] = _T('\0');
		}
	}
	// 辞書の冊数を取得
	types.nKeyHelpNum = nIndex;
	return TRUE;
}

/*! キーワードヘルプファイルリストのインポート */
bool PropTypesKeyHelp::Import(HWND hwndDlg)
{
	// インポート
	GetData(hwndDlg);

	ImpExpKeyHelp  cImpExpKeyHelp(types);
	if (!cImpExpKeyHelp.ImportUI(hInstance, hwndDlg)) {
		// インポートをしていない
		return false;
	}
	SetData(hwndDlg);

	return true;
}


/*! キーワードヘルプファイルリストのインポートエクスポート */
bool PropTypesKeyHelp::Export(HWND hwndDlg)
{
	GetData(hwndDlg);
	ImpExpKeyHelp	cImpExpKeyHelp(types);

	// エクスポート
	return cImpExpKeyHelp.ExportUI(hInstance, hwndDlg);
}


/*! 辞書の説明のフォーマット揃え */
static TCHAR* strcnv(TCHAR* str)
{
	TCHAR* p = str;
	// 改行コードの削除
	if ((p = _tcschr(p, _T('\n')))) {
		*p = _T('\0');
	}
	p = str;
	if ((p = _tcschr(p, _T('\r')))) {
		*p = _T('\0');
	}
	// カンマの置換
	p = str;
	for (; (p =_tcschr(p, _T(',')));) {
		*p = _T('.');
	}
	return str;
}

/*! フルパスからファイル名を返す */
static TCHAR* GetFileName(const TCHAR* fullpath)
{
	const TCHAR* pszName = fullpath;
	CharPointerT p = fullpath;
	while (*p != _T('\0')) {
		if (*p == _T('\\')) {
			pszName = p + 1;
			++p;
		}else {
			++p;
		}
	}
	return const_cast<TCHAR*>(pszName);
}

