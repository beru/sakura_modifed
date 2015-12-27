/*!	@file
	@brief GREP�u���_�C�A���O�{�b�N�X

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, genta
	Copyright (C) 2002, MIK, genta, Moca, YAZAKI
	Copyright (C) 2003, Moca
	Copyright (C) 2006, ryoji
	Copyright (C) 2010, ryoji
	Copyright (C) 2014, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/CDlgGrepReplace.h"
#include "View/CEditView.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "util/module.h"
#include "util/shell.h"
#include "util/os.h"
#include "util/window.h"
#include "env/DLLSHAREDATA.h"
#include "env/CSakuraEnvironment.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {
	IDC_BUTTON_CURRENTFOLDER,		HIDC_GREP_REP_BUTTON_CURRENTFOLDER,		// ���t�H���_
	IDC_BUTTON_FOLDER,				HIDC_GREP_REP_BUTTON_FOLDER,			// �t�H���_
	IDOK,							HIDOK_GREP_REP,							// �u���J�n
	IDCANCEL,						HIDCANCEL_GREP_REP,						// �L�����Z��
	IDC_BUTTON_HELP,				HIDC_GREP_REP_BUTTON_HELP,				// �w���v
	IDC_CHK_PASTE,					HIDC_GREP_REP_CHK_PASTE,				// �N���b�v�{�[�h����\��t��
	IDC_CHK_WORD,					HIDC_GREP_REP_CHK_WORD,					// �P��P��
	IDC_CHK_SUBFOLDER,				HIDC_GREP_REP_CHK_SUBFOLDER,			// �T�u�t�H���_������
//	IDC_CHK_FROMTHISTEXT,			HIDC_GREP_REP_CHK_FROMTHISTEXT,			// ���̃t�@�C������
	IDC_CHK_LOHICASE,				HIDC_GREP_REP_CHK_LOHICASE,				// �啶��������
	IDC_CHK_REGULAREXP,				HIDC_GREP_REP_CHK_REGULAREXP,			// ���K�\��
	IDC_CHK_BACKUP,					HIDC_GREP_REP_CHK_BACKUP,				// �o�b�N�A�b�v�쐬
	IDC_COMBO_CHARSET,				HIDC_GREP_REP_COMBO_CHARSET,			// �����R�[�h�Z�b�g
	IDC_CHECK_CP,					HIDC_GREP_REP_CHECK_CP,					// CP
	IDC_COMBO_TEXT,					HIDC_GREP_REP_COMBO_TEXT,				// �u���O
	IDC_COMBO_TEXT2,				HIDC_GREP_REP_COMBO_TEXT2,				// �u����
	IDC_COMBO_FILE,					HIDC_GREP_REP_COMBO_FILE,				// �t�@�C��
	IDC_COMBO_FOLDER,				HIDC_GREP_REP_COMBO_FOLDER,				// �t�H���_
	IDC_BUTTON_FOLDER_UP,			HIDC_GREP_REP_BUTTON_FOLDER_UP,			// ��
	IDC_RADIO_OUTPUTLINE,			HIDC_GREP_REP_RADIO_OUTPUTLINE,			// ���ʏo�́F�s�P��
	IDC_RADIO_OUTPUTMARKED,			HIDC_GREP_REP_RADIO_OUTPUTMARKED,		// ���ʏo�́F�Y������
	IDC_RADIO_OUTPUTSTYLE1,			HIDC_GREP_REP_RADIO_OUTPUTSTYLE1,		// ���ʏo�͌`���F�m�[�}��
	IDC_RADIO_OUTPUTSTYLE2,			HIDC_GREP_REP_RADIO_OUTPUTSTYLE2,		// ���ʏo�͌`���F�t�@�C����
	IDC_RADIO_OUTPUTSTYLE3,			HIDC_GREP_REP_RADIO_OUTPUTSTYLE3,		// ���ʏo�͌`���F���ʂ̂�
	IDC_STATIC_JRE32VER,			HIDC_GREP_REP_STATIC_JRE32VER,			// ���K�\���o�[�W����
	IDC_CHK_DEFAULTFOLDER,			HIDC_GREP_REP_CHK_DEFAULTFOLDER,		// �t�H���_�̏����l���J�����g�t�H���_�ɂ���
	IDC_CHECK_FILE_ONLY,			HIDC_GREP_REP_CHECK_FILE_ONLY,			// �t�@�C�����ŏ��̂݌���
	IDC_CHECK_BASE_PATH,			HIDC_GREP_REP_CHECK_BASE_PATH,			// �x�[�X�t�H���_�\��
	IDC_CHECK_SEP_FOLDER,			HIDC_GREP_REP_CHECK_SEP_FOLDER,			// �t�H���_���ɕ\��
	0, 0
};

CDlgGrepReplace::CDlgGrepReplace()
{
	if (0 < m_pShareData->m_sSearchKeywords.m_aReplaceKeys.size()) {
		m_strText2 = m_pShareData->m_sSearchKeywords.m_aReplaceKeys[0];
	}
	return;
}



// ���[�_���_�C�A���O�̕\��
int CDlgGrepReplace::DoModal( HINSTANCE hInstance, HWND hwndParent, const TCHAR* pszCurrentFilePath, LPARAM lParam )
{
	auto& csSearch = m_pShareData->m_Common.m_sSearch;
	m_bSubFolder = csSearch.m_bGrepSubFolder;				// Grep: �T�u�t�H���_������
	m_sSearchOption = csSearch.m_sSearchOption;				// �����I�v�V����
	m_nGrepCharSet = csSearch.m_nGrepCharSet;				// �����R�[�h�Z�b�g
	m_nGrepOutputLineType = csSearch.m_nGrepOutputLineType;	// �s���o�͂��邩�Y�����������o�͂��邩
	m_nGrepOutputStyle = csSearch.m_nGrepOutputStyle;		// Grep: �o�͌`��
	m_bPaste = false;
	m_bBackup = csSearch.m_bGrepBackup;

	auto& searchKeywords = m_pShareData->m_sSearchKeywords;
	if (m_szFile[0] == _T('\0') && searchKeywords.m_aGrepFiles.size()) {
		_tcscpy(m_szFile, searchKeywords.m_aGrepFiles[0]);		// �����t�@�C��
	}
	if (m_szFolder[0] == _T('\0') && searchKeywords.m_aGrepFolders.size()) {
		_tcscpy(m_szFolder, searchKeywords.m_aGrepFolders[0]);	// �����t�H���_
	}
	if (pszCurrentFilePath) {	// 2010.01.10 ryoji
		_tcscpy(m_szCurrentFilePath, pszCurrentFilePath);
	}

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_GREP_REPLACE, lParam );
}

BOOL CDlgGrepReplace::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );

	// �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ���
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_TEXT2), TRUE);

	HFONT hFontOld = (HFONT)::SendMessage(GetItemHwnd( IDC_COMBO_TEXT2 ), WM_GETFONT, 0, 0);
	HFONT hFont = SetMainFont(GetItemHwnd( IDC_COMBO_TEXT2 ));
	m_cFontText2.SetFont(hFontOld, hFont, GetItemHwnd( IDC_COMBO_TEXT2 ));

	return CDlgGrep::OnInitDialog( hwndDlg, wParam, lParam );
}


BOOL CDlgGrepReplace::OnDestroy()
{
	m_cFontText2.ReleaseOnDestroy();
	return CDlgGrep::OnDestroy();
}


BOOL CDlgGrepReplace::OnBnClicked( int wID )
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_GREP_REPLACE_DLG) );
		return TRUE;
	case IDC_CHK_PASTE:
	case IDOK:
		{
			bool bStop = false;
			CEditView* pcEditView = (CEditView*)m_lParam;
			if (IsButtonChecked(IDC_CHK_PASTE)
				&& !pcEditView->m_pcEditDoc->m_cDocEditor.IsEnablePaste()
			) {
				OkMessage(GetHwnd(), LS(STR_DLGREPLC_CLIPBOARD));
				CheckButton(IDC_CHK_PASTE, false);
				bStop = true;
			}
			EnableItem(IDC_COMBO_TEXT2, !IsButtonChecked(IDC_CHK_PASTE));
			if (wID == IDOK && bStop) {
				return TRUE;
			}
		}
	}
	// ���N���X�����o
	return CDlgGrep::OnBnClicked( wID );
}



// �_�C�A���O�f�[�^�̐ݒ�
void CDlgGrepReplace::SetData( void )
{
	// �u����
	SetItemText(IDC_COMBO_TEXT2, m_strText2.c_str() );
	HWND hwndCombo = GetItemHwnd(IDC_COMBO_TEXT2);
	auto& replaceKeys = m_pShareData->m_sSearchKeywords.m_aReplaceKeys;
	for (int i=0; i<replaceKeys.size(); ++i) {
		Combo_AddString(hwndCombo, replaceKeys[i]);
	}
	CheckDlgButtonBool(GetHwnd(), IDC_CHK_BACKUP, m_bBackup);
	CDlgGrep::SetData();
}


/*! �_�C�A���O�f�[�^�̎擾
	TRUE==����  FALSE==���̓G���[
*/
int CDlgGrepReplace::GetData( void )
{
	m_bPaste = IsButtonChecked(IDC_CHK_PASTE);

	// �u����
	int nBufferSize = ::GetWindowTextLength( GetItemHwnd(IDC_COMBO_TEXT2) ) + 1;
	std::vector<TCHAR> vText(nBufferSize);
	GetItemText(IDC_COMBO_TEXT2, &vText[0], nBufferSize);
	m_strText2 = to_wchar(&vText[0]);

	if (::GetWindowTextLength( GetItemHwnd(IDC_COMBO_TEXT) ) == 0){
		WarningMessage(	GetHwnd(), LS(STR_DLGREPLC_REPSTR) );
		return FALSE;
	}

	m_bBackup = IsButtonChecked(IDC_CHK_BACKUP);
	m_pShareData->m_Common.m_sSearch.m_bGrepBackup = m_bBackup;

	if (!CDlgGrep::GetData()) {
		return FALSE;
	}

	if (m_strText2.size() < _MAX_PATH) {
		CSearchKeywordManager().AddToReplaceKeyArr( m_strText2.c_str() );
	}
	m_nReplaceKeySequence = GetDllShareData().m_Common.m_sSearch.m_nReplaceKeySequence;

	return TRUE;
}

LPVOID CDlgGrepReplace::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}


