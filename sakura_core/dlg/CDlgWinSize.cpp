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
#include "dlg/CDlgWinSize.h"
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
int DlgWinSize::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	EWinSizeMode&	eSaveWinSize,	// [in/out] �E�B���h�E�ʒu�p��
	EWinSizeMode&	eSaveWinPos,	// [in/out] �E�B���h�E�T�C�Y�p��
	int&			nWinSizeType,	// [in/out] �E�B���h�E�̎��s���̑傫��
	RECT&			rc				// [in/out] ���A�����A���A��
)
{
	m_eSaveWinSize = eSaveWinSize;
	m_eSaveWinPos  = eSaveWinPos;
	m_nWinSizeType = nWinSizeType;
	m_rc = rc;
	(void)Dialog::DoModal(hInstance, hwndParent, IDD_WINPOSSIZE, (LPARAM)NULL);
	eSaveWinSize = m_eSaveWinSize;
	eSaveWinPos  = m_eSaveWinPos;
	nWinSizeType = m_nWinSizeType;
	rc = m_rc;
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
	switch (m_eSaveWinSize) {
	case 1:
		CheckButton(IDC_RADIO_WINSIZE_SAVE, true);
		break;
	case 2:
		CheckButton(IDC_RADIO_WINSIZE_SET, true);
		break;
	default:
		CheckButton(IDC_RADIO_WINSIZE_DEF, true);
	}

	switch (m_eSaveWinPos) {
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
	switch (m_nWinSizeType) {
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
	SetItemInt(IDC_EDIT_SX, m_rc.right,  TRUE);
	SetItemInt(IDC_EDIT_SY, m_rc.bottom, TRUE);
	SetItemInt(IDC_EDIT_WX, m_rc.top,  TRUE);
	SetItemInt(IDC_EDIT_WY, m_rc.left, TRUE);
	RenewItemState();
}


/*! �_�C�A���O�{�b�N�X�̃f�[�^��ǂݏo��
*/
int DlgWinSize::GetData(void)
{
	if (IsButtonChecked(IDC_RADIO_WINSIZE_DEF)) {
		m_eSaveWinSize = WINSIZEMODE_DEF;
	}else if (IsButtonChecked(IDC_RADIO_WINSIZE_SAVE)) {
		m_eSaveWinSize = WINSIZEMODE_SAVE;
	}else if (IsButtonChecked(IDC_RADIO_WINSIZE_SET)) {
		m_eSaveWinSize = WINSIZEMODE_SET;
	}
	
	if (IsButtonChecked(IDC_RADIO_WINPOS_DEF)) {
		m_eSaveWinPos = WINSIZEMODE_DEF;
	}else if (IsButtonChecked(IDC_RADIO_WINPOS_SAVE)) {
		m_eSaveWinPos = WINSIZEMODE_SAVE;
	}else if (IsButtonChecked(IDC_RADIO_WINPOS_SET)) {
		m_eSaveWinPos = WINSIZEMODE_SET;
	}

	int nCurIdx;
	nCurIdx = Combo_GetCurSel(GetItemHwnd(IDC_COMBO_WINTYPE));
	switch (nCurIdx) {
	case 2:
		m_nWinSizeType = SIZE_MINIMIZED;
		break;
	case 1:
		m_nWinSizeType = SIZE_MAXIMIZED;
		break;
	default:
		m_nWinSizeType = SIZE_RESTORED;
	}
	m_rc.right  = GetItemInt(IDC_EDIT_SX, NULL, TRUE);
	m_rc.bottom = GetItemInt(IDC_EDIT_SY, NULL, TRUE);
	m_rc.top    = GetItemInt(IDC_EDIT_WX, NULL, TRUE);
	m_rc.left   = GetItemInt(IDC_EDIT_WY, NULL, TRUE);
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


