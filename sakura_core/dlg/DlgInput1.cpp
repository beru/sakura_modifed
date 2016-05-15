/*!	@file
	@brief 1�s���̓_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/05/31 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/DlgInput1.h"
#include "EditApp.h"
#include "Funccode_enum.h"	// EFunctionCode
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

// ���� CDlgInput1.cpp	//@@@ 2002.01.07 add start MIK
static const DWORD p_helpids[] = {	//13000
	IDOK,				HIDOK_DLG1,
	IDCANCEL,			HIDCANCEL_DLG1,
	IDC_EDIT_INPUT1,	HIDC_DLG1_EDIT1,	// ���̓t�B�[���h	IDC_EDIT1->IDC_EDIT_INPUT1	2008/7/3 Uchi
	IDC_STATIC_MSG,		HIDC_DLG1_EDIT1,	// ���b�Z�[�W
//	IDC_STATIC,			-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK


// �_�C�A���O�v���V�[�W��
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
		// Modified by KEITA for WIN64 2003.9.6
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


// ���[�h���X�_�C�A���O�̕\��
BOOL DlgInput1::DoModal(
	HINSTANCE		hInstApp,
	HWND			hwndParent,
	const TCHAR*	pszTitle,
	const TCHAR*	pszMessage,
	int				nMaxTextLen,
	TCHAR*			pszText
	)
{
	BOOL bRet;
	hInstance = hInstApp;			// �A�v���P�[�V�����C���X�^���X�̃n���h��
	hwndParent = hwndParent;		// �I�[�i�[�E�B���h�E�̃n���h��
	pszTitle = pszTitle;			// �_�C�A���O�^�C�g��
	pszMessage = pszMessage;		// ���b�Z�[�W
	nMaxTextLen = nMaxTextLen;	// ���̓T�C�Y���
//	m_pszText = pszText;			// �e�L�X�g
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
	int				nMaxTextLen,
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


// �_�C�A���O�̃��b�Z�[�W����
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
		// �_�C�A���O�f�[�^�̐ݒ�
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		::SetWindowText(hwndDlg, pszTitle);	// �_�C�A���O�^�C�g��
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_INPUT1), nMaxTextLen);	// ���̓T�C�Y���
		::SetDlgItemText(hwndDlg, IDC_EDIT_INPUT1, memText.GetStringPtr());		// �e�L�X�g
		::SetDlgItemText(hwndDlg, IDC_STATIC_MSG, pszMessage);		// ���b�Z�[�W
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// �ʒm�R�[�h
		wID = LOWORD(wParam);	// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID
		switch (wNotifyCode) {
		// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
		case BN_CLICKED:
			switch (wID) {
			case IDOK:
				memText.AllocStringBuffer(::GetWindowTextLength(::GetDlgItem(hwndDlg, IDC_EDIT_INPUT1)));
				::GetWindowText(::GetDlgItem(hwndDlg, IDC_EDIT_INPUT1), memText.GetStringPtr(), nMaxTextLen + 1);	// �e�L�X�g
				::EndDialog(hwndDlg, TRUE);
				return TRUE;
			case IDCANCEL:
				::EndDialog(hwndDlg, FALSE);
				return TRUE;
			}
			break;	//@@@ 2002.01.07 add
		}
		break;	//@@@ 2002.01.07 add
	//@@@ 2002.01.07 add start
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO *)lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	//@@@ 2002.01.07 add end
	}
	return FALSE;
}

