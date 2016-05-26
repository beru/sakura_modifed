/*!	@file
	@brief �����R�[�h�Z�b�g�ݒ�_�C�A���O�{�b�N�X

	@author Uchi
	@date 2010/6/14  �V�K�쐬
*/
/*
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/DlgSetCharSet.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "env/DllSharedData.h"
#include "charset/CodePage.h"
#include "sakura_rc.h"
#include "sakura.hh"

// �����R�[�h�Z�b�g�ݒ� DlgSetCharSet
const DWORD p_helpids[] = {
	IDOK,							HIDOK_GREP,							// ����
	IDCANCEL,						HIDCANCEL_GREP,						// �L�����Z��
	IDC_BUTTON_HELP,				HIDC_GREP_BUTTON_HELP,				// �w���v
	IDC_COMBO_CHARSET,				HIDC_OPENDLG_COMBO_CODE,			// �����R�[�h�Z�b�g
	IDC_CHECK_BOM,					HIDC_OPENDLG_CHECK_BOM,				// ����
	IDC_CHECK_CP,					HIDC_OPENDLG_CHECK_CP,				// CP
	0, 0
};


DlgSetCharSet::DlgSetCharSet()
{
	pnCharSet = NULL;			// �����R�[�h�Z�b�g
	pbBom = NULL;				// �����R�[�h�Z�b�g
	bCP = false;
}


// ���[�_���_�C�A���O�̕\��
INT_PTR DlgSetCharSet::DoModal(
	HINSTANCE hInstance,
	HWND hwndParent,
	EncodingType* pnCharSet,
	bool* pbBom
	)
{
	pnCharSet = pnCharSet;	// �����R�[�h�Z�b�g
	pbBom = pbBom;			// BOM

	return Dialog::DoModal(hInstance, hwndParent, IDD_SETCHARSET, (LPARAM)NULL);
}


BOOL DlgSetCharSet::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwndDlg);
	
	hwndCharSet = GetItemHwnd(IDC_COMBO_CHARSET);	// �����R�[�h�Z�b�g�R���{�{�b�N�X
	hwndCheckBOM = GetItemHwnd(IDC_CHECK_BOM);		// BOM�`�F�b�N�{�b�N�X

	// �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ���
	Combo_SetExtendedUI(hwndCharSet, TRUE);

	// �����R�[�h�Z�b�g�I���R���{�{�b�N�X������
	CodeTypesForCombobox codeTypes;
	Combo_ResetContent(hwndCharSet);
	for (size_t i=1; i<codeTypes.GetCount(); ++i) {
		int idx = Combo_AddString(hwndCharSet, codeTypes.GetName(i));
		Combo_SetItemData(hwndCharSet, idx, codeTypes.GetCode(i));
	}

	// ���N���X�����o
	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}


BOOL DlgSetCharSet::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_CHECK_CP:
		if (!bCP) {
			bCP = true;
			EnableItem(IDC_CHECK_CP, false);
			CodePage::AddComboCodePages( GetHwnd(), hwndCharSet, -1 );
		}
		return TRUE;
	case IDC_BUTTON_HELP:
		//�u�����R�[�h�Z�b�g�ݒ�v�̃w���v
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_CHG_CHARSET));
		return TRUE;
	case IDOK:
		// �_�C�A���O�f�[�^�̎擾
		if (GetData()) {
			CloseDialog(TRUE);
		}
		return TRUE;
	case IDCANCEL:
		CloseDialog(FALSE);
		return TRUE;
	}

	// ���N���X�����o
	return Dialog::OnBnClicked(wID);
}


// BOM �̐ݒ�
void DlgSetCharSet::SetBOM(void)
{
	WPARAM fCheck;
	int nIdx = Combo_GetCurSel(hwndCharSet);
	LRESULT lRes = Combo_GetItemData(hwndCharSet, nIdx);
	CodeTypeName codeTypeName(lRes);
	if (codeTypeName.UseBom()) {
		::EnableWindow(hwndCheckBOM, TRUE);
		if (lRes == *pnCharSet) {
			fCheck = *pbBom ? BST_CHECKED : BST_UNCHECKED;
		}else {
			fCheck = codeTypeName.IsBomDefOn() ? BST_CHECKED : BST_UNCHECKED;
		}
	}else {
		::EnableWindow(hwndCheckBOM, FALSE);
		fCheck = BST_UNCHECKED;
	}
	BtnCtl_SetCheck(hwndCheckBOM, fCheck);
}


// �����R�[�h�I�����̏���
BOOL DlgSetCharSet::OnCbnSelChange(HWND hwndCtl, int wID)
{
	int 		nIdx;
	LRESULT		lRes;
	WPARAM		fCheck;

	switch (wID) {
	// �����R�[�h�̕ύX��BOM�`�F�b�N�{�b�N�X�ɔ��f
	case IDC_COMBO_CHARSET:
		SetBOM();
		nIdx = Combo_GetCurSel(hwndCtl);
		lRes = Combo_GetItemData(hwndCtl, nIdx);
		CodeTypeName	codeTypeName(lRes);
		if (codeTypeName.UseBom()) {
			::EnableWindow(hwndCheckBOM, TRUE);
			if (lRes == *pnCharSet) {
				fCheck = *pbBom ? BST_CHECKED : BST_UNCHECKED;
			}else {
				fCheck = codeTypeName.IsBomDefOn() ? BST_CHECKED : BST_UNCHECKED;
			}
		}else {
			::EnableWindow(hwndCheckBOM, FALSE);
			fCheck = BST_UNCHECKED;
		}
		BtnCtl_SetCheck(hwndCheckBOM, fCheck);
		break;
	}
	return TRUE;
}


LPVOID DlgSetCharSet::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


// �_�C�A���O�f�[�^�̐ݒ�
void DlgSetCharSet::SetData(void)
{
	// �����R�[�h�Z�b�g
	CodeTypesForCombobox codeTypes;

	int nIdxOld = Combo_GetCurSel(hwndCharSet);
	int nCurIdx = -1;
	for (int nIdx=0; nIdx<Combo_GetCount(hwndCharSet); ++nIdx) {
		EncodingType nCharSet = (EncodingType)Combo_GetItemData( hwndCharSet, nIdx );
		if (nCharSet == *pnCharSet) {
			nCurIdx = nIdx;
		}
	}
	if (nCurIdx == -1) {
		bCP = true;
		CheckButton(IDC_CHECK_CP, true);
		EnableItem(IDC_CHECK_CP, false);
		nCurIdx = CodePage::AddComboCodePages(GetHwnd(), hwndCharSet, *pnCharSet);
		if (nCurIdx == -1) {
			nCurIdx = nIdxOld;
		}
	}
	Combo_SetCurSel(hwndCharSet, nCurIdx);

	// BOM��ݒ�
	SetBOM();
}


// �_�C�A���O�f�[�^�̎擾
// TRUE==����  FALSE==���̓G���[
int DlgSetCharSet::GetData(void)
{
	// �����R�[�h�Z�b�g
	int nIdx = Combo_GetCurSel(hwndCharSet);
	*pnCharSet = (EncodingType)Combo_GetItemData(hwndCharSet, nIdx);

	// BOM
	*pbBom = (BtnCtl_GetCheck(hwndCheckBOM) == BST_CHECKED);

	return TRUE;
}

