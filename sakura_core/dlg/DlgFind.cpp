/*!	@file
	@brief �����_�C�A���O�{�b�N�X
*/

#include "StdAfx.h"
#include "dlg/DlgFind.h"
#include "view/EditView.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

// ���� CDlgFind.cpp
const DWORD p_helpids[] = {	//11800
	IDC_BUTTON_SEARCHNEXT,			HIDC_FIND_BUTTON_SEARCHNEXT,		// ��������
	IDC_BUTTON_SEARCHPREV,			HIDC_FIND_BUTTON_SEARCHPREV,		// �O������
	IDCANCEL,						HIDCANCEL_FIND,						// �L�����Z��
	IDC_BUTTON_HELP,				HIDC_FIND_BUTTON_HELP,				// �w���v
	IDC_CHK_WORD,					HIDC_FIND_CHK_WORD,					// �P��P��
	IDC_CHK_LOHICASE,				HIDC_FIND_CHK_LOHICASE,				// �啶��������
	IDC_CHK_REGULAREXP,				HIDC_FIND_CHK_REGULAREXP,			// ���K�\��
	IDC_CHECK_NOTIFYNOTFOUND,		HIDC_FIND_CHECK_NOTIFYNOTFOUND,		// ������Ȃ��Ƃ��ɒʒm
	IDC_CHECK_bAutoCloseDlgFind,	HIDC_FIND_CHECK_bAutoCloseDlgFind,	// �����I�ɕ���
	IDC_COMBO_TEXT,					HIDC_FIND_COMBO_TEXT,				// ����������
	IDC_STATIC_JRE32VER,			HIDC_FIND_STATIC_JRE32VER,			// ���K�\���o�[�W����
	IDC_BUTTON_SETMARK,				HIDC_FIND_BUTTON_SETMARK,			// �����Y���s���}�[�N
	IDC_CHECK_SEARCHALL,			HIDC_FIND_CHECK_SEARCHALL,			// �擪�i�����j����Č���
//	IDC_STATIC,						-1,
	0, 0
};

DlgFind::DlgFind()
{
	searchOption.Reset();
	return;
}


/*!
	�R���{�{�b�N�X�̃h���b�v�_�E�����b�Z�[�W��ߑ�����
*/
BOOL DlgFind::OnCbnDropDown(HWND hwndCtl, int wID)
{
	switch (wID) {
	case IDC_COMBO_TEXT:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			auto& keywords = pShareData->searchKeywords.searchKeys;
			size_t nSize = keywords.size();
			for (size_t i=0; i<nSize; ++i) {
				Combo_AddString(hwndCtl, keywords[i]);
			}
		}
		break;
	}
	return Dialog::OnCbnDropDown( hwndCtl, wID );
}


// ���[�h���X�_�C�A���O�̕\��
HWND DlgFind::DoModeless(
	HINSTANCE hInstance,
	HWND hwndParent,
	LPARAM lParam
	)
{
	auto& csSearch = pShareData->common.search;
	searchOption = csSearch.searchOption;		// �����I�v�V����
	bNotifyNotFound = csSearch.bNotifyNotFound;	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	ptEscCaretPos_PHY = ((EditView*)lParam)->GetCaret().GetCaretLogicPos();	// �����J�n���̃J�[�\���ʒu�ޔ�
	((EditView*)lParam)->bSearch = TRUE;							// �����J�n�ʒu�̓o�^�L��
	return Dialog::DoModeless(hInstance, hwndParent, IDD_FIND, lParam, SW_SHOW);
}

// ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX
void DlgFind::ChangeView(LPARAM pcEditView)
{
	lParam = pcEditView;
	return;
}


BOOL DlgFind::OnInitDialog(
	HWND hwnd,
	WPARAM wParam,
	LPARAM lParam
	)
{
	BOOL bRet = Dialog::OnInitDialog(hwnd, wParam, lParam);
	comboDel = ComboBoxItemDeleter();
	comboDel.pRecent = &recentSearch;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_TEXT), &comboDel);

	// �t�H���g�ݒ�
	HFONT hFontOld = (HFONT)::SendMessage(GetItemHwnd(IDC_COMBO_TEXT), WM_GETFONT, 0, 0);
	HFONT hFont = SetMainFont(GetItemHwnd(IDC_COMBO_TEXT));
	fontText.SetFont(hFontOld, hFont, GetItemHwnd(IDC_COMBO_TEXT));
	return bRet;
}


BOOL DlgFind::OnDestroy()
{
	fontText.ReleaseOnDestroy();
	return Dialog::OnDestroy();
}


// �_�C�A���O�f�[�^�̐ݒ�
void DlgFind::SetData(void)
{
//	MYTRACE(_T("DlgFind::SetData()"));

	/*****************************
	*           ������           *
	*****************************/
	// �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ���
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_TEXT), TRUE);


	/*****************************
	*         �f�[�^�ݒ�         *
	*****************************/
	// ����������
	SetCombosList();

	// �p�啶���Ɖp����������ʂ���
	CheckButton(IDC_CHK_LOHICASE, searchOption.bLoHiCase);

	// �P��P�ʂŌ���
	CheckButton(IDC_CHK_WORD, searchOption.bWordOnly);

	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	CheckButton(IDC_CHECK_NOTIFYNOTFOUND, bNotifyNotFound);

	// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
	// �����t���[�y�є�������̌������B�K�����K�\���̃`�F�b�N��
	// ���֌W��CheckRegexpVersion��ʉ߂���悤�ɂ����B
	if (1
		&& CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, false)
		&& searchOption.bRegularExp
	) {
		// �p�啶���Ɖp����������ʂ���
		CheckButton(IDC_CHK_REGULAREXP, true);
// ���K�\����ON�ł��A�啶������������ʂ���^���Ȃ���I���ł���悤�ɁB
//		CheckButton(IDC_CHK_LOHICASE, true);
//		EnableItem(IDC_CHK_LOHICASE, FALSE);

		// �P��P�ʂŒT��
		EnableItem(IDC_CHK_WORD, false);
	}else {
		CheckButton(IDC_CHK_REGULAREXP, false);
	}

	// �����_�C�A���O�������I�ɕ���
	CheckButton(IDC_CHECK_bAutoCloseDlgFind, pShareData->common.search.bAutoCloseDlgFind);

	// �擪�i�����j����Č���
	CheckButton(IDC_CHECK_SEARCHALL, pShareData->common.search.bSearchAll);

	return;
}


// ���������񃊃X�g�̐ݒ�
void DlgFind::SetCombosList(void)
{
	// ����������
	HWND hwndCombo = GetItemHwnd(IDC_COMBO_TEXT);
	while (Combo_GetCount(hwndCombo) > 0) {
		Combo_DeleteString(hwndCombo, 0);
	}
	int nBufferSize = ::GetWindowTextLength(GetItemHwnd(IDC_COMBO_TEXT)) + 1;
	std::vector<TCHAR> vText(nBufferSize);
	Combo_GetText(hwndCombo, &vText[0], nBufferSize);
	if (auto_strcmp(to_wchar(&vText[0]), strText.c_str()) != 0) {
		SetItemText(IDC_COMBO_TEXT, strText.c_str());
	}
}


// �_�C�A���O�f�[�^�̎擾
int DlgFind::GetData(void)
{
//	MYTRACE(_T("DlgFind::GetData()"));

	// �p�啶���Ɖp����������ʂ���
	searchOption.bLoHiCase = IsButtonChecked(IDC_CHK_LOHICASE);

	// �P��P�ʂŌ���
	searchOption.bWordOnly = IsButtonChecked(IDC_CHK_WORD);

	// ��v����P��̂݌�������
	// ���K�\��
	searchOption.bRegularExp = IsButtonChecked(IDC_CHK_REGULAREXP);

	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	bNotifyNotFound = IsButtonChecked(IDC_CHECK_NOTIFYNOTFOUND);

	pShareData->common.search.bNotifyNotFound = bNotifyNotFound;	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��

	// ����������
	int nBufferSize = ::GetWindowTextLength(GetItemHwnd(IDC_COMBO_TEXT)) + 1;
	std::vector<TCHAR> vText(nBufferSize);
	GetItemText(IDC_COMBO_TEXT, &vText[0], nBufferSize);
	strText = to_wchar(&vText[0]);

	// �����_�C�A���O�������I�ɕ���
	pShareData->common.search.bAutoCloseDlgFind = IsButtonChecked(IDC_CHECK_bAutoCloseDlgFind);

	// �擪�i�����j����Č���
	pShareData->common.search.bSearchAll = IsButtonChecked(IDC_CHECK_SEARCHALL);

	if (0 < strText.length()) {
		// ���K�\���H
		// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
		int nFlag = 0x00;
		nFlag |= searchOption.bLoHiCase ? 0x01 : 0x00;
		if (searchOption.bRegularExp
			&& !CheckRegexpSyntax(strText.c_str(), GetHwnd(), true, nFlag)
		) {
			return -1;
		}
		// ���K�\�����C�u���������ւ�

		// ����������
		if (strText.size() < _MAX_PATH) {
			SearchKeywordManager().AddToSearchKeys(strText.c_str());
			pShareData->common.search.searchOption = searchOption;		// �����I�v�V����
		}
		EditView* pEditView = (EditView*)lParam;
		if (1
			&& pEditView->strCurSearchKey == strText
			&& pEditView->curSearchOption == searchOption
		) {
		}else {
			pEditView->strCurSearchKey = strText;
			pEditView->curSearchOption = searchOption;
			pEditView->bCurSearchUpdate = true;
		}
		pEditView->nCurSearchKeySequence = GetDllShareData().common.search.nSearchKeySequence;
		if (!bModal) {
			// �_�C�A���O�f�[�^�̐ݒ�
			//SetData();
			SetCombosList();		// �R���{�݂̂̏�����
		}
		return 1;
	}else {
		return 0;
	}
}


BOOL DlgFind::OnBnClicked(int wID)
{
	int nRet;
	EditView*	pEditView = (EditView*)lParam;
	switch (wID) {
	case IDC_BUTTON_HELP:
		//�u�����v�̃w���v
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_SEARCH_DIALOG));
		break;
	case IDC_CHK_REGULAREXP:	// ���K�\��
//		MYTRACE(_T("IDC_CHK_REGULAREXP ::IsDlgButtonChecked(GetHwnd(), IDC_CHK_REGULAREXP) = %d\n"), ::IsDlgButtonChecked(GetHwnd(), IDC_CHK_REGULAREXP));
		if (IsButtonChecked(IDC_CHK_REGULAREXP)) {
			if (!CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, true)) {
				CheckButton(IDC_CHK_REGULAREXP, false);
			}else {
				// �P��P�ʂŌ���
				EnableItem(IDC_CHK_WORD, false);
			}
		}else {
			// �P��P�ʂŌ���
			EnableItem(IDC_CHK_WORD, true);
		}
		break;
	case IDC_BUTTON_SEARCHPREV:	// �㌟��
		// �_�C�A���O�f�[�^�̎擾
		nRet = GetData();
		if (0 < nRet) {
			if (bModal) {		// ���[�_���_�C�A���O��
				CloseDialog(1);
			}else {
				// �O������
				pEditView->GetCommander().HandleCommand(F_SEARCH_PREV, true, (LPARAM)GetHwnd(), 0, 0, 0);

				// 0�������}�b�`�ŃL�����b�g��\�����邽��
				pEditView->Redraw();	// �O��0�������}�b�`�̏����ɂ��K�v

				// �����J�n�ʒu��o�^
				if (pEditView->bSearch) {
					// �����J�n���̃J�[�\���ʒu�o�^�����ύX
					pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
					pEditView->bSearch = false;
				}

				// �����_�C�A���O�������I�ɕ���
				if (pShareData->common.search.bAutoCloseDlgFind) {
					CloseDialog(0);
				}
			}
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGFIND1));	// �����������w�肵�Ă��������B
		}
		return TRUE;
	case IDC_BUTTON_SEARCHNEXT:		// ������
		// �_�C�A���O�f�[�^�̎擾
		nRet = GetData();
		if (0 < nRet) {
			if (bModal) {		// ���[�_���_�C�A���O��
				CloseDialog(2);
			}else {
				// ��������
				pEditView->GetCommander().HandleCommand(F_SEARCH_NEXT, true, (LPARAM)GetHwnd(), 0, 0, 0);

				// �ĕ`�� 0�������}�b�`�ŃL�����b�g��\�����邽��
				pEditView->Redraw();	// �O��0�������}�b�`�̏����ɂ��K�v

				// �����J�n�ʒu��o�^
				if (pEditView->bSearch) {
					// �����J�n���̃J�[�\���ʒu�o�^�����ύX
					pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
					pEditView->bSearch = false;
				}

				// �����_�C�A���O�������I�ɕ���
				if (pShareData->common.search.bAutoCloseDlgFind) {
					CloseDialog(0);
				}
			}
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGFIND1));	// �����������w�肵�Ă��������B
		}
		return TRUE;
	case IDC_BUTTON_SETMARK:
		if (0 < GetData()) {
			if (bModal) {		// ���[�_���_�C�A���O��
				CloseDialog(2);
			}else {
				pEditView->GetCommander().HandleCommand(F_BOOKMARK_PATTERN, false, 0, 0, 0, 0);
				// �����_�C�A���O�������I�ɕ���
				if (pShareData->common.search.bAutoCloseDlgFind) {
					CloseDialog(0);
				}else {
					::SendMessage(GetHwnd(), WM_NEXTDLGCTL, (WPARAM)GetItemHwnd(IDC_COMBO_TEXT), TRUE);
				}
			}
		}
		return TRUE;
	case IDCANCEL:
		CloseDialog(0);
		return TRUE;
	}
	return FALSE;
}

BOOL DlgFind::OnActivate(WPARAM wParam, LPARAM lParam)
{
	// 0�������}�b�`�`���ON/OFF
	EditView* pEditView = (EditView*)(this->lParam);
	Range rangeSel = pEditView->GetSelectionInfo().select;
	if (rangeSel.IsValid() && rangeSel.IsLineOne() && rangeSel.IsOne())
		pEditView->InvalidateRect(NULL);	// �A�N�e�B�u���^��A�N�e�B�u�����������Ă���ĕ`��

	return Dialog::OnActivate(wParam, lParam);
}

LPVOID DlgFind::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

