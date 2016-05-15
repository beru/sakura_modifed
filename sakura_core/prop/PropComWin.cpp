/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�E�B���h�E�v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2001, genta, MIK, asa-o
	Copyright (C) 2002, YAZAKI, genta, Moca, aroka
	Copyright (C) 2003, MIK, KEITA, genta
	Copyright (C) 2004, Moca
	Copyright (C) 2006, ryoji, fon
	Copyright (C) 2007, genta

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "dlg/DlgWinSize.h"	//	2004.05.13 Moca
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"
#include "_main/Process.h"

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//11200
	IDC_CHECK_DispFUNCKEYWND,		HIDC_CHECK_DispFUNCKEYWND,		// �t�@���N�V�����L�[�\��
	IDC_CHECK_DispSTATUSBAR,		HIDC_CHECK_DispSTATUSBAR,		// �X�e�[�^�X�o�[�\��
	IDC_CHECK_DispTOOLBAR,			HIDC_CHECK_DispTOOLBAR,			// �c�[���o�[�\��
	IDC_CHECK_bScrollBarHorz,		HIDC_CHECK_bScrollBarHorz,		// �����X�N���[���o�[
	IDC_CHECK_bMenuIcon,			HIDC_CHECK_bMenuIcon,			// �A�C�R���t�����j���[
	IDC_CHECK_SplitterWndVScroll,	HIDC_CHECK_SplitterWndVScroll,	// �����X�N���[���̓���	//Jul. 05, 2001 JEPRO �ǉ�
	IDC_CHECK_SplitterWndHScroll,	HIDC_CHECK_SplitterWndHScroll,	// �����X�N���[���̓���	//Jul. 05, 2001 JEPRO �ǉ�
	IDC_EDIT_nRulerBottomSpace,		HIDC_EDIT_nRulerBottomSpace,	// ���[���[�̍���
	IDC_EDIT_nRulerHeight,			HIDC_EDIT_nRulerHeight,			// ���[���[�ƃe�L�X�g�̊Ԋu
	IDC_EDIT_nLineNumberRightSpace,	HIDC_EDIT_nLineNumberRightSpace,// �s�ԍ��ƃe�L�X�g�̌���
	IDC_RADIO_FUNCKEYWND_PLACE1,	HIDC_RADIO_FUNCKEYWND_PLACE1,	// �t�@���N�V�����L�[�\���ʒu
	IDC_RADIO_FUNCKEYWND_PLACE2,	HIDC_RADIO_FUNCKEYWND_PLACE2,	// �t�@���N�V�����L�[�\���ʒu
	IDC_EDIT_FUNCKEYWND_GROUPNUM,	HIDC_EDIT_FUNCKEYWND_GROUPNUM,	// �t�@���N�V�����L�[�̃O���[�v�{�^����
	IDC_SPIN_nRulerBottomSpace,		HIDC_EDIT_nRulerBottomSpace,
	IDC_SPIN_nRulerHeight,			HIDC_EDIT_nRulerHeight,
	IDC_SPIN_nLineNumberRightSpace,	HIDC_EDIT_nLineNumberRightSpace,
	IDC_SPIN_FUNCKEYWND_GROUPNUM,	HIDC_EDIT_FUNCKEYWND_GROUPNUM,
	IDC_WINCAPTION_ACTIVE,			HIDC_WINCAPTION_ACTIVE,			// �A�N�e�B�u��	//@@@ 2003.06.15 MIK
	IDC_WINCAPTION_INACTIVE,		HIDC_WINCAPTION_INACTIVE,		// ��A�N�e�B�u��	//@@@ 2003.06.15 MIK
	IDC_BUTTON_WINSIZE,				HIDC_BUTTON_WINSIZE,			// �ʒu�Ƒ傫���̐ݒ�	// 2006.08.06 ryoji
	IDC_COMBO_LANGUAGE,				HIDC_COMBO_LANGUAGE,			// ����I��
	//	Feb. 11, 2007 genta TAB�֘A�́u�^�u�o�[�v�V�[�g�ֈړ�
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
INT_PTR CALLBACK PropWin::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropWin::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
	}
//	To Here Jun. 2, 2001 genta


// ���b�Z�[�W����
INT_PTR PropWin::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,	// message
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
// From Here Sept. 9, 2000 JEPRO
	WORD		wNotifyCode;
	WORD		wID;
// To Here Sept. 9, 2000

	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	int			nVal;	// Sept.21, 2000 JEPRO �X�s���v�f���������̂ŕ���������

	switch (uMsg) {
	case WM_INITDIALOG:
		// �_�C�A���O�f�[�^�̐ݒ� Window
		SetData(hwndDlg);
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����
		// ���[���[����
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_nRulerHeight), 2);
		// ���[���[�ƃe�L�X�g�̌���
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_nRulerBottomSpace), 2);

		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch (idCtrl) {
		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_WIN);
				return TRUE;
			case PSN_KILLACTIVE:
//				MYTRACE(_T("Window PSN_KILLACTIVE\n"));
				// �_�C�A���O�f�[�^�̎擾 Window
				GetData(hwndDlg);
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				nPageNum = ID_PROPCOM_PAGENUM_WIN;
				return TRUE;
			}
			break;
		case IDC_SPIN_nRulerHeight:
			// ���[���|�̍���
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nRulerHeight, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < IDC_SPIN_nRulerHeight_MIN) {
				nVal = IDC_SPIN_nRulerHeight_MIN;
			}
			if (nVal > IDC_SPIN_nRulerHeight_MAX) {
				nVal = IDC_SPIN_nRulerHeight_MAX;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_nRulerHeight, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_nRulerBottomSpace:
			// ���[���[�ƃe�L�X�g�̌���
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nRulerBottomSpace, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 0) {
				nVal = 0;
			}
			if (nVal > 32) {
				nVal = 32;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_nRulerBottomSpace, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_nLineNumberRightSpace:
			// ���[���[�ƃe�L�X�g�̌���
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nLineNumberRightSpace, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 0) {
				nVal = 0;
			}
			if (nVal > 32) {
				nVal = 32;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_nLineNumberRightSpace, nVal, FALSE);
			return TRUE;
		case IDC_SPIN_FUNCKEYWND_GROUPNUM:
			nVal = ::GetDlgItemInt(hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM, NULL, FALSE);
			if (pMNUD->iDelta < 0) {
				++nVal;
			}else
			if (pMNUD->iDelta > 0) {
				--nVal;
			}
			if (nVal < 1) {
				nVal = 1;
			}
			if (nVal > 12) {
				nVal = 12;
			}
			::SetDlgItemInt(hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM, nVal, FALSE);
			return TRUE;
		}
		break;
//****	To Here Sept. 21, 2000
//	From Here Sept. 9, 2000 JEPRO
	case WM_COMMAND:
		wNotifyCode	= HIWORD(wParam);	// �ʒm�R�[�h
		wID			= LOWORD(wParam);	// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID
		switch (wNotifyCode) {
		// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
		case BN_CLICKED:
			switch (wID) {
			//	�t�@���N�V�����L�[��\�����鎞�������̈ʒu�w���Enable�ɐݒ�
			case IDC_CHECK_DispFUNCKEYWND:
				EnableWinPropInput(hwndDlg);
				break;

			// From Here 2004.05.13 Moca �u�ʒu�Ƒ傫���̐ݒ�v�{�^��
			//	�E�B���h�E�ݒ�_�C�A���O�ɂċN�����̃E�B���h�E��Ԏw��
			case IDC_BUTTON_WINSIZE:
				{
					auto& csWindow = common.window;
					DlgWinSize dlgWinSize;
					RECT rc;
					rc.right  = csWindow.nWinSizeCX;
					rc.bottom = csWindow.nWinSizeCY;
					rc.top    = csWindow.nWinPosX;
					rc.left   = csWindow.nWinPosY;
					dlgWinSize.DoModal(
						::GetModuleHandle(NULL),
						hwndDlg,
						csWindow.eSaveWindowSize,
						csWindow.eSaveWindowPos,
						csWindow.nWinSizeType,
						rc
					);
					csWindow.nWinSizeCX = rc.right;
					csWindow.nWinSizeCY = rc.bottom;
					csWindow.nWinPosX = rc.top;
					csWindow.nWinPosY = rc.left;
				}
				break;
			// To Here 2004.05.13 Moca
			}
			break;
		}
		break;
//	To Here Sept. 9, 2000

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

// �_�C�A���O�f�[�^�̐ݒ�
void PropWin::SetData(HWND hwndDlg)
{
//	BOOL	bRet;
	auto& csWindow = common.window;

	// ����E�B���h�E���J�����Ƃ��c�[���o�[��\������
	::CheckDlgButton(hwndDlg, IDC_CHECK_DispTOOLBAR, csWindow.bDispToolBar);

	// ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������
	::CheckDlgButton(hwndDlg, IDC_CHECK_DispFUNCKEYWND, csWindow.bDispFuncKeyWnd);

	// �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
	if (csWindow.nFuncKeyWnd_Place == 0) {
		::CheckDlgButton(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1, TRUE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2, FALSE);
	}else {
		::CheckDlgButton(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1, FALSE);
		::CheckDlgButton(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2, TRUE);
	}
	// 2002/11/04 Moca �t�@���N�V�����L�[�̃O���[�v�{�^����
	::SetDlgItemInt(hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM, csWindow.nFuncKeyWnd_GroupNum, FALSE);

	// From Here@@@ 2003.06.13 MIK
	//	Feb. 12, 2007 genta TAB�֘A�́u�^�u�o�[�v�V�[�g�ֈړ�

	// To Here@@@ 2003.06.13 MIK
	//	Feb. 11, 2007 genta TAB�֘A�́u�^�u�o�[�v�V�[�g�ֈړ�

	// ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������
	::CheckDlgButton(hwndDlg, IDC_CHECK_DispSTATUSBAR, csWindow.bDispStatusBar);

	// ���[���[����
	::SetDlgItemInt(hwndDlg, IDC_EDIT_nRulerHeight, csWindow.nRulerHeight, FALSE);
	// ���[���[�ƃe�L�X�g�̌���
	::SetDlgItemInt(hwndDlg, IDC_EDIT_nRulerBottomSpace, csWindow.nRulerBottomSpace, FALSE);
	//	Sep. 18. 2002 genta �s�ԍ��ƃe�L�X�g�̌���
	::SetDlgItemInt(hwndDlg, IDC_EDIT_nLineNumberRightSpace, csWindow.nLineNumRightSpace, FALSE);

	// ���[���[�̃^�C�v//	del 2008/7/4 Uchi
//	if (csWindow.nRulerType == 0) {
//		::CheckDlgButton(hwndDlg, IDC_RADIO_nRulerType_0, TRUE);
//		::CheckDlgButton(hwndDlg, IDC_RADIO_nRulerType_1, FALSE);
//	}else {
//		::CheckDlgButton(hwndDlg, IDC_RADIO_nRulerType_0, FALSE);
//		::CheckDlgButton(hwndDlg, IDC_RADIO_nRulerType_1, TRUE);
//	}

	// �����X�N���[���o�[
	::CheckDlgButton(hwndDlg, IDC_CHECK_bScrollBarHorz, csWindow.bScrollBarHorz);

	// �A�C�R���t�����j���[
	::CheckDlgButton(hwndDlg, IDC_CHECK_bMenuIcon, csWindow.bMenuIcon);

	//	2001/06/20 Start by asa-o:	�X�N���[���̓���
	::CheckDlgButton(hwndDlg, IDC_CHECK_SplitterWndVScroll, csWindow.bSplitterWndVScroll);
	::CheckDlgButton(hwndDlg, IDC_CHECK_SplitterWndHScroll, csWindow.bSplitterWndHScroll);
	//	2001/06/20 End

	//	Apr. 05, 2003 genta �E�B���h�E�L���v�V�����̃J�X�^�}�C�Y
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_WINCAPTION_ACTIVE  ), _countof(csWindow.szWindowCaptionActive  ) - 1);	//@@@ 2003.06.13 MIK
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_WINCAPTION_INACTIVE), _countof(csWindow.szWindowCaptionInactive) - 1);	//@@@ 2003.06.13 MIK
	::DlgItem_SetText(hwndDlg, IDC_WINCAPTION_ACTIVE, csWindow.szWindowCaptionActive);
	::DlgItem_SetText(hwndDlg, IDC_WINCAPTION_INACTIVE, csWindow.szWindowCaptionInactive);

	//	Fronm Here Sept. 9, 2000 JEPRO
	//	�t�@���N�V�����L�[��\�����鎞�������̈ʒu�w���Enable�ɐݒ�
	EnableWinPropInput(hwndDlg);
	//	To Here Sept. 9, 2000

	// ����I��
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_LANGUAGE);
	Combo_ResetContent(hwndCombo);
	int nSelPos = 0;
	UINT uiIndex = 0;
	for (uiIndex=0; uiIndex<SelectLang::psLangInfoList.size(); ++uiIndex) {
		SelectLang::SelLangInfo* psLangInfo = SelectLang::psLangInfoList.at(uiIndex);
		Combo_InsertString(hwndCombo, uiIndex, psLangInfo->szLangName);
		if (_tcscmp(csWindow.szLanguageDll, psLangInfo->szDllName) == 0) {
			nSelPos = uiIndex;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	return;
}


// �_�C�A���O�f�[�^�̎擾
int PropWin::GetData(HWND hwndDlg)
{
	auto& csWindow = common.window;

	// ����E�B���h�E���J�����Ƃ��c�[���o�[��\������
	csWindow.bDispToolBar = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispTOOLBAR);

	// ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������
	csWindow.bDispFuncKeyWnd = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispFUNCKEYWND);

	// �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1)) {
		csWindow.nFuncKeyWnd_Place = 0;
	}
	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2)) {
		csWindow.nFuncKeyWnd_Place = 1;
	}

	// 2002/11/04 Moca �t�@���N�V�����L�[�̃O���[�v�{�^����
	csWindow.nFuncKeyWnd_GroupNum = ::GetDlgItemInt(hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM, NULL, FALSE);
	if (csWindow.nFuncKeyWnd_GroupNum < 1) {
		csWindow.nFuncKeyWnd_GroupNum = 1;
	}
	if (csWindow.nFuncKeyWnd_GroupNum > 12) {
		csWindow.nFuncKeyWnd_GroupNum = 12;
	}

	// From Here@@@ 2003.06.13 MIK
	//	Feb. 12, 2007 genta TAB�֘A�́u�^�u�o�[�v�V�[�g�ֈړ�
	// To Here@@@ 2003.06.13 MIK

	// ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������ 
	csWindow.bDispStatusBar = DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispSTATUSBAR);

	// ���[���[�̃^�C�v//	del 2008/7/4 Uchi
//	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_nRulerType_0)) {
//		csWindow.nRulerType = 0;
//	}
//	if (DlgButton_IsChecked(hwndDlg, IDC_RADIO_nRulerType_1)) {
//		csWindow.nRulerType = 1;
//	}

	// ���[���[����
	csWindow.nRulerHeight = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nRulerHeight, NULL, FALSE);
	if (csWindow.nRulerHeight < IDC_SPIN_nRulerHeight_MIN) {
		csWindow.nRulerHeight = IDC_SPIN_nRulerHeight_MIN;
	}
	if (csWindow.nRulerHeight > IDC_SPIN_nRulerHeight_MAX) {
		csWindow.nRulerHeight = IDC_SPIN_nRulerHeight_MAX;
	}
	// ���[���[�ƃe�L�X�g�̌���
	csWindow.nRulerBottomSpace = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nRulerBottomSpace, NULL, FALSE);
	if (csWindow.nRulerBottomSpace < 0) {
		csWindow.nRulerBottomSpace = 0;
	}
	if (csWindow.nRulerBottomSpace > 32) {
		csWindow.nRulerBottomSpace = 32;
	}

	//	Sep. 18. 2002 genta �s�ԍ��ƃe�L�X�g�̌���
	csWindow.nLineNumRightSpace = ::GetDlgItemInt(hwndDlg, IDC_EDIT_nLineNumberRightSpace, NULL, FALSE);
	if (csWindow.nLineNumRightSpace < 0) {
		csWindow.nLineNumRightSpace = 0;
	}
	if (csWindow.nLineNumRightSpace > 32) {
		csWindow.nLineNumRightSpace = 32;
	}

	// �����X�N���[���o�[
	csWindow.bScrollBarHorz = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bScrollBarHorz);

	// �A�C�R���t�����j���[
	csWindow.bMenuIcon = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bMenuIcon);

	//	2001/06/20 Start by asa-o:	�X�N���[���̓���
	csWindow.bSplitterWndVScroll = DlgButton_IsChecked(hwndDlg, IDC_CHECK_SplitterWndVScroll);
	csWindow.bSplitterWndHScroll = DlgButton_IsChecked(hwndDlg, IDC_CHECK_SplitterWndHScroll);
	//	2001/06/20 End

	//	Apr. 05, 2003 genta �E�B���h�E�L���v�V�����̃J�X�^�}�C�Y
	::DlgItem_GetText(hwndDlg, IDC_WINCAPTION_ACTIVE, csWindow.szWindowCaptionActive,
		_countof(csWindow.szWindowCaptionActive));
	::DlgItem_GetText(hwndDlg, IDC_WINCAPTION_INACTIVE, csWindow.szWindowCaptionInactive,
		_countof(csWindow.szWindowCaptionInactive));

	// ����I��
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_LANGUAGE);
	int nSelPos = Combo_GetCurSel(hwndCombo);
	SelectLang::SelLangInfo *psLangInfo = SelectLang::psLangInfoList.at(nSelPos);
	if (_tcscmp(csWindow.szLanguageDll, psLangInfo->szDllName) != 0) {
		_tcsncpy(csWindow.szLanguageDll, psLangInfo->szDllName, _countof(csWindow.szLanguageDll));
	}
	return TRUE;
}


//	From Here Sept. 9, 2000 JEPRO
//	�`�F�b�N��Ԃɉ����ă_�C�A���O�{�b�N�X�v�f��Enable/Disable��
//	�K�؂ɐݒ肷��
void PropWin::EnableWinPropInput(HWND hwndDlg)
{
	//	�t�@�N�V�����L�[��\�����邩�ǂ���
	if (DlgButton_IsChecked(hwndDlg, IDC_CHECK_DispFUNCKEYWND)) {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM), TRUE);	// IDC_GROUP_FUNCKEYWND_POSITION->IDC_EDIT_FUNCKEYWND_GROUPNUM 2008/7/4 Uchi
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1), TRUE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2), TRUE);
	}else {
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_EDIT_FUNCKEYWND_GROUPNUM), FALSE);	// IDC_GROUP_FUNCKEYWND_POSITION->IDC_EDIT_FUNCKEYWND_GROUPNUM 2008/7/4 Uchi
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE1), FALSE);
		::EnableWindow(::GetDlgItem(hwndDlg, IDC_RADIO_FUNCKEYWND_PLACE2), FALSE);
	}
}
//	To Here Sept. 9, 2000

