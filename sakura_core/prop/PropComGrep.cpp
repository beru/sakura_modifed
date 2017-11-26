/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�����v�y�[�W
*/
#include "StdAfx.h"
#include "prop/PropCommon.h"
#include "extmodule/Bregexp.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids[] = {	//10500
	IDC_EDIT_REGEXPLIB,					HIDC_EDIT_REGEXPLIB,				// ���K�\�����C�u�����I��
	IDC_LABEL_REGEXP,					HIDC_EDIT_REGEXPLIB,
	IDC_LABEL_REGEXP_VER,				HIDC_LABEL_REGEXPVER,				// ���K�\�����C�u�����o�[�W����
	IDC_CHECK_bCaretTextForSearch,		HIDC_CHECK_bCaretTextForSearch,		// �J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ���
	IDC_CHECK_INHERIT_KEY_OTHER_VIEW,	HIDC_CHECK_INHERIT_KEY_OTHER_VIEW,	// ���E�O�����ő��̃r���[�̌��������������p��
	IDC_CHECK_bGrepExitConfirm,			HIDC_CHECK_bGrepExitConfirm,		// GREP�̕ۑ��m�F
	IDC_CHECK_GTJW_RETURN,				HIDC_CHECK_GTJW_RETURN,				// �^�O�W�����v�i�G���^�[�L�[�j
	IDC_CHECK_GTJW_LDBLCLK,				HIDC_CHECK_GTJW_LDBLCLK,			// �^�O�W�����v�i�_�u���N���b�N�j
	IDC_CHECK_GREPREALTIME,				HIDC_CHECK_GREPREALTIME,			// ���A���^�C���ŕ\������
	IDC_COMBO_TAGJUMP,					HIDC_COMBO_TAGJUMP,					// �^�O�t�@�C���̌���
	IDC_COMBO_KEYWORD_TAGJUMP,			HIDC_COMBO_KEYWORD_TAGJUMP,			// �^�O�t�@�C���̌���
//	IDC_STATIC,							-1,
	0, 0
};

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
			case PSN_SETACTIVE:
				nPageNum = ID_PROPCOM_PAGENUM_GREP;
				return TRUE;
			}
//			break;	// default
//		}
		break;	// WM_NOTIFY
	case WM_COMMAND:
		// ���K�\��DLL�̕ύX�ɉ�����Version���Ď擾����
		if (wParam == MAKEWPARAM(IDC_EDIT_REGEXPLIB, EN_KILLFOCUS)) {
			SetRegexpVersion(hwndDlg);
		}
		break;

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

struct TagJumpMode {
	DWORD nMethod;
	DWORD nNameID;
};

// �_�C�A���O�f�[�^�̐ݒ�
void PropGrep::SetData(HWND hwndDlg)
{
	auto& csSearch = common.search;
	// �J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ���
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_bCaretTextForSearch, csSearch.bCaretTextForSearch);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_INHERIT_KEY_OTHER_VIEW, csSearch.bInheritKeyOtherView);

	// Grep���[�h�ŕۑ��m�F���邩
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_bGrepExitConfirm, csSearch.bGrepExitConfirm);

	// Grep���ʂ̃��A���^�C���\��
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_GREPREALTIME, csSearch.bGrepRealTimeView);

	
	// Grep���[�h: �G���^�[�L�[�Ń^�O�W�����v
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_GTJW_RETURN, csSearch.bGTJW_Return);

	// Grep���[�h: �_�u���N���b�N�Ń^�O�W�����v
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_GTJW_LDBLCLK, csSearch.bGTJW_DoubleClick);

	// ���K�\��DLL
	EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_REGEXPLIB), _countof(csSearch.szRegexpLib) - 1);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_REGEXPLIB, csSearch.szRegexpLib);
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
	for (size_t i=0; i<_countof(tagJumpMode1Arr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(tagJumpMode1Arr[i].nNameID));
		Combo_SetItemData(hwndCombo, i, tagJumpMode1Arr[i].nMethod);
		if (tagJumpMode1Arr[i].nMethod == common.search.nTagJumpMode) {
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
	for (size_t i=0; i<_countof(tagJumpMode2Arr); ++i) {
		Combo_InsertString(hwndCombo, i, LS(tagJumpMode2Arr[i].nNameID));
		Combo_SetItemData(hwndCombo, i, tagJumpMode2Arr[i].nMethod);
		if (tagJumpMode2Arr[i].nMethod == common.search.nTagJumpModeKeyword) {
			nSelPos = i;
		}
	}
	Combo_SetCurSel(hwndCombo, nSelPos);

	return;
}


// �_�C�A���O�f�[�^�̎擾
int PropGrep::GetData(HWND hwndDlg)
{
	auto& csSearch = common.search;

	// �J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ���
	csSearch.bCaretTextForSearch = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bCaretTextForSearch);
	csSearch.bInheritKeyOtherView = DlgButton_IsChecked(hwndDlg, IDC_CHECK_INHERIT_KEY_OTHER_VIEW);

	// Grep���[�h�ŕۑ��m�F���邩
	csSearch.bGrepExitConfirm = DlgButton_IsChecked(hwndDlg, IDC_CHECK_bGrepExitConfirm);

	// Grep���ʂ̃��A���^�C���\��
	csSearch.bGrepRealTimeView = DlgButton_IsChecked(hwndDlg, IDC_CHECK_GREPREALTIME);

	// Grep���[�h: �G���^�[�L�[�Ń^�O�W�����v
	csSearch.bGTJW_Return = DlgButton_IsChecked(hwndDlg, IDC_CHECK_GTJW_RETURN);

	// Grep���[�h: �_�u���N���b�N�Ń^�O�W�����v
	csSearch.bGTJW_DoubleClick = DlgButton_IsChecked(hwndDlg, IDC_CHECK_GTJW_LDBLCLK);

	// ���K�\��DLL
	::DlgItem_GetText(hwndDlg, IDC_EDIT_REGEXPLIB, csSearch.szRegexpLib, _countof(csSearch.szRegexpLib));

	HWND hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TAGJUMP);
	int nSelPos = Combo_GetCurSel(hwndCombo);
	common.search.nTagJumpMode = Combo_GetItemData(hwndCombo, nSelPos);
	
	hwndCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_KEYWORD_TAGJUMP);
	nSelPos = Combo_GetCurSel(hwndCombo);
	common.search.nTagJumpModeKeyword = Combo_GetItemData(hwndCombo, nSelPos);

	return TRUE;
}

void PropGrep::SetRegexpVersion(HWND hwndDlg)
{
	TCHAR regexp_dll[_MAX_PATH];
	
	::DlgItem_GetText(hwndDlg, IDC_EDIT_REGEXPLIB, regexp_dll, _countof(regexp_dll));
	Bregexp breg;
	if (breg.InitDll(regexp_dll) != InitDllResultType::Success) {
		::DlgItem_SetText(hwndDlg, IDC_LABEL_REGEXP_VER, LS(STR_PROPCOMGREP_DLL));
		return;
	}
	::DlgItem_SetText(hwndDlg, IDC_LABEL_REGEXP_VER, breg.GetVersionT());
}

