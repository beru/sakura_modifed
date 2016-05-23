/*! @file
	@brief �E�B���h�E�̈ʒu�Ƒ傫���_�C�A���O

	@author Moca
	@date 2004/05/13 �쐬
*/
/*
	Copyright (C) 2004, genta, Moca
	Copyright (C) 2006, ryoji

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "dlg/DlgWinSize.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids[] = {	// 2006.10.10 ryoji
	IDOK,						HIDOK_WINSIZE,				// ����
	IDC_BUTTON_HELP,			HIDC_BUTTON_WINSIZE_HELP,	// �w���v
	IDC_EDIT_WX,				HIDC_EDIT_WX,				// ��
	IDC_EDIT_WY,				HIDC_EDIT_WY,				// ����
	IDC_EDIT_SX,				HIDC_EDIT_SX,				// X���W
	IDC_EDIT_SY,				HIDC_EDIT_SY,				// Y���W
//	IDC_CHECK_WINPOS,			HIDC_CHECK_WINPOS,
	IDC_RADIO_WINSIZE_DEF,		HIDC_RADIO_WINSIZE_DEF,		// �傫��/�w�肵�Ȃ�
	IDC_RADIO_WINSIZE_SAVE,		HIDC_RADIO_WINSIZE_SAVE,	// �傫��/�p������
	IDC_RADIO_WINSIZE_SET,		HIDC_RADIO_WINSIZE_SET,		// �傫��/���ڎw��
	IDC_RADIO_WINPOS_DEF,		HIDC_RADIO_WINPOS_DEF,		// �ʒu/�w�肵�Ȃ�
	IDC_RADIO_WINPOS_SAVE,		HIDC_RADIO_WINPOS_SAVE, 	// �ʒu/�p������
	IDC_RADIO_WINPOS_SET,		HIDC_RADIO_WINPOS_SET,  	// �ʒu/���ڎw��
	IDC_COMBO_WINTYPE,			HIDC_COMBO_WINTYPE,
	0, 0
};

DlgWinSize::DlgWinSize()
{
	return;
}

DlgWinSize::~DlgWinSize()
{
	return;
}


// !���[�_���_�C�A���O�̕\��
INT_PTR DlgWinSize::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	WinSizeMode&	eSaveWinSize,	// [in/out] �E�B���h�E�ʒu�p��
	WinSizeMode&	eSaveWinPos,	// [in/out] �E�B���h�E�T�C�Y�p��
	int&			nWinSizeType,	// [in/out] �E�B���h�E�̎��s���̑傫��
	RECT&			rc				// [in/out] ���A�����A���A��
	)
{
	eSaveWinSize = eSaveWinSize;
	eSaveWinPos  = eSaveWinPos;
	nWinSizeType = nWinSizeType;
	rc = rc;
	(void)Dialog::DoModal(hInstance, hwndParent, IDD_WINPOSSIZE, (LPARAM)NULL);
	eSaveWinSize = eSaveWinSize;
	eSaveWinPos  = eSaveWinPos;
	nWinSizeType = nWinSizeType;
	rc = rc;
	return TRUE;
}

/*! ����������
*/
BOOL DlgWinSize::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	_SetHwnd(hwndDlg);

	Combo_AddString(GetItemHwnd(IDC_COMBO_WINTYPE), LSW(STR_DLGWINSZ_NORMAL));	// L"����"
	Combo_AddString(GetItemHwnd(IDC_COMBO_WINTYPE), LSW(STR_DLGWINSZ_MAXIMIZE));	// L"�ő剻"
	Combo_AddString(GetItemHwnd(IDC_COMBO_WINTYPE), LSW(STR_DLGWINSZ_MINIMIZE));	// L"(�ŏ���)"

	UpDown_SetRange(GetItemHwnd(IDC_SPIN_SX), 30000, 0);
	UpDown_SetRange(GetItemHwnd(IDC_SPIN_SY), 30000, 0);
	// �E�B���h�E�̍��W�́A�}�C�i�X�l���L���B
	UpDown_SetRange(GetItemHwnd(IDC_SPIN_WX), 30000, -30000);
	UpDown_SetRange(GetItemHwnd(IDC_SPIN_WY), 30000, -30000);

	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}


BOOL DlgWinSize::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:	// 2006/09/09 novice id�C��
		MyWinHelp(GetHwnd(), HELP_CONTEXT, HLP000286);	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	case IDC_RADIO_WINPOS_DEF:
	case IDC_RADIO_WINPOS_SAVE:
	case IDC_RADIO_WINPOS_SET:
	case IDC_RADIO_WINSIZE_DEF:
	case IDC_RADIO_WINSIZE_SAVE:
	case IDC_RADIO_WINSIZE_SET:
		RenewItemState();
		return TRUE;
	case IDOK:
	case IDCANCEL:
		GetData();
	}
	return Dialog::OnBnClicked(wID);
}

/*! @brief �_�C�A���O�{�b�N�X�Ƀf�[�^��ݒ�
*/
void DlgWinSize::SetData(void)
{
	switch (eSaveWinSize) {
	case 1:
		CheckButton(IDC_RADIO_WINSIZE_SAVE, true);
		break;
	case 2:
		CheckButton(IDC_RADIO_WINSIZE_SET, true);
		break;
	default:
		CheckButton(IDC_RADIO_WINSIZE_DEF, true);
	}

	switch (eSaveWinPos) {
	case 1:
		CheckButton(IDC_RADIO_WINPOS_SAVE, true);
		break;
	case 2:
		CheckButton(IDC_RADIO_WINPOS_SET, true);
		break;
	default:
		CheckButton(IDC_RADIO_WINPOS_DEF, true);
	}

	int nCurIdx = 0;
	switch (nWinSizeType) {
	case SIZE_MINIMIZED:
		nCurIdx = 2;
		break;
	case SIZE_MAXIMIZED:
		nCurIdx = 1;
		break;
	default:
		nCurIdx = 0;
	}
	Combo_SetCurSel(GetItemHwnd(IDC_COMBO_WINTYPE), nCurIdx);
	SetItemInt(IDC_EDIT_SX, rc.right,  TRUE);
	SetItemInt(IDC_EDIT_SY, rc.bottom, TRUE);
	SetItemInt(IDC_EDIT_WX, rc.top,  TRUE);
	SetItemInt(IDC_EDIT_WY, rc.left, TRUE);
	RenewItemState();
}


/*! �_�C�A���O�{�b�N�X�̃f�[�^��ǂݏo��
*/
int DlgWinSize::GetData(void)
{
	if (IsButtonChecked(IDC_RADIO_WINSIZE_DEF)) {
		eSaveWinSize = WinSizeMode::Default;
	}else if (IsButtonChecked(IDC_RADIO_WINSIZE_SAVE)) {
		eSaveWinSize = WinSizeMode::Save;
	}else if (IsButtonChecked(IDC_RADIO_WINSIZE_SET)) {
		eSaveWinSize = WinSizeMode::Set;
	}
	
	if (IsButtonChecked(IDC_RADIO_WINPOS_DEF)) {
		eSaveWinPos = WinSizeMode::Default;
	}else if (IsButtonChecked(IDC_RADIO_WINPOS_SAVE)) {
		eSaveWinPos = WinSizeMode::Save;
	}else if (IsButtonChecked(IDC_RADIO_WINPOS_SET)) {
		eSaveWinPos = WinSizeMode::Set;
	}

	int nCurIdx;
	nCurIdx = Combo_GetCurSel(GetItemHwnd(IDC_COMBO_WINTYPE));
	switch (nCurIdx) {
	case 2:
		nWinSizeType = SIZE_MINIMIZED;
		break;
	case 1:
		nWinSizeType = SIZE_MAXIMIZED;
		break;
	default:
		nWinSizeType = SIZE_RESTORED;
	}
	rc.right  = GetItemInt(IDC_EDIT_SX, NULL, TRUE);
	rc.bottom = GetItemInt(IDC_EDIT_SY, NULL, TRUE);
	rc.top    = GetItemInt(IDC_EDIT_WX, NULL, TRUE);
	rc.left   = GetItemInt(IDC_EDIT_WY, NULL, TRUE);
	return TRUE;
}


/*! ���p�\�E�s�̏�Ԃ��X�V����
*/
void DlgWinSize::RenewItemState(void)
{
	bool state = IsButtonChecked(IDC_RADIO_WINPOS_SET);
	EnableItem(IDC_EDIT_WX, state);
	EnableItem(IDC_EDIT_WY, state);

	state = IsButtonChecked(IDC_RADIO_WINSIZE_SET);
	EnableItem(IDC_COMBO_WINTYPE, state);
	EnableItem(IDC_EDIT_SX, state);
	EnableItem(IDC_EDIT_SY, state);
}

LPVOID DlgWinSize::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


