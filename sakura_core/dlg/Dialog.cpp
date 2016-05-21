/*!	@file
	@brief Dialog Box�̊��N���X

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, jepro, Stonee
	Copyright (C) 2002, aroka, YAZAKI
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2005, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2011, nasukoji
	Copyright (C) 2012, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/Dialog.h"
#include "EditApp.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "DlgOpenFile.h"
#include "recent/Recent.h"
#include "util/os.h"
#include "util/shell.h"
#include "util/module.h"

// �_�C�A���O�v���V�[�W��
INT_PTR CALLBACK MyDialogProc(
	HWND hwndDlg,	// handle to dialog box
	UINT uMsg,		// message
	WPARAM wParam,	// first message parameter
	LPARAM lParam 	// second message parameter
	)
{
	Dialog* pDialog;
	switch (uMsg) {
	case WM_INITDIALOG:
		pDialog = (Dialog*) lParam;
		if (pDialog) {
			return pDialog->DispatchEvent(hwndDlg, uMsg, wParam, lParam);
		}else {
			return FALSE;
		}
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pDialog = (Dialog*)::GetWindowLongPtr(hwndDlg, DWLP_USER);
		if (pDialog) {
			return pDialog->DispatchEvent(hwndDlg, uMsg, wParam, lParam);
		}else {
			return FALSE;
		}
	}
}


/*!	�R���X�g���N�^

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
Dialog::Dialog(bool bSizable, bool bCheckShareData)
{
//	MYTRACE(_T("Dialog::Dialog()\n"));
	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	pShareData = &GetDllShareData(bCheckShareData);

	hInstance = NULL;		// �A�v���P�[�V�����C���X�^���X�̃n���h��
	hwndParent = NULL;	// �I�[�i�[�E�B���h�E�̃n���h��
	hWnd  = NULL;			// ���̃_�C�A���O�̃n���h��
	hwndSizeBox = NULL;
	bSizable = bSizable;
	lParam = (LPARAM)NULL;
	nShowCmd = SW_SHOW;
	xPos = -1;
	yPos = -1;
	nWidth = -1;
	nHeight = -1;

	return;
}

Dialog::~Dialog()
{
//	MYTRACE(_T("Dialog::~Dialog()\n"));
	CloseDialog(0);
	return;
}

// ���[�_���_�C�A���O�̕\��
/*!
	@param hInstance [in] �A�v���P�[�V�����C���X�^���X�̃n���h��
	@param hwndParent [in] �I�[�i�[�E�B���h�E�̃n���h��

	@date 2011.04.10 nasukoji	�e���ꃁ�b�Z�[�W���\�[�X�Ή�
*/
INT_PTR Dialog::DoModal(
	HINSTANCE hInstance,
	HWND hwndParent,
	int nDlgTemplete,
	LPARAM lParam
	)
{
	bInited = false;
	bModal = true;
	this->hInstance = hInstance;	// �A�v���P�[�V�����C���X�^���X�̃n���h��
	this->hwndParent = hwndParent;	// �I�[�i�[�E�B���h�E�̃n���h��
	this->lParam = lParam;
	hLangRsrcInstance = SelectLang::getLangRsrcInstance();		// ���b�Z�[�W���\�[�XDLL�̃C���X�^���X�n���h��
	return ::DialogBoxParam(
		this->hLangRsrcInstance,
		MAKEINTRESOURCE(nDlgTemplete),
		hwndParent,
		MyDialogProc,
		(LPARAM)this
	);
}

// ���[�h���X�_�C�A���O�̕\��
/*!
	@param hInstance [in] �A�v���P�[�V�����C���X�^���X�̃n���h��
	@param hwndParent [in] �I�[�i�[�E�B���h�E�̃n���h��

	@date 2011.04.10 nasukoji	�e���ꃁ�b�Z�[�W���\�[�X�Ή�
*/
HWND Dialog::DoModeless(
	HINSTANCE hInstance,
	HWND hwndParent,
	int nDlgTemplete,
	LPARAM lParam,
	int nCmdShow
	)
{
	bInited = false;
	bModal = false;
	this->hInstance = hInstance;	// �A�v���P�[�V�����C���X�^���X�̃n���h��
	this->hwndParent = hwndParent;	// �I�[�i�[�E�B���h�E�̃n���h��
	this->lParam = lParam;
	hLangRsrcInstance = SelectLang::getLangRsrcInstance();		// ���b�Z�[�W���\�[�XDLL�̃C���X�^���X�n���h��
	hWnd = ::CreateDialogParam(
		hLangRsrcInstance,
		MAKEINTRESOURCE(nDlgTemplete),
		hwndParent,
		MyDialogProc,
		(LPARAM)this
	);
	if (hWnd) {
		::ShowWindow(hWnd, nCmdShow);
	}
	return hWnd;
}

HWND Dialog::DoModeless(
	HINSTANCE hInstance,
	HWND hwndParent,
	LPCDLGTEMPLATE lpTemplate,
	LPARAM lParam,
	int nCmdShow
	)
{
	bInited = false;
	bModal = false;
	this->hInstance = hInstance;	// �A�v���P�[�V�����C���X�^���X�̃n���h��
	this->hwndParent = hwndParent;	// �I�[�i�[�E�B���h�E�̃n���h��
	this->lParam = lParam;
	hWnd = ::CreateDialogIndirectParam(
		hInstance,
		lpTemplate,
		hwndParent,
		MyDialogProc,
		(LPARAM)this
	);
	if (hWnd) {
		::ShowWindow(hWnd, nCmdShow);
	}
	return hWnd;
}

void Dialog::CloseDialog(int nModalRetVal)
{
	if (hWnd) {
		if (bModal) {
			::EndDialog(hWnd, nModalRetVal);
		}else {
			::DestroyWindow(hWnd);
		}
		hWnd = NULL;
	}
	return;
}


BOOL Dialog::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	hWnd = hwndDlg;
	// Modified by KEITA for WIN64 2003.9.6
	::SetWindowLongPtr(hWnd, DWLP_USER, lParam);

	// �_�C�A���O�f�[�^�̐ݒ�
	SetData();

	SetDialogPosSize();

	bInited = true;
	return TRUE;
}

void Dialog::SetDialogPosSize()
{
#if 0
	// �_�C�A���O�̃T�C�Y�A�ʒu�̍Č�
	if (xPos != -1 && yPos != -1) {
		::SetWindowPos(hWnd, NULL, xPos, yPos, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER);
		DEBUG_TRACE(_T("Dialog::OnInitDialog() xPos=%d yPos=%d\n"), xPos, yPos);
	}
	if (nWidth != -1 && nHeight != -1) {
		::SetWindowPos(hWnd, NULL, 0, 0, nWidth, nHeight, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
	}
#endif

	if (xPos != -1 && yPos != -1) {
		// �E�B���h�E�ʒu�E�T�C�Y���Č�
		// 2014.11.28 �t�H���g�ύX�Ή�
		if (nWidth == -1) {
			RECT rc;
			::GetWindowRect(hWnd, &rc);
			nWidth = rc.right - rc.left;
			nHeight = rc.bottom - rc.top;
		}

		if (!(::GetWindowLongPtr(hWnd, GWL_STYLE) & WS_CHILD)) {
			// 2006.06.09 ryoji
			// ���j�^�̃��[�N�̈�������E�㉺�ɂP�h�b�g�������̈���ɑS�̂����܂�悤�Ɉʒu��������
			//
			// note: �_�C�A���O�����[�N�̈拫�E�ɂ҂����荇�킹�悤�Ƃ���ƁA
			//       �����I�ɐe�̒����Ɉړ��������Ă��܂��Ƃ�������
			//      �i�}���`���j�^���Őe����v���C�}�����j�^�ɂ���ꍇ�����H�j
			//       �󋵂ɍ��킹�ď�����ς���͖̂��Ȃ̂ŁA�ꗥ�A�P�h�b�g�̋󂫂�����

			RECT rc;
			RECT rcWork;
			rc.left = xPos;
			rc.top = yPos;
			rc.right = xPos + nWidth;
			rc.bottom = yPos + nHeight;
			GetMonitorWorkRect(&rc, &rcWork);
			rcWork.top += 1;
			rcWork.bottom -= 1;
			rcWork.left += 1;
			rcWork.right -= 1;
			if (rc.bottom > rcWork.bottom) {
				rc.top -= (rc.bottom - rcWork.bottom);
				rc.bottom = rcWork.bottom;
			}
			if (rc.right > rcWork.right) {
				rc.left -= (rc.right - rcWork.right);
				rc.right = rcWork.right;
			}
			if (rc.top < rcWork.top) {
				rc.bottom += (rcWork.top - rc.top);
				rc.top = rcWork.top;
			}
			if (rc.left < rcWork.left) {
				rc.right += (rcWork.left - rc.left);
				rc.left = rcWork.left;
			}
			xPos = rc.left;
			yPos = rc.top;
			nWidth = rc.right - rc.left;
			nHeight = rc.bottom - rc.top;
		}

		WINDOWPLACEMENT windowPlacement;
		windowPlacement.length = sizeof(windowPlacement);
		windowPlacement.showCmd = nShowCmd;	// �ő剻�E�ŏ���
		windowPlacement.rcNormalPosition.left = xPos;
		windowPlacement.rcNormalPosition.top = yPos;
		windowPlacement.rcNormalPosition.right = nWidth + xPos;
		windowPlacement.rcNormalPosition.bottom = nHeight + yPos;
		::SetWindowPlacement(hWnd, &windowPlacement);
	}
}

BOOL Dialog::OnDestroy(void)
{
	// �E�B���h�E�ʒu�E�T�C�Y���L��
	WINDOWPLACEMENT windowPlacement;
	windowPlacement.length = sizeof(windowPlacement);
	if (::GetWindowPlacement(hWnd, &windowPlacement)) {
		nShowCmd = windowPlacement.showCmd;	// �ő剻�E�ŏ���
		xPos = windowPlacement.rcNormalPosition.left;
		yPos = windowPlacement.rcNormalPosition.top;
		nWidth = windowPlacement.rcNormalPosition.right - windowPlacement.rcNormalPosition.left;
		nHeight = windowPlacement.rcNormalPosition.bottom - windowPlacement.rcNormalPosition.top;
		// 2014.11.28 �t�H���g�ύX�ɂ��T�C�Y�ύX�Ή�
		if (!bSizable) {
			nWidth = -1;
			nHeight = -1;
		}
	}
	// �j��
	if (hwndSizeBox) {
		::DestroyWindow(hwndSizeBox);
		hwndSizeBox = NULL;
	}
	hWnd = NULL;
	return TRUE;
}


BOOL Dialog::OnBnClicked(int wID)
{
	switch (wID) {
	case IDCANCEL:	// Fall through.
	case IDOK:
		CloseDialog(wID);
		return TRUE;
	}
	return FALSE;
}

BOOL Dialog::OnSize()
{
	return Dialog::OnSize(0, 0);
}

BOOL Dialog::OnSize(WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	::GetWindowRect(hWnd, &rc);

	// �_�C�A���O�̃T�C�Y�̋L��
	xPos = rc.left;
	yPos = rc.top;
	nWidth = rc.right - rc.left;
	nHeight = rc.bottom - rc.top;

	// �T�C�Y�{�b�N�X�̈ړ�
	if (hwndSizeBox) {
		::GetClientRect(hWnd, &rc);
//		::SetWindowPos(hwndSizeBox, NULL,
// Sept. 17, 2000 JEPRO_16thdot �A�C�R����16dot�ڂ��\�������悤�Ɏ��s��ύX����K�v����H
// Jan. 12, 2001 JEPRO (directed by stonee) 15��16�ɕύX����ƃA�E�g���C����͂̃_�C�A���O�̉E���ɂ���
// �O���b�v�T�C�Y��`�V��'���ł��Ă��܂�(�ړ�����I)�A�_�C�A���O��傫���ł��Ȃ��Ƃ�����Q����������̂�
// �ύX���Ȃ����Ƃɂ���(�v����Ɍ���łɖ߂�������)
//			rc.right - rc.left - 15, rc.bottom - rc.top - 15,
//			13, 13,
//			SWP_NOOWNERZORDER | SWP_NOZORDER
//		);

// Jan. 12, 2001 Stonee (suggested by genta)
//		"13"�Ƃ����Œ�l�ł͂Ȃ��V�X�e������擾�����X�N���[���o�[�T�C�Y���g���悤�ɏC��
		::SetWindowPos(hwndSizeBox, NULL,
			rc.right - rc.left - GetSystemMetrics(SM_CXVSCROLL), //<-- stonee
			rc.bottom - rc.top - GetSystemMetrics(SM_CYHSCROLL), //<-- stonee
			GetSystemMetrics(SM_CXVSCROLL), //<-- stonee
			GetSystemMetrics(SM_CYHSCROLL), //<-- stonee
			SWP_NOOWNERZORDER | SWP_NOZORDER
		);

		// SizeBox���e�X�g
		if (wParam == SIZE_MAXIMIZED) {
			::ShowWindow(hwndSizeBox, SW_HIDE);
		}else {
			::ShowWindow(hwndSizeBox, SW_SHOW);
		}
		::InvalidateRect(hwndSizeBox, NULL, TRUE);
	}
	return FALSE;

}

BOOL Dialog::OnMove(WPARAM wParam, LPARAM lParam)
{
	// �_�C�A���O�̈ʒu�̋L��
	if (!bInited) {
		return TRUE;
	}
	RECT rc;
	::GetWindowRect(hWnd, &rc);

	// �_�C�A���O�̃T�C�Y�̋L��
	xPos = rc.left;
	yPos = rc.top;
	nWidth = rc.right - rc.left;
	nHeight = rc.bottom - rc.top;
	DEBUG_TRACE(_T("Dialog::OnMove() xPos=%d yPos=%d\n"), xPos, yPos);
	return TRUE;

}

void Dialog::CreateSizeBox(void)
{
	// �T�C�Y�{�b�N�X
	hwndSizeBox = ::CreateWindowEx(
		WS_EX_CONTROLPARENT,								// no extended styles
		_T("SCROLLBAR"),									// scroll bar control class
		NULL,												// text for window title bar
		WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, // scroll bar styles
		0,													// horizontal position
		0,													// vertical position
		0,													// width of the scroll bar
		0,													// default height
		hWnd/*hdlg*/, 									// handle of main window
		(HMENU) NULL,										// no menu for a scroll bar
		SelectLang::getLangRsrcInstance(),					// instance owning this window
		(LPVOID) NULL										// pointer not needed
	);
	::ShowWindow(hwndSizeBox, SW_SHOW);

}


// �_�C�A���O�̃��b�Z�[�W����
INT_PTR Dialog::DispatchEvent(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
//	DEBUG_TRACE(_T("Dialog::DispatchEvent() uMsg == %xh\n"), uMsg);
	switch (uMsg) {
	case WM_INITDIALOG:	return OnInitDialog(hwndDlg, wParam, lParam);
	case WM_DESTROY:	return OnDestroy();
	case WM_COMMAND:	return OnCommand(wParam, lParam);
	case WM_NOTIFY:		return OnNotify(wParam, lParam);
	case WM_SIZE:
		hWnd = hwndDlg;
		return OnSize(wParam, lParam);
	case WM_MOVE:
		hWnd = hwndDlg;
		return OnMove(wParam, lParam);
	case WM_DRAWITEM:	return OnDrawItem(wParam, lParam);
	case WM_TIMER:		return OnTimer(wParam);
	case WM_KEYDOWN:	return OnKeyDown(wParam, lParam);
	case WM_KILLFOCUS:	return OnKillFocus(wParam, lParam);
	case WM_ACTIVATE:	return OnActivate(wParam, lParam);	//@@@ 2003.04.08 MIK
	case WM_VKEYTOITEM:	return OnVKeyToItem(wParam, lParam);
	case WM_CHARTOITEM:	return OnCharToItem(wParam, lParam);
	case WM_HELP:		return OnPopupHelp(wParam, lParam);	//@@@ 2002.01.18 add
	case WM_CONTEXTMENU:return OnContextMenu(wParam, lParam);	//@@@ 2002.01.18 add
	}
	return FALSE;
}

BOOL Dialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	WORD	wNotifyCode;
	WORD	wID;
	HWND	hwndCtl;
	wNotifyCode = HIWORD(wParam);	// �ʒm�R�[�h
	wID			= LOWORD(wParam);	// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID
	hwndCtl		= (HWND) lParam;	// �R���g���[���̃n���h��
	TCHAR	szClass[32];

	// IDOK �� IDCANCEL �̓{�^������łȂ��Ă���������
	// MSDN [Windows Management] "Dialog Box Programming Considerations"
	if (wID == IDOK || wID == IDCANCEL) {
		return OnBnClicked(wID);
	}
	
	// �ʒm�����R���g���[���������ꍇ�̏���
	if (hwndCtl) {
		::GetClassName(hwndCtl, szClass, _countof(szClass));
		if (::lstrcmpi(szClass, _T("Button")) == 0) {
			switch (wNotifyCode) {
			// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
			case BN_CLICKED:	return OnBnClicked(wID);
			}
		}else if (::lstrcmpi(szClass, _T("Static")) == 0) {
			switch (wNotifyCode) {
			case STN_CLICKED:	return OnStnClicked(wID);
			}
		}else if (::lstrcmpi(szClass, _T("Edit")) == 0) {
			switch (wNotifyCode) {
			case EN_CHANGE:		return OnEnChange(hwndCtl, wID);
			case EN_KILLFOCUS:	return OnEnKillFocus(hwndCtl, wID);
			}
		}else if (::lstrcmpi(szClass, _T("ListBox")) == 0) {
			switch (wNotifyCode) {
			case LBN_SELCHANGE:	return OnLbnSelChange(hwndCtl, wID);
			case LBN_DBLCLK:	return OnLbnDblclk(wID);
			}
		}else if (::lstrcmpi(szClass, _T("ComboBox")) == 0) {
			switch (wNotifyCode) {
			// �R���{�{�b�N�X�p���b�Z�[�W
			case CBN_SELCHANGE:	return OnCbnSelChange(hwndCtl, wID);
			// @@2005.03.31 MIK �^�O�W�����vDialog�Ŏg���̂Œǉ�
			case CBN_EDITCHANGE:	return OnCbnEditChange(hwndCtl, wID);
			case CBN_DROPDOWN:	return OnCbnDropDown(hwndCtl, wID);
		//	case CBN_CLOSEUP:	return OnCbnCloseUp(hwndCtl, wID);
			case CBN_SELENDOK:	return OnCbnSelEndOk(hwndCtl, wID);
			}
		}
	}

	return FALSE;
}

//@@@ 2002.01.18 add start
BOOL Dialog::OnPopupHelp(WPARAM wPara, LPARAM lParam)
{
	HELPINFO* p = (HELPINFO*) lParam;
	MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)GetHelpIdTable());	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
	return TRUE;
}

BOOL Dialog::OnContextMenu(WPARAM wPara, LPARAM lParam)
{
	MyWinHelp(hWnd, HELP_CONTEXTMENU, (ULONG_PTR)GetHelpIdTable());	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
	return TRUE;
}

const DWORD p_helpids[] = {
	0, 0
};

LPVOID Dialog::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

BOOL Dialog::OnCbnSelEndOk(HWND hwndCtl, int wID)
{
	// �R���{�{�b�N�X�̃��X�g��\�������܂ܕ������ҏW���AEnter�L�[��
	// �����ƕ����񂪏����錻�ۂ̑΍�B
	// Enter�L�[�������Ă��̊֐��ɓ�������A���X�g���\���ɂ��Ă��܂��B

	// ���X�g���\���ɂ���ƑO����v���镶�����I��ł��܂��̂ŁA
	// ���O�ɕ������ޔ����A���X�g��\����ɕ�������B

	// �������ޔ�
	int nLength = ::GetWindowTextLength(hwndCtl);
	std::vector<TCHAR> buf(nLength + 1);
	LPTSTR sBuf = &buf[0];
	::GetWindowText(hwndCtl, sBuf, nLength + 1);
	sBuf[nLength] = _T('\0');

	// ���X�g���\���ɂ���
	Combo_ShowDropdown(hwndCtl, FALSE);

	// ������𕜌��E�S�I��
	::SetWindowText(hwndCtl, sBuf);
	Combo_SetEditSel(hwndCtl, 0, -1);

	return TRUE;
}

BOOL Dialog::OnCbnDropDown(HWND hwndCtl, int wID)
{
	return OnCbnDropDown( hwndCtl, false );
}

/** �R���{�{�b�N�X�̃h���b�v�_�E��������

	�R���{�{�b�N�X���h���b�v�_�E������鎞��
	�h���b�v�_�E�����X�g�̕����A�C�e��������̍ő�\�����ɍ��킹��

	@param hwndCtl [in]		�R���{�{�b�N�X�̃E�B���h�E�n���h��
	@param wID [in]			�R���{�{�b�N�X��ID

	@author ryoji
	@date 2009.03.29 �V�K�쐬
*/
BOOL Dialog::OnCbnDropDown(HWND hwndCtl, bool scrollBar)
{
	SIZE sizeText;
	const int nMargin = 8;
	int nScrollWidth = scrollBar ? ::GetSystemMetrics( SM_CXVSCROLL ) + 2 : 2;

	HDC hDC = ::GetDC(hwndCtl);
	if (!hDC)
		return FALSE;
	HFONT hFont = (HFONT)::SendMessage(hwndCtl, WM_GETFONT, 0, (LPARAM)NULL);
	hFont = (HFONT)::SelectObject(hDC, hFont);
	int nItem = Combo_GetCount(hwndCtl);
	RECT rc;
	::GetWindowRect(hwndCtl, &rc);
	LONG nWidth = rc.right - rc.left - nMargin + nScrollWidth;
	for (int iItem=0; iItem<nItem; ++iItem) {
		int nTextLen = Combo_GetLBTextLen(hwndCtl, iItem);
		if (0 < nTextLen) {
			std::vector<TCHAR> szText(nTextLen + 1);
			TCHAR* pszText = &szText[0];
			Combo_GetLBText(hwndCtl, iItem, pszText);
			if (::GetTextExtentPoint32(hDC, pszText, nTextLen, &sizeText)) {
				nWidth = std::max(nWidth, sizeText.cx + nScrollWidth);
			}
		}
	}
	Combo_SetDroppedWidth(hwndCtl, nWidth + nMargin);
	::SelectObject(hDC, hFont);
	::ReleaseDC(hwndCtl, hDC);
	return TRUE;
}

/*! �t�@�C���I��
	@note ���s�t�@�C���̃p�Xor�ݒ�t�@�C���̃p�X���܂܂��ꍇ�͑��΃p�X�ɕϊ�
*/
BOOL Dialog::SelectFile(
	HWND parent,
	HWND hwndCtl,
	const TCHAR* filter,
	bool resolvePath
	)
{
	DlgOpenFile dlgOpenFile;
	TCHAR szFilePath[_MAX_PATH + 1];
	TCHAR szPath[_MAX_PATH + 1];
	::GetWindowText(hwndCtl, szFilePath, _countof(szFilePath));
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	if (resolvePath && _IS_REL_PATH(szFilePath)) {
		GetInidirOrExedir(szPath, szFilePath);
	}else {
		auto_strcpy(szPath, szFilePath);
	}
	// �t�@�C���I�[�v���_�C�A���O�̏�����
	dlgOpenFile.Create(
		::GetModuleHandle(NULL),
		parent,
		filter,
		szPath
	);
	if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
		const TCHAR* fileName;
		if (resolvePath) {
			fileName = GetRelPath(szPath);
		}else {
			fileName = szPath;
		}
		::SetWindowText(hwndCtl, fileName);
		return TRUE;
	}
	return FALSE;
}


// static
bool Dialog::DirectoryUp(TCHAR* szDir)
{
	size_t nLen = auto_strlen(szDir);
	if (3 < nLen) {
		// X:\ ��\\. ��蒷��
		CutLastYenFromDirectoryPath(szDir);
		const TCHAR* p = GetFileTitlePointer(szDir);
		if (0 < p - szDir) {
			if (3 < p - szDir) {
				szDir[p - szDir - 1] = '\0'; // \�����̂�-1
			}else {
				// �uC:\�v��\���c��
				szDir[p - szDir] = '\0';
			}
		}
		return true;
	}
	return false;
}

// �R���g���[���ɉ�ʂ̃t�H���g��ݒ�	2012/11/27 Uchi
HFONT Dialog::SetMainFont(HWND hTarget)
{
	if (!hTarget) {
		return NULL;
	}

	// �ݒ肷��t�H���g�̍������擾
	HFONT hFont = (HFONT)::SendMessage(hTarget, WM_GETFONT, 0, 0);
	LOGFONT	lf;
	GetObject(hFont, sizeof(lf), &lf);
	LONG nfHeight = lf.lfHeight;

	// LOGFONT�̍쐬
	lf = pShareData->common.view.lf;
	lf.lfHeight			= nfHeight;
	lf.lfWidth			= 0;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= FW_NORMAL;
	lf.lfItalic			= FALSE;
	lf.lfUnderline		= FALSE;
	lf.lfStrikeOut		= FALSE;
	//lf.lfCharSet		= lf.lfCharSet;
	lf.lfOutPrecision	= OUT_TT_ONLY_PRECIS;		// Raster Font ���g��Ȃ��悤��
	//lf.lfClipPrecision	= lf.lfClipPrecision;
	//lf.lfQuality		= lf.lfQuality;
	//lf.lfPitchAndFamily	= lf.lfPitchAndFamily;
	//_tcsncpy(lf.lfFaceName, lf.lfFaceName, _countof(lf.lfFaceName));	// ��ʂ̃t�H���g�ɐݒ�	2012/11/27 Uchi

	// �t�H���g���쐬
	hFont = ::CreateFontIndirect(&lf);
	if (hFont) {
		// �t�H���g�̐ݒ�
		::SendMessage(hTarget, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
	}
	return hFont;
}

void Dialog::ResizeItem(
	HWND hTarget,
	const POINT& ptDlgDefault,
	const POINT& ptDlgNew,
	const RECT& rcItemDefault,
	AnchorStyle anchor,
	bool bUpdate
	)
{
	POINT pt;
	int height, width;
	pt.x = rcItemDefault.left;
	pt.y = rcItemDefault.top;
	width = rcItemDefault.right - rcItemDefault.left;
	height = rcItemDefault.bottom - rcItemDefault.top;
	if (((int)anchor & ((int)AnchorStyle::Left | (int)AnchorStyle::Right)) == (int)AnchorStyle::Left) {
		// �Ȃ�
	}else if (((int)anchor & ((int)AnchorStyle::Left | (int)AnchorStyle::Right)) == (int)AnchorStyle::Right) {
		/*
			[<- rcItemDefault.left ->[ ]     ]
			[<- rcItemDefault.right  [->]     ]
			[<-    ptDlgDefault.x             ->]
			[<-    ptDlgNew.x             [ ]    ->]
			[<-    pt.x                 ->[ ]     ]
		*/
		pt.x = rcItemDefault.left + (ptDlgNew.x - ptDlgDefault.x);
	}else if (((int)anchor & ((int)AnchorStyle::Left | (int)AnchorStyle::Right)) == ((int)AnchorStyle::Left | (int)AnchorStyle::Right)) {
		/*
			[<-    ptDlgNew.x        [ ]         ->]
			[                       [<-width->]    ]
		*/
		width = ptDlgNew.x - rcItemDefault.left - (ptDlgDefault.x - rcItemDefault.right);
	}
	
	if (((int)anchor & ((int)AnchorStyle::Top | (int)AnchorStyle::Bottom)) == (int)AnchorStyle::Top) {
		// �Ȃ�
	}else if (((int)anchor & ((int)AnchorStyle::Top | (int)AnchorStyle::Bottom)) == (int)AnchorStyle::Bottom) {
		pt.y = rcItemDefault.top + (ptDlgNew.y - ptDlgDefault.y);
	}else if (((int)anchor & ((int)AnchorStyle::Top | (int)AnchorStyle::Bottom)) == ((int)AnchorStyle::Top | (int)AnchorStyle::Bottom)) {
		height = ptDlgNew.y - rcItemDefault.top - (ptDlgDefault.y - rcItemDefault.bottom);
	}
//	::MoveWindow(hTarget, pt.x, pt.y, width, height, FALSE);
	::SetWindowPos(hTarget, NULL, pt.x, pt.y, width, height,
				SWP_NOOWNERZORDER | SWP_NOZORDER);
	if (bUpdate) {
		::InvalidateRect(hTarget, NULL, TRUE);
	}
}

void Dialog::GetItemClientRect(int wID, RECT& rc)
{
	POINT po;
	::GetWindowRect(GetItemHwnd(wID), &rc);
	po.x = rc.left;
	po.y = rc.top;
	::ScreenToClient(GetHwnd(), &po);
	rc.left = po.x;
	rc.top  = po.y;
	po.x = rc.right;
	po.y = rc.bottom;
	::ScreenToClient(GetHwnd(), &po);
	rc.right  = po.x;
	rc.bottom = po.y;
}

static const TCHAR* TSTR_SUBCOMBOBOXDATA = _T("SubComboBoxData");

static
void DeleteItem(HWND hwnd, Recent* pRecent)
{
	int nIndex = Combo_GetCurSel(hwnd);
	if (0 <= nIndex) {
		std::vector<TCHAR> szText;
		szText.resize(Combo_GetLBTextLen(hwnd, nIndex) + 1);
		Combo_GetLBText(hwnd, nIndex, &szText[0]);
		Combo_DeleteString(hwnd, nIndex);
		int nRecentIndex = pRecent->FindItemByText(&szText[0]);
		if (0 <= nRecentIndex) {
			pRecent->DeleteItem(nRecentIndex);
		}
	}
}

LRESULT CALLBACK SubEditProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	ComboBoxItemDeleter* data = (ComboBoxItemDeleter*)::GetProp(hwnd, TSTR_SUBCOMBOBOXDATA);
	switch (uMsg) {
	case WM_KEYDOWN:
	{
		if (wParam == VK_DELETE) {
			HWND hwndCombo = data->hwndCombo;
			BOOL bShow = Combo_GetDroppedState(hwndCombo);
			if (bShow) {
				DeleteItem(hwndCombo, data->pRecent);
				return 0;
			}
		}
		break;
	}
	case WM_DESTROY:
	{
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)data->pEditWndProc);
		::RemoveProp(hwnd, TSTR_SUBCOMBOBOXDATA);
		data->pEditWndProc = NULL;
		break;
	}
	default:
		break;
	}
	return CallWindowProc(data->pEditWndProc, hwnd, uMsg, wParam, lParam);
}


LRESULT CALLBACK SubListBoxProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	ComboBoxItemDeleter* data = (ComboBoxItemDeleter*)::GetProp(hwnd, TSTR_SUBCOMBOBOXDATA);
	switch (uMsg) {
	case WM_KEYDOWN:
	{
		if (wParam == VK_DELETE) {
			HWND hwndCombo = data->hwndCombo;
			BOOL bShow = Combo_GetDroppedState(hwndCombo);
			if (bShow) {
				DeleteItem(hwndCombo, data->pRecent);
				return 0;
			}
		}
		break;
	}
	case WM_DESTROY:
	{
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)data->pListBoxWndProc);
		::RemoveProp(hwnd, TSTR_SUBCOMBOBOXDATA);
		data->pListBoxWndProc = NULL;
		break;
	}
	default:
		break;
	}
	return CallWindowProc(data->pListBoxWndProc, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK SubComboBoxProc(
	HWND hwnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	ComboBoxItemDeleter* data = (ComboBoxItemDeleter*)::GetProp(hwnd, TSTR_SUBCOMBOBOXDATA);
	switch (uMsg) {
	case WM_CTLCOLOREDIT:
	{
		if (!data->pEditWndProc) {
			HWND hwndCtl = (HWND)lParam;
			data->pEditWndProc = (WNDPROC)::GetWindowLongPtr(hwndCtl, GWLP_WNDPROC);
			::SetProp(hwndCtl, TSTR_SUBCOMBOBOXDATA, data);
			::SetWindowLongPtr(hwndCtl, GWLP_WNDPROC, (LONG_PTR)SubEditProc);
		}
		break;
	}
	case WM_CTLCOLORLISTBOX:
	{
		if (!data->pListBoxWndProc) {
			HWND hwndCtl = (HWND)lParam;
			data->pListBoxWndProc = (WNDPROC)::GetWindowLongPtr(hwndCtl, GWLP_WNDPROC);
			::SetProp(hwndCtl, TSTR_SUBCOMBOBOXDATA, data);
			::SetWindowLongPtr(hwndCtl, GWLP_WNDPROC, (LONG_PTR)SubListBoxProc);
		}
		break;
	}
	case WM_DESTROY:
	{
		::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)data->pComboBoxWndProc);
		::RemoveProp(hwnd, TSTR_SUBCOMBOBOXDATA);
		data->pComboBoxWndProc = NULL;
		break;
	}

	default:
		break;
	}
	return CallWindowProc(data->pComboBoxWndProc, hwnd, uMsg, wParam, lParam);
}

void Dialog::SetComboBoxDeleter(HWND hwndCtl, ComboBoxItemDeleter* data)
{
	if (!data->pRecent) {
		return;
	}
	data->hwndCombo = hwndCtl;
	data->pComboBoxWndProc = (WNDPROC)::GetWindowLongPtr(hwndCtl, GWLP_WNDPROC);
	::SetProp(hwndCtl, TSTR_SUBCOMBOBOXDATA, data);
	::SetWindowLongPtr(hwndCtl, GWLP_WNDPROC, (LONG_PTR)SubComboBoxProc);
}

