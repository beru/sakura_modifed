/*!	@file
	@brief �w��s�ւ̃W�����v�_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/05/31 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, jepro, Stonee
	Copyright (C) 2002, aroka, MIK, YAZAKI
	Copyright (C) 2004, genta
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/DlgJump.h"
#include "doc/EditDoc.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "outline/FuncInfo.h"
#include "outline/FuncInfoArr.h"// 2002/2/10 aroka �w�b�_����
#include "util/shell.h"
#include "window/EditWnd.h"
#include "sakura_rc.h"
#include "sakura.hh"

// �W�����v CDlgJump.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12800
	IDC_BUTTON_JUMP,				HIDC_JUMP_BUTTON_JUMP,			// �W�����v
	IDCANCEL,						HIDCANCEL_JUMP,					// �L�����Z��
	IDC_BUTTON_HELP,				HIDC_JUMP_BUTTON_HELP,			// �w���v
	IDC_CHECK_PLSQL,				HIDC_JUMP_CHECK_PLSQL,			// PL/SQL
	IDC_COMBO_PLSQLBLOCKS,			HIDC_JUMP_COMBO_PLSQLBLOCKS,	// 
	IDC_EDIT_LINENUM,				HIDC_JUMP_EDIT_LINENUM,			// �s�ԍ�
	IDC_EDIT_PLSQL_E1,				HIDC_JUMP_EDIT_PLSQL_E1,		// 
	IDC_RADIO_LINENUM_LAYOUT,		HIDC_JUMP_RADIO_LINENUM_LAYOUT,	// �܂�Ԃ��P��
	IDC_RADIO_LINENUM_CRLF,			HIDC_JUMP_RADIO_LINENUM_CRLF,	// ���s�P��
	IDC_SPIN_LINENUM,				HIDC_JUMP_EDIT_LINENUM,			// 12870,	//
	IDC_SPIN_PLSQL_E1,				HIDC_JUMP_EDIT_PLSQL_E1,		// 12871,	//
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

DlgJump::DlgJump()
{
	nLineNum = 0;			// �s�ԍ�
	bPLSQL = FALSE;		// PL/SQL�\�[�X�̗L���s��
	nPLSQL_E1 = 1;
	nPLSQL_E2 = 1;

	return;
}

// ���[�_���_�C�A���O�̕\��
INT_PTR DlgJump::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
	)
{
	return Dialog::DoModal(hInstance, hwndParent, IDD_JUMP, lParam);
}


// From Here Oct. 6, 2000 JEPRO added �s�ԍ����̓{�b�N�X�ɃX�s���R���g���[����t���邽��
// CDlgPrintSetting.cpp��OnNotify��OnSpin�y��CpropComFile.cpp��DispatchEvent_p2����case WM_NOTIFY���Q�l�ɂ���
BOOL DlgJump::OnNotify(WPARAM wParam, LPARAM lParam)
{
	int nData;
	int idCtrl = (int)wParam;
	NM_UPDOWN* pMNUD  = (NM_UPDOWN*)lParam;
	// �X�s���R���g���[���̏���
	switch (idCtrl) {
	case IDC_SPIN_LINENUM:
	// �W�����v�������s�ԍ��̎w��
		nData = GetItemInt(IDC_EDIT_LINENUM, NULL, FALSE);
		if (pMNUD->iDelta < 0) {
			++nData;
		}else if (pMNUD->iDelta > 0) {
			--nData;
		}
		if (nData < 1) {
			nData = 1;
		}
		SetItemInt(IDC_EDIT_LINENUM, nData, FALSE);
		break;
	case IDC_SPIN_PLSQL_E1:
		nData = GetItemInt(IDC_EDIT_PLSQL_E1, NULL, FALSE);
		if (pMNUD->iDelta < 0) {
			++nData;
		}else if (pMNUD->iDelta > 0) {
			--nData;
		}
		if (nData < 1) {
			nData = 1;
		}
		SetItemInt(IDC_EDIT_PLSQL_E1, nData, FALSE);
		break;
	default:
		break;
	}
	return TRUE;
}
// To Here Oct. 6, 2000


BOOL DlgJump::OnCbnSelChange(HWND hwndCtl, int wID)
{
	int	nIndex;
	int	nWorkLine;
	switch (wID) {
	case IDC_COMBO_PLSQLBLOCKS:
		nIndex = Combo_GetCurSel(GetItemHwnd(IDC_COMBO_PLSQLBLOCKS));
		nWorkLine = (int)Combo_GetItemData(GetItemHwnd(IDC_COMBO_PLSQLBLOCKS), nIndex);
		SetItemInt(IDC_EDIT_PLSQL_E1, nWorkLine, FALSE);
		return TRUE;
	}
	return FALSE;
}

BOOL DlgJump::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		//�u�w��s�փW�����v�v�̃w���v
		// Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_JUMP_DIALOG));	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	case IDC_CHECK_PLSQL:		// PL/SQL�\�[�X�̗L���s��
		if (IsButtonChecked(IDC_CHECK_PLSQL)) {
			EnableItem(IDC_LABEL_PLSQL1, true);	// Sept. 12, 2000 JEPRO
			EnableItem(IDC_LABEL_PLSQL2, true);	// Sept. 12, 2000 JEPRO
			EnableItem(IDC_LABEL_PLSQL3, true);	// Sept. 12, 2000 JEPRO
			EnableItem(IDC_EDIT_PLSQL_E1, true);
			EnableItem(IDC_SPIN_PLSQL_E1, true);	// Oct. 6, 2000 JEPRO
			EnableItem(IDC_COMBO_PLSQLBLOCKS, true);
			pShareData->bLineNumIsCRLF_ForJump = true;
			EnableItem(IDC_RADIO_LINENUM_LAYOUT, false);
			EnableItem(IDC_RADIO_LINENUM_CRLF, false);
		}else {
			EnableItem(IDC_LABEL_PLSQL1, false);	// Sept. 12, 2000 JEPRO
			EnableItem(IDC_LABEL_PLSQL2, false);	// Sept. 12, 2000 JEPRO
			EnableItem(IDC_LABEL_PLSQL3, false);	// Sept. 12, 2000 JEPRO
			EnableItem(IDC_EDIT_PLSQL_E1, false);
			EnableItem(IDC_SPIN_PLSQL_E1, false);	// Oct. 6, 2000 JEPRO
			EnableItem(IDC_COMBO_PLSQLBLOCKS, false);
			EnableItem(IDC_RADIO_LINENUM_LAYOUT, true);
			EnableItem(IDC_RADIO_LINENUM_CRLF, true);
		}
		// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
		if (pShareData->bLineNumIsCRLF_ForJump) {
			CheckButton(IDC_RADIO_LINENUM_LAYOUT, false);
			CheckButton(IDC_RADIO_LINENUM_CRLF, true);
		}else {
			CheckButton(IDC_RADIO_LINENUM_LAYOUT, true);
			CheckButton(IDC_RADIO_LINENUM_CRLF, false);
		}
		return TRUE;
	case IDC_BUTTON_JUMP:			// �w��s�փW�����v	// Feb. 20, 2001 JEPRO �{�^������[IDOK]��[IDC_BUTTON_JUMP]�ɕύX
		// �_�C�A���O�f�[�^�̎擾
		// From Here Feb. 20, 2001 JEPRO ���s���R�����g�A�E�g (CEditView_Command.cpp �� Command_Jump ���֌W���Ă���̂ŎQ�Ƃ̂���)
//		::EndDialog(GetHwnd(), GetData());
//		���s����ǉ�
		if (0 < GetData()) {
			CloseDialog(1);
		}else {
			OkMessage(GetHwnd(), LS(STR_DLGJUMP1));
		}
// To Here Feb. 20, 2001
		{	//@@@ 2002.2.2 YAZAKI �w��s�փW�����v���A�_�C�A���O��\������R�}���h�ƁA���ۂɃW�����v����R�}���h�ɕ����B
			EditDoc* pEditDoc = (EditDoc*)lParam;
			pEditDoc->pEditWnd->GetActiveView().GetCommander().HandleCommand(F_JUMP, true, 0, 0, 0, 0);	// �W�����v�R�}���h���s
		}
		return TRUE;
	case IDCANCEL:
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;
	}
	// ���N���X�����o
	return Dialog::OnBnClicked(wID);
}


// �_�C�A���O�f�[�^�̐ݒ�
void DlgJump::SetData(void)
{
	EditDoc* pEditDoc = (EditDoc*)lParam;
	FuncInfoArr funcInfoArr;
	wchar_t szText[1024];
	int nIndexCurSel = 0;	// Sep. 11, 2004 genta ������

//	GetHwnd() = hwndDlg;
// From Here Oct. 7, 2000 JEPRO �O����͂����s�ԍ���ێ�����悤�ɉ��s��ύX
//	::DlgItem_SetText(GetHwnd(), IDC_EDIT_LINENUM, "");	// �s�ԍ�
	if (nLineNum == 0) {
		SetItemText(IDC_EDIT_LINENUM, _T(""));	// �s�ԍ�
	}else {
		SetItemInt(IDC_EDIT_LINENUM, nLineNum, FALSE);	// �O��̍s�ԍ�
	}
// To Here Oct. 7, 2000
	SetItemInt(IDC_EDIT_PLSQL_E1, nPLSQL_E1, FALSE);
	// PL/SQL�֐����X�g�쐬
	HWND hwndCtrl = GetItemHwnd(IDC_COMBO_PLSQLBLOCKS);
	// �^�C�v�ʂɐݒ肳�ꂽ�A�E�g���C����͕��@
	if (pEditDoc->docType.GetDocumentAttribute().eDefaultOutline == OutlineType::PLSQL) {
		pEditDoc->docOutline.MakeFuncList_PLSQL(&funcInfoArr);
	}
	//$$ �����ɂ��A���C�A�E�g�E���W�b�N�̒P�ʂ����݂��邽�߁A�~�X�̌����ɂȂ�₷��
	int nWorkLine = -1;
	int nIndex = 0;
	int nPLSQLBlockNum = 0;
	for (size_t i=0; i<funcInfoArr.GetNum(); ++i) {
		FuncInfo* pFI = funcInfoArr.GetAt(i);
		if (pFI->nInfo == 31 || pFI->nInfo == 41) {
		}
		if (pFI->nInfo == 31) {
			if (pShareData->bLineNumIsCRLF_ForJump) {	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
				auto_sprintf(szText, LSW(STR_DLGJUMP_PSLQL),
					pFI->nFuncLineCRLF,
					pFI->memFuncName.GetStringPtr()
				);
			}else {
				auto_sprintf(szText, LSW(STR_DLGJUMP_PSLQL),
					pFI->nFuncLineLAYOUT,
					pFI->memFuncName.GetStringPtr()
				);
			}
			nIndex = Combo_AddString(hwndCtrl, szText);
			if (pShareData->bLineNumIsCRLF_ForJump) {	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
				Combo_SetItemData(hwndCtrl, nIndex, pFI->nFuncLineCRLF);
			}else {
				Combo_SetItemData(hwndCtrl, nIndex, pFI->nFuncLineLAYOUT);
			}
			nPLSQLBlockNum++;
		}
		if (pFI->nInfo == 41) {
			if (pShareData->bLineNumIsCRLF_ForJump) {	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
				auto_sprintf(szText, LSW(STR_DLGJUMP_PSLQL),
					pFI->nFuncLineCRLF,
					pFI->memFuncName.GetStringPtr()
				);
			}else {
				auto_sprintf(szText, LSW(STR_DLGJUMP_PSLQL),
					pFI->nFuncLineLAYOUT,
					pFI->memFuncName.GetStringPtr()
				);
			}
			nIndexCurSel = nIndex = Combo_AddString(hwndCtrl, szText);
			if (pShareData->bLineNumIsCRLF_ForJump) {	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
				nWorkLine = pFI->nFuncLineCRLF;
				Combo_SetItemData(hwndCtrl, nIndex, pFI->nFuncLineCRLF);
			}else {
				nWorkLine = pFI->nFuncLineLAYOUT;
				Combo_SetItemData(hwndCtrl, nIndex, pFI->nFuncLineLAYOUT);
			}
			++nPLSQLBlockNum;
		}
	}
	Combo_SetCurSel(hwndCtrl, nIndexCurSel);

	// PL/SQL�̃p�b�P�[�W�{�̂����o���ꂽ�ꍇ
	if (nWorkLine != -1) {
		nPLSQL_E1 = nWorkLine;
		SetItemInt(IDC_EDIT_PLSQL_E1, nPLSQL_E1, FALSE);
	}
	// PL/SQL�̃p�b�P�[�W�u���b�N�����o���ꂽ�ꍇ
	if (0 < nPLSQLBlockNum) {
		bPLSQL = TRUE;
	}
	CheckButton(IDC_CHECK_PLSQL, bPLSQL);	// PL/SQL�\�[�X�̗L���s��
	if (IsButtonChecked(IDC_CHECK_PLSQL)) {
		EnableItem(IDC_LABEL_PLSQL1, true);	// Sept. 12, 2000 JEPRO
		EnableItem(IDC_LABEL_PLSQL2, true);	// Sept. 12, 2000 JEPRO
		EnableItem(IDC_LABEL_PLSQL3, true);	// Sept. 12, 2000 JEPRO
		EnableItem(IDC_EDIT_PLSQL_E1, true);
		EnableItem(IDC_SPIN_PLSQL_E1, true);	// Oct. 6, 2000 JEPRO
		EnableItem(IDC_COMBO_PLSQLBLOCKS, true);
		pShareData->bLineNumIsCRLF_ForJump = true;
		EnableItem(IDC_RADIO_LINENUM_LAYOUT, false);
		EnableItem(IDC_RADIO_LINENUM_CRLF, false);
	}else {
		EnableItem(IDC_LABEL_PLSQL1, false);	// Sept. 12, 2000 JEPRO
		EnableItem(IDC_LABEL_PLSQL2, false);	// Sept. 12, 2000 JEPRO
		EnableItem(IDC_LABEL_PLSQL3, false);	// Sept. 12, 2000 JEPRO
		EnableItem(IDC_EDIT_PLSQL_E1, false);
		EnableItem(IDC_SPIN_PLSQL_E1, false);	// Oct. 6, 2000 JEPRO
		EnableItem(IDC_COMBO_PLSQLBLOCKS, false);
		EnableItem(IDC_RADIO_LINENUM_LAYOUT, true);
		EnableItem(IDC_RADIO_LINENUM_CRLF, true);
	}
	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	if (pShareData->bLineNumIsCRLF_ForJump) {
		CheckButton(IDC_RADIO_LINENUM_LAYOUT, false);
		CheckButton(IDC_RADIO_LINENUM_CRLF, true);
	}else {
		CheckButton(IDC_RADIO_LINENUM_LAYOUT, true);
		CheckButton(IDC_RADIO_LINENUM_CRLF, false);
	}
	return;
}


// �_�C�A���O�f�[�^�̎擾
// TRUE==����   FALSE==���̓G���[
int DlgJump::GetData(void)
{
	BOOL pTranslated;

	// �s�ԍ��̕\�� false=�܂�Ԃ��P�ʁ^true=���s�P��
	pShareData->bLineNumIsCRLF_ForJump = !IsButtonChecked(IDC_RADIO_LINENUM_LAYOUT);

	// PL/SQL�\�[�X�̗L���s��
	bPLSQL = IsButtonChecked(IDC_CHECK_PLSQL);
	nPLSQL_E1 = GetItemInt(IDC_EDIT_PLSQL_E1, &pTranslated, FALSE);
	if (nPLSQL_E1 == 0 && !pTranslated) {
		nPLSQL_E1 = 1;
	}

//	nPLSQL_E2 = GetItemInt(IDC_EDIT_PLSQL_E2, &pTranslated, FALSE);
//	if (nPLSQL_E2 == 0 && !pTranslated) {
//		nPLSQL_E2 = 1;
//	}

	// �s�ԍ�
	nLineNum = GetItemInt(IDC_EDIT_LINENUM, &pTranslated, FALSE);
	if (nLineNum == 0 && !pTranslated) {
		return FALSE;
	}
	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID DlgJump::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

