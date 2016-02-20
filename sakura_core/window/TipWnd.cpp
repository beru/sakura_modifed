/*!	@file
	@brief ツールチップ

	@author Norio Nakatani
	@date 1998/10/30 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, asa-o
	Copyright (C) 2002, GAE
	Copyright (C) 2005, D.S.Koba
	Copyright (C) 2006, ryoji, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "TipWnd.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"


// TipWndクラス デストラクタ
TipWnd::TipWnd()
	:
	Wnd(_T("::TipWnd"))
{
	m_hFont = NULL;
	m_KeyWasHit = FALSE;	// キーがヒットしたか
	return;
}


// TipWndクラス デストラクタ
TipWnd::~TipWnd()
{
	if (m_hFont) {
		::DeleteObject(m_hFont);
		m_hFont = NULL;
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
		NULL,// Handle to the class icon.
		NULL,	// Handle to a small icon
		::LoadCursor(NULL, IDC_ARROW),// Handle to the class cursor.
		(HBRUSH)/*NULL*/(COLOR_INFOBK + 1),// Handle to the class background brush.
		NULL/*MAKEINTRESOURCE(MYDOCUMENT)*/,// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		pszClassName// Pointer to a null-terminated string or is an atom.
	);

	// 基底クラスメンバ呼び出し
	// 2006.01.09 ryoji 初期状態を不可視にする
	// 実質的には見えないTipWndが最前面にあると判断されてしまう場合があるため
	Wnd::Create(
		hwndParent,
		WS_EX_TOOLWINDOW, // extended window style	// 2002/2/3 GAE
		pszClassName,	// Pointer to a null-terminated string or is an atom.
		pszClassName, // pointer to window name
		WS_POPUP | WS_CLIPCHILDREN | WS_BORDER, // window style
		CW_USEDEFAULT, // horizontal position of window
		0, // vertical position of window
		CW_USEDEFAULT, // window width
		0, // window height
		NULL // handle to menu, or child-window identifier
	);

	if (m_hFont) {
		::DeleteObject(m_hFont);
		m_hFont = NULL;
	}

	m_hFont = ::CreateFontIndirect(&(GetDllShareData().m_common.helper.lf));
	return;
}

/*!	CreateWindowの後

	Wnd::AfterCreateWindowでウィンドウを表示するようになっているのを
	動かなくするための空関数

	@date 2006.01.09 genta 新規作成
*/
void TipWnd::AfterCreateWindow(void)
{
}

// Tipを表示
void TipWnd::Show(int nX, int nY, const TCHAR* szText, RECT* pRect)
{
	if (szText) {
		m_info.SetString(szText);
	}
	const TCHAR* pszInfo = m_info.GetStringPtr();
	HDC hdc = ::GetDC(GetHwnd());

	// サイズを計算済み	2001/06/19 asa-o
	RECT rc;
	if (pRect) {
		rc = *pRect;
	}else {
		// ウィンドウのサイズを決める
		ComputeWindowSize(hdc, m_hFont, pszInfo, &rc);
	}

	::ReleaseDC(GetHwnd(), hdc);

	if (m_bAlignLeft) {
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
	int nTextLength = _tcslen(pszText);
	int nBgn = 0;
	for (int i=0; i<=nTextLength; ++i) {
		// 2005-09-02 D.S.Koba GetSizeOfChar
		int nCharChars = NativeT::GetSizeOfChar(pszText, nTextLength, i);
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
	int nTextLength = _tcslen(pszText);
	int nBgn = 0;
	for (int i=0; i<=nTextLength; ++i) {
//		int nCharChars = &pszText[i] - CMemory::MemCharPrev(pszText, nTextLength, &pszText[i]);
		// 2005-09-02 D.S.Koba GetSizeOfChar
		int nCharChars = NativeT::GetSizeOfChar(pszText, nTextLength, i);
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
	DrawTipText(hdc, m_hFont, m_info.GetStringPtr());

	::EndPaint(	hwnd, &ps);
	return 0L;
}


// 2001/06/19 Start by asa-o: ウィンドウのサイズを得る
void TipWnd::GetWindowSize(LPRECT pRect)
{
	HDC hdc = ::GetDC(GetHwnd());
	const TCHAR* pszText = m_info.GetStringPtr();
	// ウィンドウのサイズを得る
	ComputeWindowSize(hdc, m_hFont, pszText , pRect);
	ReleaseDC(GetHwnd(), hdc); // 2007.10.10 kobake ReleaseDCが抜けていたのを修正
}

// 2001/06/19 End

