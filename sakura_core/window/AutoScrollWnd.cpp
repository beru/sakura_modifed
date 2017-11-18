#include "StdAfx.h"
#include "AutoScrollWnd.h"
#include "view/EditView.h"
#include "sakura_rc.h"

AutoScrollWnd::AutoScrollWnd()
	:
	Wnd(_T("::AutoScrollWnd"))
{
	hCenterImg = NULL;
	return;
}


AutoScrollWnd::~AutoScrollWnd()
{
}


HWND AutoScrollWnd::Create(
	HINSTANCE hInstance,
	HWND hwndParent,
	bool bVertical,
	bool bHorizontal,
	const Point& point,
	EditView* view
	)
{
	LPCTSTR pszClassName;

	pView = view;
	int idb, idc;
	if (bVertical) {
		if (bHorizontal) {
			idb = IDB_SCROLL_CENTER;
			idc = IDC_CURSOR_AUTOSCROLL_CENTER;
			pszClassName = _T("SakuraAutoScrollCWnd");
		}else {
			idb = IDB_SCROLL_VERTICAL;
			idc = IDC_CURSOR_AUTOSCROLL_VERTICAL;
			pszClassName = _T("SakuraAutoScrollVWnd");
		}
	}else {
		idb = IDB_SCROLL_HORIZONTAL;
		idc = IDC_CURSOR_AUTOSCROLL_HORIZONTAL;
		pszClassName = _T("SakuraAutoScrollHWnd");
	}
	hCenterImg = (HBITMAP)::LoadImage(hInstance, MAKEINTRESOURCE(idb), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	HCURSOR hCursor = ::LoadCursor(GetModuleHandle(NULL), MAKEINTRESOURCE(idc));

	// ウィンドウクラス作成
	RegisterWC(
		hInstance,
		NULL,
		NULL,
		hCursor,
		(HBRUSH)(COLOR_3DFACE + 1),
		NULL,
		pszClassName
	);

	// 基底クラスメンバ呼び出し
	return Wnd::Create(
		// 初期化
		hwndParent,
		0,
		pszClassName,	// Pointer to a null-terminated string or is an atom.
		pszClassName,	// pointer to window name
		WS_CHILD | WS_VISIBLE, // window style
		point.x - 16,	// horizontal position of window
		point.y - 16,	// vertical position of window
		32, // window width
		32, // window height
		NULL // handle to menu, or child-window identifier
	);
}

void AutoScrollWnd::Close()
{
	this->DestroyWindow();

	if (hCenterImg) {
		::DeleteObject(hCenterImg);
		hCenterImg = NULL;
	}
}

LRESULT AutoScrollWnd::OnLButtonDown(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (pView->nAutoScrollMode) {
		pView->AutoScrollExit();
	}
	return 0;
}

LRESULT AutoScrollWnd::OnRButtonDown(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (pView->nAutoScrollMode) {
		pView->AutoScrollExit();
	}
	return 0;
}

LRESULT AutoScrollWnd::OnMButtonDown(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (pView->nAutoScrollMode) {
		pView->AutoScrollExit();
	}
	return 0;
}


LRESULT AutoScrollWnd::OnPaint(HWND hwnd, UINT, WPARAM, LPARAM)
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(hwnd, &ps);
	HDC hdcBmp = ::CreateCompatibleDC(hdc);
	HBITMAP hBbmpOld = (HBITMAP)::SelectObject(hdcBmp, hCenterImg);
	::BitBlt(hdc, 0, 0, 32, 32, hdcBmp, 0, 0, SRCCOPY);
	::SelectObject(hdcBmp, hBbmpOld);
	::DeleteObject(hdcBmp);
	::EndPaint(hwnd, &ps);
	return 0;
}

