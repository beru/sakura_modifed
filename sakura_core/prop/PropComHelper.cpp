/*!	@file
	@brief 共通設定ダイアログボックス、「支援」ページ
*/

#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "dlg/DlgOpenFile.h"
#include "util/shell.h"
#include "util/module.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {	//10600
	IDC_BUTTON_OPENHELP1,			HIDC_BUTTON_OPENHELP1,			// 外部ヘルプファイル参照
	IDC_BUTTON_OPENEXTHTMLHELP,		HIDC_BUTTON_OPENEXTHTMLHELP,	// 外部HTMLファイル参照
	IDC_CHECK_m_bHokanKey_RETURN,	HIDC_CHECK_m_bHokanKey_RETURN,	// 候補決定キー（Enter）
	IDC_CHECK_m_bHokanKey_TAB,		HIDC_CHECK_m_bHokanKey_TAB,		// 候補決定キー（Tab）
	IDC_CHECK_m_bHokanKey_RIGHT,	HIDC_CHECK_m_bHokanKey_RIGHT,	// 候補決定キー（→）
	IDC_CHECK_HTMLHELPISSINGLE,		HIDC_CHECK_HTMLHELPISSINGLE,	// ビューアの複数起動
	IDC_EDIT_EXTHELP1,				HIDC_EDIT_EXTHELP1,				// 外部ヘルプファイル名
	IDC_EDIT_EXTHTMLHELP,			HIDC_EDIT_EXTHTMLHELP,			// 外部HTMLヘルプファイル名
	IDC_BUTTON_KEYWORDHELPFONT,		HIDC_BUTTON_KEYWORDHELPFONT,	// キーワードヘルプのフォント
	IDC_EDIT_MIGEMO_DLL,			HIDC_EDIT_MIGEMO_DLL,			// Migemo DLLファイル名
	IDC_BUTTON_OPENMDLL,			HIDC_BUTTON_OPENMDLL,			// Migemo DLLファイル参照
	IDC_EDIT_MIGEMO_DICT,			HIDC_EDIT_MIGEMO_DICT,			// Migemo 辞書ファイル名
	IDC_BUTTON_OPENMDICT,			HIDC_BUTTON_OPENMDICT,			// Migemo 辞書ファイル参照
//	IDC_STATIC,						-1,
	0, 0
};

/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK PropHelper::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropHelper::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}

// Helper メッセージ処理
INT_PTR PropHelper::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
	)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;

	auto& csHelper = common.helper;

	switch (uMsg) {
	case WM_INITDIALOG:
		// ダイアログデータの設定 Helper
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// ユーザーがエディット コントロールに入力できるテキストの長さを制限する
		// 外部ヘルプ１
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_EXTHELP1), _MAX_PATH - 1);
		// 外部HTMLヘルプ
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_EXTHTMLHELP), _MAX_PATH - 1);

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// 通知コード
		wID			= LOWORD(wParam);	// 項目ID､ コントロールID､ またはアクセラレータID
		switch (wNotifyCode) {
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			// ダイアログデータの取得 Helper
			GetData(hwndDlg);
			switch (wID) {
			case IDC_BUTTON_OPENHELP1:	// 外部ヘルプ１の「参照...」ボタン
				{
					DlgOpenFile	dlgOpenFile;
					TCHAR		szPath[_MAX_PATH];
					if (_IS_REL_PATH(csHelper.szExtHelp)) {
						GetInidirOrExedir(szPath, csHelper.szExtHelp, true);
					}else {
						_tcscpy(szPath, csHelper.szExtHelp);
					}
					// ファイルオープンダイアログの初期化
					dlgOpenFile.Create(
						G_AppInstance(),
						hwndDlg,
						_T("*.hlp;*.chm;*.col"),
						szPath
					);
					if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
						_tcscpy(csHelper.szExtHelp, szPath);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_EXTHELP1, csHelper.szExtHelp);
					}
				}
				return TRUE;
			case IDC_BUTTON_OPENEXTHTMLHELP:	// 外部HTMLヘルプの「参照...」ボタン
				{
					DlgOpenFile	dlgOpenFile;
					TCHAR		szPath[_MAX_PATH];
					if (_IS_REL_PATH(csHelper.szExtHtmlHelp)) {
						GetInidirOrExedir(szPath, csHelper.szExtHtmlHelp, true);
					}else {
						_tcscpy(szPath, csHelper.szExtHtmlHelp);
					}
					// ファイルオープンダイアログの初期化
					dlgOpenFile.Create(
						G_AppInstance(),
						hwndDlg,
						_T("*.chm;*.col"),
						szPath
					);
					if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
						_tcscpy(csHelper.szExtHtmlHelp, szPath);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_EXTHTMLHELP, csHelper.szExtHtmlHelp);
					}
				}
				return TRUE;
			case IDC_BUTTON_KEYWORDHELPFONT:	// キーワードヘルプの「フォント」ボタン
				{
					LOGFONT   lf = csHelper.lf;
					INT nPointSize = csHelper.nPointSize;

					if (MySelectFont(&lf, &nPointSize, hwndDlg, false)) {
						csHelper.lf = lf;
						csHelper.nPointSize = nPointSize;
						// キーワードヘルプ フォント表示
						HFONT hFont = SetFontLabel(hwndDlg, IDC_STATIC_KEYWORDHELPFONT, csHelper.lf, csHelper.nPointSize);
						if (hKeywordHelpFont) {
							::DeleteObject(hKeywordHelpFont);
						}
						hKeywordHelpFont = hFont;
					}
				}
				return TRUE;
			case IDC_BUTTON_OPENMDLL:	// MIGEMODLL場所指定「参照...」ボタン
				{
					DlgOpenFile	dlgOpenFile;
					TCHAR		szPath[_MAX_PATH];
					if (_IS_REL_PATH(csHelper.szMigemoDll)) {
						GetInidirOrExedir(szPath, csHelper.szMigemoDll, true);
					}else {
						_tcscpy(szPath, csHelper.szMigemoDll);
					}
					// ファイルオープンダイアログの初期化
					dlgOpenFile.Create(
						G_AppInstance(),
						hwndDlg,
						_T("*.dll"),
						szPath
					);
					if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
						_tcscpy(csHelper.szMigemoDll, szPath);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_MIGEMO_DLL, csHelper.szMigemoDll);
					}
				}
				return TRUE;
			case IDC_BUTTON_OPENMDICT:	// MigemoDict場所指定「参照...」ボタン
				{
					TCHAR	szPath[_MAX_PATH];
					// 検索フォルダ
					if (_IS_REL_PATH(csHelper.szMigemoDict)) {
						GetInidirOrExedir(szPath, csHelper.szMigemoDict, true);
					}else {
						_tcscpy(szPath, csHelper.szMigemoDict);
					}
					if (SelectDir(hwndDlg, LS(STR_PROPCOMHELP_MIGEMODIR), szPath, szPath)) {
						_tcscpy(csHelper.szMigemoDict, szPath);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_MIGEMO_DICT, csHelper.szMigemoDict);
					}
				}
				return TRUE;
			}
			break;	// BN_CLICKED
		}
		break;	// WM_COMMAND
	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
//		switch (idCtrl) {
//		case ???????:
//			return 0L;
//		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_HELPER);
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE(_T("Helper PSN_KILLACTIVE\n"));
				// ダイアログデータの取得 Helper
				GetData(hwndDlg);
				return TRUE;
			case PSN_SETACTIVE:
				nPageNum = ID_PROPCOM_PAGENUM_HELPER;
				return TRUE;
			}
//			break;	// default
//		}

//		MYTRACE(_T("pNMHDR->hwndFrom=%xh\n"), pNMHDR->hwndFrom);
//		MYTRACE(_T("pNMHDR->idFrom  =%xh\n"), pNMHDR->idFrom);
//		MYTRACE(_T("pNMHDR->code    =%xh\n"), pNMHDR->code);
//		MYTRACE(_T("pMNUD->iPos    =%d\n"), pMNUD->iPos);
//		MYTRACE(_T("pMNUD->iDelta  =%d\n"), pMNUD->iDelta);
		break;	// WM_NOTIFY

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
		// キーワードヘルプ フォント破棄
		if (hKeywordHelpFont) {
			::DeleteObject(hKeywordHelpFont);
			hKeywordHelpFont = NULL;
		}
		return TRUE;
	}
	return FALSE;
}

// ダイアログデータの設定 Helper
void PropHelper::SetData(HWND hwndDlg)
{
	auto& csHelper = common.helper;
	
	// 補完候補決定キー
	::CheckDlgButton(hwndDlg, IDC_CHECK_m_bHokanKey_RETURN, csHelper.bHokanKey_RETURN);	// VK_RETURN 補完決定キーが有効/無効
	::CheckDlgButton(hwndDlg, IDC_CHECK_m_bHokanKey_TAB, csHelper.bHokanKey_TAB);		// VK_TAB    補完決定キーが有効/無効
	::CheckDlgButton(hwndDlg, IDC_CHECK_m_bHokanKey_RIGHT, csHelper.bHokanKey_RIGHT);	// VK_RIGHT  補完決定キーが有効/無効

	// 外部ヘルプ１
	::DlgItem_SetText(hwndDlg, IDC_EDIT_EXTHELP1, csHelper.szExtHelp);

	// 外部HTMLヘルプ
	::DlgItem_SetText(hwndDlg, IDC_EDIT_EXTHTMLHELP, csHelper.szExtHtmlHelp);

	// HtmlHelpビューアはひとつ
	::CheckDlgButton(hwndDlg, IDC_CHECK_HTMLHELPISSINGLE, csHelper.bHtmlHelpIsSingle ? BST_CHECKED : BST_UNCHECKED);

	// キーワードヘルプ フォント
	hKeywordHelpFont = SetFontLabel(hwndDlg, IDC_STATIC_KEYWORDHELPFONT, csHelper.lf, csHelper.nPointSize);

	// migemo dict
	::DlgItem_SetText(hwndDlg, IDC_EDIT_MIGEMO_DLL, csHelper.szMigemoDll);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_MIGEMO_DICT, csHelper.szMigemoDict);
}


// ダイアログデータの取得 Helper
int PropHelper::GetData(HWND hwndDlg)
{
	auto& csHelper = common.helper;
	
	// 補完候補決定キー
	csHelper.bHokanKey_RETURN = DlgButton_IsChecked(hwndDlg, IDC_CHECK_m_bHokanKey_RETURN);// VK_RETURN 補完決定キーが有効/無効
	csHelper.bHokanKey_TAB = DlgButton_IsChecked(hwndDlg, IDC_CHECK_m_bHokanKey_TAB);		// VK_TAB    補完決定キーが有効/無効
	csHelper.bHokanKey_RIGHT = DlgButton_IsChecked(hwndDlg, IDC_CHECK_m_bHokanKey_RIGHT);	// VK_RIGHT  補完決定キーが有効/無効

	// 外部ヘルプ１
	::DlgItem_GetText(hwndDlg, IDC_EDIT_EXTHELP1, csHelper.szExtHelp, _countof(csHelper.szExtHelp));

	// 外部HTMLヘルプ
	::DlgItem_GetText(hwndDlg, IDC_EDIT_EXTHTMLHELP, csHelper.szExtHtmlHelp, _countof(csHelper.szExtHtmlHelp));

	// HtmlHelpビューアはひとつ
	csHelper.bHtmlHelpIsSingle = DlgButton_IsChecked(hwndDlg, IDC_CHECK_HTMLHELPISSINGLE);

	// migemo dict
	::DlgItem_GetText(hwndDlg, IDC_EDIT_MIGEMO_DLL, csHelper.szMigemoDll, _countof(csHelper.szMigemoDll));
	::DlgItem_GetText(hwndDlg, IDC_EDIT_MIGEMO_DICT, csHelper.szMigemoDict, _countof(csHelper.szMigemoDict));

	return TRUE;
}

