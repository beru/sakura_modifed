/*!	@file
	@brief �t�@���N�V�����L�[�E�B���h�E

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta
	Copyright (C) 2002, YAZAKI, MIK, Moca
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2004, novice
	Copyright (C) 2006, aroka, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "_main/global.h"
#include "func/FuncKeyWnd.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "window/EditWnd.h"
#include "doc/EditDoc.h"
#include "util/input.h"
#include "util/window.h"

#define IDT_FUNCWND 1248
#define TIMER_TIMEOUT 100
#define TIMER_CHECKFUNCENABLE 300

/****
LRESULT CALLBACK CFuncKeyWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	FuncKeyWnd*	pFuncKeyWnd;
	pFuncKeyWnd = (FuncKeyWnd*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (pFuncKeyWnd) {
		return pFuncKeyWnd->DispatchEvent(hwnd, uMsg, wParam, lParam);
	}
	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}
***/


// @date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
FuncKeyWnd::FuncKeyWnd()
	:
	Wnd(_T("::FuncKeyWnd")),
	shareData(GetDllShareData())
{
	pEditDoc = nullptr;
	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	nCurrentKeyState = -1;
	for (size_t i=0; i<_countof(szFuncNameArr); ++i) {
		szFuncNameArr[i][0] = 0;
	}
// 2002.11.04 Moca Open()���Őݒ�
//	nButtonGroupNum = 4;

	for (size_t i=0; i<_countof(hwndButtonArr); ++i) {
		hwndButtonArr[i] = NULL;
	}

	// �\���p�t�H���g
	// LOGFONT�̏�����
	LOGFONT	lf = {0};
	lf.lfHeight			= DpiPointsToPixels(-9);	// 2009.10.01 ryoji ��DPI�Ή��i�|�C���g������Z�o�j
	lf.lfWidth			= 0;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= 400;
	lf.lfItalic			= 0x0;
	lf.lfUnderline		= 0x0;
	lf.lfStrikeOut		= 0x0;
	lf.lfCharSet		= 0x80;
	lf.lfOutPrecision	= 0x3;
	lf.lfClipPrecision	= 0x2;
	lf.lfQuality		= 0x1;
	lf.lfPitchAndFamily	= 0x31;
	_tcscpy(lf.lfFaceName, _T("�l�r �o�S�V�b�N"));
	hFont = ::CreateFontIndirect(&lf);

	bSizeBox = false;
	hwndSizeBox = NULL;
	nTimerCount = 0;

	return;
}


FuncKeyWnd::~FuncKeyWnd()
{
	// �\���p�t�H���g
	::DeleteObject(hFont);
	return;
}

// �E�B���h�E �I�[�v��
HWND FuncKeyWnd::Open(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	EditDoc*	pEditDoc,
	bool		bSizeBox
	)
{
	LPCTSTR pszClassName = _T("FuncKeyWnd");

	this->pEditDoc = pEditDoc;
	this->bSizeBox = bSizeBox;
	hwndSizeBox = NULL;
	nCurrentKeyState = -1;

	// 2002.11.04 Moca �ύX�ł���悤��
	nButtonGroupNum = shareData.common.window.nFuncKeyWnd_GroupNum;
	if (nButtonGroupNum < 1 || 12 < nButtonGroupNum) {
		nButtonGroupNum = 4;
	}

	// �E�B���h�E�N���X�쐬
	RegisterWC(
		hInstance,
		NULL,							// Handle to the class icon.
		NULL,							// Handle to a small icon
		::LoadCursor(NULL, IDC_ARROW),	// Handle to the class cursor.
		(HBRUSH)(COLOR_3DFACE + 1),		// Handle to the class background brush.
		NULL/*MAKEINTRESOURCE(MYDOCUMENT)*/,// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		pszClassName					// Pointer to a null-terminated string or is an atom.
	);

	// ���N���X�����o�Ăяo��
	Wnd::Create(
		hwndParent,
		0,								// extended window style
		pszClassName,					// Pointer to a null-terminated string or is an atom.
		pszClassName,					// pointer to window name
		WS_CHILD/* | WS_VISIBLE*/ | WS_CLIPCHILDREN, // window style	// 2006.06.17 ryoji WS_CLIPCHILDREN �ǉ�	// 2007.03.08 ryoji WS_VISIBLE ����
		CW_USEDEFAULT,					// horizontal position of window
		0,								// vertical position of window
		0,								// window width	// 2007.02.05 ryoji 100->0�i���[�ȃT�C�Y�ň�u�\��������茩���Ȃ��ق��������j
		::GetSystemMetrics(SM_CYMENU),	// window height
		NULL							// handle to menu, or child-window identifier
	);


	hwndSizeBox = NULL;
	if (bSizeBox) {
		hwndSizeBox = ::CreateWindowEx(
			0L, 						// no extended styles
			_T("SCROLLBAR"),			// scroll bar control class
			NULL,						// text for window title bar
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, // scroll bar styles
			0,							// horizontal position
			0,							// vertical position
			200,						// width of the scroll bar
			CW_USEDEFAULT,				// default height
			GetHwnd(), 					// handle of main window
			(HMENU) NULL,				// no menu for a scroll bar
			GetAppInstance(),			// instance owning this window
			(LPVOID) NULL				// pointer not needed
		);
	}

	// �{�^���̐���
	CreateButtons();

	Timer_ONOFF(true); // 20060126 aroka
	OnTimer(GetHwnd(), WM_TIMER, IDT_FUNCWND, ::GetTickCount());	// ����X�V	// 2006.12.20 ryoji

	return GetHwnd();
}


// �E�B���h�E �N���[�Y
void FuncKeyWnd::Close(void)
{
	this->DestroyWindow();
}


//// WM_SIZE����
//void FuncKeyWnd::OnSize(
//	WPARAM	wParam,	// first message parameter
//	LPARAM	lParam 	// second message parameter

// WM_SIZE����
LRESULT FuncKeyWnd::OnSize(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!GetHwnd()) {
		return 0L;
	}

//	RECT rc;

	int nButtonNum = _countof(hwndButtonArr);

	// �{�^���̃T�C�Y���v�Z
	int nButtonWidth = CalcButtonSize();

	RECT rcParent;
	GetWindowRect(&rcParent);
	int nButtonHeight = rcParent.bottom - rcParent.top - 2;

	int nX = 1;
	for (int i=0; i<nButtonNum; ++i) {
		if (0 < i && (i % nButtonGroupNum) == 0) {
			nX += 12;
		}
		::MoveWindow(hwndButtonArr[i], nX, 1, nButtonWidth, nButtonHeight, TRUE);
		nX += nButtonWidth + 1;
	}
	::InvalidateRect(GetHwnd(), NULL, TRUE);	// �ĕ`�悵�ĂˁB	//@@@ 2003.06.11 MIK
	return 0L;
}


#if 0//////////////////////////////////////////////////////////////
LRESULT FuncKeyWnd::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
//	if (!GetHwnd()) {
//		return 0L;
//	}

	switch (uMsg) {
	case WM_TIMER:		return OnTimer(hwnd, uMsg, wParam, lParam);
	case WM_COMMAND:	return OnCommand(hwnd, uMsg, wParam, lParam);
	case WM_SIZE:		return OnSize(hwnd, uMsg, wParam, lParam);
	case WM_DESTROY:	return OnDestroy(hwnd, uMsg, wParam, lParam);
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}
#endif//////////////////////////////////////////////////////////////


LRESULT FuncKeyWnd::OnCommand(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndCtl = (HWND) lParam;		// handle of control
//	switch (wNotifyCode) {
//	case BN_PUSHED:
		for (size_t i=0; i<_countof(hwndButtonArr); ++i) {
			if (hwndCtl == hwndButtonArr[i]) {
				if (nFuncCodeArr[i] != 0) {
					::SendMessage(GetParentHwnd(), WM_COMMAND, MAKELONG(nFuncCodeArr[i], 0),  (LPARAM)hwnd);
				}
				break;
			}
		}
		::SetFocus(GetParentHwnd());
//		break;
//	}
	return 0L;
}


// WM_TIMER�^�C�}�[�̏���
LRESULT FuncKeyWnd::OnTimer(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//	return;
	if (!GetHwnd()) {
		return 0;
	}

	if (::GetActiveWindow() != GetParentHwnd() && nCurrentKeyState != -1) {	// 2002/06/02 MIK	// 2006.12.20 ryoji ����X�V�͏�������
		return 0;
	}

// novice 2004/10/10
	// Shift,Ctrl,Alt�L�[��������Ă�����
	int nIdx = GetCtrlKeyState();
	// ALT,Shift,Ctrl�L�[�̏�Ԃ��ω�������
	if (nIdx != nCurrentKeyState) {
		nTimerCount = TIMER_CHECKFUNCENABLE + 1;

		// �t�@���N�V�����L�[�̋@�\�����擾
		auto& csKeyBind = shareData.common.keyBind;
		for (size_t i=0; i<_countof(szFuncNameArr); ++i) {
			// 2007.02.22 ryoji KeyBind::GetFuncCode()���g��
			EFunctionCode nFuncCode = KeyBind::GetFuncCode(
				(WORD)(((VK_F1 + i) | ((WORD)((BYTE)(nIdx))) << 8)),
				csKeyBind.nKeyNameArrNum,
				csKeyBind.pKeyNameArr
			);
			if (nFuncCode != nFuncCodeArr[i]) {
				nFuncCodeArr[i] = nFuncCode;
				if (nFuncCodeArr[i] == 0) {
					szFuncNameArr[i][0] = 0;
				}else {
					// Oct. 2, 2001 genta
					pEditDoc->funcLookup.Funccode2Name(
						nFuncCodeArr[i],
						szFuncNameArr[i],
						_countof(szFuncNameArr[i]) - 1
					);
				}
				Wnd_SetText(hwndButtonArr[i], szFuncNameArr[i]);
			}
		}
	}
	nTimerCount += TIMER_TIMEOUT;
	if (nTimerCount > TIMER_CHECKFUNCENABLE ||
		nIdx != nCurrentKeyState
	) {
		nTimerCount = 0;
		// �@�\�����p�\�����ׂ�
		for (size_t i=0; i<_countof(szFuncNameArr); ++i) {
			::EnableWindow(
				hwndButtonArr[i],
				IsFuncEnable(*pEditDoc, shareData, nFuncCodeArr[i] ) ? TRUE : FALSE
				);
		}
	}
	nCurrentKeyState = nIdx;
	return 0;
}


// WM_DESTROY����
LRESULT FuncKeyWnd::OnDestroy(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// �^�C�}�[���폜
	Timer_ONOFF(false); // 20060126 aroka
	
	// �{�^�����폜
	for (size_t i=0; i<_countof(hwndButtonArr); ++i) {
		if (hwndButtonArr[i]) {
			::DestroyWindow(hwndButtonArr[i]	);
			hwndButtonArr[i] = NULL;
		}
	}
	
	// �T�C�Y�{�b�N�X���폜
	if (hwndSizeBox) {
		::DestroyWindow(hwndSizeBox);
		hwndSizeBox = NULL;
	}
	
	_SetHwnd(NULL);
	
	return 0L;
}


// �{�^���̃T�C�Y���v�Z
int FuncKeyWnd::CalcButtonSize(void)
{
	RECT rc;
	GetWindowRect(&rc);
	
	int nButtonNum = _countof(hwndButtonArr);
	int nCxVScroll;
	if (!hwndSizeBox) {
//		return (rc.right - rc.left - nButtonNum - ((nButtonNum + nButtonGroupNum - 1) / nButtonGroupNum - 1) * 12) / nButtonNum;
		nCxVScroll = 0;
	}else {
		// �T�C�Y�{�b�N�X�̈ʒu�A�T�C�Y�ύX
		int nCyHScroll = ::GetSystemMetrics(SM_CYHSCROLL);
		nCxVScroll = ::GetSystemMetrics(SM_CXVSCROLL);
		::MoveWindow(hwndSizeBox,  rc.right - rc.left - nCxVScroll, rc.bottom - rc.top - nCyHScroll, nCxVScroll, nCyHScroll, TRUE);
//		::MoveWindow(hwndSizeBox,  0, 0, nCxVScroll, nCyHScroll, TRUE);
//		return (rc.right - rc.left - nCxVScroll = - nButtonNum -  ((nButtonNum + nButtonGroupNum - 1) / nButtonGroupNum - 1) * 12) / nButtonNum;
	}
	return (rc.right - rc.left - nCxVScroll - nButtonNum -  ((nButtonNum + nButtonGroupNum - 1) / nButtonGroupNum - 1) * 12) / nButtonNum;
}


/*! �{�^���̐���
	@date 2007.02.05 ryoji �{�^���̐����ʒu�E���̐ݒ菈�����폜�iOnSize�ōĔz�u�����̂ŕs�v�j
*/
void FuncKeyWnd::CreateButtons(void)
{
	RECT rcParent;
	GetWindowRect(&rcParent);
	int nButtonHeight = rcParent.bottom - rcParent.top - 2;

	for (size_t i=0; i<_countof(nFuncCodeArr); ++i) {
		nFuncCodeArr[i] = F_0;
	}

	for (size_t i=0; i<_countof(hwndButtonArr); ++i) {
		hwndButtonArr[i] = ::CreateWindow(
			_T("BUTTON"),						// predefined class
			_T(""),								// button text
			WS_VISIBLE | WS_CHILD | BS_LEFT,	// styles
			// Size and position values are given explicitly, because
			// the CW_USEDEFAULT constant gives zero values for buttons.
			0,					// starting x position
			0 + 1,				// starting y position
			0,					// button width
			nButtonHeight,		// button height
			GetHwnd(),			// parent window
			NULL,				// No menu
			(HINSTANCE) GetWindowLongPtr(GetHwnd(), GWLP_HINSTANCE),	// Modified by KEITA for WIN64 2003.9.6
			NULL				// pointer not needed
		);
		// �t�H���g�ύX
		::SendMessage(hwndButtonArr[i], WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));
	}
	nCurrentKeyState = -1;
	return;
}


// �T�C�Y�{�b�N�X�̕\���^��\���؂�ւ�
void FuncKeyWnd::SizeBox_ONOFF(bool bSizeBox)
{
	RECT rc;
	GetWindowRect(&rc);
	if (bSizeBox == bSizeBox) {
		return;
	}
	if (bSizeBox) {
		::DestroyWindow(hwndSizeBox);
		hwndSizeBox = NULL;
		bSizeBox = false;
		OnSize(NULL, 0, 0, 0);
	}else {
		hwndSizeBox = ::CreateWindowEx(
			0L, 						// no extended styles
			_T("SCROLLBAR"),			// scroll bar control class
			NULL,						// text for window title bar
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, // scroll bar styles
			0,							// horizontal position
			0,							// vertical position
			200,						// width of the scroll bar
			CW_USEDEFAULT,				// default height
			GetHwnd(), 					// handle of main window
			(HMENU) NULL,				// no menu for a scroll bar
			GetAppInstance(),			// instance owning this window
			(LPVOID) NULL				// pointer not needed
		);
		::ShowWindow(hwndSizeBox, SW_SHOW);
		bSizeBox = true;
		OnSize(NULL, 0, 0, 0);
	}
	return;
}


// �^�C�}�[�̍X�V���J�n�^��~����B 20060126 aroka
// �t�@���N�V�����L�[�\���̓^�C�}�[�ɂ��X�V���Ă��邪�A
// �A�v���̃t�H�[�J�X���O�ꂽ�Ƃ��ɐe�E�B���h�E����ON/OFF��
// �Ăяo���Ă��炤���Ƃɂ��A�]�v�ȕ��ׂ��~�������B
void FuncKeyWnd::Timer_ONOFF(bool bStart)
{
	if (GetHwnd()) {
		if (bStart) {
			// �^�C�}�[���N��
			if (::SetTimer(GetHwnd(), IDT_FUNCWND, TIMER_TIMEOUT, NULL) == 0) {
				WarningMessage(	GetHwnd(), LS(STR_ERR_DLGFUNCKEYWN1));
			}
		}else {
			// �^�C�}�[���폜
			::KillTimer(GetHwnd(), IDT_FUNCWND);
			nCurrentKeyState = -1;
		}
	}
	return;
}


