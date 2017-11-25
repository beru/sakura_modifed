/*!	@file
	@brief �u���_�C�A���O
*/
#include "StdAfx.h"
#include "dlg/DlgReplace.h"
#include "view/EditView.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {	//11900
	IDC_BUTTON_SEARCHNEXT,			HIDC_REP_BUTTON_SEARCHNEXT,			// ������
	IDC_BUTTON_SEARCHPREV,			HIDC_REP_BUTTON_SEARCHPREV,			// �㌟��
	IDC_BUTTON_REPALCE,				HIDC_REP_BUTTON_REPALCE,			// �u��
	IDC_BUTTON_REPALCEALL,			HIDC_REP_BUTTON_REPALCEALL,			// �S�u��
	IDCANCEL,						HIDCANCEL_REP,						// �L�����Z��
	IDC_BUTTON_HELP,				HIDC_REP_BUTTON_HELP,				// �w���v
	IDC_CHK_PASTE,					HIDC_REP_CHK_PASTE,					// �N���b�v�{�[�h����\��t��
	IDC_CHK_WORD,					HIDC_REP_CHK_WORD,					// �P��P��
	IDC_CHK_LOHICASE,				HIDC_REP_CHK_LOHICASE,				// �啶��������
	IDC_CHK_REGULAREXP,				HIDC_REP_CHK_REGULAREXP,			// ���K�\��
	IDC_CHECK_NOTIFYNOTFOUND,		HIDC_REP_CHECK_NOTIFYNOTFOUND,		// ������Ȃ��Ƃ��ɒʒm
	IDC_CHECK_bAutoCloseDlgReplace,	HIDC_REP_CHECK_bAutoCloseDlgReplace,// �����I�ɕ���
	IDC_COMBO_TEXT,					HIDC_REP_COMBO_TEXT,				// �u���O
	IDC_COMBO_TEXT2,				HIDC_REP_COMBO_TEXT2,				// �u����
	IDC_RADIO_REPLACE,				HIDC_REP_RADIO_REPLACE,				// �u���ΏہF�u��
	IDC_RADIO_INSERT,				HIDC_REP_RADIO_INSERT,				// �u���ΏہF�}��
	IDC_RADIO_ADD,					HIDC_REP_RADIO_ADD,					// �u���ΏہF�ǉ�
	IDC_RADIO_LINEDELETE,			HIDC_REP_RADIO_LINEDELETE,			// �u���ΏہF�s�폜
	IDC_RADIO_SELECTEDAREA,			HIDC_REP_RADIO_SELECTEDAREA,		// �͈́F�S��
	IDC_RADIO_ALLAREA,				HIDC_REP_RADIO_ALLAREA,				// �͈́F�I��͈�
	IDC_STATIC_JRE32VER,			HIDC_REP_STATIC_JRE32VER,			// ���K�\���o�[�W����
	IDC_BUTTON_SETMARK,				HIDC_REP_BUTTON_SETMARK,			// �����Y���s���}�[�N
	IDC_CHECK_SEARCHALL,			HIDC_REP_CHECK_SEARCHALL,			// �擪�i�����j����Č���
	IDC_CHECK_CONSECUTIVEALL,		HIDC_REP_CHECK_CONSECUTIVEALL,		//�u���ׂĒu���v�͒u���̌J�Ԃ�
//	IDC_STATIC,						-1,
	0, 0
};

DlgReplace::DlgReplace()
{
	searchOption.Reset();	// �����I�v�V����
	bConsecutiveAll = false;	//�u���ׂĒu���v�͒u���̌J�Ԃ�
	bSelectedArea = false;	// �I��͈͓��u��
	nReplaceTarget = 0;		// �u���Ώ�
	bPaste = false;			// �\��t����H
	nReplaceCnt = 0;			// ���ׂĒu���̎��s����
	bCanceled = false;		// ���ׂĒu���𒆒f������
}

/*!
	�R���{�{�b�N�X�̃h���b�v�_�E�����b�Z�[�W��ߑ�����
*/
BOOL DlgReplace::OnCbnDropDown(HWND hwndCtl, int wID)
{
	switch (wID) {
	case IDC_COMBO_TEXT:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			size_t nSize = pShareData->searchKeywords.searchKeys.size();
			for (size_t i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, pShareData->searchKeywords.searchKeys[i] );
			}
		}
		break;
	case IDC_COMBO_TEXT2:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			size_t nSize = pShareData->searchKeywords.replaceKeys.size();
			for (size_t i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, pShareData->searchKeywords.replaceKeys[i] );
			}
		}
		break;
	}
	return Dialog::OnCbnDropDown( hwndCtl, wID );
}

// ���[�h���X�_�C�A���O�̕\��
HWND DlgReplace::DoModeless(
	HINSTANCE hInstance,
	HWND hwndParent,
	LPARAM lParam,
	bool bSelected
	)
{
	auto& csSearch = pShareData->common.search;
	searchOption = csSearch.searchOption;		// �����I�v�V����
	bConsecutiveAll = csSearch.bConsecutiveAll;	//�u���ׂĒu���v�͒u���̌J�Ԃ�
	bSelectedArea = csSearch.bSelectedArea;		// �I��͈͓��u��
	bNotifyNotFound = csSearch.bNotifyNotFound;	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	bSelected = bSelected;
	ptEscCaretPos_PHY = ((EditView*)lParam)->GetCaret().GetCaretLogicPos();	// ����/�u���J�n���̃J�[�\���ʒu�ޔ�
	((EditView*)lParam)->bSearch = true;			// ����/�u���J�n�ʒu�̓o�^�L��
	return Dialog::DoModeless(hInstance, hwndParent, IDD_REPLACE, lParam, SW_SHOW);
}

// ���[�h���X���F�u���E�����ΏۂƂȂ�r���[�̕ύX
void DlgReplace::ChangeView(LPARAM pcEditView)
{
	lParam = pcEditView;
	return;
}


// �_�C�A���O�f�[�^�̐ݒ�
void DlgReplace::SetData(void)
{
	auto& csSearch = pShareData->common.search;

	// ����������/�u���㕶���񃊃X�g�̐ݒ�(�֐���)
	SetCombosList();

	// �p�啶���Ɖp����������ʂ���
	CheckButton(IDC_CHK_LOHICASE, searchOption.bLoHiCase);

	// �P��P�ʂŒT��
	CheckButton(IDC_CHK_WORD, searchOption.bWordOnly);

	//�u���ׂĒu���v�͒u���̌J�Ԃ�
	CheckButton(IDC_CHECK_CONSECUTIVEALL, bConsecutiveAll);

	// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
	// �����t���[�y�є�������̌������B�K�����K�\���̃`�F�b�N��
	// ���֌W��CheckRegexpVersion��ʉ߂���悤�ɂ����B
	if (CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, false)
		&& searchOption.bRegularExp
	) {
		// �p�啶���Ɖp����������ʂ���
		CheckButton(IDC_CHK_REGULAREXP, true);

		// �P��P�ʂŒT��
		EnableItem(IDC_CHK_WORD, false);
	}else {
		CheckButton(IDC_CHK_REGULAREXP, false);

		//�u���ׂĒu���v�͒u���̌J�Ԃ�
		EnableItem(IDC_CHECK_CONSECUTIVEALL, false);
	}

	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	CheckButton(IDC_CHECK_NOTIFYNOTFOUND, bNotifyNotFound);

	// �u�� �_�C�A���O�������I�ɕ���
	CheckButton(IDC_CHECK_bAutoCloseDlgReplace, csSearch.bAutoCloseDlgReplace);

	// �擪�i�����j����Č��� 2002.01.26 hor
	CheckButton(IDC_CHECK_SEARCHALL, csSearch.bSearchAll);

	// �N���b�v�{�[�h����\��t����H
	CheckButton(IDC_CHK_PASTE, bPaste);
	// �u���Ώ�
	if (nReplaceTarget == 0) {
		CheckButton(IDC_RADIO_REPLACE, true);
	}else if (nReplaceTarget == 1) {
		CheckButton(IDC_RADIO_INSERT, true);
	}else if (nReplaceTarget == 2) {
		CheckButton(IDC_RADIO_ADD, true);
	}else if (nReplaceTarget == 3) {
		CheckButton(IDC_RADIO_LINEDELETE, true);
		EnableItem(IDC_COMBO_TEXT2, false);
		EnableItem(IDC_CHK_PASTE, false);
	}

	return;
}


// ����������/�u���㕶���񃊃X�g�̐ݒ�
void DlgReplace::SetCombosList(void)
{
	// ����������
	HWND hwndCombo = GetItemHwnd(IDC_COMBO_TEXT);
	while (Combo_GetCount(hwndCombo) > 0) {
		Combo_DeleteString(hwndCombo, 0);
	}
	int nBufferSize = ::GetWindowTextLength(hwndCombo) + 1;
	std::vector<TCHAR> vText;
	vText.resize(nBufferSize);
	Combo_GetText(hwndCombo, &vText[0], nBufferSize);
	if (auto_strcmp(to_wchar(&vText[0]), strText.c_str()) != 0) {
		SetItemText(IDC_COMBO_TEXT, strText.c_str());
	}

	// �u���㕶����
	hwndCombo = GetItemHwnd(IDC_COMBO_TEXT2);
	while (Combo_GetCount(hwndCombo) > 0) {
		Combo_DeleteString(hwndCombo, 0);
	}
	nBufferSize = ::GetWindowTextLength(hwndCombo) + 1;
	vText.resize(nBufferSize);
	Combo_GetText(hwndCombo, &vText[0], nBufferSize);
	if (auto_strcmp(to_wchar(&vText[0]), strText2.c_str()) != 0) {
		SetItemText(IDC_COMBO_TEXT2, strText2.c_str());
	}
}


// �_�C�A���O�f�[�^�̎擾
// 0==����������  0���傫��==����   0��菬����==���̓G���[
int DlgReplace::GetData(void)
{
	auto& csSearch = pShareData->common.search;

	// �p�啶���Ɖp����������ʂ���
	searchOption.bLoHiCase = IsButtonChecked(IDC_CHK_LOHICASE);

	// �P��P�ʂŒT��
	searchOption.bWordOnly = IsButtonChecked(IDC_CHK_WORD);

	//�u���ׂĒu���v�͒u���̌J�Ԃ�
	bConsecutiveAll = IsButtonChecked(IDC_CHECK_CONSECUTIVEALL);

	// ���K�\��
	searchOption.bRegularExp = IsButtonChecked(IDC_CHK_REGULAREXP);
	// �I��͈͓��u��
	bSelectedArea = IsButtonChecked(IDC_RADIO_SELECTEDAREA);
	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	bNotifyNotFound = IsButtonChecked(IDC_CHECK_NOTIFYNOTFOUND);

	csSearch.bConsecutiveAll = bConsecutiveAll;	// 1==�u���ׂĒu���v�͒u���̌J�Ԃ�
	csSearch.bSelectedArea = bSelectedArea;		// �I��͈͓��u��
	csSearch.bNotifyNotFound = bNotifyNotFound;	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��

	// ����������
	int nBufferSize = ::GetWindowTextLength(GetItemHwnd(IDC_COMBO_TEXT)) + 1;
	std::vector<TCHAR> vText(nBufferSize);
	GetItemText(IDC_COMBO_TEXT, &vText[0], nBufferSize);
	strText = to_wchar(&vText[0]);
	// �u���㕶����
	if (IsButtonChecked(IDC_RADIO_LINEDELETE )) {
		strText2 = L"";
	}else {
		nBufferSize = ::GetWindowTextLength(GetItemHwnd(IDC_COMBO_TEXT2)) + 1;
		vText.resize(nBufferSize);
		GetItemText(IDC_COMBO_TEXT2, &vText[0], nBufferSize);
		strText2 = to_wchar(&vText[0]);
	}

	// �u�� �_�C�A���O�������I�ɕ���
	csSearch.bAutoCloseDlgReplace = IsButtonChecked(IDC_CHECK_bAutoCloseDlgReplace);

	// �擪�i�����j����Č���
	csSearch.bSearchAll = IsButtonChecked(IDC_CHECK_SEARCHALL);

	if (0 < strText.size()) {
		// ���K�\���H
		int nFlag = 0x00;
		nFlag |= searchOption.bLoHiCase ? 0x01 : 0x00;
		if (searchOption.bRegularExp
			&& !CheckRegexpSyntax(strText.c_str(), GetHwnd(), true, nFlag)
		) {
			return -1;
		}

		// ����������
		if (strText.size() < _MAX_PATH) {
			SearchKeywordManager().AddToSearchKeys(strText.c_str());
			csSearch.searchOption = searchOption;		// �����I�v�V����
		}
		// view�ɒ��ڐݒ�
		EditView* pEditView = (EditView*)lParam;
		if (pEditView->strCurSearchKey == strText && pEditView->curSearchOption == searchOption) {
		}else {
			pEditView->strCurSearchKey = strText;
			pEditView->curSearchOption = searchOption;
			pEditView->bCurSearchUpdate = true;
		}
		pEditView->nCurSearchKeySequence = GetDllShareData().common.search.nSearchKeySequence;

		// �u���㕶����
		if (strText2.size() < _MAX_PATH) {
			SearchKeywordManager().AddToReplaceKeys(strText2.c_str());
		}
		nReplaceKeySequence = GetDllShareData().common.search.nReplaceKeySequence;

		// �N���b�v�{�[�h����\��t����H
		bPaste = IsButtonChecked(IDC_CHK_PASTE);
		EnableItem(IDC_COMBO_TEXT2, !bPaste);
		// �u���Ώ�
		nReplaceTarget = 0;
		if (IsButtonChecked(IDC_RADIO_INSERT)) {
			nReplaceTarget = 1;
		}else if (IsButtonChecked(IDC_RADIO_ADD)) {
			nReplaceTarget = 2;
		}else if (IsButtonChecked(IDC_RADIO_LINEDELETE )) {
			nReplaceTarget = 3;
			bPaste = false;
			EnableItem(IDC_COMBO_TEXT2, false);
		}
		// ����������/�u���㕶���񃊃X�g�̐ݒ�
		if (!bModal) {
			SetCombosList();
		}
		return 1;
	}else {
		return 0;
	}
}


BOOL DlgReplace::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwndDlg);
	// �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ���
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_TEXT), TRUE);
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_TEXT2), TRUE);

	// �e�L�X�g�I�𒆂�
	if (bSelected) {
		EnableItem(IDC_BUTTON_SEARCHPREV, false);
		EnableItem(IDC_BUTTON_SEARCHNEXT, false);
		EnableItem(IDC_BUTTON_REPALCE, false);
		CheckButton(IDC_RADIO_SELECTEDAREA, true);
	}else {
		CheckButton(IDC_RADIO_ALLAREA, true);
	}

	comboDelText = ComboBoxItemDeleter();
	comboDelText.pRecent = &recentSearch;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_TEXT), &comboDelText);
	comboDelText2 = ComboBoxItemDeleter();
	comboDelText2.pRecent = &recentReplace;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_TEXT2), &comboDelText2);

	// �t�H���g�ݒ�
	HFONT hFontOld = (HFONT)::SendMessage(GetItemHwnd(IDC_COMBO_TEXT), WM_GETFONT, 0, 0);
	HFONT hFont = SetMainFont(GetItemHwnd(IDC_COMBO_TEXT));
	fontText.SetFont(hFontOld, hFont, GetItemHwnd(IDC_COMBO_TEXT));

	hFontOld = (HFONT)::SendMessage(GetItemHwnd(IDC_COMBO_TEXT2), WM_GETFONT, 0, 0);
	hFont = SetMainFont(GetItemHwnd(IDC_COMBO_TEXT2));
	fontText2.SetFont(hFontOld, hFont, GetItemHwnd(IDC_COMBO_TEXT2));

	// ���N���X�����o
	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}


BOOL DlgReplace::OnDestroy()
{
	fontText.ReleaseOnDestroy();
	fontText2.ReleaseOnDestroy();
	return Dialog::OnDestroy();
}


BOOL DlgReplace::OnBnClicked(int wID)
{
	int nRet;
	EditView* pEditView = (EditView*)lParam;

	switch (wID) {
	case IDC_CHK_PASTE:
		// �e�L�X�g�̓\��t��
		if (1
			&& IsButtonChecked(IDC_CHK_PASTE)
			&& !pEditView->pEditDoc->docEditor.IsEnablePaste()
		) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_CLIPBOARD));
			CheckButton(IDC_CHK_PASTE, false);
		}
		EnableItem(IDC_COMBO_TEXT2, !IsButtonChecked(IDC_CHK_PASTE));
		return TRUE;
		// �u���Ώ�
	case IDC_RADIO_REPLACE:
	case IDC_RADIO_INSERT:
	case IDC_RADIO_ADD:
	case IDC_RADIO_LINEDELETE:
		if (IsButtonChecked(IDC_RADIO_LINEDELETE )) {
			EnableItem(IDC_COMBO_TEXT2, false);
			EnableItem(IDC_CHK_PASTE, false);
		}else {
			EnableItem(IDC_COMBO_TEXT2, true);
			EnableItem(IDC_CHK_PASTE, true);
		}
		return TRUE;
	case IDC_RADIO_SELECTEDAREA:
		// �͈͔͈�
		if (IsButtonChecked(IDC_RADIO_ALLAREA)) {
			EnableItem(IDC_BUTTON_SEARCHPREV, true);
			EnableItem(IDC_BUTTON_SEARCHNEXT, true);
			EnableItem(IDC_BUTTON_REPALCE, true);
		}else {
			EnableItem(IDC_BUTTON_SEARCHPREV, false);
			EnableItem(IDC_BUTTON_SEARCHNEXT, false);
			EnableItem(IDC_BUTTON_REPALCE, false);
		}
		return TRUE;
	case IDC_RADIO_ALLAREA:
		// �t�@�C���S��
		if (IsButtonChecked(IDC_RADIO_ALLAREA)) {
			EnableItem(IDC_BUTTON_SEARCHPREV, true);
			EnableItem(IDC_BUTTON_SEARCHNEXT, true);
			EnableItem(IDC_BUTTON_REPALCE, true);
		}else {
			EnableItem(IDC_BUTTON_SEARCHPREV, false);
			EnableItem(IDC_BUTTON_SEARCHNEXT, false);
			EnableItem(IDC_BUTTON_REPALCE, false);
		}
		return TRUE;
	case IDC_BUTTON_HELP:
		//�u�u���v�̃w���v
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_REPLACE_DIALOG));
		return TRUE;
//	case IDC_CHK_LOHICASE:	// �啶���Ə���������ʂ���
//		MYTRACE(_T("IDC_CHK_LOHICASE\n"));
//		return TRUE;
//	case IDC_CHK_WORDONLY:	// ��v����P��̂݌���
//		MYTRACE(_T("IDC_CHK_WORDONLY\n"));
//		break;
	case IDC_CHK_REGULAREXP:	// ���K�\��
//		MYTRACE(_T("IDC_CHK_REGULAREXP ::IsDlgButtonChecked(GetHwnd(), IDC_CHK_REGULAREXP) = %d\n"), ::IsDlgButtonChecked(GetHwnd(), IDC_CHK_REGULAREXP));
		if (IsButtonChecked(IDC_CHK_REGULAREXP)) {
			// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
			if (!CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, true)) {
				CheckButton(IDC_CHK_REGULAREXP, false);
			}else {
				// �P��P�ʂŒT��
				EnableItem(IDC_CHK_WORD, false);

				//�u���ׂĒu���v�͒u���̌J�Ԃ�
				EnableItem(IDC_CHECK_CONSECUTIVEALL, true);
			}
		}else {
			// �P��P�ʂŒT��
			EnableItem(IDC_CHK_WORD, true);

			//�u���ׂĒu���v�͒u���̌J�Ԃ�
			EnableItem(IDC_CHECK_CONSECUTIVEALL, false);
		}
		return TRUE;
//	case IDOK:			// ������
//		// �_�C�A���O�f�[�^�̎擾
//		nRet = GetData();
//		if (0 < nRet) {
//			::EndDialog(hwndDlg, 2);
//		}else
//		if (nRet == 0) {
//			::EndDialog(hwndDlg, 0);
//		}
//		return TRUE;


	case IDC_BUTTON_SEARCHPREV:	// �㌟��
		nRet = GetData();
		if (0 < nRet) {

			// �����J�n�ʒu��o�^
			if (pEditView->bSearch != FALSE) {
				pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
				pEditView->bSearch = FALSE;
			}

			// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
			// �O������
			pEditView->GetCommander().HandleCommand(F_SEARCH_PREV, true, (LPARAM)GetHwnd(), 0, 0, 0);
			// �ĕ`��i0�������}�b�`�ŃL�����b�g��\�����邽�߁j
			pEditView->Redraw();	// �O��0�������}�b�`�̏����ɂ��K�v
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_STR));
		}
		return TRUE;
	case IDC_BUTTON_SEARCHNEXT:	// ������
		nRet = GetData();
		if (0 < nRet) {

			// �����J�n�ʒu��o�^
			if (pEditView->bSearch) {
				pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
				pEditView->bSearch = false;
			}

			// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
			// ��������
			pEditView->GetCommander().HandleCommand(F_SEARCH_NEXT, true, (LPARAM)GetHwnd(), 0, 0, 0);
			// �ĕ`��i0�������}�b�`�ŃL�����b�g��\�����邽�߁j
			pEditView->Redraw();	// �O��0�������}�b�`�̏����ɂ��K�v
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_STR));
		}
		return TRUE;

	case IDC_BUTTON_SETMARK:	// �Y���s�}�[�N
		nRet = GetData();
		if (0 < nRet) {
			pEditView->GetCommander().HandleCommand(F_BOOKMARK_PATTERN, false, 0, 0, 0, 0);
			::SendMessage(GetHwnd(), WM_NEXTDLGCTL, (WPARAM)GetItemHwnd(IDC_COMBO_TEXT), TRUE);
		}
		return TRUE;

	case IDC_BUTTON_REPALCE:	// �u��
		nRet = GetData();
		if (0 < nRet) {

			// �u���J�n�ʒu��o�^
			if (pEditView->bSearch) {
				pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
				pEditView->bSearch = false;
			}

			// �u��
			//@@@ 2002.2.2 YAZAKI �u���R�}���h��EditView�ɐV��
			//@@@ 2002/04/08 YAZAKI �e�E�B���h�E�̃n���h����n���悤�ɕύX�B
			pEditView->GetCommander().HandleCommand(F_REPLACE, true, (LPARAM)GetHwnd(), 0, 0, 0);
			// �ĕ`��
			pEditView->GetCommander().HandleCommand(F_REDRAW, true, 0, 0, 0, 0);
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_STR));
		}
		return TRUE;
	case IDC_BUTTON_REPALCEALL:	// ���ׂĒu��
		nRet = GetData();
		if (0 < nRet) {
			// �u���J�n�ʒu��o�^
			if (pEditView->bSearch) {
				pEditView->ptSrchStartPos_PHY = ptEscCaretPos_PHY;
				pEditView->bSearch = false;
			}

			pEditView->GetCommander().HandleCommand(F_REPLACE_ALL, true, 0, 0, 0, 0);
			pEditView->GetCommander().HandleCommand(F_REDRAW, true, 0, 0, 0, 0);

			// �A�N�e�B�u�ɂ���
			ActivateFrameWindow(GetHwnd());

			TopOkMessage(GetHwnd(), LS(STR_DLGREPLC_REPLACE), nReplaceCnt);

			if (!bCanceled) {
				if (bModal) {		// ���[�_���_�C�A���O��
					// �u���_�C�A���O�����
					::EndDialog(GetHwnd(), 0);
				}else {
					// �u�� �_�C�A���O�������I�ɕ���
					if (pShareData->common.search.bAutoCloseDlgReplace) {
						::DestroyWindow(GetHwnd());
					}
				}
			}
			return TRUE;
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_REPSTR));
		}
		return TRUE;
//	case IDCANCEL:
//		::EndDialog(hwndDlg, 0);
//		return TRUE;
	}

	// ���N���X�����o
	return Dialog::OnBnClicked(wID);
}

BOOL DlgReplace::OnActivate(WPARAM wParam, LPARAM lParam)
{
	// 0�������}�b�`�`���ON/OFF
	EditView*	pEditView = (EditView*)(this->lParam);
	Range rangeSel = pEditView->GetSelectionInfo().select;
	if (rangeSel.IsValid() && rangeSel.IsLineOne() && rangeSel.IsOne())
		pEditView->InvalidateRect(NULL);	// �A�N�e�B�u���^��A�N�e�B�u�����������Ă���ĕ`��

	return Dialog::OnActivate(wParam, lParam);
}

LPVOID DlgReplace::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


