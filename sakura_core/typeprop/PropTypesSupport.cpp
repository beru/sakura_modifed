// �^�C�v�ʐݒ� - �x��

#include "StdAfx.h"
#include "PropTypes.h"
#include "dlg/DlgOpenFile.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/fileUtil.h" // _IS_REL_PATH
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids3[] = {	//11500
	IDC_EDIT_HOKANFILE,				HIDC_EDIT_HOKANFILE,				// �P��t�@�C����
	IDC_BUTTON_HOKANFILE_REF,		HIDC_BUTTON_HOKANFILE_REF,			// ���͕⊮ �P��t�@�C���Q��
	IDC_COMBO_HOKAN_TYPE,			HIDC_COMBO_HOKAN_TYPE,				// ���͕⊮�^�C�v
	IDC_CHECK_HOKANLOHICASE,		HIDC_CHECK_HOKANLOHICASE,			// ���͕⊮�̉p�啶��������
	IDC_CHECK_HOKANBYFILE,			HIDC_CHECK_HOKANBYFILE,				// ���݂̃t�@�C��������͕⊮
	IDC_CHECK_HOKANBYKEYWORD,		HIDC_CHECK_HOKANBYKEYWORD,			// �����L�[���[�h������͕⊮

	IDC_EDIT_TYPEEXTHELP,			HIDC_EDIT_TYPEEXTHELP,				// �O���w���v�t�@�C����
	IDC_BUTTON_TYPEOPENHELP,		HIDC_BUTTON_TYPEOPENHELP,			// �O���w���v�t�@�C���Q��
	IDC_EDIT_TYPEEXTHTMLHELP,		HIDC_EDIT_TYPEEXTHTMLHELP,			// �O��HTML�w���v�t�@�C����
	IDC_BUTTON_TYPEOPENEXTHTMLHELP,	HIDC_BUTTON_TYPEOPENEXTHTMLHELP,	// �O��HTML�w���v�t�@�C���Q��
	IDC_CHECK_TYPEHTMLHELPISSINGLE,	HIDC_CHECK_TYPEHTMLHELPISSINGLE,	// �r���[�A�𕡐��N�����Ȃ�

	IDC_CHECK_CHKENTERATEND,		HIDC_CHECK_CHKENTERATEND,			// �ۑ����ɉ��s�R�[�h�̍��݂��x������	// 2013/4/14 Uchi
	//	IDC_STATIC,						-1,
	0, 0
};


struct HokanMethod {
	int nMethod;
	std::wstring name;
};

static std::vector<HokanMethod>* GetHokanMethodList()
{
	static std::vector<HokanMethod> methodList;
	return &methodList;
}


// �^�C�v�ʐݒ�̎x���^�u�Ɋւ��鏈��

// ���b�Z�[�W����
INT_PTR PropTypesSupport::DispatchEvent(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
	)
{
	WORD		wNotifyCode;
	WORD		wID;
	NMHDR*		pNMHDR;

	switch (uMsg) {
	case WM_INITDIALOG:
		// �_�C�A���O�f�[�^�̐ݒ� p2
		SetData(hwndDlg);
		::SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);

		// ���[�U�[���G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����
		// ���͕⊮ �P��t�@�C��
		EditCtl_LimitText(::GetDlgItem(hwndDlg, IDC_EDIT_HOKANFILE), _MAX_PATH - 1);

		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	// �ʒm�R�[�h
		wID			= LOWORD(wParam);	// ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID
//		hwndCtl		= (HWND) lParam;	// �R���g���[���̃n���h��
		switch (wNotifyCode) {
		// �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ
		case BN_CLICKED:
			// �_�C�A���O�f�[�^�̎擾 p2
			GetData(hwndDlg);
			switch (wID) {
			case IDC_BUTTON_HOKANFILE_REF:	// ���͕⊮ �P��t�@�C���́u�Q��...�v�{�^��
				{
					DlgOpenFile	dlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					if (_IS_REL_PATH(types.szHokanFile)) {
						GetInidirOrExedir(szPath, types.szHokanFile);
					}else {
						_tcscpy(szPath, types.szHokanFile);
					}
					// �t�@�C���I�[�v���_�C�A���O�̏�����
					dlgOpenFile.Create(
						hInstance,
						hwndDlg,
						_T("*.*"),
						szPath
					);
					if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
						_tcscpy(types.szHokanFile, szPath);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_HOKANFILE, types.szHokanFile);
					}
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENHELP:	// �O���w���v�P�́u�Q��...�v�{�^��
				{
					DlgOpenFile	dlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					if (_IS_REL_PATH(types.szExtHelp)) {
						GetInidirOrExedir(szPath, types.szExtHelp, true);
					}else {
						_tcscpy(szPath, types.szExtHelp);
					}
					// �t�@�C���I�[�v���_�C�A���O�̏�����
					dlgOpenFile.Create(
						hInstance,
						hwndDlg,
						_T("*.hlp;*.chm;*.col"),
						szPath
					);
					if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
						_tcscpy(types.szExtHelp, szPath);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_TYPEEXTHELP, types.szExtHelp);
					}
				}
				return TRUE;
			case IDC_BUTTON_TYPEOPENEXTHTMLHELP:	// �O��HTML�w���v�́u�Q��...�v�{�^��
				{
					DlgOpenFile	dlgOpenFile;
					TCHAR			szPath[_MAX_PATH + 1];
					if (_IS_REL_PATH(types.szExtHtmlHelp)) {
						GetInidirOrExedir(szPath, types.szExtHtmlHelp, true);
					}else {
						_tcscpy(szPath, types.szExtHtmlHelp);
					}
					// �t�@�C���I�[�v���_�C�A���O�̏�����
					dlgOpenFile.Create(
						hInstance,
						hwndDlg,
						_T("*.chm;*.col"),
						szPath
					);
					if (dlgOpenFile.DoModal_GetOpenFileName(szPath)) {
						_tcscpy(types.szExtHtmlHelp, szPath);
						::DlgItem_SetText(hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, types.szExtHtmlHelp);
					}
				}
				return TRUE;
			}
			break;	// BN_CLICKED
		}
		break;	// WM_COMMAND
	case WM_NOTIFY:
//		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
//		pMNUD  = (NM_UPDOWN*)lParam;
		switch (pNMHDR->code) {
		case PSN_HELP:
			OnHelp(hwndDlg, IDD_PROP_SUPPORT);
			return TRUE;
		case PSN_KILLACTIVE:
			// �_�C�A���O�f�[�^�̎擾 p2
			GetData(hwndDlg);
			return TRUE;
		case PSN_SETACTIVE:
			nPageNum = ID_PROPTYPE_PAGENUM_SUPPORT;
			return TRUE;
		}
		break;

	case WM_HELP:
		{
			HELPINFO* p = (HELPINFO*) lParam;
			MyWinHelp((HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids3);
		}
		return TRUE;
		// NOTREACHED
//		break;

	// Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp(hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids3);
		return TRUE;

	}
	return FALSE;
}

// �_�C�A���O�f�[�^�̐ݒ�
void PropTypesSupport::SetData(HWND hwndDlg)
{
	// ���͕⊮ �P��t�@�C��
	::DlgItem_SetText(hwndDlg, IDC_EDIT_HOKANFILE, types.szHokanFile);

	{
		HWND hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_HOKAN_TYPE);
		std::vector<HokanMethod>* pMedothList = GetHokanMethodList();
		ApiWrap::Combo_AddString(hCombo, LS(STR_SMART_INDENT_NONE));
		Combo_SetCurSel(hCombo, 0);
		size_t nSize = pMedothList->size();
		for (size_t i=0; i<nSize; ++i) {
			ApiWrap::Combo_AddString(hCombo, (*pMedothList)[i].name.c_str());
			if (types.nHokanType == (*pMedothList)[i].nMethod) {
				Combo_SetCurSel(hCombo, i + 1);
			}
		}
	}

	// ���͕⊮�@�\�F�p�啶���������𓯈ꎋ����
	::CheckDlgButton(hwndDlg, IDC_CHECK_HOKANLOHICASE, types.bHokanLoHiCase ? BST_CHECKED : BST_UNCHECKED);

	// �t�@�C������̕⊮�@�\
	::CheckDlgButton(hwndDlg, IDC_CHECK_HOKANBYFILE, types.bUseHokanByFile ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_HOKANBYKEYWORD, types.bUseHokanByKeyword);

	::DlgItem_SetText(hwndDlg, IDC_EDIT_TYPEEXTHELP, types.szExtHelp);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, types.szExtHtmlHelp);
	::CheckDlgButton(hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE, types.bHtmlHelpIsSingle ? BST_CHECKED : BST_UNCHECKED);

	// �ۑ����ɉ��s�R�[�h�̍��݂��x������	2013/4/14 Uchi
	::CheckDlgButton(hwndDlg, IDC_CHECK_CHKENTERATEND, types.bChkEnterAtEnd ? BST_CHECKED : BST_UNCHECKED);
}

// �_�C�A���O�f�[�^�̎擾
int PropTypesSupport::GetData(HWND hwndDlg)
{
	// ���͕⊮�@�\�F�p�啶���������𓯈ꎋ����
	types.bHokanLoHiCase = DlgButton_IsChecked(hwndDlg, IDC_CHECK_HOKANLOHICASE);

	types.bUseHokanByFile = DlgButton_IsChecked(hwndDlg, IDC_CHECK_HOKANBYFILE);
	types.bUseHokanByKeyword = DlgButton_IsChecked(hwndDlg, IDC_CHECK_HOKANBYKEYWORD);

	// ���͕⊮ �P��t�@�C��
	::DlgItem_GetText(hwndDlg, IDC_EDIT_HOKANFILE, types.szHokanFile, _countof2(types.szHokanFile));

	// ���͕⊮���
	{
		HWND hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_HOKAN_TYPE);
		int i = Combo_GetCurSel(hCombo);
		if (i == 0) {
			types.nHokanType = 0;
		}else if (i != CB_ERR) {
			types.nHokanType = (*GetHokanMethodList())[i - 1].nMethod;
		}
	}

	::DlgItem_GetText(hwndDlg, IDC_EDIT_TYPEEXTHELP, types.szExtHelp, _countof2(types.szExtHelp));
	::DlgItem_GetText(hwndDlg, IDC_EDIT_TYPEEXTHTMLHELP, types.szExtHtmlHelp, _countof2(types.szExtHtmlHelp));
	types.bHtmlHelpIsSingle = DlgButton_IsChecked(hwndDlg, IDC_CHECK_TYPEHTMLHELPISSINGLE);

	// �ۑ����ɉ��s�R�[�h�̍��݂��x������	2013/4/14 Uchi
	types.bChkEnterAtEnd = DlgButton_IsChecked(hwndDlg, IDC_CHECK_CHKENTERATEND);

	return TRUE;
}

// 2001/06/13 End

// �⊮��ʂ̒ǉ�
void PropTypesSupport::AddHokanMethod(int nMethod, const wchar_t* szName)
{
	HokanMethod item = { nMethod, std::wstring(szName) };
	GetHokanMethodList()->push_back(item);
}

void PropTypesSupport::RemoveHokanMethod(int nMethod, const wchar_t* szName)
{
	int nSize = GetHokanMethodList()->size();
	for (int i=0; i<nSize; ++i) {
		if ((*GetHokanMethodList())[i].nMethod == nMethod) {
			GetHokanMethodList()->erase(GetHokanMethodList()->begin() + i);
			break;
		}
	}
}

