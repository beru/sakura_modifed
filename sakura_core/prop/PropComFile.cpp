/*! @file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�����v�y�[�W
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "PropertyManager.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {	//01310
	IDC_COMBO_FILESHAREMODE,				HIDC_COMBO_FILESHAREMODE,				// �r������
	IDC_CHECK_bCheckFileTimeStamp,			HIDC_CHECK_bCheckFileTimeStamp,			// �X�V�̊Ď�
	IDC_EDIT_AUTOLOAD_DELAY,				HIDC_EDIT_AUTOLOAD_DELAY,				// �����Ǎ����x��
	IDC_SPIN_AUTOLOAD_DELAY,				HIDC_EDIT_AUTOLOAD_DELAY,
	IDC_CHECK_bUneditableIfUnwritable,		HIDC_CHECK_bUneditableIfUnwritable,		// �㏑���֎~���o���͕ҏW�֎~�ɂ���
	IDC_CHECK_ENABLEUNMODIFIEDOVERWRITE,	HIDC_CHECK_ENABLEUNMODIFIEDOVERWRITE,	// ���ύX�ł��㏑��
	IDC_CHECK_AUTOSAVE,						HIDC_CHECK_AUTOSAVE,					// �����I�ɕۑ�
	IDC_CHECK_bDropFileAndClose,			HIDC_CHECK_bDropFileAndClose,			// ���ĊJ��
	IDC_CHECK_RestoreCurPosition,			HIDC_CHECK_RestoreCurPosition,			// �J�[�\���ʒu�̕���
	IDC_CHECK_AutoMIMEDecode,				HIDC_CHECK_AutoMIMEDecode,				// MIME�f�R�[�h
	IDC_EDIT_AUTOBACKUP_INTERVAL,			HIDC_EDIT_AUTOBACKUP_INTERVAL,			// �����ۑ��Ԋu
	IDC_EDIT_nDropFileNumMax,				HIDC_EDIT_nDropFileNumMax,				// �t�@�C���h���b�v�ő吔
	IDC_SPIN_AUTOBACKUP_INTERVAL,			HIDC_EDIT_AUTOBACKUP_INTERVAL,
	IDC_SPIN_nDropFileNumMax,				HIDC_EDIT_nDropFileNumMax,
	IDC_CHECK_RestoreBookmarks,				HIDC_CHECK_RestoreBookmarks,			// �u�b�N�}�[�N�̕���
	IDC_CHECK_QueryIfCodeChange,			HIDC_CHECK_QueryIfCodeChange,			// �O��ƈقȂ镶���R�[�h�̂Ƃ��₢���킹���s��
	IDC_CHECK_AlertIfFileNotExist,			HIDC_CHECK_AlertIfFileNotExist,			// �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������
	IDC_CHECK_ALERT_IF_LARGEFILE,			HIDC_CHECK_ALERT_IF_LARGEFILE,			// �J�����Ƃ����t�@�C�����傫���ꍇ�Ɍx������
	IDC_CHECK_NoFilterSaveNew,				HIDC_CHECK_NoFilterSaveNew,				// �V�K����ۑ����͑S�t�@�C���\��
	IDC_CHECK_NoFilterSaveFile,				HIDC_CHECK_NoFilterSaveFile,			// �V�K�ȊO����ۑ����͑S�t�@�C���\��
//	IDC_STATIC,								-1,
	0, 0
};

TYPE_NAME_ID<FileShareMode> ShareModeArr[] = {
	{ FileShareMode::NonExclusive,	STR_EXCLU_NO_EXCLUSIVE },	// _T("���Ȃ�") },
	{ FileShareMode::DenyWrite,		STR_EXCLU_DENY_READWRITE },	// _T("�㏑�����֎~����") },
	{ FileShareMode::DenyReadWrite,	STR_EXCLU_DENY_WRITE },		// _T("�ǂݏ������֎~����") },
};

/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK PropFile::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropFile::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}

// �t�@�C���y�[�W ���b�Z�[�W����
INT_PTR PropFile::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,	// message
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	int			nVal;

	switch (uMsg) {
	case WM_INITDIALOG:
		// �_�C�A���O�f�[�^�̐ݒ� File
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		return TRUE;
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch (idCtrl) {
		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_FILE);
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE(_T("File PSN_KILLACTIVE\n"));
				// �_�C�A���O�f�[�^�̎擾 File
				GetData(hwndDlg);
				return TRUE;
			case PSN_SETACTIVE:
				nPageNum = ID_PROPCOM_PAGENUM_FILE;
				return TRUE;
			}
			break;
		case IDC_SPIN_AUTOLOAD_DELAY:
			// �����Ǎ����x��
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_AUTOLOAD_DELAY, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 0) {
				nVal = 0;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_AUTOLOAD_DELAY, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_nDropFileNumMax:
			// ��x�Ƀh���b�v�\�ȃt�@�C����
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nDropFileNumMax, NULL, FALSE);
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
			::SetDlgItemInt(hwndDlg, IDC_EDIT_nDropFileNumMax, nVal, FALSE);
			return TRUE;
			// NOTREACHED
//			break;
		case IDC_SPIN_AUTOBACKUP_INTERVAL:
			//  �o�b�N�A�b�v�Ԋu
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 35791) {
				nVal = 35791;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_ALERT_FILESIZE:
			// �t�@�C���̌x���T�C�Y
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_ALERT_FILESIZE, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else 
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 2048) {
				nVal = 2048;  // �ő� 2GB �܂�
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_ALERT_FILESIZE, nVal, FALSE);
			return TRUE;
			// NOTREACHED
//			break;
		}
		break;

	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// �ʒm�R�[�h
		wID			= LOWORD(wParam);	// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID

		if (wID == IDC_COMBO_FILESHAREMODE && wNotifyCode == CBN_SELCHANGE) {	// �R���{�{�b�N�X�̑I��ύX
			EnableFilePropInput(hwndDlg);
			break;
		}

		switch (wNotifyCode) {
		// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
		case BN_CLICKED:
			switch (wID) {
			case IDC_CHECK_bCheckFileTimeStamp:	// �X�V�̊Ď�
			case IDC_CHECK_bDropFileAndClose:// �t�@�C�����h���b�v�����Ƃ��͕��ĊJ��
			case IDC_CHECK_AUTOSAVE:
			case IDC_CHECK_ALERT_IF_LARGEFILE:
				EnableFilePropInput(hwndDlg);
				break;
			}
			break;
		}
		break;

	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids);
		}
		return TRUE;
		// NOTREACHED
//		break;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids);
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}


/*! �t�@�C���y�[�W: �_�C�A���O�f�[�^�̐ݒ�
	���L����������f�[�^��ǂݏo���Ċe�R���g���[���ɒl��ݒ肷��B

	@par �o�b�N�A�b�v���㐔���Ó��Ȓl���ǂ����̃`�F�b�N���s���B�s�K�؂Ȓl�̎���
	�ł��߂��K�؂Ȓl��ݒ肷��B

	@param hwndDlg �v���p�e�B�y�[�W��Window Handle
*/
void PropFile::SetData(HWND hwndDlg)
{
	auto& csFile = common.file;
	//--- File ---
	// �t�@�C���̔r�����䃂�[�h
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_FILESHAREMODE);
	Combo_ResetContent(hwndCombo);
	size_t nSelPos = 0;
	for (size_t i=0; i<_countof(ShareModeArr); ++i) {
		Combo_InsertString(hwndCombo, (int)i, LS(ShareModeArr[i].nNameId));
		if (ShareModeArr[i].nMethod == csFile.nFileShareMode) {
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	// �X�V�̊Ď�
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_bCheckFileTimeStamp, csFile.bCheckFileTimeStamp);

	// �����Ǎ����x��
	::SetDlgItemInt(hwndDlg, IDC_EDIT_AUTOLOAD_DELAY, csFile.nAutoloadDelay, FALSE);

	// �㏑���֎~���o���͕ҏW�֎~�ɂ���
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_bUneditableIfUnwritable, csFile.bUneditableIfUnwritable);

	// ���ύX�ł��㏑�����邩
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_ENABLEUNMODIFIEDOVERWRITE, csFile.bEnableUnmodifiedOverwrite);

	// �t�@�C�����h���b�v�����Ƃ��͕��ĊJ��
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_bDropFileAndClose, csFile.bDropFileAndClose);
	// ��x�Ƀh���b�v�\�ȃt�@�C����
	::SetDlgItemInt(hwndDlg, IDC_EDIT_nDropFileNumMax, csFile.nDropFileNumMax, FALSE);

	//	�����ۑ��̗L���E����
	::CheckDlgButton(hwndDlg, IDC_CHECK_AUTOSAVE, common.backup.IsAutoBackupEnabled());

	TCHAR buf[6];
	int nN = common.backup.GetAutoBackupInterval();
	nN = nN < 1  ?  1 : nN;
	nN = nN > 35791 ? 35791 : nN;

	auto_sprintf_s(buf, _T("%d"), nN);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, buf);

	// �J�[�\���ʒu�����t���O
	::CheckDlgButton(hwndDlg, IDC_CHECK_RestoreCurPosition, csFile.GetRestoreCurPosition());
	// �u�b�N�}�[�N�����t���O
	::CheckDlgButton(hwndDlg, IDC_CHECK_RestoreBookmarks, csFile.GetRestoreBookmarks());
	//	Nov. 12, 2000 genta	MIME Decode�t���O
	::CheckDlgButton(hwndDlg, IDC_CHECK_AutoMIMEDecode, csFile.GetAutoMIMEdecode());
	//	Oct. 03, 2004 genta �O��ƈقȂ镶���R�[�h�̂Ƃ��ɖ₢���킹���s�����ǂ����̃t���O
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_QueryIfCodeChange, csFile.GetQueryIfCodeChange());
	//	Oct. 09, 2004 genta �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x�����邩�ǂ����̃t���O
	::CheckDlgButton(hwndDlg, IDC_CHECK_AlertIfFileNotExist, csFile.GetAlertIfFileNotExist());
	//	�t�@�C���T�C�Y���傫���ꍇ�Ɍx�����o��
	::CheckDlgButton(hwndDlg, IDC_CHECK_ALERT_IF_LARGEFILE, csFile.bAlertIfLargeFile);
	::SetDlgItemInt(hwndDlg, IDC_EDIT_ALERT_FILESIZE, csFile.nAlertFileSize, FALSE);

	// �t�@�C���ۑ��_�C�A���O�̃t�B���^�ݒ�
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_NoFilterSaveNew, csFile.bNoFilterSaveNew);	// �V�K����ۑ����͑S�t�@�C���\��
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_NoFilterSaveFile, csFile.bNoFilterSaveFile);	// �V�K�ȊO����ۑ����͑S�t�@�C���\��

	EnableFilePropInput(hwndDlg);
	return;
}

/*! �t�@�C���y�[�W �_�C�A���O�f�[�^�̎擾
	�_�C�A���O�{�b�N�X�ɐݒ肳�ꂽ�f�[�^�����L�������ɔ��f������

	@par �o�b�N�A�b�v���㐔���Ó��Ȓl���ǂ����̃`�F�b�N���s���B�s�K�؂Ȓl�̎���
	�ł��߂��K�؂Ȓl��ݒ肷��B

	@param hwndDlg �v���p�e�B�y�[�W��Window Handle
	@return ���TRUE
*/
int PropFile::GetData(HWND hwndDlg)
{
	auto& csFile = common.file;

	// �t�@�C���̔r�����䃂�[�h
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_FILESHAREMODE);
	int nSelPos = Combo_GetCurSel(hwndCombo);
	csFile.nFileShareMode = ShareModeArr[nSelPos].nMethod;

	// �X�V�̊Ď�
	csFile.bCheckFileTimeStamp = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bCheckFileTimeStamp);

	// �����Ǎ����x��
	csFile.nAutoloadDelay = ::GetDlgItemInt(hwndDlg, IDC_EDIT_AUTOLOAD_DELAY, NULL, FALSE);

	// �㏑���֎~���o���͕ҏW�֎~�ɂ���
	csFile.bUneditableIfUnwritable = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bUneditableIfUnwritable);

	// ���ύX�ł��㏑�����邩
	csFile.bEnableUnmodifiedOverwrite = DlgButton_IsChecked(hwndDlg, IDC_CHECK_ENABLEUNMODIFIEDOVERWRITE);

	// �t�@�C�����h���b�v�����Ƃ��͕��ĊJ��
	csFile.bDropFileAndClose = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bDropFileAndClose);
	// ��x�Ƀh���b�v�\�ȃt�@�C����
	csFile.nDropFileNumMax = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nDropFileNumMax, NULL, FALSE);
	if (1 > csFile.nDropFileNumMax) {
		csFile.nDropFileNumMax = 1;
	}
	if (99 < csFile.nDropFileNumMax) {
		csFile.nDropFileNumMax = 99;
	}

	//	�����ۑ����s�����ǂ���
	common.backup.EnableAutoBackup(DlgButton_IsChecked(hwndDlg, IDC_CHECK_AUTOSAVE));

	//	�����ۑ��Ԋu�̎擾
	TCHAR szNumBuf[7];
	::DlgItem_GetText(hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL, szNumBuf, 6);

	int nN;
	TCHAR* pDigit;
	for (nN=0, pDigit=szNumBuf; *pDigit!=_T('\0'); ++pDigit) {
		if (_T('0') <= *pDigit && *pDigit <= _T('9')) {
			nN = nN * 10 + *pDigit - _T('0');
		}else {
			break;
		}
	}
	nN = nN < 1  ?  1 : nN;
	nN = nN > 35791 ? 35791 : nN;
	common.backup.SetAutoBackupInterval(nN);

	// �J�[�\���ʒu�����t���O
	csFile.SetRestoreCurPosition(DlgButton_IsChecked(hwndDlg, IDC_CHECK_RestoreCurPosition));
	// �u�b�N�}�[�N�����t���O
	csFile.SetRestoreBookmarks(DlgButton_IsChecked(hwndDlg, IDC_CHECK_RestoreBookmarks));
	//	Nov. 12, 2000 genta	MIME Decode�t���O
	csFile.SetAutoMIMEdecode(DlgButton_IsChecked(hwndDlg, IDC_CHECK_AutoMIMEDecode));
	//	Oct. 03, 2004 genta �O��ƈقȂ镶���R�[�h�̂Ƃ��ɖ₢���킹���s�����ǂ����̃t���O
	csFile.SetQueryIfCodeChange(DlgButton_IsChecked(hwndDlg, IDC_CHECK_QueryIfCodeChange));
	//	Oct. 03, 2004 genta �O��ƈقȂ镶���R�[�h�̂Ƃ��ɖ₢���킹���s�����ǂ����̃t���O
	csFile.SetAlertIfFileNotExist(DlgButton_IsChecked(hwndDlg, IDC_CHECK_AlertIfFileNotExist));
	// �J�����Ƃ����t�@�C�����傫���ꍇ�Ɍx������
	csFile.bAlertIfLargeFile = DlgButton_IsChecked(hwndDlg, IDC_CHECK_ALERT_IF_LARGEFILE);
	csFile.nAlertFileSize = ::GetDlgItemInt(hwndDlg, IDC_EDIT_ALERT_FILESIZE, NULL, FALSE);
	if (csFile.nAlertFileSize < 1) {
		csFile.nAlertFileSize = 1;
	}
	if (csFile.nAlertFileSize > 2048) {
		csFile.nAlertFileSize = 2048;
	}

	// �t�@�C���ۑ��_�C�A���O�̃t�B���^�ݒ�
	csFile.bNoFilterSaveNew = DlgButton_IsChecked(hwndDlg, IDC_CHECK_NoFilterSaveNew);	// �V�K����ۑ����͑S�t�@�C���\��
	csFile.bNoFilterSaveFile = DlgButton_IsChecked(hwndDlg, IDC_CHECK_NoFilterSaveFile);	// �V�K�ȊO����ۑ����͑S�t�@�C���\��

	return TRUE;
}

/*!	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
	�K�؂ɐݒ肷��

	@param hwndDlg �v���p�e�B�V�[�g��Window Handle
*/
void PropFile::EnableFilePropInput(HWND hwndDlg)
{

	//	Drop���̓���
	if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_bDropFileAndClose)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE3), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE4), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_nDropFileNumMax), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_nDropFileNumMax), FALSE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE3), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE4), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_nDropFileNumMax), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_nDropFileNumMax), TRUE);
	}

	//	�r�����邩�ǂ���
	int nSelPos = Combo_GetCurSel(::GetDlgItem(hwndDlg, IDC_COMBO_FILESHAREMODE));
	if (ShareModeArr[nSelPos].nMethod == FileShareMode::NonExclusive) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_bCheckFileTimeStamp), TRUE);
		if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_bCheckFileTimeStamp)) {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOLOAD_DELAY), TRUE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_AUTOLOAD_DELAY),  TRUE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_AUTOLOAD_DELAY),  TRUE);
		}else {
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOLOAD_DELAY), FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_AUTOLOAD_DELAY),  FALSE);
			::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_AUTOLOAD_DELAY),  FALSE);
		}
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_bCheckFileTimeStamp), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOLOAD_DELAY), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_AUTOLOAD_DELAY),  FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_AUTOLOAD_DELAY),  FALSE);
	}

	//	�����ۑ�
	if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_AUTOSAVE)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE2), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_AUTOBACKUP_INTERVAL), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_AUTOBACKUP_INTERVAL), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_LABEL_AUTOSAVE2), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_AUTOBACKUP_INTERVAL), FALSE);
	}

	// �u�J�����Ƃ����t�@�C�����傫���ꍇ�Ɍx�����o���v
	if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_ALERT_IF_LARGEFILE)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_ALERT_FILESIZE), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_ALERT_FILESIZE), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_ALERT_FILESIZE), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_SPIN_ALERT_FILESIZE), FALSE);
	}
}

