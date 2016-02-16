/*!	@file
	@brief �u���_�C�A���O

	@author Norio Nakatani
	@date 2001/06/23 N.Nakatani �P��P�ʂŌ�������@�\������
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, Stonee, hor, YAZAKI
	Copyright (C) 2002, MIK, hor, novice, genta, aroka, YAZAKI
	Copyright (C) 2006, �����, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2012, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/DlgReplace.h"
#include "view/EditView.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

// �u�� CDlgReplace.cpp	//@@@ 2002.01.07 add start MIK
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
	IDC_BUTTON_SETMARK,				HIDC_REP_BUTTON_SETMARK,			// 2002.01.16 hor �����Y���s���}�[�N
	IDC_CHECK_SEARCHALL,			HIDC_REP_CHECK_SEARCHALL,			// 2002.01.26 hor �擪�i�����j����Č���
	IDC_CHECK_CONSECUTIVEALL,		HIDC_REP_CHECK_CONSECUTIVEALL,		//�u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

DlgReplace::DlgReplace()
{
	m_searchOption.Reset();	// �����I�v�V����
	m_bConsecutiveAll = false;	//�u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
	m_bSelectedArea = false;	// �I��͈͓��u��
	m_nReplaceTarget = 0;		// �u���Ώ�		// 2001.12.03 hor
	m_bPaste = false;			// �\��t����H	// 2001.12.03 hor
	m_nReplaceCnt = 0;			// ���ׂĒu���̎��s����		// 2002.02.08 hor
	m_bCanceled = false;		// ���ׂĒu���𒆒f������	// 2002.02.08 hor
}

/*!
	�R���{�{�b�N�X�̃h���b�v�_�E�����b�Z�[�W��ߑ�����

	@date 2013.03.24 novice �V�K�쐬
*/
BOOL DlgReplace::OnCbnDropDown(HWND hwndCtl, int wID)
{
	switch (wID) {
	case IDC_COMBO_TEXT:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			int nSize = m_pShareData->m_searchKeywords.m_aSearchKeys.size();
			for (int i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, m_pShareData->m_searchKeywords.m_aSearchKeys[i] );
			}
		}
		break;
	case IDC_COMBO_TEXT2:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			int nSize = m_pShareData->m_searchKeywords.m_aReplaceKeys.size();
			for (int i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, m_pShareData->m_searchKeywords.m_aReplaceKeys[i] );
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
	auto& csSearch = m_pShareData->m_common.m_search;
	m_searchOption = csSearch.m_searchOption;		// �����I�v�V����
	m_bConsecutiveAll = csSearch.m_bConsecutiveAll;	//�u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
	m_bSelectedArea = csSearch.m_bSelectedArea;		// �I��͈͓��u��
	m_bNOTIFYNOTFOUND = csSearch.m_bNOTIFYNOTFOUND;	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	m_bSelected = bSelected;
	m_ptEscCaretPos_PHY = ((EditView*)lParam)->GetCaret().GetCaretLogicPos();	// ����/�u���J�n���̃J�[�\���ʒu�ޔ�
	((EditView*)lParam)->m_bSearch = true;			// ����/�u���J�n�ʒu�̓o�^�L��			02/07/28 ai
	return Dialog::DoModeless(hInstance, hwndParent, IDD_REPLACE, lParam, SW_SHOW);
}

// ���[�h���X���F�u���E�����ΏۂƂȂ�r���[�̕ύX
void DlgReplace::ChangeView(LPARAM pcEditView)
{
	m_lParam = pcEditView;
	return;
}


// �_�C�A���O�f�[�^�̐ݒ�
void DlgReplace::SetData(void)
{
	auto& csSearch = m_pShareData->m_common.m_search;

	// ����������/�u���㕶���񃊃X�g�̐ݒ�(�֐���)	2010/5/26 Uchi
	SetCombosList();

	// �p�啶���Ɖp����������ʂ���
	CheckButton(IDC_CHK_LOHICASE, m_searchOption.bLoHiCase);

	// 2001/06/23 N.Nakatani
	// �P��P�ʂŒT��
	CheckButton(IDC_CHK_WORD, m_searchOption.bWordOnly);

	//�u���ׂĒu���v�͒u���̌J�Ԃ�  2007.01.16 ryoji
	CheckButton(IDC_CHECK_CONSECUTIVEALL, m_bConsecutiveAll);

	// From Here Jun. 29, 2001 genta
	// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
	// �����t���[�y�є�������̌������B�K�����K�\���̃`�F�b�N��
	// ���֌W��CheckRegexpVersion��ʉ߂���悤�ɂ����B
	if (CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, false)
		&& m_searchOption.bRegularExp
	) {
		// �p�啶���Ɖp����������ʂ���
		CheckButton(IDC_CHK_REGULAREXP, true);

		// 2001/06/23 N.Nakatani
		// �P��P�ʂŒT��
		EnableItem(IDC_CHK_WORD, false);
	}else {
		CheckButton(IDC_CHK_REGULAREXP, false);

		//�u���ׂĒu���v�͒u���̌J�Ԃ�
		EnableItem(IDC_CHECK_CONSECUTIVEALL, false);	// 2007.01.16 ryoji
	}
	// To Here Jun. 29, 2001 genta

	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	CheckButton(IDC_CHECK_NOTIFYNOTFOUND, m_bNOTIFYNOTFOUND);

	// �u�� �_�C�A���O�������I�ɕ���
	CheckButton(IDC_CHECK_bAutoCloseDlgReplace, csSearch.m_bAutoCloseDlgReplace);

	// �擪�i�����j����Č��� 2002.01.26 hor
	CheckButton(IDC_CHECK_SEARCHALL, csSearch.m_bSearchAll);

	// From Here 2001.12.03 hor
	// �N���b�v�{�[�h����\��t����H
	CheckButton(IDC_CHK_PASTE, m_bPaste);
	// �u���Ώ�
	if (m_nReplaceTarget == 0) {
		CheckButton(IDC_RADIO_REPLACE, true);
	}else if (m_nReplaceTarget == 1) {
		CheckButton(IDC_RADIO_INSERT, true);
	}else if (m_nReplaceTarget == 2) {
		CheckButton(IDC_RADIO_ADD, true);
	}else if (m_nReplaceTarget == 3) {
		CheckButton(IDC_RADIO_LINEDELETE, true);
		EnableItem(IDC_COMBO_TEXT2, false);
		EnableItem(IDC_CHK_PASTE, false);
	}
	// To Here 2001.12.03 hor

	return;
}


// ����������/�u���㕶���񃊃X�g�̐ݒ�
// 2010/5/26 Uchi
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
	if (auto_strcmp(to_wchar(&vText[0]), m_strText.c_str()) != 0) {
		SetItemText(IDC_COMBO_TEXT, m_strText.c_str());
	}

	// �u���㕶����
	hwndCombo = GetItemHwnd(IDC_COMBO_TEXT2);
	while (Combo_GetCount(hwndCombo) > 0) {
		Combo_DeleteString(hwndCombo, 0);
	}
	nBufferSize = ::GetWindowTextLength(hwndCombo) + 1;
	vText.resize(nBufferSize);
	Combo_GetText(hwndCombo, &vText[0], nBufferSize);
	if (auto_strcmp(to_wchar(&vText[0]), m_strText2.c_str()) != 0) {
		SetItemText(IDC_COMBO_TEXT2, m_strText2.c_str());
	}
}


// �_�C�A���O�f�[�^�̎擾
// 0==����������  0���傫��==����   0��菬����==���̓G���[
int DlgReplace::GetData(void)
{
	auto& csSearch = m_pShareData->m_common.m_search;

	// �p�啶���Ɖp����������ʂ���
	m_searchOption.bLoHiCase = IsButtonChecked(IDC_CHK_LOHICASE);

	// 2001/06/23 N.Nakatani
	// �P��P�ʂŒT��
	m_searchOption.bWordOnly = IsButtonChecked(IDC_CHK_WORD);

	//�u���ׂĒu���v�͒u���̌J�Ԃ�  2007.01.16 ryoji
	m_bConsecutiveAll = IsButtonChecked(IDC_CHECK_CONSECUTIVEALL);

	// ���K�\��
	m_searchOption.bRegularExp = IsButtonChecked(IDC_CHK_REGULAREXP);
	// �I��͈͓��u��
	m_bSelectedArea = IsButtonChecked(IDC_RADIO_SELECTEDAREA);
	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	m_bNOTIFYNOTFOUND = IsButtonChecked(IDC_CHECK_NOTIFYNOTFOUND);

	csSearch.m_bConsecutiveAll = m_bConsecutiveAll;	// 1==�u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
	csSearch.m_bSelectedArea = m_bSelectedArea;		// �I��͈͓��u��
	csSearch.m_bNOTIFYNOTFOUND = m_bNOTIFYNOTFOUND;	// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��

	// ����������
	int nBufferSize = ::GetWindowTextLength(GetItemHwnd(IDC_COMBO_TEXT)) + 1;
	std::vector<TCHAR> vText(nBufferSize);
	GetItemText(IDC_COMBO_TEXT, &vText[0], nBufferSize);
	m_strText = to_wchar(&vText[0]);
	// �u���㕶����
	if (IsButtonChecked(IDC_RADIO_LINEDELETE )) {
		m_strText2 = L"";
	}else {
		nBufferSize = ::GetWindowTextLength(GetItemHwnd(IDC_COMBO_TEXT2)) + 1;
		vText.resize(nBufferSize);
		GetItemText(IDC_COMBO_TEXT2, &vText[0], nBufferSize);
		m_strText2 = to_wchar(&vText[0]);
	}

	// �u�� �_�C�A���O�������I�ɕ���
	csSearch.m_bAutoCloseDlgReplace = IsButtonChecked(IDC_CHECK_bAutoCloseDlgReplace);

	// �擪�i�����j����Č��� 2002.01.26 hor
	csSearch.m_bSearchAll = IsButtonChecked(IDC_CHECK_SEARCHALL);

	if (0 < m_strText.size()) {
		// ���K�\���H
		// From Here Jun. 26, 2001 genta
		// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
		int nFlag = 0x00;
		nFlag |= m_searchOption.bLoHiCase ? 0x01 : 0x00;
		if (m_searchOption.bRegularExp
			&& !CheckRegexpSyntax(m_strText.c_str(), GetHwnd(), true, nFlag)
		) {
			return -1;
		}
		// To Here Jun. 26, 2001 genta ���K�\�����C�u���������ւ�

		// ����������
		//@@@ 2002.2.2 YAZAKI CShareData.AddToSearchKeyArr()�ǉ��ɔ����ύX
		if (m_strText.size() < _MAX_PATH) {
			SearchKeywordManager().AddToSearchKeyArr(m_strText.c_str());
			csSearch.m_searchOption = m_searchOption;		// �����I�v�V����
		}
		// 2011.12.18 view�ɒ��ڐݒ�
		EditView* pEditView = (EditView*)m_lParam;
		if (pEditView->m_strCurSearchKey == m_strText && pEditView->m_curSearchOption == m_searchOption) {
		}else {
			pEditView->m_strCurSearchKey = m_strText;
			pEditView->m_curSearchOption = m_searchOption;
			pEditView->m_bCurSearchUpdate = true;
		}
		pEditView->m_nCurSearchKeySequence = GetDllShareData().m_common.m_search.m_nSearchKeySequence;

		// �u���㕶����
		//@@@ 2002.2.2 YAZAKI CShareData.AddToReplaceKeyArr()�ǉ��ɔ����ύX
		if (m_strText2.size() < _MAX_PATH) {
			SearchKeywordManager().AddToReplaceKeyArr(m_strText2.c_str());
		}
		m_nReplaceKeySequence = GetDllShareData().m_common.m_search.m_nReplaceKeySequence;

		// From Here 2001.12.03 hor
		// �N���b�v�{�[�h����\��t����H
		m_bPaste = IsButtonChecked(IDC_CHK_PASTE);
		EnableItem(IDC_COMBO_TEXT2, !m_bPaste);
		// �u���Ώ�
		m_nReplaceTarget = 0;
		if (IsButtonChecked(IDC_RADIO_INSERT)) {
			m_nReplaceTarget = 1;
		}else if (IsButtonChecked(IDC_RADIO_ADD)) {
			m_nReplaceTarget = 2;
		}else if (IsButtonChecked(IDC_RADIO_LINEDELETE )) {
			m_nReplaceTarget = 3;
			m_bPaste = false;
			EnableItem(IDC_COMBO_TEXT2, false);
		}
		// To Here 2001.12.03 hor

		// ����������/�u���㕶���񃊃X�g�̐ݒ�	2010/5/26 Uchi
		if (!m_bModal) {
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
	// Jun. 26, 2001 genta
	// ���̈ʒu�Ő��K�\���̏�����������K�v�͂Ȃ�
	// ���Ƃ̈�ѐ���ۂ��ߍ폜

	// ���[�U�[���R���{ �{�b�N�X�̃G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����
	// Combo_LimitText(GetItemHwnd(IDC_COMBO_TEXT), _MAX_PATH - 1);
	// Combo_LimitText(GetItemHwnd(IDC_COMBO_TEXT2), _MAX_PATH - 1);

	// �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ���
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_TEXT), TRUE);
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_TEXT2), TRUE);

	// �e�L�X�g�I�𒆂�
	if (m_bSelected) {
		EnableItem(IDC_BUTTON_SEARCHPREV, false);	// 2001.12.03 hor �R�����g����
		EnableItem(IDC_BUTTON_SEARCHNEXT, false);	// 2001.12.03 hor �R�����g����
		EnableItem(IDC_BUTTON_REPALCE, false);		// 2001.12.03 hor �R�����g����
		CheckButton(IDC_RADIO_SELECTEDAREA, true);
//		CheckButton(IDC_RADIO_ALLAREA, false);					// 2001.12.03 hor �R�����g
	}else {
//		EnableItem(IDC_RADIO_SELECTEDAREA), false);	// 2001.12.03 hor �R�����g
//		CheckButton(IDC_RADIO_SELECTEDAREA, false);				// 2001.12.03 hor �R�����g
		CheckButton(IDC_RADIO_ALLAREA, true);
	}

	m_comboDelText = ComboBoxItemDeleter();
	m_comboDelText.pRecent = &m_recentSearch;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_TEXT), &m_comboDelText);
	m_comboDelText2 = ComboBoxItemDeleter();
	m_comboDelText2.pRecent = &m_recentReplace;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_TEXT2), &m_comboDelText2);

	// �t�H���g�ݒ�	2012/11/27 Uchi
	HFONT hFontOld = (HFONT)::SendMessage(GetItemHwnd(IDC_COMBO_TEXT), WM_GETFONT, 0, 0);
	HFONT hFont = SetMainFont(GetItemHwnd(IDC_COMBO_TEXT));
	m_fontText.SetFont(hFontOld, hFont, GetItemHwnd(IDC_COMBO_TEXT));

	hFontOld = (HFONT)::SendMessage(GetItemHwnd(IDC_COMBO_TEXT2), WM_GETFONT, 0, 0);
	hFont = SetMainFont(GetItemHwnd(IDC_COMBO_TEXT2));
	m_fontText2.SetFont(hFontOld, hFont, GetItemHwnd(IDC_COMBO_TEXT2));

	// ���N���X�����o
	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
}


BOOL DlgReplace::OnDestroy()
{
	m_fontText.ReleaseOnDestroy();
	m_fontText2.ReleaseOnDestroy();
	return Dialog::OnDestroy();
}


BOOL DlgReplace::OnBnClicked(int wID)
{
	int nRet;
	EditView* pEditView = (EditView*)m_lParam;

	switch (wID) {
	case IDC_CHK_PASTE:
		// �e�L�X�g�̓\��t��
		if (1
			&& IsButtonChecked(IDC_CHK_PASTE)
			&& !pEditView->m_pEditDoc->m_docEditor.IsEnablePaste()
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
// To Here 2001.12.03 hor
	case IDC_BUTTON_HELP:
		//�u�u���v�̃w���v
		// Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_REPLACE_DIALOG));	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
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
			// From Here Jun. 26, 2001 genta
			// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
			if (!CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, true)) {
				CheckButton(IDC_CHK_REGULAREXP, false);
			}else {
			// To Here Jun. 26, 2001 genta

				// �p�啶���Ɖp����������ʂ���
				// Jan. 31, 2002 genta
				// �啶���E�������̋�ʂ͐��K�\���̐ݒ�Ɋւ�炸�ۑ�����
				//CheckButton(IDC_CHK_LOHICASE, true);
				//EnableItem(IDC_CHK_LOHICASE, false);

				// 2001/06/23 N.Nakatani
				// �P��P�ʂŒT��
				EnableItem(IDC_CHK_WORD, false);

				//�u���ׂĒu���v�͒u���̌J�Ԃ�
				EnableItem(IDC_CHECK_CONSECUTIVEALL, true);	// 2007.01.16 ryoji
			}
		}else {
			// �p�啶���Ɖp����������ʂ���
			//EnableItem(IDC_CHK_LOHICASE, true);
			// Jan. 31, 2002 genta
			// �啶���E�������̋�ʂ͐��K�\���̐ݒ�Ɋւ�炸�ۑ�����
			//CheckButton(IDC_CHK_LOHICASE, false);

			// 2001/06/23 N.Nakatani
			// �P��P�ʂŒT��
			EnableItem(IDC_CHK_WORD, true);

			//�u���ׂĒu���v�͒u���̌J�Ԃ�
			EnableItem(IDC_CHECK_CONSECUTIVEALL, false);	// 2007.01.16 ryoji
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

			// �����J�n�ʒu��o�^ 02/07/28 ai start
			if (pEditView->m_bSearch != FALSE) {
				pEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
				pEditView->m_bSearch = FALSE;
			}// 02/07/28 ai end

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

			// �����J�n�ʒu��o�^ 02/07/28 ai start
			if (pEditView->m_bSearch) {
				pEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
				pEditView->m_bSearch = false;
			}// 02/07/28 ai end

			// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
			// ��������
			pEditView->GetCommander().HandleCommand(F_SEARCH_NEXT, true, (LPARAM)GetHwnd(), 0, 0, 0);
			// �ĕ`��i0�������}�b�`�ŃL�����b�g��\�����邽�߁j
			pEditView->Redraw();	// �O��0�������}�b�`�̏����ɂ��K�v
		}else if (nRet == 0) {
			OkMessage(GetHwnd(), LS(STR_DLGREPLC_STR));
		}
		return TRUE;

	case IDC_BUTTON_SETMARK:	// 2002.01.16 hor �Y���s�}�[�N
		nRet = GetData();
		if (0 < nRet) {
			pEditView->GetCommander().HandleCommand(F_BOOKMARK_PATTERN, false, 0, 0, 0, 0);
			::SendMessage(GetHwnd(), WM_NEXTDLGCTL, (WPARAM)GetItemHwnd(IDC_COMBO_TEXT), TRUE);
		}
		return TRUE;

	case IDC_BUTTON_REPALCE:	// �u��
		nRet = GetData();
		if (0 < nRet) {

			// �u���J�n�ʒu��o�^ 02/07/28 ai start
			if (pEditView->m_bSearch) {
				pEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
				pEditView->m_bSearch = false;
			}// 02/07/28 ai end

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
			// �u���J�n�ʒu��o�^ 02/07/28 ai start
			if (pEditView->m_bSearch) {
				pEditView->m_ptSrchStartPos_PHY = m_ptEscCaretPos_PHY;
				pEditView->m_bSearch = false;
			}// 02/07/28 ai end

			// ���ׂčs�u�����̏��u�́u���ׂĒu���v�͒u���̌J�Ԃ��I�v�V����OFF�̏ꍇ�ɂ��č폜 2007.01.16 ryoji
			pEditView->GetCommander().HandleCommand(F_REPLACE_ALL, true, 0, 0, 0, 0);
			pEditView->GetCommander().HandleCommand(F_REDRAW, true, 0, 0, 0, 0);

			// �A�N�e�B�u�ɂ���
			ActivateFrameWindow(GetHwnd());

			TopOkMessage(GetHwnd(), LS(STR_DLGREPLC_REPLACE), m_nReplaceCnt);

			if (!m_bCanceled) {
				if (m_bModal) {		// ���[�_���_�C�A���O��
					// �u���_�C�A���O�����
					::EndDialog(GetHwnd(), 0);
				}else {
					// �u�� �_�C�A���O�������I�ɕ���
					if (m_pShareData->m_common.m_search.m_bAutoCloseDlgReplace) {
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
	// 0�������}�b�`�`���ON/OFF	// 2009.11.29 ryoji
	EditView*	pEditView = (EditView*)m_lParam;
	LayoutRange rangeSel = pEditView->GetSelectionInfo().m_select;
	if (rangeSel.IsValid() && rangeSel.IsLineOne() && rangeSel.IsOne())
		pEditView->InvalidateRect(NULL);	// �A�N�e�B�u���^��A�N�e�B�u�����������Ă���ĕ`��

	return Dialog::OnActivate(wParam, lParam);
}

//@@@ 2002.01.18 add start
LPVOID DlgReplace::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


