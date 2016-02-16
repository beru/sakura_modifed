/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�����v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK, genta
	Copyright (C) 2002, YAZAKI, MIK
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "extmodule/Bregexp.h"	// 2007.08/12 genta �o�[�W�����擾
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10500
	IDC_EDIT_REGEXPLIB,					HIDC_EDIT_REGEXPLIB,				// ���K�\�����C�u�����I��	// 2007.09.02 genta
	IDC_LABEL_REGEXP,					HIDC_EDIT_REGEXPLIB,
	IDC_LABEL_REGEXP_VER,				HIDC_LABEL_REGEXPVER,				// ���K�\�����C�u�����o�[�W����	// 2007.09.02 genta
	IDC_CHECK_bCaretTextForSearch,		HIDC_CHECK_bCaretTextForSearch,		// �J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ���	// 2006.08.23 ryoji
	IDC_CHECK_INHERIT_KEY_OTHER_VIEW,	HIDC_CHECK_INHERIT_KEY_OTHER_VIEW,	// ���E�O�����ő��̃r���[�̌��������������p��	// 2011.12.18 Moca
	IDC_CHECK_bGrepExitConfirm,			HIDC_CHECK_bGrepExitConfirm,		// GREP�̕ۑ��m�F
	IDC_CHECK_GTJW_RETURN,				HIDC_CHECK_GTJW_RETURN,				// �^�O�W�����v�i�G���^�[�L�[�j
	IDC_CHECK_GTJW_LDBLCLK,				HIDC_CHECK_GTJW_LDBLCLK,			// �^�O�W�����v�i�_�u���N���b�N�j
	IDC_CHECK_GREPREALTIME,				HIDC_CHECK_GREPREALTIME,			// ���A���^�C���ŕ\������	// 2006.08.08 ryoji
	IDC_COMBO_TAGJUMP,					HIDC_COMBO_TAGJUMP,					// �^�O�t�@�C���̌���
	IDC_COMBO_KEYWORD_TAGJUMP,			HIDC_COMBO_KEYWORD_TAGJUMP,			// �^�O�t�@�C���̌���
//	IDC_STATIC,							-1,
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
INT_PTR CALLBACK PropGrep::DlgProc_page(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	return DlgProc(reinterpret_cast<pDispatchPage>(&PropGrep::DispatchEvent), hwndDlg, uMsg, wParam, lParam);
}
//	To Here Jun. 2, 2001 genta

// ���b�Z�[�W����
INT_PTR PropGrep::DispatchEvent(
	HWND hwndDlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
//	WORD		wNotifyCode;
//	WORD		wID;
//	HWND		hwndCtl;
	NMHDR*		pNMHDR;
//	int			nVal;
//    LPDRAWITEMSTRUCT pDis;

	switch (uMsg) {
	case WM_INITDIALOG:
		// �_�C�A���O�f�[�^�̐ݒ� Grep
		SetData(hwndDlg);
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����

		return TRUE;
	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
//		switch (idCtrl) {
//		default:
			switch (pNMHDR->code) {
			case PSN_HELP:
				OnHelp(hwndDlg, IDD_PROP_GREP);
				return TRUE;
			case PSN_KILLACTIVE:
				// �_�C�A���O�f�[�^�̎擾 Grep
				GetData(hwndDlg);
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = ID_PROPCOM_PAGENUM_GREP;
				return TRUE;
			}
//			break;	// default
//		}
		break;	// WM_NOTIFY
	case WM_COMMAND:
		//	2007.08.12 genta ���K�\��DLL�̕ύX�ɉ�����Version���Ď擾����
		if (wParam == MAKEWPARAM(IDC_EDIT_REGEXPLIB, EN_KILLFOCUS)) {
			SetRegexpVersion(hwndDlg);
		}
		break;

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

struct TagJumpMode {
	DWORD nMethod;
	DWORD nNameID;
};

// �_�C�A���O�f�[�^�̐ݒ�
void PropGrep::SetData(HWND hwndDlg)
{
	auto& csSearch = m_common.m_search;
	// 2006.08.23 ryoji �J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ���
	::CheckDlgButton(hwndDlg, IDC_CHECK_bCaretTextForSearch, csSearch.m_bCaretTextForSearch);

	CheckDlgButtonBool(hwndDlg, IDC_CHECK_INHERIT_KEY_OTHER_VIEW, csSearch.m_bInheritKeyOtherView);

	// Grep���[�h�ŕۑ��m�F���邩
	::CheckDlgButton(hwndDlg, IDC_CHECK_bGrepExitConfirm, csSearch.m_bGrepExitConfirm);

	// Grep���ʂ̃��A���^�C���\��
	::CheckDlgButton(hwndDlg, IDC_CHECK_GREPREALTIME, csSearch.m_bGrepRealTimeView);	// 2006.08.08 ryoji ID�C��


	// Grep���[�h: �G���^�[�L�[�Ń^�O�W�����v
	::CheckDlgButton(hwndDlg, IDC_CHECK_GTJW_RETURN, csSearch.m_bGTJW_RETURN);

	// Grep���[�h: �_�u���N���b�N�Ń^�O�W�����v
	::CheckDlgButton(hwndDlg, IDC_CHECK_GTJW_LDBLCLK, csSearch.m_bGTJW_LDBLCLK);

	//	2007.08.12 genta ���K�\��DLL
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_REGEXPLIB), _countof(csSearch.m_szRegexpLib) - 1);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_REGEXPLIB, csSearch.m_szRegexpLib);
	SetRegexpVersion(hwndDlg);

	TagJumpMode tagJumpMode1Arr[] ={
		{ 0, STR_TAGJUMP_0 },
		{ 1, STR_TAGJUMP_1 },
		//{ 2, STR_TAGJUMP_2 },
		{ 3, STR_TAGJUMP_3 }
	};
	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAGJUMP);
	Combo_ResetContent(hwndCombo);
	int nSelPos = 0;
	for (int i=0; i<_countof(tagJumpMode1Arr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(tagJumpMode1Arr[i].nNameID));
		Combo_SetItemData(hwndCombo, i, tagJumpMode1Arr[i].nMethod);
		if (tagJumpMode1Arr[i].nMethod == m_common.m_search.m_nTagJumpMode) {
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	TagJumpMode tagJumpMode2Arr[] ={
		{ 0, STR_TAGJUMP_0 },
		{ 1, STR_TAGJUMP_1 },
		{ 2, STR_TAGJUMP_2 },
		{ 3, STR_TAGJUMP_3 }
	};
	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_KEYWORD_TAGJUMP);
	Combo_ResetContent(hwndCombo);
	nSelPos = 0;
	for (int i=0; i<_countof(tagJumpMode2Arr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(tagJumpMode2Arr[i].nNameID));
		Combo_SetItemData(hwndCombo, i, tagJumpMode2Arr[i].nMethod);
		if (tagJumpMode2Arr[i].nMethod == m_common.m_search.m_nTagJumpModeKeyword) {
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	return;
}


// �_�C�A���O�f�[�^�̎擾
int PropGrep::GetData(HWND hwndDlg)
{
	auto& csSearch = m_common.m_search;

	// 2006.08.23 ryoji �J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ���
	csSearch.m_bCaretTextForSearch = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bCaretTextForSearch);

	csSearch.m_bInheritKeyOtherView = DlgButton_IsChecked(hwndDlg, IDC_CHECK_INHERIT_KEY_OTHER_VIEW);

	// Grep���[�h�ŕۑ��m�F���邩
	csSearch.m_bGrepExitConfirm = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bGrepExitConfirm);

	// Grep���ʂ̃��A���^�C���\��
	csSearch.m_bGrepRealTimeView = DlgButton_IsChecked(hwndDlg, IDC_CHECK_GREPREALTIME);	// 2006.08.08 ryoji ID�C��

	// Grep���[�h: �G���^�[�L�[�Ń^�O�W�����v
	csSearch.m_bGTJW_RETURN = DlgButton_IsChecked(hwndDlg, IDC_CHECK_GTJW_RETURN);

	// Grep���[�h: �_�u���N���b�N�Ń^�O�W�����v
	csSearch.m_bGTJW_LDBLCLK = DlgButton_IsChecked(hwndDlg, IDC_CHECK_GTJW_LDBLCLK);

	//	2007.08.12 genta ���K�\��DLL
	::DlgItem_GetText(hwndDlg, IDC_EDIT_REGEXPLIB, csSearch.m_szRegexpLib, _countof(csSearch.m_szRegexpLib));

	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAGJUMP);
	int nSelPos = Combo_GetCurSel(hwndCombo);
	m_common.m_search.m_nTagJumpMode = Combo_GetItemData(hwndCombo, nSelPos);
	
	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_KEYWORD_TAGJUMP);
	nSelPos = Combo_GetCurSel(hwndCombo);
	m_common.m_search.m_nTagJumpModeKeyword = Combo_GetItemData(hwndCombo, nSelPos);

	return TRUE;
}

void PropGrep::SetRegexpVersion(HWND hwndDlg)
{
	TCHAR regexp_dll[_MAX_PATH];
	
	::DlgItem_GetText(hwndDlg, IDC_EDIT_REGEXPLIB, regexp_dll, _countof(regexp_dll));
	Bregexp breg;
	if (DLL_SUCCESS != breg.InitDll(regexp_dll)) {
		::DlgItem_SetText(hwndDlg, IDC_LABEL_REGEXP_VER, LS(STR_PROPCOMGREP_DLL));
		return;
	}
	::DlgItem_SetText(hwndDlg, IDC_LABEL_REGEXP_VER, breg.GetVersionT());
}

