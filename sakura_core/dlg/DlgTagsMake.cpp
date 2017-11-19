/*!	@file
	@brief タグファイル作成ダイアログボックス
*/

#include "StdAfx.h"
#include "dlg/DlgTagsMake.h"
#include "env/DllSharedData.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {	//13700
	IDC_EDIT_TAG_MAKE_FOLDER,	HIDC_EDIT_TAG_MAKE_FOLDER,		// タグ作成フォルダ
	IDC_BUTTON_TAG_MAKE_REF,	HIDC_BUTTON_TAG_MAKE_REF,		// 参照
	IDC_BUTTON_FOLDER_UP,		HIDC_BUTTON_TAG_MAKE_FOLDER_UP,	// 上
	IDC_EDIT_TAG_MAKE_CMDLINE,	HIDC_EDIT_TAG_MAKE_CMDLINE,		// コマンドライン
	IDC_CHECK_TAG_MAKE_RECURSE,	HIDC_CHECK_TAG_MAKE_RECURSE,	// サブフォルダも対象
	IDOK,						HIDC_TAG_MAKE_IDOK,
	IDCANCEL,					HIDC_TAG_MAKE_IDCANCEL,
	IDC_BUTTON_HELP,			HIDC_BUTTON_TAG_MAKE_HELP,
//	IDC_STATIC,					-1,
	0, 0
};

DlgTagsMake::DlgTagsMake()
{
	szPath[0] = 0;
	szTagsCmdLine[0] = 0;
	nTagsOpt = 0;
	return;
}

// モーダルダイアログの表示
INT_PTR DlgTagsMake::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	const TCHAR*	pszPath		// パス
	)
{
	_tcscpy( szPath, pszPath );
	return Dialog::DoModal(hInstance, hwndParent, IDD_TAG_MAKE, lParam);
}

BOOL DlgTagsMake::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		// ヘルプ
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_TAGS_MAKE));
		return TRUE;

	case IDC_BUTTON_TAG_MAKE_REF:	// 参照
		SelectFolder();
		return TRUE;

	case IDC_BUTTON_FOLDER_UP:
		{
			TCHAR szDir[_MAX_PATH];
			HWND hwnd = GetItemHwnd(IDC_EDIT_TAG_MAKE_FOLDER);
			::GetWindowText(hwnd, szDir, _countof(szDir));
			if (DirectoryUp(szDir)) {
				::SetWindowText(hwnd, szDir);
			}
		}
		return TRUE;

	case IDOK:
		// ダイアログデータの取得
		::EndDialog(GetHwnd(), GetData());
		return TRUE;

	case IDCANCEL:
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;

	}

	// 基底クラスメンバ
	return Dialog::OnBnClicked(wID);
}

/*!
	フォルダを選択する
	
	@param hwndDlg [in] ダイアログボックスのウィンドウハンドル
*/
void DlgTagsMake::SelectFolder()
{
	HWND hwndDlg = GetHwnd();
	TCHAR szPath[_MAX_PATH + 1];

	// フォルダ
	::DlgItem_GetText(hwndDlg, IDC_EDIT_TAG_MAKE_FOLDER, szPath, _MAX_PATH);

	if (SelectDir(hwndDlg, LS(STR_DLGTAGMAK_SELECTDIR), szPath, szPath)) {
		// 末尾に\\マークを追加する．
		size_t pos = _tcslen(szPath);
		if (pos > 0 && szPath[pos - 1] != _T('\\')) {
			szPath[pos    ] = _T('\\');
			szPath[pos + 1] = _T('\0');
		}

		SetItemText(IDC_EDIT_TAG_MAKE_FOLDER, szPath);
	}
}

// ダイアログデータの設定
void DlgTagsMake::SetData(void)
{
	// 作成フォルダ
	Combo_LimitText(GetItemHwnd(IDC_EDIT_TAG_MAKE_FOLDER), _countof(szPath));
	SetItemText(IDC_EDIT_TAG_MAKE_FOLDER, szPath);

	// オプション
	nTagsOpt = pShareData->nTagsOpt;
	if (nTagsOpt & 0x0001) {
		CheckButton(IDC_CHECK_TAG_MAKE_RECURSE, true);
	}

	// コマンドライン
	Combo_LimitText(GetItemHwnd(IDC_EDIT_TAG_MAKE_CMDLINE), _countof(pShareData->szTagsCmdLine));
	_tcscpy(szTagsCmdLine, pShareData->szTagsCmdLine);
	SetItemText(IDC_EDIT_TAG_MAKE_CMDLINE, pShareData->szTagsCmdLine);

	return;
}

// ダイアログデータの取得
// TRUE==正常  FALSE==入力エラー
int DlgTagsMake::GetData(void)
{
	// フォルダ
	GetItemText(IDC_EDIT_TAG_MAKE_FOLDER, szPath, _countof(szPath));
	size_t length = _tcslen(szPath);
	if (length > 0) {
		if (szPath[length - 1] != _T('\\')) _tcscat(szPath, _T("\\"));
	}

	// CTAGSオプション
	nTagsOpt = 0;
	if (IsButtonChecked(IDC_CHECK_TAG_MAKE_RECURSE)) {
		nTagsOpt |= 0x0001;
	}
	pShareData->nTagsOpt = nTagsOpt;

	// コマンドライン
	GetItemText(IDC_EDIT_TAG_MAKE_CMDLINE, szTagsCmdLine, _countof(szTagsCmdLine));
	_tcscpy(pShareData->szTagsCmdLine, szTagsCmdLine);

	return TRUE;
}

LPVOID DlgTagsMake::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


