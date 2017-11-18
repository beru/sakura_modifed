// �����L�[���[�h�I���_�C�A���O

#include "StdAfx.h"
#include "DlgKeywordSelect.h"
#include "env/DllSharedData.h"
#include "KeywordSetMgr.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids[] = {	// 2006.10.10 ryoji
	IDOK,				HIDOK_KEYWORD_SELECT,
	IDCANCEL,			HIDCANCEL_KEYWORD_SELECT,
	IDC_COMBO1,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO2,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO3,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO4,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO5,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO6,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO7,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO8,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO9,			HIDC_COMBO_KEYWORD_SELECT,
	IDC_COMBO10,		HIDC_COMBO_KEYWORD_SELECT,
	0, 0
};

static const int keyword_select_target_combo[KEYWORD_SELECT_NUM] = {
	IDC_COMBO1,
	IDC_COMBO2,
	IDC_COMBO3,
	IDC_COMBO4,
	IDC_COMBO5,
	IDC_COMBO6,
	IDC_COMBO7,
	IDC_COMBO8,
	IDC_COMBO9,
	IDC_COMBO10
};


DlgKeywordSelect::DlgKeywordSelect()
{
	pKeywordSetMgr = &(pShareData->common.specialKeyword.keywordSetMgr);

	return;
}

DlgKeywordSelect::~DlgKeywordSelect()
{
	return;
}


// ���[�_���_�C�A���O�̕\��
INT_PTR DlgKeywordSelect::DoModal(HINSTANCE hInstance, HWND hwndParent, int* pnSet)
{
	for (int i=0; i<KEYWORD_SELECT_NUM; ++i) {
		nSet[i] = pnSet[i];
	}

	(void)Dialog::DoModal(hInstance, hwndParent, IDD_DIALOG_KEYWORD_SELECT, (LPARAM)NULL);

	for (int i=0; i<KEYWORD_SELECT_NUM; ++i) {
		pnSet[i] = nSet[i];
	}

	return TRUE;
}

// ����������
BOOL DlgKeywordSelect::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	_SetHwnd(hwndDlg);

	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}


BOOL DlgKeywordSelect::OnBnClicked(int wID)
{
	switch (wID) {
	case IDOK:
		GetData();
		break;
	case IDCANCEL:
		break;
	}
	return Dialog::OnBnClicked(wID);
}

// �_�C�A���O�f�[�^�̐ݒ�
void DlgKeywordSelect::SetData(void)
{
	for (int index=0; index<KEYWORD_SELECT_NUM; ++index) {
		HWND hwndCombo = GetItemHwnd(keyword_select_target_combo[index]);

		// �R���{�{�b�N�X����ɂ���
		Combo_ResetContent(hwndCombo);
		
		// ��s�ڂ͋�
		Combo_AddString(hwndCombo, L" ");

		if (pKeywordSetMgr->nKeywordSetNum > 0) {
			for (size_t i=0; i<pKeywordSetMgr->nKeywordSetNum; ++i) {
				Combo_AddString(hwndCombo, pKeywordSetMgr->GetTypeName(i));
			}

			if (nSet[index] == -1) {
				// �Z�b�g���R���{�{�b�N�X�̃f�t�H���g�I��
				Combo_SetCurSel(hwndCombo, 0);
			}else {
				// �Z�b�g���R���{�{�b�N�X�̃f�t�H���g�I��
				Combo_SetCurSel(hwndCombo, nSet[index] + 1);
			}
		}
	}
}


// �_�C�A���O�f�[�^�̐ݒ�
int DlgKeywordSelect::GetData(void)
{
	for (int index=0; index<KEYWORD_SELECT_NUM; ++index) {
		HWND hwndCombo = GetItemHwnd(keyword_select_target_combo[index]);

		int n = Combo_GetCurSel(hwndCombo);
		if (n == CB_ERR || n == 0) {
			nSet[index] = -1;
		}else {
			nSet[index] = n - 1;
		}
	}

	return TRUE;
}

LPVOID DlgKeywordSelect::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

