/*!	@file
	@brief 1行入力ダイアログボックス
*/
#include "StdAfx.h"
#include "dlg/DlgInput1.h"
#include "EditApp.h"
#include "Funccode_enum.h"	// EFunctionCode
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids[] = {	//13000
	IDOK,				HIDOK_DLG1,
	IDCANCEL,			HIDCANCEL_DLG1,
	IDC_EDIT_INPUT1,	HIDC_DLG1_EDIT1,	// 入力フィールド	
	IDC_STATIC_MSG,		HIDC_DLG1_EDIT1,	// メッセージ
//	IDC_STATIC,			-1,
	0, 0
};


// ダイアログプロシージャ
INT_PTR CALLBACK CDlgInput1Proc(
	HWND hwndDlg,	// handle to dialog box
	UINT uMsg,		// message
	WPARAM wParam,	// first message parameter
	LPARAM lParam 	// second message parameter
	)
{
	DlgInput1* pDlgInput1;
	switch (uMsg) {
	case WM_INITDIALOG:
		pDlgInput1 = (DlgInput1*)lParam;
		if (pDlgInput1) {
			return pDlgInput1->DispatchEvent(hwndDlg, uMsg, wParam, lParam);
		}else {
			return FALSE;
		}
	default:
		pDlgInput1 = (DlgInput1*)::GetWindowLongPtr(hwndDlg, DWLP_USER);
		if (pDlgInput1) {
			return pDlgInput1->DispatchEvent(hwndDlg, uMsg, wParam, lParam);
		}else {
			return FALSE;
		}
	}
}


DlgInput1::DlgInput1()
{
	return;
}


DlgInput1::~DlgInput1()
{
	return;
}


// モードレスダイアログの表示
BOOL DlgInput1::DoModal(
	HINSTANCE		hInstApp,
	HWND			hwndParent,
	const TCHAR*	pszTitle,
	const TCHAR*	pszMessage,
	size_t			nMaxTextLen,
	TCHAR*			pszText
	)
{
	BOOL bRet;
	hInstance = hInstApp;			// アプリケーションインスタンスのハンドル
	this->hwndParent = hwndParent;		// オーナーウィンドウのハンドル
	this->pszTitle = pszTitle;			// ダイアログタイトル
	this->pszMessage = pszMessage;		// メッセージ
	this->nMaxTextLen = nMaxTextLen;	// 入力サイズ上限
//	pszText = pszText;			// テキスト
	memText.SetString(pszText);
	bRet = (BOOL)::DialogBoxParam(
		hInstance,
		MAKEINTRESOURCE(IDD_INPUT1),
		hwndParent,
		CDlgInput1Proc,
		(LPARAM)this
	);
	_tcscpy(pszText, memText.GetStringPtr());
	return bRet;
}

BOOL DlgInput1::DoModal(
	HINSTANCE		hInstApp,
	HWND			hwndParent,
	const TCHAR*	pszTitle,
	const TCHAR*	pszMessage,
	size_t			nMaxTextLen,
	NOT_TCHAR*		pszText
	)
{
	TCHAR buf[1024];
	buf[0] = _T('\0');
	BOOL ret = DoModal(hInstApp, hwndParent, pszTitle, pszMessage, nMaxTextLen, buf);
	if (ret) {
		auto_strcpy(pszText, to_not_tchar(buf));
	}
	return ret;
}


// ダイアログのメッセージ処理
INT_PTR DlgInput1::DispatchEvent(
	HWND hwndDlg,	// handle to dialog box
	UINT uMsg,		// message
	WPARAM wParam,	// first message parameter
	LPARAM lParam 	// second message parameter
	)
{
	WORD	wNotifyCode;
	WORD	wID;
//	int		nRet;
	switch (uMsg) {
	case WM_INITDIALOG:
		// ダイアログデータの設定
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		::SetWindowText(hwndDlg, pszTitle);	// ダイアログタイトル
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_INPUT1), nMaxTextLen);	// 入力サイズ上限
		::SetDlgItemText(hwndDlg, IDC_EDIT_INPUT1, memText.GetStringPtr());		// テキスト
		::SetDlgItemText(hwndDlg, IDC_STATIC_MSG, pszMessage);		// メッセージ
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// 通知コード
		wID = LOWORD(wParam);	// 項目ID､ コントロールID､ またはアクセラレータID
		switch (wNotifyCode) {
		// ボタン／チェックボックスがクリックされた
		case BN_CLICKED:
			switch (wID) {
			case IDOK:
				memText.AllocStringBuffer(::GetWindowTextLength(::GetDlgItem(hwndDlg, IDC_EDIT_INPUT1)));
				::GetWindowText(::GetDlgItem(hwndDlg, IDC_EDIT_INPUT1), memText.GetStringPtr(), nMaxTextLen + 1);	// テキスト
				::EndDialog(hwndDlg, TRUE);
				return TRUE;
			case IDCANCEL:
				::EndDialog(hwndDlg, FALSE);
				return TRUE;
			}
			break;
		}
		break;
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO *)lParam;
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

