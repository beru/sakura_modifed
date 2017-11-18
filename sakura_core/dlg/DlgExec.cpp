/*!	@file
	@brief �O���R�}���h���s�_�C�A���O
*/

#include "StdAfx.h"
#include "dlg/DlgExec.h"
#include "dlg/DlgOpenFile.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "util/window.h"
#include "_main/AppMode.h"
#include "doc/EditDoc.h"
#include "sakura_rc.h"
#include "sakura.hh"

// �O���R�}���h CDlgExec.cpp
const DWORD p_helpids[] = {	//12100
	IDC_BUTTON_REFERENCE,			HIDC_EXEC_BUTTON_REFERENCE,		// �Q��
	IDOK,							HIDOK_EXEC,						// ���s
	IDCANCEL,						HIDCANCEL_EXEC,					// �L�����Z��
	IDC_BUTTON_HELP,				HIDC_EXEC_BUTTON_HELP,			// �w���v
	IDC_CHECK_GETSTDOUT,			HIDC_EXEC_CHECK_GETSTDOUT,		// �W���o�͂𓾂�
	IDC_COMBO_CODE_GET,				HIDC_COMBO_CODE_GET,			// �W���o�͕����R�[�h
	IDC_COMBO_m_szCommand,			HIDC_EXEC_COMBO_m_szCommand,	// �R�}���h
	IDC_RADIO_OUTPUT,				HIDC_RADIO_OUTPUT,				// �W���o�̓��_�C���N�g��F�A�E�g�v�b�g�E�B���h�E
	IDC_RADIO_EDITWINDOW,			HIDC_RADIO_EDITWINDOW,			// �W���o�̓��_�C���N�g��F�ҏW���̃E�B���h�E
	IDC_CHECK_SENDSTDIN,			HIDC_CHECK_SENDSTDIN,			// �W�����͂ɑ���
	IDC_COMBO_CODE_SEND,			HIDC_COMBO_CODE_SEND,			// �W���o�͕����R�[�h
	IDC_CHECK_CUR_DIR,				HIDC_CHECK_CUR_DIR,				// �J�����g�f�B���N�g��
	IDC_COMBO_CUR_DIR,				HIDC_COMBO_CUR_DIR,				// �J�����g�f�B���N�g���w��
	IDC_BUTTON_REFERENCE2,			HIDC_COMBO_CUR_DIR,				// �J�����g�f�B���N�g���w��(�Q��)
//	IDC_STATIC,						-1,
	0, 0
};

DlgExec::DlgExec()
{
	szCommand[0] = _T('\0');	// �R�}���h���C��
	return;
}

static const int codeTable1[] = { 0x00, 0x08, 0x80 };
static const int codeTable2[] = { 0x00, 0x10, 0x100 };


// ���[�_���_�C�A���O�̕\��
INT_PTR DlgExec::DoModal(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam)
{
	szCommand[0] = _T('\0');	// �R�}���h���C��
	bEditable = EditDoc::GetInstance(0)->IsEditable();
	return Dialog::DoModal(hInstance, hwndParent, IDD_EXEC, lParam);
}


BOOL DlgExec::OnInitDialog(
	HWND hwnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwnd);
	
	EncodingType codes[] = { CODE_SJIS, CODE_UNICODE, CODE_UTF8 };
	HWND hwndCombo;
	hwndCombo = GetItemHwnd(IDC_COMBO_CODE_GET);
	for (size_t i=0; i<_countof(codes); ++i) {
		Combo_AddString(hwndCombo, CodeTypeName(codes[i]).Normal());
	}
	hwndCombo = GetItemHwnd(IDC_COMBO_CODE_SEND);
	for (size_t i=0; i<_countof(codes); ++i) {
		Combo_AddString(hwndCombo, CodeTypeName(codes[i]).Normal());
	}

	BOOL bRet = Dialog::OnInitDialog(hwnd, wParam, lParam);

	comboDel = ComboBoxItemDeleter();
	comboDel.pRecent = &recentCmd;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_m_szCommand), &comboDel);
	comboDelCur = ComboBoxItemDeleter();
	comboDelCur.pRecent = &recentCur;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_CUR_DIR), &comboDelCur);
	return bRet;
}

// �_�C�A���O�f�[�^�̐ݒ�
void DlgExec::SetData(void)
{
//	MYTRACE(_T("DlgExec::SetData()"));
	/*****************************
	*           ����             *
	*****************************/
	// ���[�U�[���R���{ �{�b�N�X�̃G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����
	Combo_LimitText(GetItemHwnd(IDC_COMBO_m_szCommand), _countof(szCommand) - 1);
	Combo_LimitText(GetItemHwnd(IDC_COMBO_CUR_DIR), _countof2(szCurDir) - 1);
	// �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ���
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_m_szCommand), TRUE);

	{	// From Here 2007.01.02 maru �������g���̂���
		// �}�N������̌Ăяo���ł�ShareData�ɕۑ������Ȃ��悤�ɁCShareData�Ƃ̎󂯓n����ExecCmd�̊O��
		int nExecFlgOpt;
		nExecFlgOpt = pShareData->nExecFlgOpt;
		
		// �r���[���[�h��㏑���֎~�̂Ƃ��͕ҏW���E�B���h�E�ւ͏o�͂��Ȃ�	
		if (!bEditable) {
			nExecFlgOpt &= ~0x02;
		}

		CheckButton(IDC_CHECK_GETSTDOUT,	(nExecFlgOpt & 0x01) ? true : false);
		CheckButton(IDC_RADIO_OUTPUT,		(nExecFlgOpt & 0x02) ? false : true);
		CheckButton(IDC_RADIO_EDITWINDOW,	(nExecFlgOpt & 0x02) ? true : false);
		CheckButton(IDC_CHECK_SENDSTDIN,	(nExecFlgOpt & 0x04) ? true : false);
		CheckButton(IDC_CHECK_CUR_DIR,		(nExecFlgOpt & 0x200) ? true : false);

		EnableItem(IDC_RADIO_OUTPUT,		(nExecFlgOpt & 0x01) ? true : false);
		EnableItem(IDC_RADIO_EDITWINDOW,	((nExecFlgOpt & 0x01) && bEditable)? true : false);
		EnableItem(IDC_COMBO_CODE_GET,		(nExecFlgOpt & 0x01) ? true : false);		// �W���o��Off���AUnicode���g�p�����Desable����	2008/6/20 Uchi
		EnableItem(IDC_COMBO_CODE_SEND,		(nExecFlgOpt & 0x04) ? true : false);		// �W������Off���AUnicode���g�p�����Desable����	2008/6/20 Uchi
		EnableItem(IDC_COMBO_CUR_DIR,		(nExecFlgOpt & 0x200) ? true : false);
		EnableItem(IDC_BUTTON_REFERENCE2,	(nExecFlgOpt & 0x200) ? true : false);
	}

	/*****************************
	*         �f�[�^�ݒ�         *
	*****************************/
	_tcscpy(szCommand, pShareData->history.aCommands[0]);
	HWND hwndCombo = GetItemHwnd(IDC_COMBO_m_szCommand);
	Combo_ResetContent(hwndCombo);
	SetItemText(IDC_COMBO_TEXT, szCommand);
	size_t nSize = pShareData->history.aCommands.size();
	for (size_t i=0; i<nSize; ++i) {
		Combo_AddString(hwndCombo, pShareData->history.aCommands[i]);
	}
	Combo_SetCurSel(hwndCombo, 0);

	_tcscpy(szCurDir, pShareData->history.aCurDirs[0]);
	hwndCombo = GetItemHwnd(IDC_COMBO_CUR_DIR);
	Combo_ResetContent(hwndCombo);
	SetItemText(IDC_COMBO_TEXT, szCurDir);
	for (size_t i=0; i<pShareData->history.aCurDirs.size(); ++i) {
		Combo_AddString(hwndCombo, pShareData->history.aCurDirs[i]);
	}
	Combo_SetCurSel(hwndCombo, 0);
	
	int nOpt;
	hwndCombo = GetItemHwnd(IDC_COMBO_CODE_GET);
	nOpt = pShareData->nExecFlgOpt & 0x88;
	for (size_t i=0; _countof(codeTable1); ++i) {
		if (codeTable1[i] == nOpt) {
			Combo_SetCurSel(hwndCombo, i);
			break;
		}
	}
	hwndCombo = GetItemHwnd(IDC_COMBO_CODE_SEND);
	nOpt = pShareData->nExecFlgOpt & 0x110;
	for (size_t i=0; _countof(codeTable2); ++i) {
		if (codeTable2[i] == nOpt) {
			Combo_SetCurSel(hwndCombo, i);
			break;
		}
	}
	return;
}


// �_�C�A���O�f�[�^�̎擾
int DlgExec::GetData(void)
{
	GetItemText(IDC_COMBO_m_szCommand, szCommand, _countof(szCommand));
	if (IsButtonChecked(IDC_CHECK_CUR_DIR)) {
		GetItemText(IDC_COMBO_CUR_DIR, &szCurDir[0], _countof2(szCurDir));
	}else {
		szCurDir[0] = _T('\0');
	}
	
	{
		// �}�N������̌Ăяo���ł�ShareData�ɕۑ������Ȃ��悤�ɁCShareData�Ƃ̎󂯓n����ExecCmd�̊O��
		int nFlgOpt = 0;
		nFlgOpt |= (IsButtonChecked(IDC_CHECK_GETSTDOUT)) ? 0x01 : 0;	// �W���o�͂𓾂�
		nFlgOpt |= (IsButtonChecked(IDC_RADIO_EDITWINDOW)) ? 0x02 : 0;	// �W���o�͂�ҏW���̃E�B���h�E��
		nFlgOpt |= (IsButtonChecked(IDC_CHECK_SENDSTDIN)) ? 0x04 : 0;	// �ҏW���t�@�C����W�����͂�
		nFlgOpt |= (IsButtonChecked(IDC_CHECK_CUR_DIR)) ? 0x200 : 0;	// �J�����g�f�B���N�g���w��
		int sel;
		sel = Combo_GetCurSel(GetItemHwnd(IDC_COMBO_CODE_GET));
		nFlgOpt |= codeTable1[sel];
		sel = Combo_GetCurSel(GetItemHwnd(IDC_COMBO_CODE_SEND));
		nFlgOpt |= codeTable2[sel];
		pShareData->nExecFlgOpt = nFlgOpt;
	}
	return 1;
}


BOOL DlgExec::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_CHECK_GETSTDOUT:
		{
			bool bEnabled = IsButtonChecked(IDC_CHECK_GETSTDOUT);
			EnableItem(IDC_RADIO_OUTPUT, bEnabled);
			EnableItem(IDC_RADIO_EDITWINDOW, bEnabled && bEditable);	// �r���[���[�h��㏑���֎~�̏����ǉ�	// 2009.02.21 ryoji
		}

		// �W���o��Off���AUnicode���g�p�����Desable����	
		EnableItem(IDC_COMBO_CODE_GET, IsButtonChecked(IDC_CHECK_GETSTDOUT));
		break;
	case IDC_CHECK_SENDSTDIN:	// �W������Off���AUnicode���g�p�����Desable����
		EnableItem(IDC_COMBO_CODE_SEND, IsButtonChecked(IDC_CHECK_SENDSTDIN));
		break;
	case IDC_CHECK_CUR_DIR:
		EnableItem(IDC_COMBO_CUR_DIR, IsButtonChecked(IDC_CHECK_CUR_DIR));
		EnableItem(IDC_BUTTON_REFERENCE2, IsButtonChecked(IDC_CHECK_CUR_DIR));
		break;

	case IDC_BUTTON_HELP:
		//�u�����v�̃w���v
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_EXECMD_DIALOG));
		break;

	case IDC_BUTTON_REFERENCE:	// �t�@�C�����́u�Q��...�v�{�^��
		{
			DlgOpenFile	dlgOpenFile;
			TCHAR		szPath[_MAX_PATH + 1];
			int			size = _countof(szPath) - 1;
			_tcsncpy(szPath, szCommand, size);
			szPath[size] = _T('\0');
			// �t�@�C���I�[�v���_�C�A���O�̏�����
			dlgOpenFile.Create(
				hInstance,
				GetHwnd(),
				_T("*.com;*.exe;*.bat;*.cmd"),
				szCommand
			);
			if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
				_tcscpy(szCommand, szPath);
				SetItemText(IDC_COMBO_m_szCommand, szCommand);
			}
		}
		return TRUE;

	case IDC_BUTTON_REFERENCE2:
		{
			if (SelectDir(GetHwnd(), LS(STR_DLGEXEC_SELECT_CURDIR), &szCurDir[0], &szCurDir[0])) {
				SetItemText(IDC_COMBO_CUR_DIR, &szCurDir[0]);
			}
		}
		return TRUE;

	case IDOK:			// ������
		// �_�C�A���O�f�[�^�̎擾
		GetData();
		CloseDialog(1);
		return TRUE;
	case IDCANCEL:
		CloseDialog(0);
		return TRUE;
	}
	return FALSE;
}

LPVOID DlgExec::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


