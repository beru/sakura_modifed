/*!	@file
	@brief GREP�_�C�A���O�{�b�N�X
*/
#include "StdAfx.h"
#include <ShellAPI.h>
#include "dlg/DlgGrep.h"
#include "GrepAgent.h"
#include "GrepEnumKeys.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "charset/CodePage.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/os.h"
#include "util/window.h"
#include "env/DllSharedData.h"
#include "env/SakuraEnvironment.h"
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

DlgGrep::DlgGrep()
{
	bSubFolder = false;				// �T�u�t�H���_�������������
	bFromThisText = false;			// ���̕ҏW���̃e�L�X�g���猟������
	searchOption.Reset();			// �����I�v�V����
	nGrepCharSet = CODE_SJIS;			// �����R�[�h�Z�b�g
	nGrepOutputLineType = 1;			// �s���o��/�Y������/�ۃ}�b�`�s ���o��
	nGrepOutputStyle = 1;				// Grep: �o�͌`��
	bGrepOutputFileOnly = false;
	bGrepOutputBaseFolder = false;
	bGrepSeparateFolder = false;

	bSetText = false;
	szFile[0] = 0;
	szFolder[0] = 0;
	return;
}

/*!
	�R���{�{�b�N�X�̃h���b�v�_�E�����b�Z�[�W��ߑ�����

	@date 2013.03.24 novice �V�K�쐬
*/
BOOL DlgGrep::OnCbnDropDown(HWND hwndCtl, int wID)
{
	auto& searchKeywords = pShareData->searchKeywords;
	switch (wID) {
	case IDC_COMBO_TEXT:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			size_t nSize = searchKeywords.searchKeys.size();
			for (size_t i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, searchKeywords.searchKeys[i] );
			}
		}
		break;
	case IDC_COMBO_FILE:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			size_t nSize = searchKeywords.grepFiles.size();
			for (size_t i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, searchKeywords.grepFiles[i] );
			}
		}
		break;
	case IDC_COMBO_FOLDER:
		if (::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L) == 0) {
			size_t nSize = searchKeywords.grepFolders.size();
			for (size_t i=0; i<nSize; ++i) {
				Combo_AddString( hwndCtl, searchKeywords.grepFolders[i] );
			}
		}
		break;
	}
	return Dialog::OnCbnDropDown( hwndCtl, wID );
}

// ���[�_���_�C�A���O�̕\��
INT_PTR DlgGrep::DoModal(
	HINSTANCE hInstance,
	HWND hwndParent,
	const TCHAR* pszCurrentFilePath
	)
{
	auto& csSearch = pShareData->common.search;
	bSubFolder = csSearch.bGrepSubFolder;			// Grep: �T�u�t�H���_������
	searchOption = csSearch.searchOption;			// �����I�v�V����
	nGrepCharSet = csSearch.nGrepCharSet;			// �����R�[�h�Z�b�g
	nGrepOutputLineType = csSearch.nGrepOutputLineType;	// �s���o��/�Y������/�ۃ}�b�`�s ���o��
	nGrepOutputStyle = csSearch.nGrepOutputStyle;	// Grep: �o�͌`��
	bGrepOutputFileOnly = csSearch.bGrepOutputFileOnly;
	bGrepOutputBaseFolder = csSearch.bGrepOutputBaseFolder;
	bGrepSeparateFolder = csSearch.bGrepSeparateFolder;

	// 2013.05.21 �R���X�g���N�^����DoModal�Ɉړ�
	// strText �͌Ăяo�����Őݒ�ς�
	auto& searchKeywords = pShareData->searchKeywords;
	if (szFile[0] == _T('\0') && searchKeywords.grepFiles.size()) {
		_tcscpy(szFile, searchKeywords.grepFiles[0]);		// �����t�@�C��
	}
	if (szFolder[0] == _T('\0') && searchKeywords.grepFolders.size()) {
		_tcscpy(szFolder, searchKeywords.grepFolders[0]);	// �����t�H���_
	}

	if (pszCurrentFilePath) {	// 2010.01.10 ryoji
		_tcscpy(szCurrentFilePath, pszCurrentFilePath);
	}

	return Dialog::DoModal(hInstance, hwndParent, IDD_GREP, (LPARAM)NULL);
}

// 2007.02.09 bosagami
LRESULT CALLBACK OnFolderProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
WNDPROC g_pOnFolderProc;

BOOL DlgGrep::OnInitDialog(
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
	HICON hIconBig   = GetAppIcon(hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, false);
	HICON hIconSmall = GetAppIcon(hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, true);
	::SendMessage(GetHwnd(), WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
	::SendMessage(GetHwnd(), WM_SETICON, ICON_BIG, (LPARAM)hIconBig);

	// �����R�[�h�Z�b�g�I���R���{�{�b�N�X������
	CodeTypesForCombobox codeTypes;
	for (size_t i=0; i<codeTypes.GetCount(); ++i) {
		LONG_PTR idx = Combo_AddString(GetItemHwnd(IDC_COMBO_CHARSET), codeTypes.GetName(i));
		Combo_SetItemData(GetItemHwnd(IDC_COMBO_CHARSET), idx, codeTypes.GetCode(i));
	}
	// 2007.02.09 bosagami
	HWND hFolder = GetItemHwnd(IDC_COMBO_FOLDER);
	DragAcceptFiles(hFolder, true);
	g_pOnFolderProc = (WNDPROC)GetWindowLongPtr(hFolder, GWLP_WNDPROC);
	SetWindowLongPtr(hFolder, GWLP_WNDPROC, (LONG_PTR)OnFolderProc);

	comboDelText = ComboBoxItemDeleter();
	comboDelText.pRecent = &recentSearch;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_TEXT), &comboDelText);
	comboDelFile = ComboBoxItemDeleter();
	comboDelFile.pRecent = &recentGrepFile;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_FILE), &comboDelFile);
	comboDelFolder = ComboBoxItemDeleter();
	comboDelFolder.pRecent = &recentGrepFolder;
	SetComboBoxDeleter(GetItemHwnd(IDC_COMBO_FOLDER), &comboDelFolder);

	// �t�H���g�ݒ�	2012/11/27 Uchi
	HFONT hFontOld = (HFONT)::SendMessage(GetItemHwnd(IDC_COMBO_TEXT), WM_GETFONT, 0, 0);
	HFONT hFont = SetMainFont(GetItemHwnd(IDC_COMBO_TEXT));
	fontText.SetFont(hFontOld, hFont, GetItemHwnd(IDC_COMBO_TEXT));

	// ���N���X�����o
//	CreateSizeBox();
	return Dialog::OnInitDialog(hwndDlg, wParam, lParam);
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
		SFilePath path;
		if (DragQueryFile((HDROP)wparam, 0, NULL, 0) > _countof2(path) - 1) {
			// skip if the length of the path exceeds buffer capacity
			break;
		}
		DragQueryFile((HDROP)wparam, 0, path, _countof2(path) - 1);

		// �t�@�C���p�X�̉���
		SakuraEnvironment::ResolvePath(path);
		
		// �t�@�C�����h���b�v���ꂽ�ꍇ�̓t�H���_��؂�o��
		// �t�H���_�̏ꍇ�͍Ōオ������̂�split���Ă͂����Ȃ��D
		if (IsFileExists(path, true)) {	// ��2������true���ƃf�B���N�g���͑ΏۊO
			SFilePath szWork;
			SplitPath_FolderAndFile(path, szWork, NULL);
			_tcscpy(path, szWork);
		}

		SetGrepFolder(hwnd, path);
	}while (0);	// 1�񂵂��ʂ�Ȃ�. break�ł����܂Ŕ��

	return  CallWindowProc(g_pOnFolderProc, hwnd, msg, wparam, lparam);
}

BOOL DlgGrep::OnDestroy()
{
	fontText.ReleaseOnDestroy();
	return Dialog::OnDestroy();
}

BOOL DlgGrep::OnBnClicked(int wID)
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
		if (szCurrentFilePath[0] != _T('\0')) {
			TCHAR	szWorkFolder[MAX_PATH];
			TCHAR	szWorkFile[MAX_PATH];
			SplitPath_FolderAndFile(szCurrentFilePath, szWorkFolder, szWorkFile);
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
			GrepAgent::CreateFolders(szFolder, vPaths);
			if (0 < vPaths.size()) {
				// �Ō�̃p�X������Ώ�
				auto_strncpy(szFolder, vPaths.rbegin()->c_str(), _MAX_PATH);
				szFolder[_MAX_PATH-1] = _T('\0');
				if (DirectoryUp(szFolder)) {
					*(vPaths.rbegin()) = szFolder;
					szFolder[0] = _T('\0');
					for (size_t i=0; i<vPaths.size(); ++i) {
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
				CodePage::AddComboCodePages(GetHwnd(), combo, -1);
			}
		}
		return TRUE;
	case IDC_CHK_DEFAULTFOLDER:
		// �t�H���_�̏����l���J�����g�t�H���_�ɂ���
		{
			pShareData->common.search.bGrepDefaultFolder = IsButtonChecked(IDC_CHK_DEFAULTFOLDER);
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
	return Dialog::OnBnClicked(wID);
}


// �_�C�A���O�f�[�^�̐ݒ�
void DlgGrep::SetData(void)
{
	// ����������
	SetItemText(IDC_COMBO_TEXT, strText.c_str());

	// �����t�@�C��
	SetItemText(IDC_COMBO_FILE, szFile);

	// �����t�H���_
	SetItemText(IDC_COMBO_FOLDER, szFolder);

	if (1
		&& (szFolder[0] == _T('\0') || pShareData->common.search.bGrepDefaultFolder)
		&& szCurrentFilePath[0] != _T('\0')
	) {
		TCHAR szWorkFolder[MAX_PATH];
		TCHAR szWorkFile[MAX_PATH];
		SplitPath_FolderAndFile(szCurrentFilePath, szWorkFolder, szWorkFile);
		SetGrepFolder(GetItemHwnd(IDC_COMBO_FOLDER), szWorkFolder);
	}

	// �T�u�t�H���_�������������
	CheckButton(IDC_CHK_SUBFOLDER, bSubFolder);

	// ���̕ҏW���̃e�L�X�g���猟������
	CheckButton(IDC_CHK_FROMTHISTEXT, bFromThisText);
	// 2010.05.30 �֐���
	SetDataFromThisText(bFromThisText);

	// �p�啶���Ɖp����������ʂ���
	CheckButton(IDC_CHK_LOHICASE, searchOption.bLoHiCase);

	// 2001/06/23 N.Nakatani �����_�ł�Grep�ł͒P��P�ʂ̌����̓T�|�[�g�ł��Ă��܂���
	// 2002/03/07 �e�X�g�T�|�[�g
	// ��v����P��̂݌�������
	CheckButton(IDC_CHK_WORD, searchOption.bWordOnly);
//	EnableItem(IDC_CHK_WORD) , false);	// �`�F�b�N�{�b�N�X���g�p�s�ɂ���

	// �����R�[�h��������
//	CheckButton(IDC_CHK_KANJICODEAUTODETECT, bKanjiCode_AutoDetect);

	// 2002/09/22 Moca Add
	// �����R�[�h�Z�b�g
	{
		int	nCurIdx = -1;
		EncodingType nCharSet;
		HWND hWndCombo = GetItemHwnd(IDC_COMBO_CHARSET);
		nCurIdx = Combo_GetCurSel(hWndCombo);
		CodeTypesForCombobox codeTypes;
		for (size_t nIdx=0; nIdx<codeTypes.GetCount(); ++nIdx) {
			nCharSet = (EncodingType)Combo_GetItemData(hWndCombo, nIdx);
			if (nCharSet == nGrepCharSet) {
				nCurIdx = (int)nIdx;
			}
		}
		if (nCurIdx != -1) {
			Combo_SetCurSel(hWndCombo, nCurIdx);
		}else {
			CheckButton(IDC_CHECK_CP, true);
			EnableItem(IDC_CHECK_CP, false);
			nCurIdx = CodePage::AddComboCodePages(GetHwnd(), hWndCombo, nGrepCharSet);
			if (nCurIdx == -1) {
				Combo_SetCurSel( hWndCombo, 0 );
			}
		}
	}

	// �s���o�͂��邩�Y�����������o�͂��邩
	if (nGrepOutputLineType == 1) {
		CheckButton(IDC_RADIO_OUTPUTLINE, true);
	}else if (nGrepOutputLineType == 2) {
		CheckButton(IDC_RADIO_NOHIT, true);
	}else {
		CheckButton(IDC_RADIO_OUTPUTMARKED, true);
	}

	EnableItem(IDC_CHECK_BASE_PATH, true);
	EnableItem(IDC_CHECK_SEP_FOLDER, true);
	// Grep: �o�͌`��
	if (nGrepOutputStyle == 1) {
		CheckButton(IDC_RADIO_OUTPUTSTYLE1, true);
	}else if (nGrepOutputStyle == 2) {
		CheckButton(IDC_RADIO_OUTPUTSTYLE2, true);
	}else if (nGrepOutputStyle == 3) {
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
		&& searchOption.bRegularExp
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

	EnableItem(IDC_CHK_FROMTHISTEXT, szCurrentFilePath[0] != _T('\0'));

	CheckButton(IDC_CHECK_FILE_ONLY, bGrepOutputFileOnly);
	CheckButton(IDC_CHECK_BASE_PATH, bGrepOutputBaseFolder);
	CheckButton(IDC_CHECK_SEP_FOLDER, bGrepSeparateFolder);

	// �t�H���_�̏����l���J�����g�t�H���_�ɂ���
	CheckButton(IDC_CHK_DEFAULTFOLDER, pShareData->common.search.bGrepDefaultFolder);

	return;
}


/*!
	���ݕҏW���t�@�C�����猟���`�F�b�N�ł̐ݒ�
*/
void DlgGrep::SetDataFromThisText(bool bChecked)
{
	bool bEnableControls = true;
	if (szCurrentFilePath[0] != 0 && bChecked) {
		TCHAR szWorkFolder[MAX_PATH];
		TCHAR szWorkFile[MAX_PATH];
		// 2003.08.01 Moca �t�@�C�����̓X�y�[�X�Ȃǂ͋�؂�L���ɂȂ�̂ŁA""�ň͂��A�G�X�P�[�v����
		szWorkFile[0] = _T('"');
		SplitPath_FolderAndFile(szCurrentFilePath, szWorkFolder, szWorkFile + 1);
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
int DlgGrep::GetData(void)
{
	// �T�u�t�H���_�������������
	bSubFolder = IsButtonChecked(IDC_CHK_SUBFOLDER);

	auto& csSearch = pShareData->common.search;
	csSearch.bGrepSubFolder = bSubFolder;		// Grep�F�T�u�t�H���_������

	// ���̕ҏW���̃e�L�X�g���猟������
	bFromThisText = IsButtonChecked(IDC_CHK_FROMTHISTEXT);
	// �p�啶���Ɖp����������ʂ���
	searchOption.bLoHiCase = IsButtonChecked(IDC_CHK_LOHICASE);

	// 2001/06/23 N.Nakatani
	// �P��P�ʂŌ���
	searchOption.bWordOnly = IsButtonChecked(IDC_CHK_WORD);

	// ���K�\��
	searchOption.bRegularExp = IsButtonChecked(IDC_CHK_REGULAREXP);

	// �����R�[�h��������
//	bKanjiCode_AutoDetect = IsButtonChecked(IDC_CHK_KANJICODEAUTODETECT);

	// �����R�[�h�Z�b�g
	{
		int		nIdx;
		HWND	hWndCombo = GetItemHwnd(IDC_COMBO_CHARSET);
		nIdx = Combo_GetCurSel(hWndCombo);
		nGrepCharSet = (EncodingType)Combo_GetItemData(hWndCombo, nIdx);
	}

	// �s���o��/�Y������/�ۃ}�b�`�s ���o��
	if (IsButtonChecked(IDC_RADIO_OUTPUTLINE )) {
		nGrepOutputLineType = 1;
	}else if (IsButtonChecked(IDC_RADIO_NOHIT )) {
		nGrepOutputLineType = 2;
	}else {
		nGrepOutputLineType = 0;
	}
	
	// Grep: �o�͌`��
	if (IsButtonChecked(IDC_RADIO_OUTPUTSTYLE1)) {
		nGrepOutputStyle = 1;				// Grep: �o�͌`��
	}
	if (IsButtonChecked(IDC_RADIO_OUTPUTSTYLE2)) {
		nGrepOutputStyle = 2;				// Grep: �o�͌`��
	}
	if (IsButtonChecked(IDC_RADIO_OUTPUTSTYLE3)) {
		nGrepOutputStyle = 3;
	}

	bGrepOutputFileOnly = IsButtonChecked(IDC_CHECK_FILE_ONLY);
	bGrepOutputBaseFolder = IsButtonChecked(IDC_CHECK_BASE_PATH);
	bGrepSeparateFolder = IsButtonChecked(IDC_CHECK_SEP_FOLDER);

	// ����������
	int nBufferSize = ::GetWindowTextLength(GetItemHwnd(IDC_COMBO_TEXT)) + 1;
	std::vector<TCHAR> vText(nBufferSize);
	GetItemText(IDC_COMBO_TEXT, &vText[0], nBufferSize);
	strText = to_wchar(&vText[0]);
	bSetText = true;
	
	// �����t�@�C��
	GetItemText(IDC_COMBO_FILE, szFile, _countof2(szFile));
	// �����t�H���_
	GetItemText(IDC_COMBO_FOLDER, szFolder, _countof2(szFolder));

	csSearch.nGrepCharSet = nGrepCharSet;				// �����R�[�h��������
	csSearch.nGrepOutputLineType = nGrepOutputLineType;	// �s���o��/�Y������/�ۃ}�b�`�s ���o��
	csSearch.nGrepOutputStyle = nGrepOutputStyle;		// Grep: �o�͌`��
	csSearch.bGrepOutputFileOnly = bGrepOutputFileOnly;
	csSearch.bGrepOutputBaseFolder = bGrepOutputBaseFolder;
	csSearch.bGrepSeparateFolder = bGrepSeparateFolder;

// ��߂܂���
//	if (wcslen(szText) == 0) {
//		WarningMessage(	GetHwnd(), _T("�����̃L�[���[�h���w�肵�Ă��������B"));
//		return FALSE;
//	}
	if (auto_strlen(szFile) != 0) {
		GrepEnumKeys enumKeys;
		int nErrorNo = enumKeys.SetFileKeys(szFile);
		if (nErrorNo == 1) {
			WarningMessage(	GetHwnd(), LS(STR_DLGGREP2));
			return FALSE;
		}else if (nErrorNo == 2) {
			WarningMessage(	GetHwnd(), LS(STR_DLGGREP3));
			return FALSE;
		}
	}
	// ���̕ҏW���̃e�L�X�g���猟������
	if (szFile[0] == _T('\0')) {
		// Jun. 16, 2003 Moca
		// �����p�^�[�����w�肳��Ă��Ȃ��ꍇ�̃��b�Z�[�W�\������߁A
		//�u*.*�v���w�肳�ꂽ���̂ƌ��Ȃ��D
		_tcscpy(szFile, _T("*.*"));
	}
	if (szFolder[0] == _T('\0')) {
		WarningMessage(	GetHwnd(), LS(STR_DLGGREP4));
		return FALSE;
	}

	{
		// �J�����g�f�B���N�g����ۑ��B���̃u���b�N���甲����Ƃ��Ɏ����ŃJ�����g�f�B���N�g���͕��������B
		CurrentDirectoryBackupPoint cCurDirBackup;

		// 2011.11.24 Moca �����t�H���_�w��
		std::vector<std::tstring> paths;
		GrepAgent::CreateFolders(szFolder, paths);
		size_t nFolderLen = 0;
		TCHAR szFolder[_MAX_PATH];
		szFolder[0] = _T('\0');
		for (size_t i=0; i<paths.size(); ++i) {
			// ���΃p�X����΃p�X
			if (!::SetCurrentDirectory(paths[i].c_str())) {
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
			size_t nFolderItemLen = auto_strlen(szFolderItem);
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
		auto_strcpy(szFolder, szFolder);
	}

//@@@ 2002.2.2 YAZAKI CShareData.AddToSearchKeys()�ǉ��ɔ����ύX
	// ����������
	if (0 < strText.size()) {
		// From Here Jun. 26, 2001 genta
		// ���K�\�����C�u�����̍����ւ��ɔ��������̌�����
		int nFlag = 0;
		nFlag |= searchOption.bLoHiCase ? 0x01 : 0x00;
		if (searchOption.bRegularExp  && !CheckRegexpSyntax(strText.c_str(), GetHwnd(), true, nFlag)) {
			return FALSE;
		}
		// To Here Jun. 26, 2001 genta ���K�\�����C�u���������ւ�
		if (strText.size() < _MAX_PATH) {
			SearchKeywordManager().AddToSearchKeys(strText.c_str());
			csSearch.searchOption = searchOption;		// �����I�v�V����
		}
	}else {
		// 2014.07.01 ��L�[���o�^����
		SearchKeywordManager().AddToSearchKeys( L"" );
	}

	// ���̕ҏW���̃e�L�X�g���猟������ꍇ�A�����Ɏc���Ȃ�	Uchi 2008/5/23
	if (!bFromThisText) {
		// �����t�@�C��
		SearchKeywordManager().AddToGrepFiles(szFile);

		// �����t�H���_
		SearchKeywordManager().AddToGrepFolders(szFolder);
	}

	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID DlgGrep::GetHelpIdTable(void)
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

