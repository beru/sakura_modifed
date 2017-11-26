/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�o�b�N�A�b�v�v�y�[�W
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

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
	IDC_CHECK_BACKUP_DUSTBOX,		HIDC_CHECK_BACKUP_DUSTBOX,		// �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞��
	IDC_EDIT_BACKUPFOLDER,			HIDC_EDIT_BACKUPFOLDER,			// �ۑ��t�H���_��
	IDC_EDIT_BACKUP_3,				HIDC_EDIT_BACKUP_3,				// ���㐔
	IDC_RADIO_BACKUP_TYPE1,			HIDC_RADIO_BACKUP_TYPE1,		// �o�b�N�A�b�v�̎�ށi�g���q�j
	IDC_RADIO_BACKUP_TYPE3,			HIDC_RADIO_BACKUP_TYPE3NEWHID,	// �o�b�N�A�b�v�̎�ށi�A�ԁj
	IDC_RADIO_BACKUP_DATETYPE1,		HIDC_RADIO_BACKUP_DATETYPE1,	// �t����������̎�ށi�쐬�����j
	IDC_RADIO_BACKUP_DATETYPE2,		HIDC_RADIO_BACKUP_DATETYPE2,	// �t����������̎�ށi�X�V�����j
	IDC_SPIN_BACKUP_GENS,			HIDC_EDIT_BACKUP_3,				// �ۑ����鐢�㐔�̃X�s��
	IDC_CHECK_BACKUP_RETAINEXT,		HIDC_CHECK_BACKUP_RETAINEXT,	// ���̊g���q��ۑ�
	IDC_CHECK_BACKUP_ADVANCED,		HIDC_CHECK_BACKUP_ADVANCED,		// �ڍאݒ�
	IDC_EDIT_BACKUPFILE,			HIDC_EDIT_BACKUPFILE,			// �ڍאݒ�̃G�f�B�b�g�{�b�N�X
	IDC_RADIO_BACKUP_DATETYPE1A,	HIDC_RADIO_BACKUP_DATETYPE1A,	// �t����������̎�ށi�쐬�����j���ڍאݒ�ON�p
	IDC_RADIO_BACKUP_DATETYPE2A,	HIDC_RADIO_BACKUP_DATETYPE2A,	// �t����������̎�ށi�X�V�����j���ڍאݒ�ON�p
//	IDC_STATIC,						-1,
	0, 0
};

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
	int			nVal;

	auto& csBackup = common.backup;

	switch (uMsg) {
	case WM_INITDIALOG:
		// �_�C�A���O�f�[�^�̐ݒ� Backup
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_BACKUPFOLDER), _countof2(csBackup.szBackUpFolder) - 1 - 1);
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
			case PSN_SETACTIVE:
				nPageNum = ID_PROPCOM_PAGENUM_BACKUP;
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
		break;

	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// �ʒm�R�[�h
		wID			= LOWORD(wParam);	// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID
		switch (wNotifyCode) {
		// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
		case BN_CLICKED:
			switch (wID) {
			case IDC_RADIO_BACKUP_TYPE1:
			case IDC_RADIO_BACKUP_TYPE3:
			case IDC_CHECK_BACKUP:
			case IDC_CHECK_BACKUPFOLDER:
			case IDC_CHECK_AUTOSAVE:
			case IDC_RADIO_BACKUP_DATETYPE1:
			case IDC_RADIO_BACKUP_DATETYPE2:
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
			default:
				GetData(hwndDlg);
				UpdateBackupFile(hwndDlg);
			}
			break;	// BN_CLICKED
		case EN_CHANGE:
			switch (wID) {
			case IDC_EDIT_BACKUPFOLDER:
				// ����\���ǉ������̂ŁC1�����]�T���݂�K�v������D
				::DlgItem_GetText(hwndDlg, IDC_EDIT_BACKUPFOLDER, csBackup.szBackUpFolder, _countof2(csBackup.szBackUpFolder) - 1);
				UpdateBackupFile(hwndDlg);
				break;
			}
			break;	// EN_CHANGE
		}
		break;	// WM_COMMAND

	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);
		}
		return TRUE;
		// NOTREACHED
		//break;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);
		return TRUE;

	}
	return FALSE;
}


/*! �_�C�A���O�f�[�^�̐ݒ� */
void PropBackup::SetData(HWND hwndDlg)
{
//	BOOL	bRet;

//	BOOL	bGrepExitConfirm;	// Grep���[�h�ŕۑ��m�F���邩

	auto& csBackup = common.backup;

	// �o�b�N�A�b�v�̍쐬
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP, csBackup.bBackUp);
	// �o�b�N�A�b�v�̍쐬�O�Ɋm�F
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUPDIALOG, csBackup.bBackUpDialog);
	// �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.*
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

	// �w��t�H���_�Ƀo�b�N�A�b�v���쐬����
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUPFOLDER, csBackup.bBackUpFolder);
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_BACKUP_FOLDER_RM, csBackup.bBackUpFolderRM);

	// �o�b�N�A�b�v���쐬����t�H���_
	::DlgItem_SetText(hwndDlg, IDC_EDIT_BACKUPFOLDER, csBackup.szBackUpFolder);

	// �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞��	
	::CheckDlgButton(hwndDlg, IDC_CHECK_BACKUP_DUSTBOX, csBackup.bBackUpDustBox ? BST_CHECKED : BST_UNCHECKED);

	// �o�b�N�A�b�v��t�H���_���ڍאݒ肷��
	::CheckDlgButton(hwndDlg, IDC_CHECK_BACKUP_ADVANCED, csBackup.bBackUpPathAdvanced ? BST_CHECKED : BST_UNCHECKED);

	// �o�b�N�A�b�v���쐬����t�H���_�̏ڍאݒ�
	::DlgItem_SetText(hwndDlg, IDC_EDIT_BACKUPFILE, csBackup.szBackUpPathAdvanced);

	// �o�b�N�A�b�v���쐬����t�H���_�̏ڍאݒ�
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

	int nN = csBackup.GetBackupCount();
	nN = nN < 1  ?  1 : nN;
	nN = nN > 99 ? 99 : nN;

	::SetDlgItemInt(hwndDlg, IDC_EDIT_BACKUP_3, nN, FALSE);

	UpdateBackupFile(hwndDlg);

	EnableBackupInput(hwndDlg);
	return;
}


// �_�C�A���O�f�[�^�̎擾
int PropBackup::GetData(HWND hwndDlg)
{
	auto& csBackup = common.backup;

	// �o�b�N�A�b�v�̍쐬
	csBackup.bBackUp = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP);
	// �o�b�N�A�b�v�̍쐬�O�Ɋm�F
	csBackup.bBackUpDialog = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUPDIALOG);
//	// �w��t�H���_�Ƀo�b�N�A�b�v���쐬����
//	csBackup.bBackUpFolder = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUPFOLDER);

	// �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.*
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_TYPE1)) {
		if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_RETAINEXT)) {
			csBackup.SetBackupType(5);
		}else {
			csBackup.SetBackupType(1);
		}
	}
		// ���t�̃^�C�v
		if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE1)) {
			csBackup.SetBackupType(2);	// ������
		}
		if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE2)) {
			csBackup.SetBackupType(4);	// �O��̕ۑ�����
		}

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

	// �w��t�H���_�Ƀo�b�N�A�b�v���쐬����
	csBackup.bBackUpFolder = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUPFOLDER);
	csBackup.bBackUpFolderRM = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_FOLDER_RM);

	// �o�b�N�A�b�v���쐬����t�H���_
	::DlgItem_GetText(hwndDlg, IDC_EDIT_BACKUPFOLDER, csBackup.szBackUpFolder, _countof2(csBackup.szBackUpFolder) - 1);

	// �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞��
	csBackup.bBackUpDustBox = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_DUSTBOX);

	// �w��t�H���_�Ƀo�b�N�A�b�v���쐬����ڍאݒ�
	csBackup.bBackUpPathAdvanced = DlgButton_IsChecked(hwndDlg, IDC_CHECK_BACKUP_ADVANCED);
	// �o�b�N�A�b�v���쐬����t�H���_
	::DlgItem_GetText(hwndDlg, IDC_EDIT_BACKUPFILE, csBackup.szBackUpPathAdvanced, _countof2(csBackup.szBackUpPathAdvanced) - 1);

	// �ڍאݒ�̓��t�̃^�C�v
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE1A)) {
		csBackup.SetBackupTypeAdv(2);	// ������
	}
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_BACKUP_DATETYPE2A)) {
		csBackup.SetBackupTypeAdv(4);	// �O��̕ۑ�����
	}

	//	���㐔�̎擾
	int	 nN;
	nN = ::GetDlgItemInt(hwndDlg, IDC_EDIT_BACKUP_3, NULL, FALSE);

	nN = nN < 1  ?  1 : nN;
	nN = nN > 99 ? 99 : nN;
	csBackup.SetBackupCount(nN);

	return TRUE;
}

/*!	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��K�؂ɐݒ肷�� */
static inline
void ShowEnable(
	HWND hWnd,
	BOOL bShow,
	BOOL bEnable
	)
{
	::ShowWindow(hWnd, bShow? SW_SHOW: SW_HIDE);
	::EnableWindow(hWnd, bEnable && bShow);		// bShow=false,bEnable=true�̏ꍇ�V���[�g�J�b�g�L�[���ςȓ���������̂�
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

	SHOWENABLE(IDC_CHECK_BACKUP_ADVANCED,	TRUE, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_TYPE1,		!bAdvanced, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_TYPE3,		!bAdvanced, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_DATETYPE1,	!bAdvanced, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_DATETYPE2,	!bAdvanced, bBackup);
	SHOWENABLE(IDC_LABEL_BACKUP_3,			!bAdvanced, bBackup && bType3);
	SHOWENABLE(IDC_EDIT_BACKUP_3,			!bAdvanced, bBackup && bType3);
	SHOWENABLE(IDC_SPIN_BACKUP_GENS,		!bAdvanced, bBackup && bType3);
	SHOWENABLE(IDC_CHECK_BACKUP_RETAINEXT,	!bAdvanced, bBackup && (bType1 || bType3));
	SHOWENABLE(IDC_CHECK_BACKUP_YEAR,		!bAdvanced, bBackup && (bDate1 || bDate2));
	SHOWENABLE(IDC_CHECK_BACKUP_MONTH,		!bAdvanced, bBackup && (bDate1 || bDate2));
	SHOWENABLE(IDC_CHECK_BACKUP_DAY,		!bAdvanced, bBackup && (bDate1 || bDate2));
	SHOWENABLE(IDC_CHECK_BACKUP_HOUR,		!bAdvanced, bBackup && (bDate1 || bDate2));
	SHOWENABLE(IDC_CHECK_BACKUP_MIN,		!bAdvanced, bBackup && (bDate1 || bDate2));
	SHOWENABLE(IDC_CHECK_BACKUP_SEC,		!bAdvanced, bBackup && (bDate1 || bDate2));

	// �ڍאݒ�
	SHOWENABLE(IDC_EDIT_BACKUPFILE,			TRUE, bBackup && bAdvanced);
//	SHOWENABLE(IDC_LABEL_BACKUP_HELP,		bAdvanced, bBackup);	// �s���̂܂ܕ��u�i���R���g���[���B���̕����͔p�~�j
	SHOWENABLE(IDC_LABEL_BACKUP_HELP2,		bAdvanced, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_DATETYPE1A,	bAdvanced, bBackup);
	SHOWENABLE(IDC_RADIO_BACKUP_DATETYPE2A,	bAdvanced, bBackup);

	SHOWENABLE(IDC_CHECK_BACKUPFOLDER,			TRUE, bBackup);
	SHOWENABLE(IDC_LABEL_BACKUP_4,				TRUE, bBackup && bFolder);
	SHOWENABLE(IDC_CHECK_BACKUP_FOLDER_RM,		TRUE, bBackup && bFolder);
	SHOWENABLE(IDC_EDIT_BACKUPFOLDER,			TRUE, bBackup && bFolder);
	SHOWENABLE(IDC_BUTTON_BACKUP_FOLDER_REF,	TRUE, bBackup && bFolder);
	SHOWENABLE(IDC_CHECK_BACKUP_DUSTBOX,		TRUE, bBackup);

	// �쐬�O�Ɋm�F
	SHOWENABLE(IDC_CHECK_BACKUPDIALOG,		TRUE, bBackup);

	#undef SHOWENABLE
}


/*!	�o�b�N�A�b�v�t�@�C���̏ڍאݒ�G�f�B�b�g�{�b�N�X��K�؂ɍX�V����
	@note �ڍאݒ�؂�ւ����̃f�t�H���g���I�v�V�����ɍ��킹�邽�߁A
		szBackUpPathAdvanced ���X�V����
*/
void PropBackup::UpdateBackupFile(HWND hwndDlg)	//	�o�b�N�A�b�v�t�@�C���̏ڍאݒ�
{
	wchar_t temp[MAX_PATH];
	auto& csBackup = common.backup;
	// �o�b�N�A�b�v���쐬����t�@�C��
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

