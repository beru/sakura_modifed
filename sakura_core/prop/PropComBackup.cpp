/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�o�b�N�A�b�v�v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, jepro
	Copyright (C) 2001, MIK, asa-o, genta, jepro
	Copyright (C) 2002, MIK, YAZAKI, genta, Moca
	Copyright (C) 2003, KEITA
	Copyright (C) 2004, genta
	Copyright (C) 2005, genta, aroka
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10000
	IDC_BUTTON_BACKUP_FOLDER_REF,	HIDC_BUTTON_BACKUP_FOLDER_REF,	// �o�b�N�A�b�v�t�H���_�Q��
	IDC_CHECK_BACKUP,				HIDC_CHECK_BACKUP,				// �o�b�N�A�b�v�̍쐬
	IDC_CHECK_BACKUP_YEAR,			HIDC_CHECK_BACKUP_YEAR,			// �o�b�N�A�b�v�t�@�C�����i����N�j
	IDC_CHECK_BACKUP_MONTH,			HIDC_CHECK_BACKUP_MONTH,		// �o�b�N�A�b�v�t�@�C�����i���j
	IDC_CHECK_BACKUP_DAY,			HIDC_CHECK_BACKUP_DAY,			// �o�b�N�A�b�v�t�@�C�����i���j
	IDC_CHECK_BACKUP_HOUR,			HIDC_CHECK_BACKUP_HOUR,			// �o�b�N�A�b�v�t�@�C�����i���j
	IDC_CHECK_BACKUP_MIN,			HIDC_CHECK_BACKUP_MIN,			// �o�b�N�A�b�v�t�@�C�����i���j
	IDC_CHECK_BACKUP_SEC,			HIDC_CHECK_BACKUP_SEC,			// �o�b�N�A�b�v�t�@�C�����i�b�j
	IDC_CHECK_BACKUPDIALOG,			HIDC_CHECK_BACKUPDIALOG,		// �쐬�O�Ɋm�F
	IDC_CHECK_BACKUPFOLDER,			HIDC_CHECK_BACKUPFOLDER,		// �w��t�H���_�ɍ쐬
	IDC_CHECK_BACKUP_FOLDER_RM,		HIDC_CHECK_BACKUP_FOLDER_RM,	// �w��t�H���_�ɍ쐬(�����[�o�u�����f�B�A�̂�)
	IDC_CHECK_BACKUP_DUSTBOX,		HIDC_CHECK_BACKUP_DUSTBOX,		// �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞��	//@@@ 2001.12.11 add MIK
	IDC_EDIT_BACKUPFOLDER,			HIDC_EDIT_BACKUPFOLDER,			// �ۑ��t�H���_��
	IDC_EDIT_BACKUP_3,				HIDC_EDIT_BACKUP_3,				// ���㐔
	IDC_RADIO_BACKUP_TYPE1,			HIDC_RADIO_BACKUP_TYPE1,		// �o�b�N�A�b�v�̎�ށi�g���q�j
//	IDC_RADIO_BACKUP_TYPE2,			HIDC_RADIO_BACKUP_TYPE2NEWHID,	// �o�b�N�A�b�v�̎�ށi���t�E�����j // 2002.11.09 Moca HID��.._TYPE3�Ƌt������	// Jun.  5, 2004 genta �p�~
	IDC_RADIO_BACKUP_TYPE3,			HIDC_RADIO_BACKUP_TYPE3NEWHID,	// �o�b�N�A�b�v�̎�ށi�A�ԁj// 2002.11.09 Moca HID��.._TYPE2�Ƌt������
	IDC_RADIO_BACKUP_DATETYPE1,		HIDC_RADIO_BACKUP_DATETYPE1,	// �t����������̎�ށi�쐬�����j	// Jul. 05, 2001 JEPRO �ǉ�
	IDC_RADIO_BACKUP_DATETYPE2,		HIDC_RADIO_BACKUP_DATETYPE2,	// �t����������̎�ށi�X�V�����j	// Jul. 05, 2001 JEPRO �ǉ�
	IDC_SPIN_BACKUP_GENS,			HIDC_EDIT_BACKUP_3,				// �ۑ����鐢�㐔�̃X�s��
	IDC_CHECK_BACKUP_RETAINEXT,		HIDC_CHECK_BACKUP_RETAINEXT,	// ���̊g���q��ۑ�	// 2006.08.06 ryoji
	IDC_CHECK_BACKUP_ADVANCED,		HIDC_CHECK_BACKUP_ADVANCED,		// �ڍאݒ�	// 2006.08.06 ryoji
	IDC_EDIT_BACKUPFILE,			HIDC_EDIT_BACKUPFILE,			// �ڍאݒ�̃G�f�B�b�g�{�b�N�X	// 2006.08.06 ryoji
	IDC_RADIO_BACKUP_DATETYPE1A,	HIDC_RADIO_BACKUP_DATETYPE1A,	// �t����������̎�ށi�쐬�����j���ڍאݒ�ON�p	// 2009.02.20 ryoji
	IDC_RADIO_BACKUP_DATETYPE2A,	HIDC_RADIO_BACKUP_DATETYPE2A,	// �t����������̎�ށi�X�V�����j���ڍאݒ�ON�p	// 2009.02.20 ryoji
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK PropBackup::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropBackup::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}
//	To Here Jun. 2, 2001 genta


// ���b�Z�[�W����
INT_PTR PropBackup::DispatchEvent(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
//	int			nVal;
	int			nVal;	// Sept.21, 2000 JEPRO �X�s���v�f���������̂ŕ���������
//	int			nDummy;
//	int			nCharChars;

	auto& csBackup = m_common.backup;

	switch (uMsg) {
	case WM_INITDIALOG:
		// �_�C�A���O�f�[�^�̐ݒ� Backup
		SetData(hwndDlg);
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����
		//	Oct. 5, 2002 genta �o�b�N�A�b�v�t�H���_���̓��̓T�C�Y���w��
		//	Oct. 8, 2002 genta �Ō�ɕt�������\�̗̈���c�����߃o�b�t�@�T�C�Y-1�������͂����Ȃ�
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_BACKUPFOLDER), _countof2(csBackup.szBackUpFolder) - 1 - 1);
		// 20051107 aroka
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_BACKUPFILE), _countof2(csBackup.szBackUpPathAdvanced) - 1 - 1);
		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch (idCtrl) {
		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_BACKUP);
				return TRUE;
			case PSN_KILLACTIVE:
				// �_�C�A���O�f�[�^�̎擾 Backup
				GetData(hwndDlg);
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = ID_PROPCOM_PAGENUM_BACKUP;
				return TRUE;
			}
			break;

		case IDC_SPIN_BACKUP_GENS:
			// �o�b�N�A�b�v�t�@�C���̐��㐔
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_BACKUP_3, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 99) {
				nVal = 99;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_BACKUP_3, nVal, FALSE);
			return TRUE;
		}
//****	To Here Sept. 21, 2000 JEPRO �_�C�A���O�v�f�ɃX�s��������̂ňȉ���WM_NOTIFY���R�����g�A�E�g�ɂ����ɏC����u����
		break;

	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// �ʒm�R�[�h
		wID			= LOWORD(wParam);	// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID
		switch (wNotifyCode) {
		// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
		case BN_CLICKED:
			switch (wID) {
			case IDC_RADIO_BACKUP_TYPE1:
				//	Aug. 16, 2000 genta
				//	�o�b�N�A�b�v�����ǉ�
			case IDC_RADIO_BACKUP_TYPE3:
			case IDC_CHECK_BACKUP:
			case IDC_CHECK_BACKUPFOLDER:
				//	Aug. 21, 2000 genta
			case IDC_CHECK_AUTOSAVE:
			//	Jun.  5, 2004 genta IDC_RADIO_BACKUP_TYPE2��p�~���āC
			//	IDC_RADIO_BACKUP_DATETYPE1, IDC_RADIO_BACKUP_DATETYPE2�𓯗�Ɏ����Ă���
			case IDC_RADIO_BACKUP_DATETYPE1:
			case IDC_RADIO_BACKUP_DATETYPE2:
			// 20051107 aroka
			case IDC_CHECK_BACKUP_ADVANCED:
				GetData(hwndDlg);
				UpdateBackupFile(hwndDlg);
				EnableBackupInput(hwndDlg);
				return TRUE;
			case IDC_BUTTON_BACKUP_FOLDER_REF:	// �t�H���_�Q��
				{
					// �o�b�N�A�b�v���쐬����t�H���_
					TCHAR szFolder[_MAX_PATH];
					::DlgItem_GetText(hwndDlg, IDC_EDIT_BACKUPFOLDER, szFolder, _countof(szFolder));

					if (SelectDir(hwndDlg, LS(STR_PROPCOMBK_SEL_FOLDER), szFolder, szFolder)) {
						_tcscpy(csBackup.szBackUpFolder, szFolder);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_BACKUPFOLDER, csBackup.szBackUpFolder);
					}
					UpdateBackupFile(hwndDlg);
				}
				return TRUE;
			default: // 20051107 aroka Default�� �ǉ�
				GetData(hwndDlg);
				UpdateBackupFile(hwndDlg);
			}
			break;	// BN_CLICKED
		case EN_CHANGE: // 20051107 aroka �t�H���_���ύX���ꂽ�烊�A���^�C���ɃG�f�B�b�g�{�b�N�X�����X�V
			switch (wID) {
			case IDC_EDIT_BACKUPFOLDER:
				// 2009.02.21 ryoji ����\���ǉ������̂ŁC1�����]�T���݂�K�v������D
				::DlgItem_GetText(hwndDlg, IDC_EDIT_BACKUPFOLDER, csBackup.szBackUpFolder, _countof2(csBackup.szBackUpFolder) - 1);
				UpdateBackupFile(hwndDlg);
				break;
			}
			break;	// EN_CHANGE
		}
		break;	// WM_COMMAND

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		// NOTREACHED
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}


/*! �_�C�A���O�f�[�^�̐ݒ�
	@date 2004.06.05 genta ���̊g���q���c���ݒ��ǉ��D
		�����w��Ń`�F�b�N�{�b�N�X���󗓂Ŏc��Ɛݒ肳��Ȃ���������邽�߁C
		IDC_RADIO_BACKUP_TYPE2
		��p�~���ă��C�A�E�g�ύX
*/
void PropBackup::SetData(HWND hwndDlg)
{
//	BOOL	bRet;

//	BOOL	bGrepExitConfirm;	// Grep���[�h�ŕۑ��m�F���邩

	auto& csBackup = m_common.backup;

	// �o�b�N�A�b�v�̍쐬
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP, csBackup.bBackUp);
	// �o�b�N�A�b�v�̍쐬�O�Ɋm�F
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUPDIALOG, csBackup.bBackUpDialog);
//	// �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� //	20051107 aroka �u�o�b�N�A�b�v�̍쐬�v�ɘA��������
//	::CheckDlgButton(hwndDlg, IDC_CHECK_BACKUPFOLDER, .backup.bBackUpFolder);

	// �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.*
	//	Jun.  5, 2004 genta ���̊g���q���c���ݒ�(5,6)��ǉ��D
	switch (csBackup.GetBackupType()) {
	case 2:
		::CheckDlgButton(hwndDlg, IDC_RADIO_BACKUP_DATETYPE1, 1);	// �t��������t�̃^�C�v(������)
		break;
	case 3:
	case 6:
		::CheckDlgButton(hwndDlg, IDC_RADIO_BACKUP_TYPE3, 1);
		break;
	case 4:
		::CheckDlgButton(hwndDlg, IDC_RADIO_BACKUP_DATETYPE2, 1);	// �t��������t�̃^�C�v(�O��̕ۑ�����)
		break;
	case 5:
	case 1:
	default:
		::CheckDlgButton(hwndDlg, IDC_RADIO_BACKUP_TYPE1, 1);
		break;
	}
	
	//	Jun.  5, 2004 genta ���̊g���q���c���ݒ�(5,6)��ǉ��D
	::CheckDlgButton(hwndDlg, IDC_CHECK_BACKUP_RETAINEXT,
		(csBackup.GetBackupType() == 5 || csBackup.GetBackupType() == 6) ? 1 : 0
	);

	// �o�b�N�A�b�v�t�@�C�����F���t�̔N
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP_YEAR, csBackup.GetBackupOpt(BKUP_YEAR));
	// �o�b�N�A�b�v�t�@�C�����F���t�̌�
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP_MONTH, csBackup.GetBackupOpt(BKUP_MONTH));
	// �o�b�N�A�b�v�t�@�C�����F���t�̓�
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP_DAY, csBackup.GetBackupOpt(BKUP_DAY));
	// �o�b�N�A�b�v�t�@�C�����F���t�̎�
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP_HOUR, csBackup.GetBackupOpt(BKUP_HOUR));
	// �o�b�N�A�b�v�t�@�C�����F���t�̕�
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP_MIN, csBackup.GetBackupOpt(BKUP_MIN));
	// �o�b�N�A�b�v�t�@�C�����F���t�̕b
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP_SEC, csBackup.GetBackupOpt(BKUP_SEC));

	// �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� // 20051107 aroka �ړ��F�A���Ώۂɂ���B
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUPFOLDER, csBackup.bBackUpFolder);
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP_FOLDER_RM, csBackup.bBackUpFolderRM);	// 2010/5/27 Uchi

	// �o�b�N�A�b�v���쐬����t�H���_
	::DlgItem_SetText(hwndDlg, IDC_EDIT_BACKUPFOLDER, csBackup.szBackUpFolder);

	// �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞��	//@@@ 2001.12.11 add MIK
	::CheckDlgButton(hwndDlg, IDC_CHECK_BACKUP_DUSTBOX, csBackup.bBackUpDustBox ? BST_CHECKED : BST_UNCHECKED);	//@@@ 2001.12.11 add MIK

	// �o�b�N�A�b�v��t�H���_���ڍאݒ肷�� // 20051107 aroka
	::CheckDlgButton(hwndDlg, IDC_CHECK_BACKUP_ADVANCED, csBackup.bBackUpPathAdvanced ? BST_CHECKED : BST_UNCHECKED);

	// �o�b�N�A�b�v���쐬����t�H���_�̏ڍאݒ� // 20051107 aroka
	::DlgItem_SetText(hwndDlg, IDC_EDIT_BACKUPFILE, csBackup.szBackUpPathAdvanced);

	// �o�b�N�A�b�v���쐬����t�H���_�̏ڍאݒ� // 20051128 aroka
	switch (csBackup.GetBackupTypeAdv()) {
	case 2:
		::CheckDlgButton(hwndDlg, IDC_RADIO_BACKUP_DATETYPE1A, 1);	// �t��������t�̃^�C�v(������)
		break;
	case 4:
		::CheckDlgButton(hwndDlg, IDC_RADIO_BACKUP_DATETYPE2A, 1);	// �t��������t�̃^�C�v(�O��̕ۑ�����)
		break;
	default:
		::CheckDlgButton(hwndDlg, IDC_RADIO_BACKUP_DATETYPE1A, 1);
		break;
	}

	//	From Here Aug. 16, 2000 genta
	int nN = csBackup.GetBackupCount();
	nN = nN < 1  ?  1 : nN;
	nN = nN > 99 ? 99 : nN;

	::SetDlgItemInt(hwndDlg, IDC_EDIT_BACKUP_3, nN, FALSE);	//	Oct. 29, 2001 genta
	//	To Here Aug. 16, 2000 genta

	UpdateBackupFile(hwndDlg);

	EnableBackupInput(hwndDlg);
	return;
}


// �_�C�A���O�f�[�^�̎擾
int PropBackup::GetData(HWND hwndDlg)
{
	auto& csBackup = m_common.backup;

	// �o�b�N�A�b�v�̍쐬
	csBackup.bBackUp = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP);
	// �o�b�N�A�b�v�̍쐬�O�Ɋm�F
	csBackup.bBackUpDialog = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUPDIALOG);
//	// �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� // 20051107 aroka �u�o�b�N�A�b�v�̍쐬�v�ɘA��������
//	csBackup.bBackUpFolder = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUPFOLDER);

	// �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.*
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_TYPE1)) {
		//	Jun.  5, 2005 genta �g���q���c���p�^�[����ǉ�
		if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_RETAINEXT)) {
			csBackup.SetBackupType(5);
		}else {
			csBackup.SetBackupType(1);
		}
	}
//	if (::DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_TYPE2)) {
		// 2001/06/05 Start by asa-o: ���t�̃^�C�v
		if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE1)) {
			csBackup.SetBackupType(2);	// ������
		}
		if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE2)) {
			csBackup.SetBackupType(4);	// �O��̕ۑ�����
		}
		// 2001/06/05 End
//	}

	//	Aug. 16, 2000 genta
	//	3 = *.b??
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_TYPE3)) {
		if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_RETAINEXT)) {
			csBackup.SetBackupType(6);
		}else {
			csBackup.SetBackupType(3);
		}
	}

	// �o�b�N�A�b�v�t�@�C�����F���t�̔N
	csBackup.SetBackupOpt(BKUP_YEAR, DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_YEAR));
	// �o�b�N�A�b�v�t�@�C�����F���t�̌�
	csBackup.SetBackupOpt(BKUP_MONTH, DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_MONTH));
	// �o�b�N�A�b�v�t�@�C�����F���t�̓�
	csBackup.SetBackupOpt(BKUP_DAY, DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_DAY));
	// �o�b�N�A�b�v�t�@�C�����F���t�̎�
	csBackup.SetBackupOpt(BKUP_HOUR, DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_HOUR));
	// �o�b�N�A�b�v�t�@�C�����F���t�̕�
	csBackup.SetBackupOpt(BKUP_MIN, DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_MIN));
	// �o�b�N�A�b�v�t�@�C�����F���t�̕b
	csBackup.SetBackupOpt(BKUP_SEC, DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_SEC));

	// �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� // 20051107 aroka �ړ�
	csBackup.bBackUpFolder = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUPFOLDER);
	csBackup.bBackUpFolderRM = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_FOLDER_RM);	// 2010/5/27 Uchi

	// �o�b�N�A�b�v���쐬����t�H���_
	//	Oct. 5, 2002 genta �T�C�Y��sizeof()�Ŏw��
	//	Oct. 8, 2002 genta ����\���ǉ������̂ŁC1�����]�T������K�v������D
	::DlgItem_GetText(hwndDlg, IDC_EDIT_BACKUPFOLDER, csBackup.szBackUpFolder, _countof2(csBackup.szBackUpFolder) - 1);

	// �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞��	//@@@ 2001.12.11 add MIK
	csBackup.bBackUpDustBox = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_DUSTBOX);	//@@@ 2001.12.11 add MIK

	// �w��t�H���_�Ƀo�b�N�A�b�v���쐬����ڍאݒ� // 20051107 aroka
	csBackup.bBackUpPathAdvanced = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_ADVANCED);
	// �o�b�N�A�b�v���쐬����t�H���_ // 20051107 aroka
	::DlgItem_GetText(hwndDlg, IDC_EDIT_BACKUPFILE, csBackup.szBackUpPathAdvanced, _countof2(csBackup.szBackUpPathAdvanced) - 1);

	// 20051128 aroka �ڍאݒ�̓��t�̃^�C�v
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE1A)) {
		csBackup.SetBackupTypeAdv(2);	// ������
	}
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE2A)) {
		csBackup.SetBackupTypeAdv(4);	// �O��̕ۑ�����
	}

	//	From Here Aug. 16, 2000 genta
	//	���㐔�̎擾
	int	 nN;
	nN = ::GetDlgItemInt(hwndDlg, IDC_EDIT_BACKUP_3, NULL, FALSE);	//	Oct. 29, 2001 genta

//	for (nN=0, pDigit=szNumBuf; *pDigit!='\0'; ++pDigit) {
//		if ('0' <= *pDigit && *pDigit <= '9') {
//			nN = nN * 10 + *pDigit - '0';
//		}
//		else
//			break;
//	}
	nN = nN < 1  ?  1 : nN;
	nN = nN > 99 ? 99 : nN;
	csBackup.SetBackupCount(nN);
	//	To Here Aug. 16, 2000 genta

	return TRUE;
}

//	From Here Aug. 16, 2000 genta
/*!	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
	�K�؂ɐݒ肷��

	@date 2004.06.05 genta ���̊g���q���c���ݒ��ǉ��D
		�����w��Ń`�F�b�N�{�b�N�X���󗓂Ŏc��Ɛݒ肳��Ȃ���������邽�߁C
		IDC_RADIO_BACKUP_TYPE2
		��p�~���ă��C�A�E�g�ύX
	@date 2005.11.07 aroka ���C�A�E�g�ɍ��킹�ď��������ւ��A�C���f���g�𐮗�
	@date 2005.11.21 aroka �ڍאݒ胂�[�h�̐����ǉ�
	@date 2009.02.20 ryoji IDC_LABEL_BACKUP_HELP�ɂ��ʃR���g���[���B����p�~�Aif�������ShowEnable�t���O����ɒu�������Ċȑf�����ĉ��L���C���B
	                       �EVista Aero���Əڍאݒ�ON�ɂ��Ă��ڍאݒ�OFF���ڂ���ʂ�������Ȃ�
	                       �E�ڍאݒ�OFF���ڂ���\���ł͂Ȃ������̂ŉB��Ă��Ă�Tooltip�w���v���\�������
	                       �E�ڍאݒ�ON�Ȃ̂Ƀo�b�N�A�b�v�쐬OFF���Əڍאݒ�OFF���ڂ̂ق����\�������
*/
static inline
void ShowEnable(
	HWND hWnd,
	BOOL bShow,
	BOOL bEnable
	)
{
	::ShowWindow(hWnd, bShow? SW_SHOW: SW_HIDE);
	::EnableWindow(hWnd, bEnable && bShow);		// bShow=false,bEnable=true�̏ꍇ�V���[�g�J�b�g�L�[���ςȓ���������̂ŏC��	2010/5/27 Uchi
}

void PropBackup::EnableBackupInput(HWND hwndDlg)
{
	#define SHOWENABLE(id, show, enable) ShowEnable(::GetDlgItem(hwndDlg, id), show, enable)

	bool bBackup = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP);
	bool bAdvanced = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_ADVANCED);
	bool bType1 = DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_TYPE1);
	//bool bType2 = DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_TYPE2);
	bool bType3 = DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_TYPE3);
	bool bDate1 = DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE1);
	bool bDate2 = DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE2);
	bool bFolder = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUPFOLDER);

	SHOWENABLE(IDC_CHECK_BACKUP_ADVANCED,	TRUE, bBackup);	// 20050628 aroka
	SHOWENABLE(IDC_RADIO_BACKUP_TYPE1,		!bAdvanced, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_TYPE3,		!bAdvanced, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_DATETYPE1,	!bAdvanced, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_DATETYPE2,	!bAdvanced, bBackup);
	SHOWENABLE(IDC_LABEL_BACKUP_3,			!bAdvanced, bBackup && bType3);
	SHOWENABLE(IDC_EDIT_BACKUP_3,			!bAdvanced, bBackup && bType3);
	SHOWENABLE(IDC_SPIN_BACKUP_GENS,		!bAdvanced, bBackup && bType3);	//	20051107 aroka �ǉ�
	SHOWENABLE(IDC_CHECK_BACKUP_RETAINEXT,	!bAdvanced, bBackup && (bType1 || bType3));	//	Jun.  5, 2005 genta �ǉ�
	SHOWENABLE(IDC_CHECK_BACKUP_YEAR,		!bAdvanced, bBackup && (bDate1 || bDate2));
	SHOWENABLE(IDC_CHECK_BACKUP_MONTH,		!bAdvanced, bBackup && (bDate1 || bDate2));
	SHOWENABLE(IDC_CHECK_BACKUP_DAY,		!bAdvanced, bBackup && (bDate1 || bDate2));
	SHOWENABLE(IDC_CHECK_BACKUP_HOUR,		!bAdvanced, bBackup && (bDate1 || bDate2));
	SHOWENABLE(IDC_CHECK_BACKUP_MIN,		!bAdvanced, bBackup && (bDate1 || bDate2));
	SHOWENABLE(IDC_CHECK_BACKUP_SEC,		!bAdvanced, bBackup && (bDate1 || bDate2));

	// �ڍאݒ�
	SHOWENABLE(IDC_EDIT_BACKUPFILE,			TRUE, bBackup && bAdvanced);
//	SHOWENABLE(IDC_LABEL_BACKUP_HELP,		bAdvanced, bBackup);	// �s���̂܂ܕ��u�i���R���g���[���B���̕����͔p�~�j 2009.02.20 ryoji
	SHOWENABLE(IDC_LABEL_BACKUP_HELP2,		bAdvanced, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_DATETYPE1A,	bAdvanced, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_DATETYPE2A,	bAdvanced, bBackup);

	SHOWENABLE(IDC_CHECK_BACKUPFOLDER,			TRUE, bBackup);
	SHOWENABLE(IDC_LABEL_BACKUP_4,				TRUE, bBackup && bFolder);	// added Sept. 6, JEPRO �t�H���_�w�肵���Ƃ�����Enable�ɂȂ�悤�ɕύX
	SHOWENABLE(IDC_CHECK_BACKUP_FOLDER_RM,		TRUE, bBackup && bFolder);	// 2010/5/27 Uchi
	SHOWENABLE(IDC_EDIT_BACKUPFOLDER,			TRUE, bBackup && bFolder);
	SHOWENABLE(IDC_BUTTON_BACKUP_FOLDER_REF,	TRUE, bBackup && bFolder);
	SHOWENABLE(IDC_CHECK_BACKUP_DUSTBOX,		TRUE, bBackup);	//@@@ 2001.12.11 add MIK

	// �쐬�O�Ɋm�F
	SHOWENABLE(IDC_CHECK_BACKUPDIALOG,		TRUE, bBackup);

	#undef SHOWENABLE
}
//	To Here Aug. 16, 2000 genta


/*!	�o�b�N�A�b�v�t�@�C���̏ڍאݒ�G�f�B�b�g�{�b�N�X��K�؂ɍX�V����

	@date 2005.11.07 aroka �V�K�ǉ�

	@note �ڍאݒ�؂�ւ����̃f�t�H���g���I�v�V�����ɍ��킹�邽�߁A
		szBackUpPathAdvanced ���X�V����
*/
void PropBackup::UpdateBackupFile(HWND hwndDlg)	//	�o�b�N�A�b�v�t�@�C���̏ڍאݒ�
{
	wchar_t temp[MAX_PATH];
	auto& csBackup = m_common.backup;
	// �o�b�N�A�b�v���쐬����t�@�C�� // 20051107 aroka
	if (!csBackup.bBackUp) {
		temp[0] = 0;
	}else {
		if (csBackup.bBackUpFolder) {
			temp[0] = 0;
		}else if (csBackup.bBackUpDustBox) {
			auto_sprintf_s(temp, LTEXT("%ls\\"), LSW(STR_PROPCOMBK_DUSTBOX));
		}else {
			auto_sprintf_s(temp, LTEXT(".\\"));
		}

		switch (csBackup.GetBackupType()) {
		case 1: // .bak
			wcscat(temp, LTEXT("$0.bak"));
			break;
		case 5: // .*.bak
			wcscat(temp, LTEXT("$0.*.bak"));
			break;
		case 3: // .b??
			wcscat(temp, LTEXT("$0.b??"));
			break;
		case 6: // .*.b??
			wcscat(temp, LTEXT("$0.*.b??"));
			break;
		case 2:	//	���t�C����
		case 4:	//	���t�C����
			wcscat(temp, LTEXT("$0_"));

			if (csBackup.GetBackupOpt(BKUP_YEAR)) {	// �o�b�N�A�b�v�t�@�C�����F���t�̔N
				wcscat(temp, LTEXT("%Y"));
			}
			if (csBackup.GetBackupOpt(BKUP_MONTH)) {	// �o�b�N�A�b�v�t�@�C�����F���t�̌�
				wcscat(temp, LTEXT("%m"));
			}
			if (csBackup.GetBackupOpt(BKUP_DAY)) {	// �o�b�N�A�b�v�t�@�C�����F���t�̓�
				wcscat(temp, LTEXT("%d"));
			}
			if (csBackup.GetBackupOpt(BKUP_HOUR)) {	// �o�b�N�A�b�v�t�@�C�����F���t�̎�
				wcscat(temp, LTEXT("%H"));
			}
			if (csBackup.GetBackupOpt(BKUP_MIN)) {	// �o�b�N�A�b�v�t�@�C�����F���t�̕�
				wcscat(temp, LTEXT("%M"));
			}
			if (csBackup.GetBackupOpt(BKUP_SEC)) {	// �o�b�N�A�b�v�t�@�C�����F���t�̕b
				wcscat(temp, LTEXT("%S"));
			}

			wcscat(temp, LTEXT(".*"));
			break;
		default:
			break;
		}
	}
	if (!csBackup.bBackUpPathAdvanced) {	// �ڍאݒ胂�[�h�łȂ��Ƃ����������X�V����
		auto_sprintf(csBackup.szBackUpPathAdvanced, _T("%ls"), temp);
		::DlgItem_SetText(hwndDlg, IDC_EDIT_BACKUPFILE, csBackup.szBackUpPathAdvanced);
	}
	return;
}

