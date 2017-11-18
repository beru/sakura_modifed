#pragma once

/*
	MFCのCWnd的なクラス。
*/

class MyWnd {
public:
	MyWnd() : hWnd(NULL) { }

	void SetHwnd(HWND hwnd) { this->hWnd = hwnd; }
	HWND GetHwnd() const { return hWnd; }
	HWND GetSafeHwnd() const { return this ? hWnd : NULL; }
	void InvalidateRect(LPCRECT lpRect, BOOL bErase = TRUE) { ::InvalidateRect(hWnd, lpRect, bErase); }
	int ScrollWindowEx(
		int dx,
		int dy,
		const RECT* prcScroll,
		const RECT* prcClip,
		HRGN hrgnUpdate,
		RECT* prcUpdate,
		UINT uFlags
		)
	{
		return ::ScrollWindowEx(hWnd, dx, dy, prcScroll, prcClip, hrgnUpdate, prcUpdate, uFlags);
	}
	HDC GetDC() const {
		return ::GetDC(hWnd);
	}
	int ReleaseDC(HDC hdc) const {
		return ::ReleaseDC(hWnd, hdc);
	}
	HWND GetAncestor(UINT gaFlags) const {
		return ::GetAncestor(hWnd, gaFlags);
	}
	BOOL CreateCaret(HBITMAP hBitmap, int nWidth, int nHeight) {
		return ::CreateCaret(hWnd, hBitmap, nWidth, nHeight);
	}
	BOOL ClientToScreen(LPPOINT lpPoint) const {
		return ::ClientToScreen(hWnd, lpPoint);
	}

	BOOL UpdateWindow() {
		return ::UpdateWindow(hWnd);
	}
	HWND SetFocus() {
		return ::SetFocus(hWnd);
	}
	BOOL GetClientRect(LPRECT lpRect) const {
		return ::GetClientRect(hWnd, lpRect);
	}

private:
	HWND hWnd;
};

