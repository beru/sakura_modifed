/*!	@file
	@brief 共通設定ダイアログボックス、「ステータスバー」ページ
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {
	IDC_CHECK_DISP_UNICODE_IN_SJIS,		HIDC_CHECK_DISP_UNICODE_IN_SJIS,		// SJISで文字コード値をUnicodeで表示する
	IDC_CHECK_DISP_UNICODE_IN_JIS,		HIDC_CHECK_DISP_UNICODE_IN_JIS,			// JISで文字コード値をUnicodeで表示する
	IDC_CHECK_DISP_UNICODE_IN_EUC,		HIDC_CHECK_DISP_UNICODE_IN_EUC,			// EUCで文字コード値をUnicodeで表示する
	IDC_CHECK_DISP_UTF8_CODEPOINT,		HIDC_CHECK_DISP_UTF8_CODEPOINT,			// UTF-8をコードポイントで表示する
	IDC_CHECK_DISP_SP_CODEPOINT,		HIDC_CHECK_DISP_SP_CODEPOINT,			// サロゲートペアをコードポイントで表示する
	IDC_CHECK_DISP_SELCOUNT_BY_BYTE,	HIDC_CHECK_DISP_SELCOUNT_BY_BYTE,		// 選択文字数を文字単位ではなくバイト単位で表示する
	0, 0
};

/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK PropStatusbar::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropStatusbar::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}

// メッセージ処理
INT_PTR PropStatusbar::DispatchEvent(
    HWND	hwndDlg,	// handle to dialog box
    UINT	uMsg,		// message
    WPARAM	wParam,		// first message parameter
    LPARAM	lParam 		// second message parameter
	)
{
	NMHDR* pNMHDR;

	switch (uMsg) {
	case WM_INITDIALOG:
		// ダイアログデータの設定
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		return TRUE;
	case WM_COMMAND:
		break;

	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
		switch (pNMHDR->code) {
		case PSN_HELP:
			OnHelp(hwndDlg, IDD_PROP_STATUSBAR);
			return TRUE;
		case PSN_KILLACTIVE:
			DEBUG_TRACE(_T("statusbar PSN_KILLACTIVE\n"));

			// ダイアログデータの取得
			GetData(hwndDlg);
			return TRUE;

		case PSN_SETACTIVE:
			nPageNum = ID_PROPCOM_PAGENUM_STATUSBAR;
			return TRUE;
		}
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

	}
	return FALSE;
}

// ダイアログデータの設定
void PropStatusbar::SetData(HWND hwndDlg)
{
	auto& csStatusbar = common.statusBar;
	// 示文字コードの指定
	// SJISで文字コード値をUnicodeで出力する
	::CheckDlgButton(hwndDlg, IDC_CHECK_DISP_UNICODE_IN_SJIS, csStatusbar.bDispUniInSjis);
	// JISで文字コード値をUnicodeで出力する
	::CheckDlgButton(hwndDlg, IDC_CHECK_DISP_UNICODE_IN_JIS,  csStatusbar.bDispUniInJis);
	// EUCで文字コード値をUnicodeで出力する
	::CheckDlgButton(hwndDlg, IDC_CHECK_DISP_UNICODE_IN_EUC,  csStatusbar.bDispUniInEuc);
	// UTF-8で表示をバイトコードで行う
	::CheckDlgButton(hwndDlg, IDC_CHECK_DISP_UTF8_CODEPOINT,  csStatusbar.bDispUtf8Codepoint);
	// サロゲートペアをコードポイントで表示
	::CheckDlgButton(hwndDlg, IDC_CHECK_DISP_SP_CODEPOINT,    csStatusbar.bDispSPCodepoint);
	// 選択文字数を文字単位ではなくバイト単位で表示する
	::CheckDlgButton(hwndDlg, IDC_CHECK_DISP_SELCOUNT_BY_BYTE,csStatusbar.bDispSelCountByByte);
	return;
}

// ダイアログデータの取得
int PropStatusbar::GetData(HWND hwndDlg)
{
	auto& csStatusbar = common.statusBar;
	// 表示文字コードの指定
	// SJISで文字コード値をUnicodeで出力する
	csStatusbar.bDispUniInSjis	= DlgButton_IsChecked(hwndDlg, IDC_CHECK_DISP_UNICODE_IN_SJIS);
	// JISで文字コード値をUnicodeで出力する
	csStatusbar.bDispUniInJis		= DlgButton_IsChecked(hwndDlg, IDC_CHECK_DISP_UNICODE_IN_JIS);
	// EUCで文字コード値をUnicodeで出力する
	csStatusbar.bDispUniInEuc		= DlgButton_IsChecked(hwndDlg, IDC_CHECK_DISP_UNICODE_IN_EUC);
	// UTF-8で表示をバイトコードで行う
	csStatusbar.bDispUtf8Codepoint	= DlgButton_IsChecked(hwndDlg, IDC_CHECK_DISP_UTF8_CODEPOINT);
	// サロゲートペアをコードポイントで表示
	csStatusbar.bDispSPCodepoint	= DlgButton_IsChecked(hwndDlg, IDC_CHECK_DISP_SP_CODEPOINT);
	// 選択文字数を文字単位ではなくバイト単位で表示する
	csStatusbar.bDispSelCountByByte	= DlgButton_IsChecked(hwndDlg, IDC_CHECK_DISP_SELCOUNT_BY_BYTE);

	return TRUE;
}

