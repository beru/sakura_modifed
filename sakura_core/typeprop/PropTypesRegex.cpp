// タイプ別設定 - 正規表現キーワード ダイアログボックス

#include "StdAfx.h"

#include <memory>

#include "PropTypes.h"
#include "env/ShareData.h"
#include "RegexKeyword.h"
#include "typeprop/ImpExpManager.h"
#include "util/shell.h"
#include "view/colors/EColorIndexType.h"
#include "sakura_rc.h"
#include "sakura.hh"

using namespace std;

static const DWORD p_helpids[] = {	//11600
	IDC_BUTTON_REGEX_IMPORT,	HIDC_BUTTON_REGEX_IMPORT,	// インポート
	IDC_BUTTON_REGEX_EXPORT,	HIDC_BUTTON_REGEX_EXPORT,	// エクスポート
	IDC_BUTTON_REGEX_INS,		HIDC_BUTTON_REGEX_INS,		// 挿入
	IDC_BUTTON_REGEX_ADD,		HIDC_BUTTON_REGEX_ADD,		// 追加
	IDC_BUTTON_REGEX_UPD,		HIDC_BUTTON_REGEX_UPD,		// 更新
	IDC_BUTTON_REGEX_DEL,		HIDC_BUTTON_REGEX_DEL,		// 削除
	IDC_BUTTON_REGEX_TOP,		HIDC_BUTTON_REGEX_TOP,		// 先頭
	IDC_BUTTON_REGEX_LAST,		HIDC_BUTTON_REGEX_LAST,		// 最終
	IDC_BUTTON_REGEX_UP,		HIDC_BUTTON_REGEX_UP,		// 上へ
	IDC_BUTTON_REGEX_DOWN,		HIDC_BUTTON_REGEX_DOWN,		// 下へ
	IDC_CHECK_REGEX,			HIDC_CHECK_REGEX,			// 正規表現キーワードを使用する
	IDC_COMBO_REGEX_COLOR,		HIDC_COMBO_REGEX_COLOR,		// 色
	IDC_EDIT_REGEX,				HIDC_EDIT_REGEX,			// 正規表現キーワード
	IDC_LIST_REGEX,				HIDC_LIST_REGEX,			// リスト
	IDC_LABEL_REGEX_KEYWORD,	HIDC_EDIT_REGEX,			
	IDC_LABEL_REGEX_COLOR,		HIDC_COMBO_REGEX_COLOR,		
	IDC_FRAME_REGEX,			HIDC_LIST_REGEX,			
	IDC_LABEL_REGEX_VERSION,	HIDC_LABEL_REGEX_VERSION,	// バージョン
//	IDC_STATIC,						-1,
	0, 0
};

// Import
bool PropTypesRegex::Import(HWND hwndDlg)
{
	ImpExpRegex impExpRegex(types);

	// インポート
	bool bImport = impExpRegex.ImportUI(hInstance, hwndDlg);
	if (bImport) {
		SetDataKeywordList(hwndDlg);
	}
	return bImport;
}

// Export
bool PropTypesRegex::Export(HWND hwndDlg)
{
	GetData(hwndDlg);
	ImpExpRegex cImpExpRegex(types);

	// エクスポート
	return cImpExpRegex.ExportUI(hInstance, hwndDlg);
}

// 正規表現キーワード メッセージ処理
INT_PTR PropTypesRegex::DispatchEvent(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
	)
{
	WORD	wNotifyCode;
	WORD	wID;
	NMHDR*	pNMHDR;
	int	nIndex, nIndex2, i, j, nRet;
	LV_ITEM	lvi;
	LV_COLUMN	col;
	RECT		rc;
	static int nPrevIndex = -1;	// 更新時におかしくなるバグ修正

	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_REGEX);

	// ANSIビルドではCP932だと2倍程度必要
	const int nKeywordSize = MAX_REGEX_KEYWORDLEN;
	TCHAR szColorIndex[256];

	switch (uMsg) {
	case WM_INITDIALOG:
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// カラム追加
		//ListView_DeleteColumn(hwndList, 1);
		//ListView_DeleteColumn(hwndList, 0);
		::GetWindowRect(hwndList, &rc);
		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 54 / 100;
		col.pszText  = const_cast<TCHAR*>(LS(STR_PROPTYPEREGEX_LIST1));
		col.iSubItem = 0;
		ListView_InsertColumn(hwndList, 0, &col);
		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 38 / 100;
		col.pszText  = const_cast<TCHAR*>(LS(STR_PROPTYPEREGEX_LIST2));
		col.iSubItem = 1;
		ListView_InsertColumn(hwndList, 1, &col);

		nPrevIndex = -1;
		SetData(hwndDlg);	// ダイアログデータの設定 正規表現キーワード
		if (!CheckRegexpVersion(hwndDlg, IDC_LABEL_REGEX_VERSION, false)) {
			::DlgItem_SetText(hwndDlg, IDC_LABEL_REGEX_VERSION, LS(STR_PROPTYPEREGEX_NOUSE));
			// ライブラリがなくて、使用しないになっている場合は、無効にする。
			if (!IsDlgButtonChecked(hwndDlg, IDC_CHECK_REGEX)) {
				// Disableにする。
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_REGEX), FALSE);
			}else {
				// 使用するになってるんだけどDisableにする。もうユーザは変更できない。
				EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_REGEX), FALSE);
			}
		}
		return TRUE;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// 通知コード
		wID	= LOWORD(wParam);	// 項目ID､ コントロールID､ またはアクセラレータID
		switch (wNotifyCode) {
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			case IDC_CHECK_REGEX:	// 正規表現キーワードを使う
				if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_REGEX)) {
					if (!CheckRegexpVersion(NULL, 0, false)) {
						nRet = ::MYMESSAGEBOX(
								hwndDlg,
								MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2,
								GSTR_APPNAME,
								LS(STR_PROPTYPEREGEX_NOTFOUND));
						if (nRet != IDYES) {
							CheckDlgButton(hwndDlg, IDC_CHECK_REGEX, BST_UNCHECKED);
							// Disableにする。
							EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_REGEX), FALSE);
							return TRUE;
						}
					}
				}else {
					if (!CheckRegexpVersion(NULL, 0, false)) {
						// Disableにする。
						EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_REGEX), FALSE);
					}
				}
				types.nRegexKeyMagicNumber = RegexKeyword::GetNewMagicNumber();	// Need Compile
				return TRUE;

			case IDC_BUTTON_REGEX_INS:	// 挿入
			{
				// 挿入するキー情報を取得する。
				auto szKeyword = std::make_unique<TCHAR[]>(nKeywordSize);
				szKeyword[0] = _T('\0');
				::DlgItem_GetText(hwndDlg, IDC_EDIT_REGEX, &szKeyword[0], nKeywordSize);
				if (szKeyword[0] == _T('\0')) {
					return FALSE;
				}
				// 同じキーがないか調べる。
				nIndex2 = ListView_GetItemCount(hwndList);
				if (nIndex2 >= MAX_REGEX_KEYWORD) {
					ErrorMessage(hwndDlg, LS(STR_PROPTYPEREGEX_NOREG));
					return FALSE;
				}
				// 選択中のキーを探す。
				nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
				if (nIndex == -1) {
					// 選択中でなければ最後にする。
					nIndex = nIndex2;
				}
				if (!CheckKeywordList(hwndDlg, &szKeyword[0], -1)) {
					return FALSE;
				}
				
				// 挿入するキー情報を取得する。
				auto_memset(szColorIndex, 0, _countof(szColorIndex));
				::DlgItem_GetText(hwndDlg, IDC_COMBO_REGEX_COLOR, szColorIndex, _countof(szColorIndex));
				// キー情報を挿入する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = &szKeyword[0];
				lvi.iItem    = nIndex;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem(hwndList, &lvi);
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem(hwndList, &lvi);
				// 挿入したキーを選択する。
				ListView_SetItemState(hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				GetData(hwndDlg);
				return TRUE;
			}

			case IDC_BUTTON_REGEX_ADD:	// 追加
			{
				auto szKeyword = std::make_unique<TCHAR[]>(nKeywordSize);
				// 最後のキー番号を取得する。
				nIndex = ListView_GetItemCount(hwndList);
				// 追加するキー情報を取得する。
				szKeyword[0] = _T('\0');
				::DlgItem_GetText(hwndDlg, IDC_EDIT_REGEX, &szKeyword[0], nKeywordSize);
				if (szKeyword[0] == L'\0') {
					return FALSE;
				}
				nIndex2 = ListView_GetItemCount(hwndList);
				if (nIndex2 >= MAX_REGEX_KEYWORD) {
					ErrorMessage(hwndDlg, LS(STR_PROPTYPEREGEX_NOREG));
					return FALSE;
				}
				if (!CheckKeywordList(hwndDlg, &szKeyword[0], -1)) {
					return FALSE;
				}
				// 追加するキー情報を取得する。
				auto_memset(szColorIndex, 0, _countof(szColorIndex));
				::DlgItem_GetText(hwndDlg, IDC_COMBO_REGEX_COLOR, szColorIndex, _countof(szColorIndex));
				// キーを追加する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = &szKeyword[0];
				lvi.iItem    = nIndex;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem(hwndList, &lvi);
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem(hwndList, &lvi);
				// 追加したキーを選択する。
				ListView_SetItemState(hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				GetData(hwndDlg);
				return TRUE;
			}

			case IDC_BUTTON_REGEX_UPD:	// 更新
			{
				auto szKeyword = std::make_unique<TCHAR[]>(nKeywordSize);
				// 選択中のキーを探す。
				nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
				if (nIndex == -1) {
					ErrorMessage(hwndDlg, LS(STR_PROPTYPEREGEX_NOSEL));
					return FALSE;
				}
				// 更新するキー情報を取得する。
				szKeyword[0] = _T('\0');
				::DlgItem_GetText(hwndDlg, IDC_EDIT_REGEX, &szKeyword[0], nKeywordSize);
				if (&szKeyword[0] == L'\0') {
					return FALSE;
				}
				if (!CheckKeywordList(hwndDlg, &szKeyword[0], nIndex)) {
					return FALSE;
				}
				// 追加するキー情報を取得する。
				auto_memset(szColorIndex, 0, _countof(szColorIndex));
				::DlgItem_GetText(hwndDlg, IDC_COMBO_REGEX_COLOR, szColorIndex, _countof(szColorIndex));
				// キーを更新する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = &szKeyword[0];
				lvi.iItem    = nIndex;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_SetItem(hwndList, &lvi);

				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem(hwndList, &lvi);

				// 更新したキーを選択する。
				ListView_SetItemState(hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				GetData(hwndDlg);
				return TRUE;
			}

			case IDC_BUTTON_REGEX_DEL:	// 削除
				// 選択中のキー番号を探す。
				nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
				if (nIndex == -1) {
					return FALSE;
				}
				// 削除する。
				ListView_DeleteItem(hwndList, nIndex);
				// 同じ位置のキーを選択状態にする。
				ListView_SetItemState(hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				GetData(hwndDlg);
				return TRUE;

			case IDC_BUTTON_REGEX_TOP:	// 先頭
			{
				auto szKeyword = std::make_unique<TCHAR[]>(nKeywordSize);
				szKeyword[0] = _T('\0');
				// 選択中のキーを探す。
				nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
				if (nIndex == -1) {
					return FALSE;
				}
				if (nIndex == 0) {
					return TRUE;	// すでに先頭にある。
				}
				nIndex2 = 0;
				ListView_GetItemText(hwndList, nIndex, 0, &szKeyword[0], nKeywordSize);
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
				ListView_DeleteItem(hwndList, nIndex);	// 古いキーを削除
				// キーを追加する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = &szKeyword[0];
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem(hwndList, &lvi);
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem(hwndList, &lvi);
				// 移動したキーを選択状態にする。
				ListView_SetItemState(hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				GetData(hwndDlg);
				return TRUE;
			}

			case IDC_BUTTON_REGEX_LAST:	// 最終
			{
				auto szKeyword = std::make_unique<TCHAR[]>(nKeywordSize);
				szKeyword[0] = _T('\0');
				nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
				if (nIndex == -1) {
					return FALSE;
				}
				nIndex2 = ListView_GetItemCount(hwndList);
				if (nIndex2 - 1 == nIndex) {
					return TRUE;	// すでに最終にある。
				}
				ListView_GetItemText(hwndList, nIndex, 0, &szKeyword[0], nKeywordSize);
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
				// キーを追加する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = &szKeyword[0];
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem(hwndList, &lvi);
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem(hwndList, &lvi);
				// 移動したキーを選択状態にする。
				ListView_SetItemState(hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				ListView_DeleteItem(hwndList, nIndex);	// 古いキーを削除
				GetData(hwndDlg);
				return TRUE;
			}

			case IDC_BUTTON_REGEX_UP:	// 上へ
			{
				auto szKeyword = std::make_unique<TCHAR[]>(nKeywordSize);
				szKeyword[0] = _T('\0');
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
				ListView_GetItemText(hwndList, nIndex, 0, &szKeyword[0], nKeywordSize);
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
				ListView_DeleteItem(hwndList, nIndex);	// 古いキーを削除
				// キーを追加する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = &szKeyword[0];
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem(hwndList, &lvi);
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem(hwndList, &lvi);
				// 移動したキーを選択状態にする。
				ListView_SetItemState(hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				GetData(hwndDlg);
				return TRUE;
			}

			case IDC_BUTTON_REGEX_DOWN:	// 下へ
			{
				auto szKeyword = std::make_unique<TCHAR[]>(nKeywordSize);
				szKeyword[0] = _T('\0');
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
				ListView_GetItemText(hwndList, nIndex, 0, &szKeyword[0], nKeywordSize);
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
				// キーを追加する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = &szKeyword[0];
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem(hwndList, &lvi);
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem(hwndList, &lvi);
				// 移動したキーを選択状態にする。
				ListView_SetItemState(hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
				ListView_DeleteItem(hwndList, nIndex);	// 古いキーを削除
				GetData(hwndDlg);
				return TRUE;
			}

			case IDC_BUTTON_REGEX_IMPORT:	// インポート
				Import(hwndDlg);
				types.nRegexKeyMagicNumber = RegexKeyword::GetNewMagicNumber();
				return TRUE;

			case IDC_BUTTON_REGEX_EXPORT:	// エクスポート
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
			OnHelp(hwndDlg, IDD_PROP_REGEX);
			return TRUE;
		case PSN_KILLACTIVE:
			// ダイアログデータの取得 正規表現キーワード
			GetData(hwndDlg);
			return TRUE;
		case PSN_SETACTIVE:
			nPageNum = ID_PROPTYPE_PAGENUM_REGEX;
			return TRUE;
		case LVN_ITEMCHANGED:
			if (pNMHDR->hwndFrom == hwndList) {
				HWND	hwndCombo;
				nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_SELECTED);
				if (nIndex == -1) {
					nIndex = ListView_GetNextItem(hwndList, -1, LVNI_ALL | LVNI_FOCUSED);
				}
				if (nIndex == -1) {
					// 初期値を設定する
					::DlgItem_SetText(hwndDlg, IDC_EDIT_REGEX, _T("//k"));	// 正規表現
					hwndCombo = GetDlgItem(hwndDlg, IDC_COMBO_REGEX_COLOR);
					for (i=0, j=0; i<COLORIDX_LAST; ++i) {
						if ((g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_TEXT) == 0 &&
							(g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) == 0
						) {
							if (types.colorInfoArr[i].nColorIdx == COLORIDX_REGEX1) {
								Combo_SetCurSel(hwndCombo, j);	// コンボボックスのデフォルト選択
								break;
							}
							++j;
						}
					}
					return FALSE;
				}
				if (nPrevIndex != nIndex) {
					// 更新時にListViewのSubItemを正しく取得できないので、その対策
					auto szKeyword = std::make_unique<TCHAR[]>(nKeywordSize);
					szKeyword[0] = _T('\0');
					ListView_GetItemText(hwndList, nIndex, 0, &szKeyword[0], nKeywordSize);
					ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
					::DlgItem_SetText(hwndDlg, IDC_EDIT_REGEX, &szKeyword[0]);	// 正規表現
					hwndCombo = GetDlgItem(hwndDlg, IDC_COMBO_REGEX_COLOR);
					for (i=0, j=0; i<COLORIDX_LAST; ++i) {
						if ((g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_TEXT) == 0 &&
							(g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) == 0
						) {
							if (_tcscmp(types.colorInfoArr[i].szName, szColorIndex) == 0) {
								Combo_SetCurSel(hwndCombo, j);
								break;
							}
							++j;
						}
					}
				}
				nPrevIndex = nIndex;
			}
			break;
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

	}
	return FALSE;
}

// ダイアログデータの設定 正規表現キーワード
void PropTypesRegex::SetData(HWND hwndDlg)
{
	// ユーザーがエディット コントロールに入力できるテキストの長さを制限する
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_REGEX), MAX_REGEX_KEYWORDLEN - 1);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_REGEX, _T("//k"));	// 正規表現

	// 色種類のリスト
	HWND hwndWork = ::GetDlgItem(hwndDlg, IDC_COMBO_REGEX_COLOR);
	Combo_ResetContent(hwndWork);  // コンボボックスを空にする
	for (int i=0; i<COLORIDX_LAST; ++i) {
		GetDefaultColorInfoName(&types.colorInfoArr[i], i);
		if ((g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_TEXT) == 0 &&
			(g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) == 0
		) {
			int j = Combo_AddString(hwndWork, types.colorInfoArr[i].szName);
			if (types.colorInfoArr[i].nColorIdx == COLORIDX_REGEX1) {
				Combo_SetCurSel(hwndWork, j);	// コンボボックスのデフォルト選択
			}
		}
	}

	if (types.bUseRegexKeyword) {
		CheckDlgButton(hwndDlg, IDC_CHECK_REGEX, BST_CHECKED);
	}else {
		CheckDlgButton(hwndDlg, IDC_CHECK_REGEX, BST_UNCHECKED);
	}

	// 行選択
	hwndWork = ::GetDlgItem(hwndDlg, IDC_LIST_REGEX);
	DWORD		dwStyle;
	dwStyle = ListView_GetExtendedListViewStyle(hwndWork);
	dwStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle(hwndWork, dwStyle);

	SetDataKeywordList(hwndDlg);
}

// ダイアログデータの設定 正規表現キーワードの一覧部分
void PropTypesRegex::SetDataKeywordList(HWND hwndDlg)
{
	LV_ITEM		lvi;

	// リスト
	HWND hwndWork = ::GetDlgItem(hwndDlg, IDC_LIST_REGEX);
	ListView_DeleteAllItems(hwndWork);  // リストを空にする

	// データ表示
	wchar_t* pKeyword = &types.regexKeywordList[0];
	for (int i=0; i<MAX_REGEX_KEYWORD; ++i) {
		if (*pKeyword == L'\0') {
			break;
		}
		
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = const_cast<TCHAR*>(to_tchar(pKeyword));
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.lParam   = 0; //types.regexKeywordArr[i].nColorIndex;
		ListView_InsertItem(hwndWork, &lvi);
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = types.colorInfoArr[types.regexKeywordArr[i].nColorIndex].szName;
		ListView_SetItem(hwndWork, &lvi);
		for (; *pKeyword!='\0'; ++pKeyword) {
			;
		}
		++pKeyword;
	}
	ListView_SetItemState(hwndWork, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	return;
}

// ダイアログデータの取得 正規表現キーワード
int PropTypesRegex::GetData(HWND hwndDlg)
{
	HWND	hwndList;
	int	nIndex, i, j;
	const int szKeywordSize = _countof(types.regexKeywordList) * 2 + 1;
	auto szKeyword = std::make_unique<TCHAR[]>(szKeywordSize);
	TCHAR	szColorIndex[256];

	// 使用する・使用しない
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_REGEX)) {
		types.bUseRegexKeyword = true;
	}else {
		types.bUseRegexKeyword = false;
	}

	// リストに登録されている情報を配列に取り込む
	hwndList = GetDlgItem(hwndDlg, IDC_LIST_REGEX);
	nIndex = ListView_GetItemCount(hwndList);
	wchar_t* pKeyword = &types.regexKeywordList[0];
	wchar_t* pKeywordLast = pKeyword + _countof(types.regexKeywordList) - 1;
	// key1\0key2\0\0 の形式
	for (i=0; i<MAX_REGEX_KEYWORD; ++i) {
		if (i < nIndex) {
			szKeyword[0]    = _T('\0');
			szColorIndex[0] = _T('\0');
			ListView_GetItemText(hwndList, i, 0, &szKeyword[0], szKeywordSize);
			ListView_GetItemText(hwndList, i, 1, szColorIndex, _countof(szColorIndex));
			if (pKeyword < pKeywordLast - 1) {
				_tcstowcs(pKeyword, &szKeyword[0], pKeywordLast - pKeyword);
			}
			// 色指定文字列を番号に変換する
			types.regexKeywordArr[i].nColorIndex = COLORIDX_REGEX1;
			for (j=0; j<COLORIDX_LAST; ++j) {
				if (_tcscmp(types.colorInfoArr[j].szName, szColorIndex) == 0) {
					types.regexKeywordArr[i].nColorIndex = j;
					break;
				}
			}
			if (*pKeyword) {
				for (; *pKeyword!=L'\0'; ++pKeyword) {}
				++pKeyword;
			}
		}else { // 未登録部分はクリアする
			types.regexKeywordArr[i].nColorIndex = COLORIDX_REGEX1;
		}
	}
	*pKeyword = L'\0'; // 番兵

	// タイプ設定の変更があった
	types.nRegexKeyMagicNumber = RegexKeyword::GetNewMagicNumber();
//	types.nRegexKeyMagicNumber = 0;	// Not Compiled.

	return TRUE;
}

BOOL PropTypesRegex::RegexKakomiCheck(const wchar_t* s)
{
	return RegexKeyword::RegexKeyCheckSyntax(s);
}

bool PropTypesRegex::CheckKeywordList(
	HWND hwndDlg,
	const TCHAR* szNewKeyword,
	int nUpdateItem
	)
{
	int nRet;
	// 書式をチェックする。
	if (!RegexKakomiCheck(to_wchar(szNewKeyword))) {	// 囲みをチェックする。
		nRet = ::MYMESSAGEBOX(
				hwndDlg,
				MB_OK | MB_ICONSTOP | MB_TOPMOST | MB_DEFBUTTON2,
				GSTR_APPNAME,
				LS(STR_PROPTYPEREGEX_KAKOMI));
		return false;
	}
	if (!CheckRegexpSyntax(to_wchar(szNewKeyword), hwndDlg, false, -1, true)) {
		nRet = ::MYMESSAGEBOX(
				hwndDlg,
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2,
				GSTR_APPNAME,
				LS(STR_PROPTYPEREGEX_INVALID));
		if (nRet != IDYES) {
			return false;
		}
	}
	// 重複確認・文字列長制限チェック
	const int nKeywordSize = MAX_REGEX_KEYWORDLEN;
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST_REGEX);
	int  nIndex  = ListView_GetItemCount(hwndList);
	auto szKeyword = std::make_unique<TCHAR[]>(nKeywordSize);
	size_t nKeywordLen = auto_strlen(to_wchar(szNewKeyword)) + 1;
	for (int i=0; i<nIndex; ++i) {
		if (i != nUpdateItem) {
			szKeyword[0] = _T('\0');
			ListView_GetItemText(hwndList, i, 0, &szKeyword[0], nKeywordSize);
			if (_tcscmp(szNewKeyword, &szKeyword[0]) == 0) {
				ErrorMessage(hwndDlg, LS(STR_PROPTYPEREGEX_ALREADY));
				return false;
			}
			// 長さには\0も含む
			nKeywordLen += auto_strlen(to_wchar(&szKeyword[0])) + 1;
			if (_countof(types.regexKeywordList) - 1 < nKeywordLen) {
				ErrorMessage(hwndDlg, LS(STR_PROPTYPEREGEX_FULL));
				return false;
			}
		}
	}
	return true;
}

