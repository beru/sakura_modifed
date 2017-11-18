#include "StdAfx.h"
#include "dlg/DlgCancel.h"

// キャンセルボタンダイアログボックス

DlgCancel::DlgCancel()
{
	bCANCEL = false;	// IDCANCELボタンが押された
	bAutoCleanup = false;
}

/** 標準以外のメッセージを捕捉する
	@date 2008.05.28 ryoji 新規作成
*/
INT_PTR DlgCancel::DispatchEvent(
	HWND hWnd,
	UINT wMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	INT_PTR result = Dialog::DispatchEvent(hWnd, wMsg, wParam, lParam);
	switch (wMsg) {
	case WM_CLOSE:
		if (bAutoCleanup) {
			::DestroyWindow(GetHwnd());
			return TRUE;
		}
		break;
	case WM_NCDESTROY:
		if (bAutoCleanup) {
			delete this;
			return TRUE;
		}
		break;
	}
	return result;
}

/** 自動破棄を遅延実行する
	@date 2008.05.28 ryoji 新規作成
*/
void DlgCancel::DeleteAsync(void)
{
	bAutoCleanup = true;
	::PostMessage(GetHwnd(), WM_CLOSE, 0, 0);
}

// モーダルダイアログの表示
INT_PTR DlgCancel::DoModal(
	HINSTANCE hInstance,
	HWND hwndParent,
	int nDlgTemplete
	)
{
	bCANCEL = false;	// IDCANCELボタンが押された
	return Dialog::DoModal(hInstance, hwndParent, nDlgTemplete, (LPARAM)NULL);
}

// モードレスダイアログの表示
HWND DlgCancel::DoModeless(
	HINSTANCE hInstance,
	HWND hwndParent,
	int nDlgTemplete
	)
{
	bCANCEL = false;	// IDCANCELボタンが押された
	return Dialog::DoModeless(hInstance, hwndParent, nDlgTemplete, (LPARAM)NULL, SW_SHOW);
}

BOOL DlgCancel::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwndDlg);
	HICON	hIcon;
	hIcon = ::LoadIcon(NULL, IDI_ASTERISK);
//	hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_GREP));
	::SendMessage(GetHwnd(), WM_SETICON, ICON_SMALL, (LPARAM)NULL);
	::SendMessage(GetHwnd(), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	::SendMessage(GetHwnd(), WM_SETICON, ICON_BIG, (LPARAM)NULL);
	::SendMessage(GetHwnd(), WM_SETICON, ICON_BIG, (LPARAM)hIcon);


	// 基底クラスメンバ
//	CreateSizeBox();
	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}

BOOL DlgCancel::OnBnClicked(int wID)
{
	switch (wID) {
	case IDCANCEL:
		bCANCEL = true;	// IDCANCELボタンが押された
//		CloseDialog(0);
		return TRUE;
	}
	return FALSE;
}

//@@@ 2002.01.18 add start
const DWORD p_helpids[] = {
	0, 0
};

LPVOID DlgCancel::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


