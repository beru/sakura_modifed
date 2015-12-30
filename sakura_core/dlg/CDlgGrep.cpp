/*!	@file
	@brief GREP�_�C�A���O�{�b�N�X

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, genta
	Copyright (C) 2002, MIK, genta, Moca, YAZAKI
	Copyright (C) 2003, Moca
	Copyright (C) 2006, ryoji
	Copyright (C) 2010, ryoji
	Copyright (C) 2012, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include <ShellAPI.h>
#include "dlg/CDlgGrep.h"
#include "CGrepAgent.h"
#include "CGrepEnumKeys.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "charset/CCodePage.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/os.h"
#include "util/window.h"
#include "env/DLLSHAREDATA.h"
#include "env/CSakuraEnvironment.h"
#include "sakura_rc.h"
#include "sakura.hh"

//GREP CDlgGrep.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12000
	IDC_BUTTON_FOLDER,				HIDC_GREP_BUTTON_FOLDER,			// �t�H���_
	IDC_BUTTON_CURRENTFOLDER,		HIDC_GREP_BUTTON_CURRENTFOLDER,		// ���t�H���_
	IDOK,							HIDOK_GREP,							// ����
	IDCANCEL,						HIDCANCEL_GREP,						// �L�����Z��
	IDC_BUTTON_HELP,				HIDC_GREP_BUTTON_HELP,				// �w���v
	IDC_CHK_WORD,					HIDC_GREP_CHK_WORD,					// �P��P��
	IDC_CHK_SUBFOLDER,				HIDC_GREP_CHK_SUBFOLDER,			// �T�u�t�H���_������
	IDC_CHK_FROMTHISTEXT,			HIDC_GREP_CHK_FROMTHISTEXT,			// ���̃t�@�C������
	IDC_CHK_LOHICASE,				HIDC_GREP_CHK_LOHICASE,				// �啶��������
	IDC_CHK_REGULAREXP,				HIDC_GREP_CHK_REGULAREXP,			// ���K�\��
	IDC_COMBO_CHARSET,				HIDC_GREP_COMBO_CHARSET,			// �����R�[�h�Z�b�g
	IDC_CHECK_CP,					HIDC_GREP_CHECK_CP,					// �R�[�h�y�[�W
	IDC_COMBO_TEXT,					HIDC_GREP_COMBO_TEXT,				// ����
	IDC_COMBO_FILE,					HIDC_GREP_COMBO_FILE,				// �t�@�C��
	IDC_COMBO_FOLDER,				HIDC_GREP_COMBO_FOLDER,				// �t�H���_
	IDC_BUTTON_FOLDER_UP,			HIDC_GREP_BUTTON_FOLDER_UP,			// ��
	IDC_RADIO_OUTPUTLINE,			HIDC_GREP_RADIO_OUTPUTLINE,			// ���ʏo�́F�s�P��
	IDC_RADIO_OUTPUTMARKED,			HIDC_GREP_RADIO_OUTPUTMARKED,		// ���ʏo�́F�Y������
	IDC_RADIO_OUTPUTSTYLE1,			HIDC_GREP_RADIO_OUTPUTSTYLE1,		// ���ʏo�͌`���F�m�[�}��
	IDC_RADIO_OUTPUTSTYLE2,			HIDC_GREP_RADIO_OUTPUTSTYLE2,		// ���ʏo�͌`���F�t�@�C����
	IDC_RADIO_OUTPUTSTYLE3,			HIDC_RADIO_OUTPUTSTYLE3,			// ���ʏo�͌`���F���ʂ̂�
	IDC_STATIC_JRE32VER,			HIDC_GREP_STATIC_JRE32VER,			// ���K�\���o�[�W����
	IDC_CHK_DEFAULTFOLDER,			HIDC_GREP_CHK_DEFAULTFOLDER,		// �t�H���_�̏����l���J�����g�t�H���_�ɂ���
	IDC_CHECK_FILE_ONLY,			HIDC_CHECK_FILE_ONLY,				// �t�@�C�����ŏ��̂݌���
	IDC_CHECK_BASE_PATH,			HIDC_CHECK_BASE_PATH,				// �x�[�X�t�H���_�\��
	IDC_CHECK_SEP_FOLDER,			HIDC_CHECK_SEP_FOLDER,				// �t�H���_���ɕ\��
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

static void SetGrepFolder(HWND hwndCtrl, LPCTSTR folder);

CDlgGrep::CDlgGrep()
{
	m_bSubFolder = FALSE;				// �T�u�t�H���_�������������
	m_bFromThisText = FALSE;			// ���̕ҏW���̃e�L�X�g���猟������
	m_sSearchOption.Reset();			// �����I�v�V����
	m_nGrepCharSet = CODE_SJIS;			// �����R�[�h�Z�b�g
	m_nGrepOutputLineType = 1;			// �s���o��/�Y������/�ۃ}�b�`�s ���o��
	m_nGrepOutputStyle = 1;				// Grep: �o�͌`��
	m_bGrepOutputFileOnly = false;
	m_bGrepOutputBaseFolder = false;
	m_bGrepSeparateFolder = false;

	m_bSetText = false;
	m_szFile[0] = 0;
	m_szFolder[0] = 0;
	return;
}

/*!
	�R���{�{�b�N�X�̃h���b�v�_�E�����b�Z�[�W��ߑ�����

	@date 2013.03.24 novice �V�K�쐬
*/
BOOL CDlgGrep::OnCbnDropDown(HWND hwndCtl, int wID)
{
	auto& searchKeywords = m_pShareData->m_sSearchKeywords;
	switch (wID) {
	case IDC_COMBO_TEXT:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			int nSize = searchKeywords.m_aSearchKeys.size();
			for (int i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, searchKeywords.m_aSearchKeys[i] );
			}
		}
		break;
	case IDC_COMBO_FILE:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			int nSize = searchKeywords.m_aGrepFiles.size();
			for (int i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, searchKeywords.m_aGrepFiles[i] );
			}
		}
		break;
	case IDC_COMBO_FOLDER:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			int nSize = searchKeywords.m_aGrepFolders.size();
			for (int i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, searchKeywords.m_aGrepFolders[i] );
			}
		}
		break;
	}
	return CDialog::OnCbnDropDown( hwndCtl, wID );
}

// ���[�_���_�C�A���O�̕\��
int CDlgGrep::DoModal(
	HINSTANCE hInstance,
	HWND hwndParent,
	const TCHAR* pszCurrentFilePath
	)
{
	auto& csSearch = m_pShareData->m_Common.m_sSearch;
	m_bSubFolder = csSearch.m_bGrepSubFolder;			// Grep: �T�u�t�H���_������
	m_sSearchOption = csSearch.m_sSearchOption;			// �����I�v�V����
	m_nGrepCharSet = csSearch.m_nGrepCharSet;			// �����R�[�h�Z�b�g
	m_nGrepOutputLineType = csSearch.m_nGrepOutputLineType;	// �s���o��/�Y������/�ۃ}�b�`�s ���o��
	m_nGrepOutputStyle = csSearch.m_nGrepOutputStyle;	// Grep: �o�͌`��
	m_bGrepOutputFileOnly = csSearch.m_bGrepOutputFileOnly;
	m_bGrepOutputBaseFolder = csSearch.m_bGrepOutputBaseFolder;
	m_bGrepSeparateFolder = csSearch.m_bGrepSeparateFolder;

	// 2013.05.21 �R���X�g���N�^����DoModal�Ɉړ�
	// m_strText �͌Ăяo�����Őݒ�ς�
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

	return (int)CDialog::DoModal(hInstance, hwndParent, IDD_GREP, (LPARAM)NULL);
}

// 2007.02.09 bosagami
LRESULT CALLBACK OnFolderProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
WNDPROC g_pOnFolderProc;

BOOL CDlgGrep::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwndDlg);

	// ���[�U�[���R���{�{�b�N�X�̃G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌�����
	// Combo_LimitText(GetItemHwnd(IDC_COMBO_TEXT), _MAX_PATH - 1);
	Combo_LimitText(GetItemHwnd(IDC_COMBO_FILE), _MAX_PATH - 1);
	Combo_LimitText(GetItemHwnd(IDC_COMBO_FOLDER), _MAX_PATH - 1);
	
	// �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ���
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_TEXT), TRUE);
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_FILE), TRUE);
	Combo_SetExtendedUI(GetItemHwnd(IDC_COMBO_FOLDER), TRUE);

	// �_�C�A���O�̃A�C�R��
// 2002.02.08 Grep�A�C�R�����傫���A�C�R���Ə������A�C�R����ʁX�ɂ���B
	// Dec, 2, 2002 genta �A�C�R���ǂݍ��ݕ��@�ύX
	HICON hIconBig   = GetAppIcon(m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, false);
	HICON hIconSmall = GetAppIcon(m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, true);
	::SendMessage(GetHwnd(), WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
	::SendMessage(GetHwnd(), WM_SETICON, ICON_BIG, (LPARAM)hIconBig);

	// �����R�[�h�Z�b�g�I���R���{�{�b�N�X������
	CCodeTypesForCombobox cCodeTypes;
	for (int i=0; i<cCodeTypes.GetCount(); ++i) {
		int idx = Combo_AddString(GetItemHwnd(IDC_COMBO_CHARSET), cCodeTypes.GetName(i));
		Combo_SetItemData(GetItemHwnd(IDC_COMBO_CHARSET), idx, cCodeTypes.GetCode(i));
	}
	// 2007.02.09 bosagami
	HWND hFolder = GetItemHwnd(IDC_COMBO_FOLDER);
	DragAcceptFiles(hFolder, true);
	g_pOnFolderProc = (WNDPROC)GetWindowLongPtr(hFolder, GWLP_WNDPROC);
	SetWindowLongPtr(hFolder, GWLP_WNDPROC, (LONG_PTR)OnFolderProc);

	m_comboDelText = SComboBoxItemDeleter();
	m_comboDelText.pRecent = &m_cRecentSearch;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_TEXT), &m_comboDelText);
	m_comboDelFile = SComboBoxItemDeleter();
	m_comboDelFile.pRecent = &m_cRecentGrepFile;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_FILE), &m_comboDelFile);
	m_comboDelFolder = SComboBoxItemDeleter();
	m_comboDelFolder.pRecent = &m_cRecentGrepFolder;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_FOLDER), &m_comboDelFolder);

	// �t�H���g�ݒ�	2012/11/27 Uchi
	HFONT hFontOld = (HFONT)::SendMessage(GetItemHwnd(IDC_COMBO_TEXT), WM_GETFONT, 0, 0);
	HFONT hFont = SetMainFont(GetItemHwnd(IDC_COMBO_TEXT));
	m_cFontText.SetFont(hFontOld, hFont, GetItemHwnd(IDC_COMBO_TEXT));

	// ���N���X�����o
//	CreateSizeBox();
	return CDialog::OnInitDialog(hwndDlg, wParam, lParam);
}

/*! @brief �t�H���_�w��EditBox�̃R�[���o�b�N�֐�

	@date 2007.02.09 bosagami �V�K�쐬
	@date 2007.09.02 genta �f�B���N�g���`�F�b�N������
*/
LRESULT CALLBACK OnFolderProc(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
	)
{
	if (msg == WM_DROPFILES) do {
		// From Here 2007.09.02 genta 
		SFilePath sPath;
		if (DragQueryFile((HDROP)wparam, 0, NULL, 0) > _countof2(sPath) - 1) {
			// skip if the length of the path exceeds buffer capacity
			break;
		}
		DragQueryFile((HDROP)wparam, 0, sPath, _countof2(sPath) - 1);

		// �t�@�C���p�X�̉���
		CSakuraEnvironment::ResolvePath(sPath);
		
		// �t�@�C�����h���b�v���ꂽ�ꍇ�̓t�H���_��؂�o��
		// �t�H���_�̏ꍇ�͍Ōオ������̂�split���Ă͂����Ȃ��D
		if (IsFileExists(sPath, true)) {	// ��2������true���ƃf�B���N�g���͑ΏۊO
			SFilePath szWork;
			SplitPath_FolderAndFile(sPath, szWork, NULL);
			_tcscpy(sPath, szWork);
		}

		SetGrepFolder(hwnd, sPath);
	}while (0);	// 1�񂵂��ʂ�Ȃ�. break�ł����܂Ŕ��

	return  CallWindowProc(g_pOnFolderProc, hwnd, msg, wparam, lparam);
}

BOOL CDlgGrep::OnDestroy()
{
	m_cFontText.ReleaseOnDestroy();
	return CDialog::OnDestroy();
}

BOOL CDlgGrep::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_HELP:
		//�uGrep�v�̃w���v
		// Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_GREP_DIALOG));	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	case IDC_CHK_FROMTHISTEXT:	// ���̕ҏW���̃e�L�X�g���猟������
		// 2010.05.30 �֐���
		SetDataFromThisText(IsButtonChecked(IDC_CHK_FROMTHISTEXT));
		return TRUE;
	case IDC_BUTTON_CURRENTFOLDER:	// ���ݕҏW���̃t�@�C���̃t�H���_
		// �t�@�C�����J���Ă��邩
		if (m_szCurrentFilePath[0] != _T('\0')) {
			TCHAR	szWorkFolder[MAX_PATH];
			TCHAR	szWorkFile[MAX_PATH];
			SplitPath_FolderAndFile(m_szCurrentFilePath, szWorkFolder, szWorkFile);
			SetGrepFolder(GetItemHwnd(IDC_COMBO_FOLDER), szWorkFolder);
		}else {
			// ���݂̃v���Z�X�̃J�����g�f�B���N�g�����擾���܂�
			TCHAR	szWorkFolder[MAX_PATH];
			::GetCurrentDirectory(_countof(szWorkFolder) - 1, szWorkFolder);
			SetGrepFolder(GetItemHwnd(IDC_COMBO_FOLDER), szWorkFolder);
		}
		return TRUE;
	case IDC_BUTTON_FOLDER_UP:
		{
			HWND hwnd = GetItemHwnd(IDC_COMBO_FOLDER);
			TCHAR szFolder[_MAX_PATH];
			::GetWindowText(hwnd, szFolder, _countof(szFolder));
			std::vector<std::tstring> vPaths;
			CGrepAgent::CreateFolders(szFolder, vPaths);
			if (0 < vPaths.size()) {
				// �Ō�̃p�X������Ώ�
				auto_strncpy(szFolder, vPaths.rbegin()->c_str(), _MAX_PATH);
				szFolder[_MAX_PATH-1] = _T('\0');
				if (DirectoryUp(szFolder)) {
					*(vPaths.rbegin()) = szFolder;
					szFolder[0] = _T('\0');
					for (int i=0; i<(int)vPaths.size(); ++i) {
						TCHAR szFolderItem[_MAX_PATH];
						auto_strncpy(szFolderItem, vPaths[i].c_str(), _MAX_PATH);
						szFolderItem[_MAX_PATH-1] = _T('\0');
						if (auto_strchr(szFolderItem, _T(';'))) {
							szFolderItem[0] = _T('"');
							auto_strncpy(szFolderItem + 1, vPaths[i].c_str(), _MAX_PATH - 1);
							szFolderItem[_MAX_PATH-1] = _T('\0');
							auto_strcat(szFolderItem, _T("\""));
							szFolderItem[_MAX_PATH-1] = _T('\0');
						}
						if (i) {
							auto_strcat(szFolder, _T(";"));
							szFolder[_MAX_PATH-1] = _T('\0');
						}
						auto_strcat_s(szFolder, _MAX_PATH, szFolderItem);
					}
					::SetWindowText(hwnd, szFolder);
				}
			}
		}
		return TRUE;

//	case IDC_CHK_LOHICASE:	// �p�啶���Ɖp����������ʂ���
//		MYTRACE(_T("IDC_CHK_LOHICASE\n"));
//		return TRUE;
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
				// ���K�\���̂Ƃ����I���ł���悤�ɁB
//				CheckButton(IDC_CHK_LOHICASE, true);
//				EnableItem(IDC_CHK_LOHICASE), false);

				// 2001/06/23 N.Nakatani
				// �P��P�ʂŌ���
				EnableItem(IDC_CHK_WORD, false);
			}
		}else {
			// �p�啶���Ɖp����������ʂ���
			// ���K�\���̂Ƃ����I���ł���悤�ɁB
//			EnableItem(IDC_CHK_LOHICASE), true);
//			CheckButton(IDC_CHK_LOHICASE, false);


// 2001/06/23 N.Nakatani
// �P��P�ʂ�grep���������ꂽ��R�����g���O���Ǝv���܂�
// 2002/03/07�������Ă݂��B
			// �P��P�ʂŌ���
			EnableItem(IDC_CHK_WORD, true);
		}
		return TRUE;

	case IDC_BUTTON_FOLDER:
		// �t�H���_�Q�ƃ{�^��
		{
			TCHAR	szFolder[MAX_PATH];
			// �����t�H���_
			GetItemText(IDC_COMBO_FOLDER, szFolder, _MAX_PATH - 1);
			if (szFolder[0] == _T('\0')) {
				::GetCurrentDirectory(_countof(szFolder), szFolder);
			}
			if (SelectDir(GetHwnd(), LS(STR_DLGGREP1), szFolder, szFolder)) {
				SetGrepFolder(GetItemHwnd(IDC_COMBO_FOLDER), szFolder);
			}
		}

		return TRUE;
	case IDC_CHECK_CP:
		{
			if (IsButtonChecked(IDC_CHECK_CP)) {
				EnableItem(IDC_CHECK_CP, false);
				HWND combo = GetItemHwnd(IDC_COMBO_CHARSET );
				CCodePage::AddComboCodePages(GetHwnd(), combo, -1);
			}
		}
		return TRUE;
	case IDC_CHK_DEFAULTFOLDER:
		// �t�H���_�̏����l���J�����g�t�H���_�ɂ���
		{
			m_pShareData->m_Common.m_sSearch.m_bGrepDefaultFolder = IsButtonChecked(IDC_CHK_DEFAULTFOLDER);
		}
		return TRUE;
	case IDC_RADIO_OUTPUTSTYLE3:
		{
			EnableItem(IDC_CHECK_BASE_PATH, false);
			EnableItem(IDC_CHECK_SEP_FOLDER, false);
		}
		break;
	case IDC_RADIO_OUTPUTSTYLE1:
	case IDC_RADIO_OUTPUTSTYLE2:
		{
			EnableItem(IDC_CHECK_BASE_PATH, true);
			EnableItem(IDC_CHECK_SEP_FOLDER, true);
		}
		break;
	case IDOK:
		// �_�C�A���O�f�[�^�̎擾
		if (GetData()) {
//			::EndDialog(hwndDlg, TRUE);
			CloseDialog(TRUE);
		}
		return TRUE;
	case IDCANCEL:
//		::EndDialog(hwndDlg, FALSE);
		CloseDialog(FALSE);
		return TRUE;
	}

	// ���N���X�����o
	return CDialog::OnBnClicked(wID);
}


// �_�C�A���O�f�[�^�̐ݒ�
void CDlgGrep::SetData(void)
{
	// ����������
	SetItemText(IDC_COMBO_TEXT, m_strText.c_str());

	// �����t�@�C��
	SetItemText(IDC_COMBO_FILE, m_szFile);

	// �����t�H���_
	SetItemText(IDC_COMBO_FOLDER, m_szFolder);

	if (1
		&& (m_szFolder[0] == _T('\0') || m_pShareData->m_Common.m_sSearch.m_bGrepDefaultFolder)
		&& m_szCurrentFilePath[0] != _T('\0')
	) {
		TCHAR szWorkFolder[MAX_PATH];
		TCHAR szWorkFile[MAX_PATH];
		SplitPath_FolderAndFile(m_szCurrentFilePath, szWorkFolder, szWorkFile);
		SetGrepFolder(GetItemHwnd(IDC_COMBO_FOLDER), szWorkFolder);
	}

	// �T�u�t�H���_�������������
	CheckButton(IDC_CHK_SUBFOLDER, m_bSubFolder);

	// ���̕ҏW���̃e�L�X�g���猟������
	CheckButton(IDC_CHK_FROMTHISTEXT, m_bFromThisText);
	// 2010.05.30 �֐���
	SetDataFromThisText(m_bFromThisText);

	// �p�啶���Ɖp����������ʂ���
	CheckButton(IDC_CHK_LOHICASE, m_sSearchOption.bLoHiCase);

	// 2001/06/23 N.Nakatani �����_�ł�Grep�ł͒P��P�ʂ̌����̓T�|�[�g�ł��Ă��܂���
	// 2002/03/07 �e�X�g�T�|�[�g
	// ��v����P��̂݌�������
	CheckButton(IDC_CHK_WORD, m_sSearchOption.bWordOnly);
//	EnableItem(IDC_CHK_WORD) , false);	// �`�F�b�N�{�b�N�X���g�p�s�ɂ���

	// �����R�[�h��������
//	CheckButton(IDC_CHK_KANJICODEAUTODETECT, m_bKanjiCode_AutoDetect);

	// 2002/09/22 Moca Add
	// �����R�[�h�Z�b�g
	{
		int	nIdx, nCurIdx = -1;
		ECodeType nCharSet;
		HWND hWndCombo = GetItemHwnd(IDC_COMBO_CHARSET);
		nCurIdx = Combo_GetCurSel(hWndCombo);
		CCodeTypesForCombobox cCodeTypes;
		for (nIdx=0; nIdx<cCodeTypes.GetCount(); ++nIdx) {
			nCharSet = (ECodeType)Combo_GetItemData(hWndCombo, nIdx);
			if (nCharSet == m_nGrepCharSet) {
				nCurIdx = nIdx;
			}
		}
		if (nCurIdx != -1) {
			Combo_SetCurSel(hWndCombo, nCurIdx);
		}else {
			CheckButton(IDC_CHECK_CP, true);
			EnableItem(IDC_CHECK_CP, false);
			nCurIdx = CCodePage::AddComboCodePages(GetHwnd(), hWndCombo, m_nGrepCharSet);
			if (nCurIdx == -1) {
				Combo_SetCurSel( hWndCombo, 0 );
			}
		}
	}

	// �s���o�͂��邩�Y�����������o�͂��邩
	if (m_nGrepOutputLineType == 1) {
		CheckButton(IDC_RADIO_OUTPUTLINE, true);
	}else if (m_nGrepOutputLineType == 2) {
		CheckButton(IDC_RADIO_NOHIT, true);
	}else {
		CheckButton(IDC_RADIO_OUTPUTMARKED, true);
	}

	EnableItem(IDC_CHECK_BASE_PATH, true);
	EnableItem(IDC_CHECK_SEP_FOLDER, true);
	// Grep: �o�͌`��
	if (m_nGrepOutputStyle == 1) {
		CheckButton(IDC_RADIO_OUTPUTSTYLE1, true);
	}else if (m_nGrepOutputStyle == 2) {
		CheckButton(IDC_RADIO_OUTPUTSTYLE2, true);
	}else if (m_nGrepOutputStyle == 3) {
		CheckButton(IDC_RADIO_OUTPUTSTYLE3, true);
		EnableItem(IDC_CHECK_BASE_PATH, false);
		EnableItem(IDC_CHECK_SEP_FOLDER, false);
	}else {
		CheckButton(IDC_RADIO_OUTPUTSTYLE1, true);
	}

	// From Here Jun. 29, 2001 genta
	// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
	// �����t���[�y�є�������̌������B�K�����K�\���̃`�F�b�N��
	// ���֌W��CheckRegexpVersion��ʉ߂���悤�ɂ����B
	if (1
		&& CheckRegexpVersion(GetHwnd(), IDC_STATIC_JRE32VER, false)
		&& m_sSearchOption.bRegularExp
	) {
		// �p�啶���Ɖp����������ʂ���
		CheckButton(IDC_CHK_REGULAREXP, true);
		// ���K�\���̂Ƃ����I���ł���悤�ɁB
//		CheckButton(IDC_CHK_LOHICASE, true);
//		EnableItem(IDC_CHK_LOHICASE), false);

		// 2001/06/23 N.Nakatani
		// �P��P�ʂŒT��
		EnableItem(IDC_CHK_WORD, false);
	}else {
		CheckButton(IDC_CHK_REGULAREXP, false);
	}
	// To Here Jun. 29, 2001 genta

	EnableItem(IDC_CHK_FROMTHISTEXT, m_szCurrentFilePath[0] != _T('\0'));

	CheckDlgButtonBool(GetHwnd(), IDC_CHECK_FILE_ONLY, m_bGrepOutputFileOnly);
	CheckDlgButtonBool(GetHwnd(), IDC_CHECK_BASE_PATH, m_bGrepOutputBaseFolder);
	CheckDlgButtonBool(GetHwnd(), IDC_CHECK_SEP_FOLDER, m_bGrepSeparateFolder);

	// �t�H���_�̏����l���J�����g�t�H���_�ɂ���
	CheckButton(IDC_CHK_DEFAULTFOLDER, m_pShareData->m_Common.m_sSearch.m_bGrepDefaultFolder);

	return;
}


/*!
	���ݕҏW���t�@�C�����猟���`�F�b�N�ł̐ݒ�
*/
void CDlgGrep::SetDataFromThisText(bool bChecked)
{
	bool bEnableControls = true;
	if (m_szCurrentFilePath[0] != 0 && bChecked) {
		TCHAR szWorkFolder[MAX_PATH];
		TCHAR szWorkFile[MAX_PATH];
		// 2003.08.01 Moca �t�@�C�����̓X�y�[�X�Ȃǂ͋�؂�L���ɂȂ�̂ŁA""�ň͂��A�G�X�P�[�v����
		szWorkFile[0] = _T('"');
		SplitPath_FolderAndFile(m_szCurrentFilePath, szWorkFolder, szWorkFile + 1);
		_tcscat(szWorkFile, _T("\"")); // 2003.08.01 Moca
		SetItemText(IDC_COMBO_FILE, szWorkFile);
		SetGrepFolder(GetItemHwnd(IDC_COMBO_FOLDER), szWorkFolder);

		CheckButton(IDC_CHK_SUBFOLDER, false);
		bEnableControls = false;
	}
	EnableItem(IDC_COMBO_FILE,    bEnableControls);
	EnableItem(IDC_COMBO_FOLDER,  bEnableControls);
	EnableItem(IDC_BUTTON_FOLDER, bEnableControls);
	EnableItem(IDC_CHK_SUBFOLDER, bEnableControls);
	return;
}

// �_�C�A���O�f�[�^�̎擾
// TRUE==����  FALSE==���̓G���[
int CDlgGrep::GetData(void)
{
	// �T�u�t�H���_�������������
	m_bSubFolder = IsButtonChecked(IDC_CHK_SUBFOLDER);

	auto& csSearch = m_pShareData->m_Common.m_sSearch;
	csSearch.m_bGrepSubFolder = m_bSubFolder;		// Grep�F�T�u�t�H���_������

	// ���̕ҏW���̃e�L�X�g���猟������
	m_bFromThisText = IsButtonChecked(IDC_CHK_FROMTHISTEXT);
	// �p�啶���Ɖp����������ʂ���
	m_sSearchOption.bLoHiCase = IsButtonChecked(IDC_CHK_LOHICASE);

	// 2001/06/23 N.Nakatani
	// �P��P�ʂŌ���
	m_sSearchOption.bWordOnly = IsButtonChecked(IDC_CHK_WORD);

	// ���K�\��
	m_sSearchOption.bRegularExp = IsButtonChecked(IDC_CHK_REGULAREXP);

	// �����R�[�h��������
//	m_bKanjiCode_AutoDetect = IsButtonChecked(IDC_CHK_KANJICODEAUTODETECT);

	// �����R�[�h�Z�b�g
	{
		int		nIdx;
		HWND	hWndCombo = GetItemHwnd(IDC_COMBO_CHARSET);
		nIdx = Combo_GetCurSel(hWndCombo);
		m_nGrepCharSet = (ECodeType)Combo_GetItemData(hWndCombo, nIdx);
	}

	// �s���o��/�Y������/�ۃ}�b�`�s ���o��
	if (IsButtonChecked(IDC_RADIO_OUTPUTLINE )) {
		m_nGrepOutputLineType = 1;
	}else if (IsButtonChecked(IDC_RADIO_NOHIT )) {
		m_nGrepOutputLineType = 2;
	}else {
		m_nGrepOutputLineType = 0;
	}
	
	// Grep: �o�͌`��
	if (IsButtonChecked(IDC_RADIO_OUTPUTSTYLE1)) {
		m_nGrepOutputStyle = 1;				// Grep: �o�͌`��
	}
	if (IsButtonChecked(IDC_RADIO_OUTPUTSTYLE2)) {
		m_nGrepOutputStyle = 2;				// Grep: �o�͌`��
	}
	if (IsButtonChecked(IDC_RADIO_OUTPUTSTYLE3)) {
		m_nGrepOutputStyle = 3;
	}

	m_bGrepOutputFileOnly = IsButtonChecked(IDC_CHECK_FILE_ONLY);
	m_bGrepOutputBaseFolder = IsButtonChecked(IDC_CHECK_BASE_PATH);
	m_bGrepSeparateFolder = IsButtonChecked(IDC_CHECK_SEP_FOLDER);

	// ����������
	int nBufferSize = ::GetWindowTextLength(GetItemHwnd(IDC_COMBO_TEXT)) + 1;
	std::vector<TCHAR> vText(nBufferSize);
	GetItemText(IDC_COMBO_TEXT, &vText[0], nBufferSize);
	m_strText = to_wchar(&vText[0]);
	m_bSetText = true;
	
	// �����t�@�C��
	GetItemText(IDC_COMBO_FILE, m_szFile, _countof2(m_szFile));
	// �����t�H���_
	GetItemText(IDC_COMBO_FOLDER, m_szFolder, _countof2(m_szFolder));

	csSearch.m_nGrepCharSet = m_nGrepCharSet;				// �����R�[�h��������
	csSearch.m_nGrepOutputLineType = m_nGrepOutputLineType;	// �s���o��/�Y������/�ۃ}�b�`�s ���o��
	csSearch.m_nGrepOutputStyle = m_nGrepOutputStyle;		// Grep: �o�͌`��
	csSearch.m_bGrepOutputFileOnly = m_bGrepOutputFileOnly;
	csSearch.m_bGrepOutputBaseFolder = m_bGrepOutputBaseFolder;
	csSearch.m_bGrepSeparateFolder = m_bGrepSeparateFolder;

// ��߂܂���
//	if (wcslen(m_szText) == 0) {
//		WarningMessage(	GetHwnd(), _T("�����̃L�[���[�h���w�肵�Ă��������B"));
//		return FALSE;
//	}
	if (auto_strlen(m_szFile) != 0) {
		CGrepEnumKeys enumKeys;
		int nErrorNo = enumKeys.SetFileKeys(m_szFile);
		if (nErrorNo == 1) {
			WarningMessage(	GetHwnd(), LS(STR_DLGGREP2));
			return FALSE;
		}else if (nErrorNo == 2) {
			WarningMessage(	GetHwnd(), LS(STR_DLGGREP3));
			return FALSE;
		}
	}
	// ���̕ҏW���̃e�L�X�g���猟������
	if (m_szFile[0] == _T('\0')) {
		// Jun. 16, 2003 Moca
		// �����p�^�[�����w�肳��Ă��Ȃ��ꍇ�̃��b�Z�[�W�\������߁A
		//�u*.*�v���w�肳�ꂽ���̂ƌ��Ȃ��D
		_tcscpy(m_szFile, _T("*.*"));
	}
	if (m_szFolder[0] == _T('\0')) {
		WarningMessage(	GetHwnd(), LS(STR_DLGGREP4));
		return FALSE;
	}

	{
		// �J�����g�f�B���N�g����ۑ��B���̃u���b�N���甲����Ƃ��Ɏ����ŃJ�����g�f�B���N�g���͕��������B
		CCurrentDirectoryBackupPoint cCurDirBackup;

		// 2011.11.24 Moca �����t�H���_�w��
		std::vector<std::tstring> vPaths;
		CGrepAgent::CreateFolders(m_szFolder, vPaths);
		int nFolderLen = 0;
		TCHAR szFolder[_MAX_PATH];
		szFolder[0] = _T('\0');
		for (int i=0; i<(int)vPaths.size(); ++i) {
			// ���΃p�X����΃p�X
			if (!::SetCurrentDirectory(vPaths[i].c_str())) {
				WarningMessage(	GetHwnd(), LS(STR_DLGGREP5));
				return FALSE;
			}
			TCHAR szFolderItem[_MAX_PATH];
			::GetCurrentDirectory(_MAX_PATH, szFolderItem);
			// ;���t�H���_���Ɋ܂܂�Ă�����""�ň͂�
			if (auto_strchr(szFolderItem, _T(';'))) {
				szFolderItem[0] = _T('"');
				::GetCurrentDirectory(_MAX_PATH, szFolderItem + 1);
				auto_strcat(szFolderItem, _T("\""));
			}
			int nFolderItemLen = auto_strlen(szFolderItem);
			if (_MAX_PATH < nFolderLen + nFolderItemLen + 1) {
				WarningMessage(	GetHwnd(), LS(STR_DLGGREP6));
				return FALSE;
			}
			if (i) {
				auto_strcat(szFolder, _T(";"));
			}
			auto_strcat(szFolder, szFolderItem);
			nFolderLen = auto_strlen(szFolder);
		}
		auto_strcpy(m_szFolder, szFolder);
	}

//@@@ 2002.2.2 YAZAKI CShareData.AddToSearchKeyArr()�ǉ��ɔ����ύX
	// ����������
	if (0 < m_strText.size()) {
		// From Here Jun. 26, 2001 genta
		// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
		int nFlag = 0;
		nFlag |= m_sSearchOption.bLoHiCase ? 0x01 : 0x00;
		if (m_sSearchOption.bRegularExp  && !CheckRegexpSyntax(m_strText.c_str(), GetHwnd(), true, nFlag)) {
			return FALSE;
		}
		// To Here Jun. 26, 2001 genta ���K�\�����C�u���������ւ�
		if (m_strText.size() < _MAX_PATH) {
			CSearchKeywordManager().AddToSearchKeyArr(m_strText.c_str());
			csSearch.m_sSearchOption = m_sSearchOption;		// �����I�v�V����
		}
	}else {
		// 2014.07.01 ��L�[���o�^����
		CSearchKeywordManager().AddToSearchKeyArr( L"" );
	}

	// ���̕ҏW���̃e�L�X�g���猟������ꍇ�A�����Ɏc���Ȃ�	Uchi 2008/5/23
	if (!m_bFromThisText) {
		// �����t�@�C��
		CSearchKeywordManager().AddToGrepFileArr(m_szFile);

		// �����t�H���_
		CSearchKeywordManager().AddToGrepFolderArr(m_szFolder);
	}

	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgGrep::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


static void SetGrepFolder(HWND hwndCtrl, LPCTSTR folder)
{
	if (auto_strchr(folder, _T(';'))) {
		TCHAR szQuoteFolder[MAX_PATH];
		szQuoteFolder[0] = _T('"');
		auto_strcpy(szQuoteFolder + 1, folder);
		auto_strcat(szQuoteFolder, _T("\""));
		::SetWindowText(hwndCtrl, szQuoteFolder);
	}else {
		::SetWindowText(hwndCtrl, folder);
	}
}

