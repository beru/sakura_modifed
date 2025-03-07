#include "StdAfx.h"
#include "TipWnd.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"


// TipWndクラス デストラクタ
TipWnd::TipWnd()
	:
	Wnd(_T("::TipWnd"))
{
	hFont = NULL;
	KeyWasHit = false;	// キーがヒットしたか
	return;
}


// TipWndクラス デストラクタ
TipWnd::~TipWnd()
{
	if (hFont) {
		::DeleteObject(hFont);
		hFont = NULL;
	}
	return;
}


// 初期化
void TipWnd::Create(HINSTANCE hInstance, HWND hwndParent)
{
	LPCTSTR pszClassName = _T("TipWnd");
	
	// ウィンドウクラス作成
	RegisterWC(
		hInstance,
		// WNDCLASS用
		NULL,	// Handle to the class icon.
		NULL,	// Handle to a small icon
		::LoadCursor(NULL, IDC_ARROW),// Handle to the class cursor.
		(HBRUSH)/*NULL*/(COLOR_INFOBK + 1),// Handle to the class background brush.
		NULL/*MAKEINTRESOURCE(MYDOCUMENT)*/,// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		pszClassName// Pointer to a null-terminated string or is an atom.
	);

	// 基底クラスメンバ呼び出し
	Wnd::Create(
		hwndParent,
		WS_EX_TOOLWINDOW, // extended window style
		pszClassName,	// Pointer to a null-terminated string or is an atom.
		pszClassName, // pointer to window name
		WS_POPUP | WS_CLIPCHILDREN | WS_BORDER, // window style
		CW_USEDEFAULT, // horizontal position of window
		0, // vertical position of window
		CW_USEDEFAULT, // window width
		0, // window height
		NULL // handle to menu, or child-window identifier
	);

	if (hFont) {
		::DeleteObject(hFont);
		hFont = NULL;
	}

	hFont = ::CreateFontIndirect(&(GetDllShareData().common.helper.lf));
	return;
}

/*!	CreateWindowの後

	Wnd::AfterCreateWindowでウィンドウを表示するようになっているのを
	動かなくするための空関数
*/
void TipWnd::AfterCreateWindow(void)
{
}

// Tipを表示
void TipWnd::Show(int nX, int nY, const TCHAR* szText, RECT* pRect)
{
	if (szText) {
		info.SetString(szText);
	}
	const TCHAR* pszInfo = info.GetStringPtr();
	HDC hdc = ::GetDC(GetHwnd());

	// サイズを計算済み
	RECT rc;
	if (pRect) {
		rc = *pRect;
	}else {
		// ウィンドウのサイズを決める
		ComputeWindowSize(hdc, hFont, pszInfo, &rc);
	}

	::ReleaseDC(GetHwnd(), hdc);

	if (bAlignLeft) {
		// 右側固定で表示(MiniMap)
		::MoveWindow(GetHwnd(), nX - rc.right, nY, rc.right + 8, rc.bottom + 8, TRUE);
	}else {
		// 左側固定で表示(通常)
		::MoveWindow(GetHwnd(), nX, nY, rc.right + 8, rc.bottom + 8/*nHeight*/, TRUE);
	}
	::InvalidateRect(GetHwnd(), NULL, TRUE);
	::ShowWindow(GetHwnd(), SW_SHOWNA);
	return;

}

// ウィンドウのサイズを決める
void TipWnd::ComputeWindowSize(
	HDC				hdc,
	HFONT			hFont,
	const TCHAR*	pszText,
	RECT*			pRect
	)
{
	RECT rc;
	HFONT hFontOld = (HFONT)::SelectObject(hdc, hFont);
	int nCurMaxWidth = 0;
	int nCurHeight = 0;
	size_t nTextLength = _tcslen(pszText);
	int nBgn = 0;
	for (size_t i=0; i<=nTextLength; ++i) {
		size_t nCharChars = NativeT::GetSizeOfChar(pszText, nTextLength, i);
		if ((nCharChars == 1 && _T('\\') == pszText[i] && _T('n') == pszText[i + 1]) || _T('\0') == pszText[i]) {
			if (0 < i - nBgn) {
				std::vector<TCHAR> szWork(i - nBgn + 1);
				TCHAR* pszWork = &szWork[0];
				auto_memcpy(pszWork, &pszText[nBgn], i - nBgn);
				pszWork[i - nBgn] = _T('\0');

				rc.left = 0;
				rc.top = 0;
				rc.right = ::GetSystemMetrics(SM_CXSCREEN);
				rc.bottom = 0;
				::DrawText(hdc, pszWork, _tcslen(pszWork), &rc,
					DT_CALCRECT | DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & (4 << 8))*/
				);
				if (nCurMaxWidth < rc.right) {
					nCurMaxWidth = rc.right;
				}
			}else {
				::DrawText(hdc, _T(" "), 1, &rc,
					DT_CALCRECT | DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & (4 << 8))*/
				);
			}
			nCurHeight += rc.bottom;

			nBgn = i + 2;
		}
		if (nCharChars == 2) {
			++i;
		}
	}

	pRect->left = 0;
	pRect->top = 0;
	pRect->right = nCurMaxWidth + 4;
	pRect->bottom = nCurHeight + 2;

	::SelectObject(hdc, hFontOld);

	return;
}


// ウィンドウのテキストを表示
void TipWnd::DrawTipText(
	HDC				hdc,
	HFONT			hFont,
	const TCHAR*	pszText
	)
{
	RECT rc;

	int nBkMode_Old = ::SetBkMode(hdc, TRANSPARENT);
	HFONT hFontOld = (HFONT)::SelectObject(hdc, hFont);
	COLORREF colText_Old = ::SetTextColor(hdc, ::GetSysColor(COLOR_INFOTEXT));
	int nCurMaxWidth = 0;
	int nCurHeight = 0;
	size_t nTextLength = _tcslen(pszText);
	int nBgn = 0;
	for (size_t i=0; i<=nTextLength; ++i) {
		size_t nCharChars = NativeT::GetSizeOfChar(pszText, nTextLength, i);
		if ((nCharChars == 1 && _T('\\') == pszText[i] && _T('n') == pszText[i + 1]) || _T('\0') == pszText[i]) {
			if (0 < i - nBgn) {
				std::vector<TCHAR> szWork(i - nBgn + 1);
				TCHAR* pszWork = &szWork[0];
				auto_memcpy(pszWork, &pszText[nBgn], i - nBgn);
				pszWork[i - nBgn] = _T('\0');

				rc.left = 4;
				rc.top = 4 + nCurHeight;
				rc.right = ::GetSystemMetrics(SM_CXSCREEN);
				rc.bottom = rc.top + 200;
				nCurHeight += ::DrawText(hdc, pszWork, _tcslen(pszWork), &rc,
					DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & (4 << 8))*/
				);
				if (nCurMaxWidth < rc.right) {
					nCurMaxWidth = rc.right;
				}
			}else {
				rc.left = 4;
				rc.top = 4 + nCurHeight;
				rc.right = ::GetSystemMetrics(SM_CXSCREEN);
				rc.bottom = rc.top + 200;
				nCurHeight += ::DrawText(hdc, _T(" "), 1, &rc,
					DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & (4 << 8))*/
				);
			}

			nBgn = i + 2;
		}
		if (nCharChars == 2) {
			++i;
		}
	}
	
	::SetTextColor(hdc, colText_Old);
	::SelectObject(hdc, hFontOld);
	::SetBkMode(hdc, nBkMode_Old);
	return;
}


// Tipを消す
void TipWnd::Hide(void)
{
	::ShowWindow(GetHwnd(), SW_HIDE);
//	::DestroyWindow(GetHwnd());
	return;
}


// 描画処理
LRESULT TipWnd::OnPaint(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM l_Param)
{
	PAINTSTRUCT	ps;
	RECT	rc;
	HDC		hdc = ::BeginPaint(	hwnd, &ps);
	::GetClientRect(hwnd, &rc);

	// ウィンドウのテキストを表示
	DrawTipText(hdc, hFont, info.GetStringPtr());

	::EndPaint(	hwnd, &ps);
	return 0L;
}


// ウィンドウのサイズを得る
void TipWnd::GetWindowSize(LPRECT pRect)
{
	HDC hdc = ::GetDC(GetHwnd());
	const TCHAR* pszText = info.GetStringPtr();
	// ウィンドウのサイズを得る
	ComputeWindowSize(hdc, hFont, pszText , pRect);
	ReleaseDC(GetHwnd(), hdc);
}


