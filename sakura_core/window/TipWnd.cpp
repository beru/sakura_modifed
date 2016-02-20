/*!	@file
	@brief �c�[���`�b�v

	@author Norio Nakatani
	@date 1998/10/30 �V�K�쐬
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


// TipWnd�N���X �f�X�g���N�^
TipWnd::TipWnd()
	:
	Wnd(_T("::TipWnd"))
{
	m_hFont = NULL;
	m_KeyWasHit = FALSE;	// �L�[���q�b�g������
	return;
}


// TipWnd�N���X �f�X�g���N�^
TipWnd::~TipWnd()
{
	if (m_hFont) {
		::DeleteObject(m_hFont);
		m_hFont = NULL;
	}
	return;
}


// ������
void TipWnd::Create(HINSTANCE hInstance, HWND hwndParent)
{
	LPCTSTR pszClassName = _T("TipWnd");
	
	// �E�B���h�E�N���X�쐬
	RegisterWC(
		hInstance,
		// WNDCLASS�p
		NULL,// Handle to the class icon.
		NULL,	// Handle to a small icon
		::LoadCursor(NULL, IDC_ARROW),// Handle to the class cursor.
		(HBRUSH)/*NULL*/(COLOR_INFOBK + 1),// Handle to the class background brush.
		NULL/*MAKEINTRESOURCE(MYDOCUMENT)*/,// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		pszClassName// Pointer to a null-terminated string or is an atom.
	);

	// ���N���X�����o�Ăяo��
	// 2006.01.09 ryoji ������Ԃ�s���ɂ���
	// �����I�ɂ͌����Ȃ�TipWnd���őO�ʂɂ���Ɣ��f����Ă��܂��ꍇ�����邽��
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

/*!	CreateWindow�̌�

	Wnd::AfterCreateWindow�ŃE�B���h�E��\������悤�ɂȂ��Ă���̂�
	�����Ȃ����邽�߂̋�֐�

	@date 2006.01.09 genta �V�K�쐬
*/
void TipWnd::AfterCreateWindow(void)
{
}

// Tip��\��
void TipWnd::Show(int nX, int nY, const TCHAR* szText, RECT* pRect)
{
	if (szText) {
		m_info.SetString(szText);
	}
	const TCHAR* pszInfo = m_info.GetStringPtr();
	HDC hdc = ::GetDC(GetHwnd());

	// �T�C�Y���v�Z�ς�	2001/06/19 asa-o
	RECT rc;
	if (pRect) {
		rc = *pRect;
	}else {
		// �E�B���h�E�̃T�C�Y�����߂�
		ComputeWindowSize(hdc, m_hFont, pszInfo, &rc);
	}

	::ReleaseDC(GetHwnd(), hdc);

	if (m_bAlignLeft) {
		// �E���Œ�ŕ\��(MiniMap)
		::MoveWindow(GetHwnd(), nX - rc.right, nY, rc.right + 8, rc.bottom + 8, TRUE);
	}else {
		// �����Œ�ŕ\��(�ʏ�)
		::MoveWindow(GetHwnd(), nX, nY, rc.right + 8, rc.bottom + 8/*nHeight*/, TRUE);
	}
	::InvalidateRect(GetHwnd(), NULL, TRUE);
	::ShowWindow(GetHwnd(), SW_SHOWNA);
	return;

}

// �E�B���h�E�̃T�C�Y�����߂�
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


// �E�B���h�E�̃e�L�X�g��\��
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


// Tip������
void TipWnd::Hide(void)
{
	::ShowWindow(GetHwnd(), SW_HIDE);
//	::DestroyWindow(GetHwnd());
	return;
}


// �`�揈��
LRESULT TipWnd::OnPaint(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM l_Param)
{
	PAINTSTRUCT	ps;
	RECT	rc;
	HDC		hdc = ::BeginPaint(	hwnd, &ps);
	::GetClientRect(hwnd, &rc);

	// �E�B���h�E�̃e�L�X�g��\��
	DrawTipText(hdc, m_hFont, m_info.GetStringPtr());

	::EndPaint(	hwnd, &ps);
	return 0L;
}


// 2001/06/19 Start by asa-o: �E�B���h�E�̃T�C�Y�𓾂�
void TipWnd::GetWindowSize(LPRECT pRect)
{
	HDC hdc = ::GetDC(GetHwnd());
	const TCHAR* pszText = m_info.GetStringPtr();
	// �E�B���h�E�̃T�C�Y�𓾂�
	ComputeWindowSize(hdc, m_hFont, pszText , pRect);
	ReleaseDC(GetHwnd(), hdc); // 2007.10.10 kobake ReleaseDC�������Ă����̂��C��
}

// 2001/06/19 End

