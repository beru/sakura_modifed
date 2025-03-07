/*!	@file
	共通設定ダイアログボックス、「マクロ」ページ
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "env/DllSharedData.h"
#include "util/shell.h"
#include "util/string_ex2.h"
#include "util/module.h"
#include "sakura_rc.h"
#include "sakura.hh"

// Popup Help用ID
static const DWORD p_helpids[] = {	//11700
	IDC_MACRODIRREF,	HIDC_MACRODIRREF,	// マクロディレクトリ参照
	IDC_MACRO_REG,		HIDC_MACRO_REG,		// マクロ設定
	IDC_COMBO_MACROID,	HIDC_COMBO_MACROID,	// ID
	IDC_MACROPATH,		HIDC_MACROPATH,		// File
	IDC_MACRONAME,		HIDC_MACRONAME,		// マクロ名
	IDC_MACROLIST,		HIDC_MACROLIST,		// マクロリスト
	IDC_MACRODIR,		HIDC_MACRODIR,		// マクロ一覧
	IDC_CHECK_RELOADWHENEXECUTE,	HIDC_CHECK_RELOADWHENEXECUTE,	// マクロを実行するたびにファイルを読み込みなおす
	IDC_CHECK_MacroOnOpened,		HIDC_CHECK_MacroOnOpened,		// オープン後自動実行マクロ
	IDC_CHECK_MacroOnTypeChanged,	HIDC_CHECK_MacroOnTypeChanged,	// タイプ変更後自動実行マクロ
	IDC_CHECK_MacroOnSave,			HIDC_CHECK_MacroOnSave,			// 保存前自動実行マクロ
	IDC_MACROCANCELTIMER,			HIDC_MACROCANCELTIMER,			// マクロ停止ダイアログ表示待ち時間
//	IDC_STATIC,			-1,
	0, 0
};

/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK PropMacro::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropMacro::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}

/*! Macroページのメッセージ処理
	@param hwndDlg ダイアログボックスのWindow Handlw
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR PropMacro::DispatchEvent(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	NMHDR*		pNMHDR;
	int			idCtrl;

	WORD		wNotifyCode;
	WORD		wID;

	auto& csMacro = common.macro;

	switch (uMsg) {
	case WM_INITDIALOG:
		// ダイアログデータの設定 Macro
		InitDialog(hwndDlg);
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// エディット コントロールに入力できるテキストの長さを制限する
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_MACRONAME), _countof(csMacro.macroTable[0].szName) - 1);
		Combo_LimitText(::GetDlgItem(hwndDlg, IDC_MACROPATH), _countof(csMacro.macroTable[0].szFile) - 1);
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_MACRODIR), _countof2(csMacro.szMACROFOLDER) - 1);
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_MACROCANCELTIMER), 4);
		return TRUE;
		
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		switch (idCtrl) {
		case IDC_MACROLIST:
			switch (pNMHDR->code) {
			case LVN_ITEMCHANGED:
				CheckListPosition_Macro(hwndDlg);
				break;
			}
			break;
		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_MACRO);
				return TRUE;
			case PSN_KILLACTIVE:
				// ダイアログデータの取得 Macro
				GetData(hwndDlg);
				return TRUE;
			case PSN_SETACTIVE:
				nPageNum = ID_PROPCOM_PAGENUM_MACRO;
				return TRUE;
			}
			break;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// 通知コード
		wID = LOWORD(wParam);			// 項目ID､ コントロールID､ またはアクセラレータID

		switch (wNotifyCode) {
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			case IDC_MACRODIRREF:	// マクロディレクトリ参照
				SelectBaseDir_Macro(hwndDlg);
				break;
			case IDC_MACRO_REG:		// マクロ設定
				SetMacro2List_Macro(hwndDlg);
				break;
			}
			break;
		case CBN_DROPDOWN:
			switch (wID) {
			case IDC_MACROPATH:
				OnFileDropdown_Macro(hwndDlg);
				break;
			}
			break;	// CBN_DROPDOWN
		// マクロフォルダの最後の\がなければ付ける
		case EN_KILLFOCUS:
			switch (wID) {
			case IDC_MACRODIR:
				{
					TCHAR szDir[_MAX_PATH];
					::DlgItem_GetText(hwndDlg, IDC_MACRODIR, szDir, _MAX_PATH);
					if (AddLastChar(szDir, _MAX_PATH, _T('\\')) == 1) {
						::DlgItem_SetText(hwndDlg, IDC_MACRODIR, szDir);
					}
				}
				break;
			}
			break;
		}

		break;	// WM_COMMAND
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


/*!
	ダイアログ上のコントロールにデータを設定する

	@param hwndDlg ダイアログボックスのウィンドウハンドル
*/
void PropMacro::SetData(HWND hwndDlg)
{
	int index;
	LVITEM lvItem;

	// マクロデータ
	HWND hListView = ::GetDlgItem(hwndDlg, IDC_MACROLIST);
	auto& csMacro = pShareData->common.macro;
	
	for (index=0; index<MAX_CUSTMACRO; ++index) {
		auto& macroRec = csMacro.macroTable[index];
		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 1;
		lvItem.pszText = macroRec.szName;
		ListView_SetItem(hListView, &lvItem);

		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 2;
		lvItem.pszText = macroRec.szFile;
		ListView_SetItem(hListView, &lvItem);

		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 3;
		lvItem.pszText = const_cast<TCHAR*>(macroRec.bReloadWhenExecute ? _T("on") : _T("off"));
		ListView_SetItem(hListView, &lvItem);

		// 自動実行マクロ
		TCHAR szText[8];
		szText[0] = _T('\0');
		if (index == csMacro.nMacroOnOpened) {
			::lstrcat(szText, _T("O"));
		}
		if (index == csMacro.nMacroOnTypeChanged) {
			::lstrcat(szText, _T("T"));
		}
		if (index == csMacro.nMacroOnSave) {
			::lstrcat(szText, _T("S"));
		}
		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 4;
		lvItem.pszText = szText;
		ListView_SetItem(hListView, &lvItem);
	}
	
	//	マクロディレクトリ
	::DlgItem_SetText(hwndDlg, IDC_MACRODIR, /*pShareData->*/common.macro.szMACROFOLDER);

	nLastPos_Macro = -1;
	
	//	リストビューの行選択を可能にする．
	//	IE 3.x以降が入っている場合のみ動作する．
	//	これが無くても，番号部分しか選択できないだけで操作自体は可能．
	DWORD dwStyle;
	dwStyle = ListView_GetExtendedListViewStyle(hListView);
	dwStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle(hListView, dwStyle);
	
	//	マクロ停止ダイアログ表示待ち時間
	TCHAR szCancelTimer[16] = {0};
	::DlgItem_SetText(hwndDlg, IDC_MACROCANCELTIMER, _itot(common.macro.nMacroCancelTimer, szCancelTimer, 10));

	return;
}

/*!
	ダイアログ上のコントロールからデータを取得してメモリに格納する

	@param hwndDlg ダイアログボックスのウィンドウハンドル
*/

int PropMacro::GetData(HWND hwndDlg)
{
	int index;
	LVITEM lvItem;

	auto& csMacro = common.macro;

	// 自動実行マクロ変数初期化
	csMacro.nMacroOnOpened = -1;
	csMacro.nMacroOnTypeChanged = -1;
	csMacro.nMacroOnSave = -1;

	//	マクロデータ
	HWND hListView = ::GetDlgItem(hwndDlg, IDC_MACROLIST);

	for (index=0; index<MAX_CUSTMACRO; ++index) {
		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 1;
		lvItem.cchTextMax = MACRONAME_MAX - 1;
		lvItem.pszText = /*pShareData->*/csMacro.macroTable[index].szName;
		ListView_GetItem(hListView, &lvItem);

		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 2;
		lvItem.cchTextMax = _MAX_PATH;
		lvItem.pszText = /*pShareData->*/csMacro.macroTable[index].szFile;
		ListView_GetItem(hListView, &lvItem);

		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 3;
		TCHAR buf[MAX_PATH];
		lvItem.pszText = buf;
		lvItem.cchTextMax = MAX_PATH;
		ListView_GetItem(hListView, &lvItem);
		if (_tcscmp(buf, _T("on")) == 0) {
			csMacro.macroTable[index].bReloadWhenExecute = true;
		}else {
			csMacro.macroTable[index].bReloadWhenExecute = false;
		}

		// 自動実行マクロ
		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = index;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 4;
		TCHAR szText[8];
		lvItem.pszText = szText;
		lvItem.cchTextMax = _countof(szText);
		ListView_GetItem(hListView, &lvItem);
		int nLen = ::lstrlen(szText);
		for (int i=0; i<nLen; ++i) {
			if (szText[i] == _T('O')) {
				csMacro.nMacroOnOpened = index;
			}
			if (szText[i] == _T('T')) {
				csMacro.nMacroOnTypeChanged = index;
			}
			if (szText[i] == _T('S')) {
				csMacro.nMacroOnSave = index;
			}
		}
	}

	//	マクロディレクトリ
	::DlgItem_GetText(hwndDlg, IDC_MACRODIR, csMacro.szMACROFOLDER, _MAX_PATH);
	// マクロフォルダの最後の\がなければ付ける
	AddLastChar(csMacro.szMACROFOLDER, _MAX_PATH, _T('\\'));
	
	//	マクロ停止ダイアログ表示待ち時間
	TCHAR szCancelTimer[16] = {0};
	::DlgItem_GetText(hwndDlg, IDC_MACROCANCELTIMER, szCancelTimer, _countof(szCancelTimer));
	csMacro.nMacroCancelTimer = _ttoi(szCancelTimer);

	return TRUE;
}

struct ColumnData_CPropMacro_Init {
	int titleId;
	int width;
};

void PropMacro::InitDialog(HWND hwndDlg)
{
	struct ColumnData_CPropMacro_Init ColumnList[] = {
		{ STR_PROPCOMMACR_LIST1, 40 },
		{ STR_PROPCOMMACR_LIST2, 150 },
		{ STR_PROPCOMMACR_LIST3, 150 },
		{ STR_PROPCOMMACR_LIST4, 40 },
		{ STR_PROPCOMMACR_LIST5, 40 },
	};

	//	ListViewの初期化
	HWND hListView = ::GetDlgItem(hwndDlg, IDC_MACROLIST);
	if (!hListView) {
		PleaseReportToAuthor(hwndDlg, _T("PropComMacro::InitDlg::NoListView"));
		return;	//	よくわからんけど失敗した	
	}

	LVCOLUMN sColumn;
	int pos;
	RECT rc;
	::GetWindowRect(hListView, &rc);
	int width = rc.right - rc.left;
	
	for (pos=0; pos<_countof(ColumnList); ++pos) {
		
		memset_raw(&sColumn, 0, sizeof(sColumn));
		sColumn.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM | LVCF_FMT;
		sColumn.pszText = const_cast<TCHAR*>(LS(ColumnList[pos].titleId));
		sColumn.cx = ColumnList[pos].width * width / 499;
		sColumn.iSubItem = pos;
		sColumn.fmt = LVCFMT_LEFT;
		
		if (ListView_InsertColumn(hListView, pos, &sColumn) < 0) {
			PleaseReportToAuthor(hwndDlg, _T("PropComMacro::InitDlg::ColumnRegistrationFail"));
			return;	//	よくわからんけど失敗した
		}
	}

	//	メモリの確保
	//	必要な数だけ先に確保する．
	ListView_SetItemCount(hListView, MAX_CUSTMACRO);

	//	Index部分の登録
	for (pos=0; pos<MAX_CUSTMACRO; ++pos) {
		LVITEM lvItem;
		TCHAR buf[4];
		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.mask = LVIF_TEXT | LVIF_PARAM;
		lvItem.iItem = pos;
		lvItem.iSubItem = 0;
		_itot(pos, buf, 10);
		lvItem.pszText = buf;
		lvItem.lParam = pos;
		ListView_InsertItem(hListView, &lvItem);
	}
	
	// 登録先指定 ComboBoxの初期化
	HWND hNumCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_MACROID);
	for (pos=0; pos<MAX_CUSTMACRO; ++pos) {
		wchar_t buf[10];
		auto_sprintf(buf, L"%d", pos);
		LRESULT result = Combo_AddString(hNumCombo, buf);
		if (result == CB_ERR) {
			PleaseReportToAuthor(hwndDlg, _T("PropComMacro::InitDlg::AddMacroId"));
			return;	//	よくわからんけど失敗した
		}else if (result == CB_ERRSPACE) {
			PleaseReportToAuthor(hwndDlg, _T("PropComMacro::InitDlg::AddMacroId/InsufficientSpace"));
			return;	//	よくわからんけど失敗した
		}
	}
	Combo_SetCurSel(hNumCombo, 0);
}

void PropMacro::SetMacro2List_Macro(HWND hwndDlg)
{
	int index;
	LVITEM lvItem;
	
	HWND hListView = ::GetDlgItem(hwndDlg, IDC_MACROLIST);
	HWND hNum = ::GetDlgItem(hwndDlg, IDC_COMBO_MACROID);

	//	設定先取得
	index = Combo_GetCurSel(hNum);
	if (index == CB_ERR) {
		PleaseReportToAuthor(hwndDlg, _T("PropComMacro::SetMacro2List::GetCurSel"));
		return;	//	よくわからんけど失敗した
	}

	// マクロ名
	memset_raw(&lvItem, 0, sizeof(lvItem));
	lvItem.iItem = index;
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	
	TCHAR buf[256];
	::DlgItem_GetText(hwndDlg, IDC_MACRONAME, buf, MACRONAME_MAX);
	lvItem.pszText = buf;
	ListView_SetItem(hListView, &lvItem);

	// ファイル名
	memset_raw(&lvItem, 0, sizeof(lvItem));
	lvItem.iItem = index;
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 2;

	::DlgItem_GetText(hwndDlg, IDC_MACROPATH, buf, _MAX_PATH);
	lvItem.pszText = buf;
	ListView_SetItem(hListView, &lvItem);

	// チェック
	memset_raw(&lvItem, 0, sizeof(lvItem));
	lvItem.iItem = index;
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 3;
	lvItem.pszText = const_cast<TCHAR*>(IsDlgButtonChecked(hwndDlg, IDC_CHECK_RELOADWHENEXECUTE) ? _T("on") : _T("off"));
	ListView_SetItem(hListView, &lvItem);

	// 自動実行マクロ
	int nMacroOnOpened = -1;
	int nMacroOnTypeChanged = -1;
	int nMacroOnSave = -1;
	TCHAR szText[8];
	int iItem;
	for (iItem=0; iItem<MAX_CUSTMACRO; ++iItem) {
		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = iItem;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 4;
		lvItem.pszText = szText;
		lvItem.cchTextMax = _countof(szText);
		ListView_GetItem(hListView, &lvItem);
		int nLen = ::lstrlen(szText);
		for (int i=0; i<nLen; ++i) {
			if (szText[i] == _T('O')) {
				nMacroOnOpened = iItem;
			}
			if (szText[i] == _T('T')) {
				nMacroOnTypeChanged = iItem;
			}
			if (szText[i] == _T('S')) {
				nMacroOnSave = iItem;
			}
		}
	}
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_MacroOnOpened)) {
		nMacroOnOpened = index;
	}else if (nMacroOnOpened == index) {
		nMacroOnOpened = -1;
	}
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_MacroOnTypeChanged)) {
		nMacroOnTypeChanged = index;
	}else if (nMacroOnTypeChanged == index) {
		nMacroOnTypeChanged = -1;
	}
	if (IsDlgButtonChecked(hwndDlg, IDC_CHECK_MacroOnSave)) {
		nMacroOnSave = index;
	}else if (nMacroOnSave == index) {
		nMacroOnSave = -1;
	}
	for (iItem=0; iItem<MAX_CUSTMACRO; ++iItem) {
		szText[0] = _T('\0');
		if (iItem == nMacroOnOpened) {
			::lstrcat(szText, _T("O"));
		}
		if (iItem == nMacroOnTypeChanged) {
			::lstrcat(szText, _T("T"));
		}
		if (iItem == nMacroOnSave) {
			::lstrcat(szText, _T("S"));
		}
		memset_raw(&lvItem, 0, sizeof(lvItem));
		lvItem.iItem = iItem;
		lvItem.mask = LVIF_TEXT;
		lvItem.iSubItem = 4;
		lvItem.pszText = szText;
		ListView_SetItem(hListView, &lvItem);
	}
}

/*!
	Macro格納用ディレクトリを選択する

	@param hwndDlg [in] ダイアログボックスのウィンドウハンドル
*/
void PropMacro::SelectBaseDir_Macro(HWND hwndDlg)
{
	TCHAR szDir[_MAX_PATH];

	// 検索フォルダ
	::DlgItem_GetText(hwndDlg, IDC_MACRODIR, szDir, _countof(szDir));

	if (_IS_REL_PATH(szDir)) {
		TCHAR folder[_MAX_PATH];
		_tcscpy(folder, szDir);
		GetInidirOrExedir(szDir, folder);
	}

	if (SelectDir(hwndDlg, LS(STR_PROPCOMMACR_SEL_DIR), szDir, szDir)) {
		//	末尾に\\マークを追加する．
		AddLastChar(szDir, _countof(szDir), _T('\\'));
		::DlgItem_SetText(hwndDlg, IDC_MACRODIR, szDir);
	}
}


/*!
	マクロファイル指定用コンボボックスのドロップダウンリストが開かれるときに，
	指定ディレクトリのファイル一覧から候補を生成する．

	@param hwndDlg [in] ダイアログボックスのウィンドウハンドル
*/
void PropMacro::OnFileDropdown_Macro(HWND hwndDlg)
{
	HANDLE hFind;
	HWND hCombo = ::GetDlgItem(hwndDlg, IDC_MACROPATH);

	TCHAR path[_MAX_PATH * 2];
	::DlgItem_GetText(hwndDlg, IDC_MACRODIR, path, _countof(path));

	if (_IS_REL_PATH(path)) {
		TCHAR folder[_MAX_PATH * 2];
		_tcscpy(folder, path);
		GetInidirOrExedir(path, folder);
	}
	_tcscat(path, _T("*.*"));

	// 候補の初期化
	Combo_ResetContent(hCombo);

	// ファイルの検索
	WIN32_FIND_DATA wf;
	hFind = FindFirstFile(path, &wf);

	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	do {
		//	コンボボックスに設定
		//	でも.と..は勘弁。
		//if (_tcscmp(wf.cFileName, _T(".")) != 0 && _tcscmp(wf.cFileName, _T("..")) != 0) {
		if ((wf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
			int result = Combo_AddString(hCombo, wf.cFileName);
			if (result == CB_ERR || result == CB_ERRSPACE) {
				break;
			}
		}
	}while (FindNextFile(hFind, &wf));

    FindClose(hFind);
}

void PropMacro::CheckListPosition_Macro(HWND hwndDlg)
{
	HWND hListView = ::GetDlgItem(hwndDlg, IDC_MACROLIST);
	HWND hNum = ::GetDlgItem(hwndDlg, IDC_COMBO_MACROID);
	
	//	現在のFocus取得
	int current = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);

	if (current == -1 || current == nLastPos_Macro) {
		return;
	}

	nLastPos_Macro = current;
	
	//	初期値の設定
	Combo_SetCurSel(hNum, nLastPos_Macro);
	
	TCHAR buf[MAX_PATH + MACRONAME_MAX];	// MAX_PATHとMACRONAME_MAXの両方より大きい値
	LVITEM lvItem = {0};
	lvItem.iItem = current;
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 1;
	lvItem.pszText = buf;
	lvItem.cchTextMax = MACRONAME_MAX;

	ListView_GetItem(hListView, &lvItem);
	::DlgItem_SetText(hwndDlg, IDC_MACRONAME, buf);

	memset_raw(&lvItem, 0, sizeof(lvItem));
	lvItem.iItem = current;
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 2;
	lvItem.pszText = buf;
	lvItem.cchTextMax = MAX_PATH;

	ListView_GetItem(hListView, &lvItem);
	::DlgItem_SetText(hwndDlg, IDC_MACROPATH, buf);

	memset_raw(&lvItem, 0, sizeof(lvItem));
	lvItem.iItem = current;
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 3;
	lvItem.pszText = buf;
	lvItem.cchTextMax = MAX_PATH;
	ListView_GetItem(hListView, &lvItem);
	if (_tcscmp(buf, _T("on")) == 0) {
		::CheckDlgButton(hwndDlg, IDC_CHECK_RELOADWHENEXECUTE, true);
	}else {
		::CheckDlgButton(hwndDlg, IDC_CHECK_RELOADWHENEXECUTE, false);
	}

	// 自動実行マクロ
	memset_raw(&lvItem, 0, sizeof(lvItem));
	lvItem.iItem = current;
	lvItem.mask = LVIF_TEXT;
	lvItem.iSubItem = 4;
	TCHAR szText[8];
	lvItem.pszText = szText;
	lvItem.cchTextMax = _countof(szText);
	ListView_GetItem(hListView, &lvItem);
	::CheckDlgButton(hwndDlg, IDC_CHECK_MacroOnOpened, false);
	::CheckDlgButton(hwndDlg, IDC_CHECK_MacroOnTypeChanged, false);
	::CheckDlgButton(hwndDlg, IDC_CHECK_MacroOnSave, false);
	int nLen = ::lstrlen(szText);
	for (int i=0; i<nLen; ++i) {
		if (szText[i] == _T('O')) {
			::CheckDlgButton(hwndDlg, IDC_CHECK_MacroOnOpened, true);
		}
		if (szText[i] == _T('T')) {
			::CheckDlgButton(hwndDlg, IDC_CHECK_MacroOnTypeChanged, true);
		}
		if (szText[i] == _T('S')) {
			::CheckDlgButton(hwndDlg, IDC_CHECK_MacroOnSave, true);
		}
	}
}

