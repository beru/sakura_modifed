/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�S�ʁv�y�[�W
*/

#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "PropertyManager.h"
#include "recent/MRUFile.h"
#include "recent/MRUFolder.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

TYPE_NAME_ID<int> SpecialScrollModeArr[] = {
	{ 0,									STR_SCROLL_WITH_NO_KEY },		// _T("�g�ݍ��킹�Ȃ�") },
	{ (int)MouseFunctionType::CenterClick,		STR_SCROLL_WITH_MID_BTN },		// _T("�}�E�X���{�^��") },
	{ (int)MouseFunctionType::LeftSideClick,	STR_SCROLL_WITH_SIDE_1_BTN },	// _T("�}�E�X�T�C�h�{�^��1") },
	{ (int)MouseFunctionType::RightSideClick,	STR_SCROLL_WITH_SIDE_2_BTN },	// _T("�}�E�X�T�C�h�{�^��2") },
	{ VK_CONTROL,							STR_SCROLL_WITH_CTRL_KEY },		// _T("CONTROL�L�[") },
	{ VK_SHIFT,								STR_SCROLL_WITH_SHIFT_KEY },	// _T("SHIFT�L�[") },
};

static const DWORD p_helpids[] = {	//10900
	IDC_BUTTON_CLEAR_MRU_FILE,		HIDC_BUTTON_CLEAR_MRU_FILE,			// �������N���A�i�t�@�C���j
	IDC_BUTTON_CLEAR_MRU_FOLDER,	HIDC_BUTTON_CLEAR_MRU_FOLDER,		// �������N���A�i�t�H���_�j
	IDC_CHECK_FREECARET,			HIDC_CHECK_FREECARET,				// �t���[�J�[�\��
//DEL	IDC_CHECK_INDENT,				HIDC_CHECK_INDENT,				// �����C���f���g �F�^�C�v�ʂֈړ�
//DEL	IDC_CHECK_INDENT_WSPACE,		HIDC_CHECK_INDENT_WSPACE,		// �S�p�󔒂��C���f���g �F�^�C�v�ʂֈړ�
	IDC_CHECK_USETRAYICON,			HIDC_CHECK_USETRAYICON,				// �^�X�N�g���C���g��
	IDC_CHECK_STAYTASKTRAY,			HIDC_CHECK_STAYTASKTRAY,			// �^�X�N�g���C�ɏ풓
	IDC_CHECK_REPEATEDSCROLLSMOOTH,	HIDC_CHECK_REPEATEDSCROLLSMOOTH,	// �������炩�ɂ���
	IDC_CHECK_CLOSEALLCONFIRM,		HIDC_CHECK_CLOSEALLCONFIRM,			// [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F����
	IDC_CHECK_EXITCONFIRM,			HIDC_CHECK_EXITCONFIRM,				// �I���̊m�F
	IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_WORD, HIDC_CHECK_STOPS_WORD,	// �P��P�ʂňړ�����Ƃ��ɒP��̗��[�Ɏ~�܂�
	IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_PARAGRAPH, HIDC_CHECK_STOPS_PARAGRAPH,	// �i���P�ʂňړ�����Ƃ��ɒi���̗��[�Ɏ~�܂�
	IDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE, HIDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE,		// �}�E�X�N���b�N�ŃA�N�e�B�u�ɂȂ����Ƃ��̓J�[�\�����N���b�N�ʒu�Ɉړ����Ȃ�
	IDC_HOTKEY_TRAYMENU,			HIDC_HOTKEY_TRAYMENU,				// ���N���b�N���j���[�̃V���[�g�J�b�g�L�[
	IDC_EDIT_REPEATEDSCROLLLINENUM,	HIDC_EDIT_REPEATEDSCROLLLINENUM,	// �X�N���[���s��
	IDC_EDIT_MAX_MRU_FILE,			HIDC_EDIT_MAX_MRU_FILE,				// �t�@�C�������̍ő吔
	IDC_EDIT_MAX_MRU_FOLDER,		HIDC_EDIT_MAX_MRU_FOLDER,			// �t�H���_�����̍ő吔
	IDC_RADIO_CARETTYPE0,			HIDC_RADIO_CARETTYPE0,				// �J�[�\���`��iWindows���j
	IDC_RADIO_CARETTYPE1,			HIDC_RADIO_CARETTYPE1,				// �J�[�\���`��iMS-DOS���j
	IDC_SPIN_REPEATEDSCROLLLINENUM,	HIDC_EDIT_REPEATEDSCROLLLINENUM,
	IDC_SPIN_MAX_MRU_FILE,			HIDC_EDIT_MAX_MRU_FILE,
	IDC_SPIN_MAX_MRU_FOLDER,		HIDC_EDIT_MAX_MRU_FOLDER,
	IDC_CHECK_MEMDC,				HIDC_CHECK_MEMDC,					// ��ʃL���b�V�����g��
	IDC_COMBO_WHEEL_PAGESCROLL,		HIDC_COMBO_WHEEL_PAGESCROLL,		// �g�ݍ��킹�ăz�C�[�����삵�����y�[�W�X�N���[������
	IDC_COMBO_WHEEL_HSCROLL,		HIDC_COMBO_WHEEL_HSCROLL,			// �g�ݍ��킹�ăz�C�[�����삵�������X�N���[������
//	IDC_STATIC,						-1,
	0, 0
};

/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK PropGeneral::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropGeneral::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}


// General ���b�Z�[�W����
INT_PTR PropGeneral::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
	)
{
	WORD		wNotifyCode;
	WORD		wID;
//	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	int			nVal;
//	LPDRAWITEMSTRUCT pDis;

	switch (uMsg) {
	case WM_INITDIALOG:
		// �_�C�A���O�f�[�^�̐ݒ� General
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
		// ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����
		return TRUE;
		
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// �ʒm�R�[�h
		wID			= LOWORD(wParam);	// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID
//		hwndCtl		= (HWND) lParam;	// �R���g���[���̃n���h��
		switch (wNotifyCode) {
		// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
		case BN_CLICKED:
			switch (wID) {
			case IDC_CHECK_USETRAYICON:	// �^�X�N�g���C���g��
				if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_USETRAYICON)) {
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STAYTASKTRAY), TRUE);
				}else {
					::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STAYTASKTRAY), FALSE);
				}
				return TRUE;

			case IDC_CHECK_STAYTASKTRAY:	// �^�X�N�g���C�ɏ풓
				return TRUE;

			case IDC_BUTTON_CLEAR_MRU_FILE:
				// �t�@�C���̗������N���A
				if (::MYMESSAGEBOX(hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
					LS(STR_PROPCOMGEN_FILE1)) == IDCANCEL
				) {
					return TRUE;
				}
				{
					MruFile mru;
					mru.ClearAll();
				}
				InfoMessage(hwndDlg, LS(STR_PROPCOMGEN_FILE2));
				return TRUE;
			case IDC_BUTTON_CLEAR_MRU_FOLDER:
				// �t�H���_�̗������N���A
				if (::MYMESSAGEBOX(hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
					LS(STR_PROPCOMGEN_DIR1)) == IDCANCEL
				) {
					return TRUE;
				}
				{
					MruFolder mruFolder;	//	MRU���X�g�̏������B���x�������Ɩ�肠��H
					mruFolder.ClearAll();
				}
				InfoMessage(hwndDlg, LS(STR_PROPCOMGEN_DIR2));
				return TRUE;
			}
			break;	// BN_CLICKED
		// �R���{�{�b�N�X�̃��X�g�̍��ڂ��I�����ꂽ
		case CBN_SELENDOK:
			HWND	hwndCombo;
			int		nSelPos;

			switch (wID) {
			// �g�ݍ��킹�ăz�C�[�����삵�����y�[�W�X�N���[������
			case IDC_COMBO_WHEEL_PAGESCROLL:
				hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_PAGESCROLL);
				nSelPos = Combo_GetCurSel(hwndCombo);
				hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_HSCROLL);
				if (nSelPos && nSelPos == Combo_GetCurSel(hwndCombo)) {
					Combo_SetCurSel(hwndCombo, 0);
				}
				return TRUE;
			// �g�ݍ��킹�ăz�C�[�����삵�������X�N���[������
			case IDC_COMBO_WHEEL_HSCROLL:
				hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_HSCROLL);
				nSelPos = Combo_GetCurSel(hwndCombo);
				hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_PAGESCROLL);
				if (nSelPos && nSelPos == Combo_GetCurSel(hwndCombo)) {
					Combo_SetCurSel(hwndCombo, 0);
				}
				return TRUE;
			}
			break;	// CBN_SELENDOK
		}
		break;	// WM_COMMAND
	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch (idCtrl) {
		case IDC_SPIN_REPEATEDSCROLLLINENUM:
			// �L�[���s�[�g���̃X�N���[���s��
//			MYTRACE(_T("IDC_SPIN_REPEATEDSCROLLLINENUM\n"));
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 10) {
				nVal = 10;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_MAX_MRU_FILE:
			// �t�@�C���̗���MAX
//			MYTRACE(_T("IDC_SPIN_MAX_MRU_FILE\n"));
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FILE, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 0) {
				nVal = 0;
			}
			if (nVal > MAX_MRU) {
				nVal = MAX_MRU;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FILE, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_MAX_MRU_FOLDER:
			// �t�H���_�̗���MAX
//			MYTRACE(_T("IDC_SPIN_MAX_MRU_FOLDER\n"));
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 0) {
				nVal = 0;
			}
			if (nVal > MAX_OPENFOLDER) {
				nVal = MAX_OPENFOLDER;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, nVal, FALSE);
			return TRUE;
		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_GENERAL);
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE(_T("General PSN_KILLACTIVE\n"));
				// �_�C�A���O�f�[�^�̎擾 General
				GetData(hwndDlg);
				return TRUE;
			case PSN_SETACTIVE:
				nPageNum = ID_PROPCOM_PAGENUM_GENERAL;
				return TRUE;
			}
			break;
		}

//		MYTRACE(_T("pNMHDR->hwndFrom=%xh\n"), pNMHDR->hwndFrom);
//		MYTRACE(_T("pNMHDR->idFrom  =%xh\n"), pNMHDR->idFrom);
//		MYTRACE(_T("pNMHDR->code    =%xh\n"), pNMHDR->code);
//		MYTRACE(_T("pMNUD->iPos    =%d\n"), pMNUD->iPos);
//		MYTRACE(_T("pMNUD->iDelta  =%d\n"), pMNUD->iDelta);
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

	}
	return FALSE;
}


// �_�C�A���O�f�[�^�̐ݒ� General
void PropGeneral::SetData(HWND hwndDlg)
{
	auto& csGeneral = common.general;

	// �J�[�\���̃^�C�v 0=win 1=dos 
	if (csGeneral.GetCaretType() == 0) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_CARETTYPE0, TRUE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_CARETTYPE1, FALSE);
	}else {
		::CheckDlgButton(hwndDlg, IDC_RADIO_CARETTYPE0, FALSE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_CARETTYPE1, TRUE);
	}

	// �t���[�J�[�\�����[�h
	::CheckDlgButton(hwndDlg, IDC_CHECK_FREECARET, csGeneral.bIsFreeCursorMode ? 1 : 0);

	// �P��P�ʂňړ�����Ƃ��ɁA�P��̗��[�Ŏ~�܂邩
	::CheckDlgButton(hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_WORD, csGeneral.bStopsBothEndsWhenSearchWord);

	// �i���P�ʂňړ�����Ƃ��ɁA�i���̗��[�Ŏ~�܂邩
	::CheckDlgButton(hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_PARAGRAPH, csGeneral.bStopsBothEndsWhenSearchParagraph);

	//	�}�E�X�N���b�N�ŃA�N�e�B�u�ɂȂ����Ƃ��̓J�[�\�����N���b�N�ʒu�Ɉړ����Ȃ�
	::CheckDlgButton(hwndDlg, IDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE, csGeneral.bNoCaretMoveByActivation);

	// [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F����
	::CheckDlgButton(hwndDlg, IDC_CHECK_CLOSEALLCONFIRM, csGeneral.bCloseAllConfirm);

	// �I�����̊m�F������
	::CheckDlgButton(hwndDlg, IDC_CHECK_EXITCONFIRM, csGeneral.bExitConfirm);

	// �L�[���s�[�g���̃X�N���[���s��
	::SetDlgItemInt(hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, csGeneral.nRepeatedScrollLineNum, FALSE);

	// �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩
	::CheckDlgButton(hwndDlg, IDC_CHECK_REPEATEDSCROLLSMOOTH, csGeneral.nRepeatedScroll_Smooth);

	// �g�ݍ��킹�ăz�C�[�����삵�����y�[�W�X�N���[������
	HWND	hwndCombo;
	int		nSelPos;

	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_PAGESCROLL);
	Combo_ResetContent(hwndCombo);
	nSelPos = 0;
	for (size_t i=0; i<_countof(SpecialScrollModeArr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(SpecialScrollModeArr[i].nNameId));
		if (SpecialScrollModeArr[i].nMethod == csGeneral.nPageScrollByWheel) {	// �y�[�W�X�N���[���Ƃ���g�ݍ��킹����
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	// �g�ݍ��킹�ăz�C�[�����삵�������X�N���[������
	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_HSCROLL);
	Combo_ResetContent(hwndCombo);
	nSelPos = 0;
	for (size_t i=0; i<_countof(SpecialScrollModeArr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(SpecialScrollModeArr[i].nNameId));
		if (SpecialScrollModeArr[i].nMethod == csGeneral.nHorizontalScrollByWheel) {	// ���X�N���[���Ƃ���g�ݍ��킹����
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	// ��ʃL���b�V�����g��
	::CheckDlgButton(hwndDlg, IDC_CHECK_MEMDC, common.window.bUseCompatibleBMP);

	// �t�@�C���̗���MAX
	::SetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FILE, csGeneral.nMRUArrNum_MAX, FALSE);

	// �t�H���_�̗���MAX
	::SetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, csGeneral.nOPENFOLDERArrNum_MAX, FALSE);

	// �^�X�N�g���C���g��
	::CheckDlgButton(hwndDlg, IDC_CHECK_USETRAYICON, csGeneral.bUseTaskTray);
	if (csGeneral.bUseTaskTray) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STAYTASKTRAY), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STAYTASKTRAY), FALSE);
	}
	// �^�X�N�g���C�ɏ풓
	::CheckDlgButton(hwndDlg, IDC_CHECK_STAYTASKTRAY, csGeneral.bStayTaskTray);

	// �^�X�N�g���C���N���b�N���j���[�̃V���[�g�J�b�g
	HotKey_SetHotKey(::GetDlgItem(hwndDlg, IDC_HOTKEY_TRAYMENU), csGeneral.wTrayMenuHotKeyCode, csGeneral.wTrayMenuHotKeyMods);

	return;
}


// �_�C�A���O�f�[�^�̎擾 General
int PropGeneral::GetData(HWND hwndDlg)
{
	auto& csGeneral = common.general;
	
	// �J�[�\���̃^�C�v 0=win 1=dos 
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_CARETTYPE0)) {
		csGeneral.SetCaretType(0);
	}
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_CARETTYPE1)) {
		csGeneral.SetCaretType(1);
	}

	// �t���[�J�[�\�����[�h
	csGeneral.bIsFreeCursorMode = DlgButton_IsChecked(hwndDlg, IDC_CHECK_FREECARET);

	// �P��P�ʂňړ�����Ƃ��ɁA�P��̗��[�Ŏ~�܂邩
	csGeneral.bStopsBothEndsWhenSearchWord = DlgButton_IsChecked(hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_WORD);
	// �}�E�X�N���b�N�ŃA�N�e�B�u�ɂȂ����Ƃ��̓J�[�\�����N���b�N�ʒu�Ɉړ����Ȃ�
	csGeneral.bNoCaretMoveByActivation = DlgButton_IsChecked(hwndDlg, IDC_CHECK_NOMOVE_ACTIVATE_BY_MOUSE);

	// �i���P�ʂňړ�����Ƃ��ɁA�i���̗��[�Ŏ~�܂邩
	csGeneral.bStopsBothEndsWhenSearchParagraph = DlgButton_IsChecked(hwndDlg, IDC_CHECK_STOPS_BOTH_ENDS_WHEN_SEARCH_PARAGRAPH);

	// [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F����
	csGeneral.bCloseAllConfirm = DlgButton_IsChecked(hwndDlg, IDC_CHECK_CLOSEALLCONFIRM);

	// �I�����̊m�F������
	csGeneral.bExitConfirm = DlgButton_IsChecked(hwndDlg, IDC_CHECK_EXITCONFIRM);

	// �L�[���s�[�g���̃X�N���[���s��
	csGeneral.nRepeatedScrollLineNum = ::GetDlgItemInt(hwndDlg, IDC_EDIT_REPEATEDSCROLLLINENUM, NULL, FALSE);
	csGeneral.nRepeatedScrollLineNum = std::max(1, csGeneral.nRepeatedScrollLineNum);
	csGeneral.nRepeatedScrollLineNum = std::min(10, csGeneral.nRepeatedScrollLineNum);

	// �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩
	csGeneral.nRepeatedScroll_Smooth = DlgButton_IsChecked(hwndDlg, IDC_CHECK_REPEATEDSCROLLSMOOTH);

	HWND	hwndCombo;
	int		nSelPos;

	// ��ʃL���b�V�����g��
	common.window.bUseCompatibleBMP = DlgButton_IsChecked(hwndDlg, IDC_CHECK_MEMDC);

	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_PAGESCROLL);
	nSelPos = Combo_GetCurSel(hwndCombo);
	csGeneral.nPageScrollByWheel = SpecialScrollModeArr[nSelPos].nMethod;		// �y�[�W�X�N���[���Ƃ���g�ݍ��킹����

	// �g�ݍ��킹�ăz�C�[�����삵�������X�N���[������
	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_WHEEL_HSCROLL);
	nSelPos = Combo_GetCurSel(hwndCombo);
	csGeneral.nHorizontalScrollByWheel = SpecialScrollModeArr[nSelPos].nMethod;	// ���X�N���[���Ƃ���g�ݍ��킹����

	// �t�@�C���̗���MAX
	csGeneral.nMRUArrNum_MAX = ::GetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FILE, NULL, FALSE);
	if (csGeneral.nMRUArrNum_MAX < 0) {
		csGeneral.nMRUArrNum_MAX = 0;
	}
	if (csGeneral.nMRUArrNum_MAX > MAX_MRU) {
		csGeneral.nMRUArrNum_MAX = MAX_MRU;
	}

	{	// �����̊Ǘ�
		RecentFile	cRecentFile;
		cRecentFile.UpdateView();
		cRecentFile.Terminate();
	}

	// �t�H���_�̗���MAX
	csGeneral.nOPENFOLDERArrNum_MAX = ::GetDlgItemInt(hwndDlg, IDC_EDIT_MAX_MRU_FOLDER, NULL, FALSE);
	if (csGeneral.nOPENFOLDERArrNum_MAX < 0) {
		csGeneral.nOPENFOLDERArrNum_MAX = 0;
	}
	if (csGeneral.nOPENFOLDERArrNum_MAX > MAX_OPENFOLDER) {
		csGeneral.nOPENFOLDERArrNum_MAX = MAX_OPENFOLDER;
	}

	{	// �����̊Ǘ�
		RecentFolder	cRecentFolder;
		cRecentFolder.UpdateView();
		cRecentFolder.Terminate();
	}

	// �^�X�N�g���C���g��
	csGeneral.bUseTaskTray = DlgButton_IsChecked(hwndDlg, IDC_CHECK_USETRAYICON);
	if (csGeneral.bUseTaskTray) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STAYTASKTRAY), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_CHECK_STAYTASKTRAY), FALSE);
	}
	// �^�X�N�g���C�ɏ풓
	csGeneral.bStayTaskTray = DlgButton_IsChecked(hwndDlg, IDC_CHECK_STAYTASKTRAY);

	// �^�X�N�g���C���N���b�N���j���[�̃V���[�g�J�b�g
	LRESULT	lResult;
	lResult = HotKey_GetHotKey(::GetDlgItem(hwndDlg, IDC_HOTKEY_TRAYMENU));
	csGeneral.wTrayMenuHotKeyCode = LOBYTE(lResult);
	csGeneral.wTrayMenuHotKeyMods = HIBYTE(lResult);

	return TRUE;
}

