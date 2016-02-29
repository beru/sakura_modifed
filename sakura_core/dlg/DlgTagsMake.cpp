/*!	@file
	@brief �^�O�t�@�C���쐬�_�C�A���O�{�b�N�X

	@author MIK
	@date 2003.5.12
*/
/*
	Copyright (C) 2003, MIK
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
#include "dlg/DlgTagsMake.h"
#include "env/DllSharedData.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {	//13700
	IDC_EDIT_TAG_MAKE_FOLDER,	HIDC_EDIT_TAG_MAKE_FOLDER,		// �^�O�쐬�t�H���_
	IDC_BUTTON_TAG_MAKE_REF,	HIDC_BUTTON_TAG_MAKE_REF,		// �Q��
	IDC_BUTTON_FOLDER_UP,		HIDC_BUTTON_TAG_MAKE_FOLDER_UP,	// ��
	IDC_EDIT_TAG_MAKE_CMDLINE,	HIDC_EDIT_TAG_MAKE_CMDLINE,		// �R�}���h���C��
	IDC_CHECK_TAG_MAKE_RECURSE,	HIDC_CHECK_TAG_MAKE_RECURSE,	// �T�u�t�H���_���Ώ�
	IDOK,						HIDC_TAG_MAKE_IDOK,
	IDCANCEL,					HIDC_TAG_MAKE_IDCANCEL,
	IDC_BUTTON_HELP,			HIDC_BUTTON_TAG_MAKE_HELP,
//	IDC_STATIC,					-1,
	0, 0
};

DlgTagsMake::DlgTagsMake()
{
	m_szPath[0] = 0;
	m_szTagsCmdLine[0] = 0;
	m_nTagsOpt = 0;
	return;
}

// ���[�_���_�C�A���O�̕\��
int DlgTagsMake::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	const TCHAR*	pszPath		// �p�X
	)
{
	_tcscpy( m_szPath, pszPath );
	return (int)Dialog::DoModal(hInstance, hwndParent, IDD_TAG_MAKE, lParam);
}

BOOL DlgTagsMake::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		// �w���v
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_TAGS_MAKE));	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;

	case IDC_BUTTON_TAG_MAKE_REF:	// �Q��
		SelectFolder();
		return TRUE;

	case IDC_BUTTON_FOLDER_UP:
		{
			TCHAR szDir[_MAX_PATH];
			HWND hwnd = GetItemHwnd(IDC_EDIT_TAG_MAKE_FOLDER);
			::GetWindowText(hwnd, szDir, _countof(szDir));
			if (DirectoryUp(szDir)) {
				::SetWindowText(hwnd, szDir);
			}
		}
		return TRUE;

	case IDOK:
		// �_�C�A���O�f�[�^�̎擾
		::EndDialog(GetHwnd(), GetData());
		return TRUE;

	case IDCANCEL:
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;

	}

	// ���N���X�����o
	return Dialog::OnBnClicked(wID);
}

/*!
	�t�H���_��I������
	
	@param hwndDlg [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h��
*/
void DlgTagsMake::SelectFolder()
{
	HWND hwndDlg = GetHwnd();
	TCHAR szPath[_MAX_PATH + 1];

	// �t�H���_
	::DlgItem_GetText(hwndDlg, IDC_EDIT_TAG_MAKE_FOLDER, szPath, _MAX_PATH);

	if (SelectDir(hwndDlg, LS(STR_DLGTAGMAK_SELECTDIR), szPath, szPath)) {
		// ������\\�}�[�N��ǉ�����D
		int pos = _tcslen(szPath);
		if (pos > 0 && szPath[pos - 1] != _T('\\')) {
			szPath[pos    ] = _T('\\');
			szPath[pos + 1] = _T('\0');
		}

		SetItemText(IDC_EDIT_TAG_MAKE_FOLDER, szPath);
	}
}

// �_�C�A���O�f�[�^�̐ݒ�
void DlgTagsMake::SetData(void)
{
	// �쐬�t�H���_
	Combo_LimitText(GetItemHwnd(IDC_EDIT_TAG_MAKE_FOLDER), _countof(m_szPath));
	SetItemText(IDC_EDIT_TAG_MAKE_FOLDER, m_szPath);

	// �I�v�V����
	m_nTagsOpt = m_pShareData->nTagsOpt;
	if (m_nTagsOpt & 0x0001) {
		CheckButton(IDC_CHECK_TAG_MAKE_RECURSE, true);
	}

	// �R�}���h���C��
	Combo_LimitText(GetItemHwnd(IDC_EDIT_TAG_MAKE_CMDLINE), _countof(m_pShareData->szTagsCmdLine));
	_tcscpy(m_szTagsCmdLine, m_pShareData->szTagsCmdLine);
	SetItemText(IDC_EDIT_TAG_MAKE_CMDLINE, m_pShareData->szTagsCmdLine);

	return;
}

// �_�C�A���O�f�[�^�̎擾
// TRUE==����  FALSE==���̓G���[
int DlgTagsMake::GetData(void)
{
	// �t�H���_
	GetItemText(IDC_EDIT_TAG_MAKE_FOLDER, m_szPath, _countof(m_szPath));
	int length = _tcslen(m_szPath);
	if (length > 0) {
		if (m_szPath[length - 1] != _T('\\')) _tcscat(m_szPath, _T("\\"));
	}

	// CTAGS�I�v�V����
	m_nTagsOpt = 0;
	if (IsButtonChecked(IDC_CHECK_TAG_MAKE_RECURSE)) {
		m_nTagsOpt |= 0x0001;
	}
	m_pShareData->nTagsOpt = m_nTagsOpt;

	// �R�}���h���C��
	GetItemText(IDC_EDIT_TAG_MAKE_CMDLINE, m_szTagsCmdLine, _countof(m_szTagsCmdLine));
	_tcscpy(m_pShareData->szTagsCmdLine, m_szTagsCmdLine);

	return TRUE;
}

LPVOID DlgTagsMake::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


