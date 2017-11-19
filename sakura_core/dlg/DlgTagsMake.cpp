/*!	@file
	@brief �^�O�t�@�C���쐬�_�C�A���O�{�b�N�X
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
	szPath[0] = 0;
	szTagsCmdLine[0] = 0;
	nTagsOpt = 0;
	return;
}

// ���[�_���_�C�A���O�̕\��
INT_PTR DlgTagsMake::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	const TCHAR*	pszPath		// �p�X
	)
{
	_tcscpy( szPath, pszPath );
	return Dialog::DoModal(hInstance, hwndParent, IDD_TAG_MAKE, lParam);
}

BOOL DlgTagsMake::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		// �w���v
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_TAGS_MAKE));
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
		size_t pos = _tcslen(szPath);
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
	Combo_LimitText(GetItemHwnd(IDC_EDIT_TAG_MAKE_FOLDER), _countof(szPath));
	SetItemText(IDC_EDIT_TAG_MAKE_FOLDER, szPath);

	// �I�v�V����
	nTagsOpt = pShareData->nTagsOpt;
	if (nTagsOpt & 0x0001) {
		CheckButton(IDC_CHECK_TAG_MAKE_RECURSE, true);
	}

	// �R�}���h���C��
	Combo_LimitText(GetItemHwnd(IDC_EDIT_TAG_MAKE_CMDLINE), _countof(pShareData->szTagsCmdLine));
	_tcscpy(szTagsCmdLine, pShareData->szTagsCmdLine);
	SetItemText(IDC_EDIT_TAG_MAKE_CMDLINE, pShareData->szTagsCmdLine);

	return;
}

// �_�C�A���O�f�[�^�̎擾
// TRUE==����  FALSE==���̓G���[
int DlgTagsMake::GetData(void)
{
	// �t�H���_
	GetItemText(IDC_EDIT_TAG_MAKE_FOLDER, szPath, _countof(szPath));
	size_t length = _tcslen(szPath);
	if (length > 0) {
		if (szPath[length - 1] != _T('\\')) _tcscat(szPath, _T("\\"));
	}

	// CTAGS�I�v�V����
	nTagsOpt = 0;
	if (IsButtonChecked(IDC_CHECK_TAG_MAKE_RECURSE)) {
		nTagsOpt |= 0x0001;
	}
	pShareData->nTagsOpt = nTagsOpt;

	// �R�}���h���C��
	GetItemText(IDC_EDIT_TAG_MAKE_CMDLINE, szTagsCmdLine, _countof(szTagsCmdLine));
	_tcscpy(pShareData->szTagsCmdLine, szTagsCmdLine);

	return TRUE;
}

LPVOID DlgTagsMake::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


