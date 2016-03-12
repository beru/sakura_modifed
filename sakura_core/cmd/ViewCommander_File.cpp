/*!	@file
@brief ViewCommander�N���X�̃R�}���h(�t�@�C������n)�֐��Q

	2012/12/20	ViewCommander.cpp���番��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2003, MIK, genta, �����, Moca
	Copyright (C) 2004, genta
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji, maru
	Copyright (C) 2007, ryoji, maru, genta

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "ViewCommander.h"
#include "ViewCommander_inline.h"

#include "_main/ControlTray.h"
#include "uiparts/WaitCursor.h"
#include "dlg/DlgProperty.h"
#include "dlg/DlgCancel.h"// 2002/2/8 hor
#include "dlg/DlgProfileMgr.h"
#include "doc/DocReader.h"	//  Command_PROPERTY_FILE for _DEBUG
#include "print/PrintPreview.h"
#include "io/BinaryStream.h"
#include "io/FileLoad.h"
#include "WriteManager.h"
#include "EditApp.h"
#include "recent/MRUFile.h"
#include "util/window.h"
#include "charset/CodeFactory.h"
#include "plugin/Plugin.h"
#include "plugin/JackManager.h"
#include "env/SakuraEnvironment.h"
#include "debug/RunningTimer.h"
#include "sakura_rc.h"


// �V�K�쐬
void ViewCommander::Command_FILENEW(void)
{
	// �V���ȕҏW�E�B���h�E���N��
	LoadInfo loadInfo;
	loadInfo.filePath = _T("");
	loadInfo.eCharCode = CODE_NONE;
	loadInfo.bViewMode = false;
	std::tstring curDir = SakuraEnvironment::GetDlgInitialDir();
	ControlTray::OpenNewEditor(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
		loadInfo,
		NULL,
		false,
		curDir.c_str(),
		false
	);
	return;
}


// �V�K�쐬�i�V�����E�B���h�E�ŊJ���j
void ViewCommander::Command_FILENEW_NEWWINDOW(void)
{
	// �V���ȕҏW�E�B���h�E���N��
	LoadInfo loadInfo;
	loadInfo.filePath = _T("");
	loadInfo.eCharCode = CODE_DEFAULT;
	loadInfo.bViewMode = false;
	std::tstring curDir = SakuraEnvironment::GetDlgInitialDir();
	ControlTray::OpenNewEditor(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
		loadInfo,
		NULL,
		false,
		curDir.c_str(),
		true
	);
	return;
}


/*! @brief �t�@�C�����J��

	@date 2003.03.30 genta �u���ĊJ���v���痘�p���邽�߂Ɉ����ǉ�
	@date 2004.10.09 genta ������EditDoc�ֈړ�
*/
void ViewCommander::Command_FILEOPEN(
	const WCHAR* filename,
	EncodingType nCharCode,
	bool bViewMode,
	const WCHAR* defaultName
	)
{
	if (!IsValidCodeType(nCharCode) && nCharCode != CODE_AUTODETECT) {
		nCharCode = CODE_AUTODETECT;
	}
	// ���[�h���
	LoadInfo loadInfo(filename ? to_tchar(filename) : _T(""), nCharCode, bViewMode);
	std::vector<std::tstring> files;
	std::tstring defName = (defaultName ? to_tchar(defaultName) : _T(""));

	// �K�v�ł���΁u�t�@�C�����J���v�_�C�A���O
	if (!loadInfo.filePath.IsValidPath()) {
		if (!defName.empty()) {
			TCHAR szPath[_MAX_PATH];
			TCHAR szDir[_MAX_DIR];
			TCHAR szName[_MAX_FNAME];
			TCHAR szExt  [_MAX_EXT];
			my_splitpath_t(defName.c_str(), szPath, szDir, szName, szExt);
			auto_strcat(szPath, szDir);
			if (auto_stricmp(defName.c_str(), szPath) == 0) {
				// defName�̓t�H���_��������
			}else {
				FilePath path = defName.c_str();
				if (auto_stricmp(path.GetDirPath().c_str(), szPath) == 0) {
					// �t�H���_���܂ł͎��݂��Ă���
					loadInfo.filePath = defName.c_str();
				}
			}
		}
		bool bDlgResult = GetDocument()->m_docFileOperation.OpenFileDialog(
			EditWnd::getInstance()->GetHwnd(),	// [in]  �I�[�i�[�E�B���h�E
			defName.length() == 0 ? NULL : defName.c_str(),	// [in]  �t�H���_
			&loadInfo,							// [out] ���[�h���󂯎��
			files								// [out] �t�@�C����
		);
		if (!bDlgResult) return;

		loadInfo.filePath = files[0].c_str();
		// ���̃t�@�C���͐V�K�E�B���h�E
		int nSize = (int)files.size();
		for (int i=1; i<nSize; ++i) {
			LoadInfo filesLoadInfo = loadInfo;
			filesLoadInfo.filePath = files[i].c_str();
			ControlTray::OpenNewEditor(
				G_AppInstance(),
				EditWnd::getInstance()->GetHwnd(),
				filesLoadInfo,
				NULL,
				true
			);
		}
	}

	// �J��
	GetDocument()->m_docFileOperation.FileLoad(&loadInfo);
}


/*! �㏑���ۑ�

	F_FILESAVEALL�Ƃ̑g�ݍ��킹�݂̂Ŏg����R�}���h�D
	@param warnbeep [in] true: �ۑ��s�v or �ۑ��֎~�̂Ƃ��Ɍx�����o��
	@param askname	[in] true: �t�@�C�������ݒ�̎��ɓ��͂𑣂�

	@date 2004.02.28 genta ����warnbeep�ǉ�
	@date 2005.01.24 genta ����askname�ǉ�

*/
bool ViewCommander::Command_FILESAVE(bool warnbeep, bool askname)
{
	EditDoc* pDoc = GetDocument();

	// �t�@�C�������w�肳��Ă��Ȃ��ꍇ�́u���O��t���ĕۑ��v�̃t���[�֑J��
	if (!GetDocument()->m_docFile.GetFilePathClass().IsValidPath()) {
		if (!askname) {
			return false;	// �ۑ����Ȃ�
		}
		return pDoc->m_docFileOperation.FileSaveAs();
	}

	// �Z�[�u���
	SaveInfo saveInfo;
	pDoc->GetSaveInfo(&saveInfo);
	saveInfo.eol = EolType::None; // ���s�R�[�h���ϊ�
	saveInfo.bOverwriteMode = true; // �㏑���v��

	// �㏑������
	auto& soundSet = EditApp::getInstance()->m_soundSet;
	if (!warnbeep) soundSet.MuteOn();
	bool bRet = pDoc->m_docFileOperation.DoSaveFlow(&saveInfo);
	if (!warnbeep) soundSet.MuteOff();

	return bRet;
}


// ���O��t���ĕۑ��_�C�A���O
bool ViewCommander::Command_FILESAVEAS_DIALOG(
	const WCHAR* fileNameDef,
	EncodingType eCodeType,
	EolType eEolType
	)
{
	return GetDocument()->m_docFileOperation.FileSaveAs(fileNameDef, eCodeType, eEolType, true);
}


/* ���O��t���ĕۑ�
	filename�ŕۑ��BNULL�͌��ցB
*/
bool ViewCommander::Command_FILESAVEAS(
	const WCHAR* filename,
	EolType eEolType
	)
{
	return GetDocument()->m_docFileOperation.FileSaveAs(filename, CODE_NONE, eEolType, false);
}


/*!	�S�ď㏑���ۑ�

	�ҏW���̑S�ẴE�B���h�E�ŏ㏑���ۑ����s���D
	�������C�㏑���ۑ��̎w�����o���݂̂Ŏ��s���ʂ̊m�F�͍s��Ȃ��D

	�㏑���֎~�y�уt�@�C�������ݒ�̃E�B���h�E�ł͉����s��Ȃ��D

	@date 2005.01.24 genta �V�K�쐬
*/
bool ViewCommander::Command_FILESAVEALL(void)
{
	AppNodeGroupHandle(0).SendMessageToAllEditors(
		WM_COMMAND,
		MAKELONG(F_FILESAVE_QUIET, 0),
		0,
		NULL
	);
	return true;
}


// ����(����)	// Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
void ViewCommander::Command_FILECLOSE(void)
{
	GetDocument()->m_docFileOperation.FileClose();
}


/*! @brief ���ĊJ��

	@date 2003.03.30 genta �J���_�C�A���O�ŃL�����Z�������Ƃ����̃t�@�C�����c��悤�ɁB
				���ł�FILEOPEN�Ɠ����悤�Ɉ�����ǉ����Ă���
*/
void ViewCommander::Command_FILECLOSE_OPEN(
	LPCWSTR filename,
	EncodingType nCharCode,
	bool bViewMode
	)
{
	GetDocument()->m_docFileOperation.FileCloseOpen(LoadInfo(to_tchar(filename), nCharCode, bViewMode));

	// �v���O�C���FDocumentOpen�C�x���g���s
	Plug::Array plugs;
	WSHIfObj::List params;
	JackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_OPEN, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		(*it)->Invoke(&GetEditWindow()->GetActiveView(), params);
	}
}


// �t�@�C���̍ăI�[�v��
void ViewCommander::Command_FILE_REOPEN(
	EncodingType	nCharCode,	// [in] �J�������ۂ̕����R�[�h
	bool			bNoConfirm	// [in] �t�@�C�����X�V���ꂽ�ꍇ�Ɋm�F���s��u�Ȃ��v���ǂ����Btrue:�m�F���Ȃ� false:�m�F����
	)
{
	EditDoc* pDoc = GetDocument();
	if (!bNoConfirm && fexist(pDoc->m_docFile.GetFilePath()) && pDoc->m_docEditor.IsModified()) {
		int nDlgResult = MYMESSAGEBOX(
			m_pCommanderView->GetHwnd(),
			MB_OKCANCEL | MB_ICONQUESTION | MB_TOPMOST,
			GSTR_APPNAME,
			LS(STR_ERR_CEDITVIEW_CMD29),
			pDoc->m_docFile.GetFilePath()
		);
		if (nDlgResult == IDOK) {
			// �p���B���֐i��
		}else {
			return; // ���f
		}
	}

	// ����t�@�C���̍ăI�[�v��
	pDoc->m_docFileOperation.ReloadCurrentFile(nCharCode);
}


// ���
void ViewCommander::Command_PRINT(void)
{
	// �g���Ă��Ȃ��������폜 2003.05.04 �����
	Command_PRINT_PREVIEW();

	// ������s
	GetEditWindow()->m_pPrintPreview->OnPrint();
}


// ����v���r���[
void ViewCommander::Command_PRINT_PREVIEW(void)
{
	// ����v���r���[���[�h�̃I��/�I�t
	GetEditWindow()->PrintPreviewModeONOFF();
	return;
}


// ����̃y�[�W���C�A�E�g�̐ݒ�
void ViewCommander::Command_PRINT_PAGESETUP(void)
{
	// ����y�[�W�ݒ�
	GetEditWindow()->OnPrintPageSetting();
	return;
}


// From Here Feb. 10, 2001 JEPRO �ǉ�
// C/C++�w�b�_�t�@�C���܂��̓\�[�X�t�@�C�� �I�[�v���@�\
bool ViewCommander::Command_OPEN_HfromtoC(bool bCheckOnly)
{
	if (Command_OPEN_HHPP(bCheckOnly, false))	return true;
	if (Command_OPEN_CCPP(bCheckOnly, false))	return true;
	ErrorBeep();
	return false;
// 2002/03/24 YAZAKI �R�[�h�̏d�����팸
// 2003.06.28 Moca �R�����g�Ƃ��Ďc���Ă����R�[�h���폜
}


// C/C++�w�b�_�t�@�C�� �I�[�v���@�\		// Feb. 10, 2001 jepro	�������u�C���N���[�h�t�@�C���v����ύX
//BOOL ViewCommander::Command_OPENINCLUDEFILE(bool bCheckOnly)
bool ViewCommander::Command_OPEN_HHPP(bool bCheckOnly, bool bBeepWhenMiss)
{
	// 2003.06.28 Moca �w�b�_�E�\�[�X�̃R�[�h�𓝍����폜
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return m_pCommanderView->OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, source_ext, header_ext,
		_countof(source_ext), _countof(header_ext),
		LS(STR_ERR_CEDITVIEW_CMD08));
}


// C/C++�\�[�X�t�@�C�� �I�[�v���@�\
//BOOL ViewCommander::Command_OPENCCPP(bool bCheckOnly)	//Feb. 10, 2001 JEPRO	�R�}���h�����኱�ύX
bool ViewCommander::Command_OPEN_CCPP(bool bCheckOnly, bool bBeepWhenMiss)
{
	// 2003.06.28 Moca �w�b�_�E�\�[�X�̃R�[�h�𓝍����폜
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return m_pCommanderView->OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, header_ext, source_ext,
		_countof(header_ext), _countof(source_ext),
		LS(STR_ERR_CEDITVIEW_CMD09));
}


// Oracle SQL*Plus���A�N�e�B�u�\��
void ViewCommander::Command_ACTIVATE_SQLPLUS(void)
{
	HWND hwndSQLPLUS = ::FindWindow(_T("SqlplusWClass"), _T("Oracle SQL*Plus"));
	if (!hwndSQLPLUS) {
		ErrorMessage(m_pCommanderView->GetHwnd(), LS(STR_SQLERR_ACTV_BUT_NOT_RUN));	// "Oracle SQL*Plus���A�N�e�B�u�\�����܂��B\n\n\nOracle SQL*Plus���N������Ă��܂���B\n"
		return;
	}
	// Oracle SQL*Plus���A�N�e�B�u�ɂ���
	// �A�N�e�B�u�ɂ���
	ActivateFrameWindow(hwndSQLPLUS);
	return;
}


// Oracle SQL*Plus�Ŏ��s
void ViewCommander::Command_PLSQL_COMPILE_ON_SQLPLUS(void)
{
//	HGLOBAL		hgClip;
//	char*		pszClip;
	int			nRet;
	BOOL		nBool;
	TCHAR		szPath[MAX_PATH + 2];
	BOOL		bResult;

	HWND hwndSQLPLUS = ::FindWindow(_T("SqlplusWClass"), _T("Oracle SQL*Plus"));
	if (!hwndSQLPLUS) {
		ErrorMessage(m_pCommanderView->GetHwnd(), LS(STR_SQLERR_EXEC_BUT_NOT_RUN));	// "Oracle SQL*Plus�Ŏ��s���܂��B\n\n\nOracle SQL*Plus���N������Ă��܂���B\n"
		return;
	}
	// �e�L�X�g���ύX����Ă���ꍇ
	if (GetDocument()->m_docEditor.IsModified()) {
		nRet = ::MYMESSAGEBOX(
			m_pCommanderView->GetHwnd(),
			MB_YESNOCANCEL | MB_ICONEXCLAMATION,
			GSTR_APPNAME,
			LS(STR_ERR_CEDITVIEW_CMD18),
			GetDocument()->m_docFile.GetFilePathClass().IsValidPath() ? GetDocument()->m_docFile.GetFilePath() : LS(STR_NO_TITLE1)
		);
		switch (nRet) {
		case IDYES:
			if (GetDocument()->m_docFile.GetFilePathClass().IsValidPath()) {
				//nBool = HandleCommand(F_FILESAVE, true, 0, 0, 0, 0);
				nBool = Command_FILESAVE();
			}else {
				//nBool = HandleCommand(F_FILESAVEAS_DIALOG, true, 0, 0, 0, 0);
				nBool = Command_FILESAVEAS_DIALOG(NULL, CODE_NONE, EolType::None);
			}
			if (!nBool) {
				return;
			}
			break;
		case IDNO:
			return;
		case IDCANCEL:
		default:
			return;
		}
	}
	if (GetDocument()->m_docFile.GetFilePathClass().IsValidPath()) {
		// �t�@�C���p�X�ɋ󔒂��܂܂�Ă���ꍇ�̓_�u���N�H�[�e�[�V�����ň͂�
		// 2003.10.20 MIK �R�[�h�ȗ���
		if (_tcschr(GetDocument()->m_docFile.GetFilePath(), TCODE::SPACE) ? TRUE : FALSE) {
			auto_sprintf( szPath, _T("@\"%ts\"\r\n"), GetDocument()->m_docFile.GetFilePath() );
		}else {
			auto_sprintf( szPath, _T("@%ts\r\n"), GetDocument()->m_docFile.GetFilePath() );
		}
		// �N���b�v�{�[�h�Ƀf�[�^��ݒ�
		m_pCommanderView->MySetClipboardData(szPath, _tcslen(szPath), false);

		// Oracle SQL*Plus���A�N�e�B�u�ɂ���
		// �A�N�e�B�u�ɂ���
		ActivateFrameWindow(hwndSQLPLUS);

		// Oracle SQL*Plus�Ƀy�[�X�g�̃R�}���h�𑗂�
		DWORD_PTR	dwResult;
		bResult = ::SendMessageTimeout(
			hwndSQLPLUS,
			WM_COMMAND,
			MAKELONG(201, 0),
			0,
			SMTO_ABORTIFHUNG | SMTO_NORMAL,
			3000,
			&dwResult
		);
		if (!bResult) {
			TopErrorMessage(m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD20));
		}
	}else {
		ErrorBeep();
		ErrorMessage(m_pCommanderView->GetHwnd(), LS(STR_ERR_CEDITVIEW_CMD21));
		return;
	}
	return;
}


// �u���E�Y
void ViewCommander::Command_BROWSE(void)
{
	if (!GetDocument()->m_docFile.GetFilePathClass().IsValidPath()) {
		ErrorBeep();
		return;
	}
//	char	szURL[MAX_PATH + 64];
//	auto_sprintf( szURL, L"%ls", GetDocument()->m_docFile.GetFilePath() );
	// URL���J��
//	::ShellExecuteEx(NULL, L"open", szURL, NULL, NULL, SW_SHOW);

    SHELLEXECUTEINFO info; 
    info.cbSize = sizeof(info);
    info.fMask = 0;
    info.hwnd = NULL;
    info.lpVerb = NULL;
    info.lpFile = GetDocument()->m_docFile.GetFilePath();
    info.lpParameters = NULL;
    info.lpDirectory = NULL;
    info.nShow = SW_SHOWNORMAL;
    info.hInstApp = 0;
    info.lpIDList = NULL;
    info.lpClass = NULL;
    info.hkeyClass = 0; 
    info.dwHotKey = 0;
    info.hIcon =0;

	::ShellExecuteEx(&info);

	return;
}


// �r���[���[�h
void ViewCommander::Command_VIEWMODE(void)
{
	// �r���[���[�h�𔽓]
	AppMode::getInstance()->SetViewMode(!AppMode::getInstance()->IsViewMode());

	// �r������̐؂�ւ�
	// ���r���[���[�h ON ���͔r������ OFF�A�r���[���[�h OFF ���͔r������ ON �̎d�l�i>>data:5262�j�𑦎����f����
	GetDocument()->m_docFileOperation.DoFileUnlock();	// �t�@�C���̔r�����b�N����
	GetDocument()->m_docLocker.CheckWritable(!AppMode::getInstance()->IsViewMode());	// �t�@�C�������\�̃`�F�b�N
	if (GetDocument()->m_docLocker.IsDocWritable()) {
		GetDocument()->m_docFileOperation.DoFileLock();	// �t�@�C���̔r�����b�N
	}

	// �e�E�B���h�E�̃^�C�g�����X�V
	GetEditWindow()->UpdateCaption();
}


// �t�@�C���̃v���p�e�B
void ViewCommander::Command_PROPERTY_FILE(void)
{
#ifdef _DEBUG
	{
		// �S�s�f�[�^��Ԃ��e�X�g
		wchar_t*	pDataAll;
		int		nDataAllLen;
		RunningTimer runningTimer("ViewCommander::Command_PROPERTY_FILE �S�s�f�[�^��Ԃ��e�X�g");
		runningTimer.Reset();
		pDataAll = DocReader(GetDocument()->m_docLineMgr).GetAllData(&nDataAllLen);
//		MYTRACE(_T("�S�f�[�^�擾             (%d�o�C�g) ���v����(�~���b) = %d\n"), nDataAllLen, runningTimer.Read());
		free(pDataAll);
		pDataAll = NULL;
//		MYTRACE(_T("�S�f�[�^�擾�̃������J�� (%d�o�C�g) ���v����(�~���b) = %d\n"), nDataAllLen, runningTimer.Read());
	}
#endif


	DlgProperty	cDlgProperty;
//	cDlgProperty.Create(G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument());
	cDlgProperty.DoModal(G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument());
	return;
}


void ViewCommander::Command_PROFILEMGR( void )
{
	DlgProfileMgr profMgr;
	if (profMgr.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), 0 )) {
		TCHAR szOpt[MAX_PATH+10];
		auto_sprintf( szOpt, _T("-PROF=\"%ts\""), profMgr.m_strProfileName.c_str() );
		LoadInfo loadInfo;
		loadInfo.filePath = _T("");
		loadInfo.eCharCode = CODE_DEFAULT;
		loadInfo.bViewMode = false;
		ControlTray::OpenNewEditor(
			G_AppInstance(),
			m_pCommanderView->GetHwnd(),
			loadInfo,
			szOpt,
			false,
			NULL,
			false
		);
	}
}

// �ҏW�̑S�I��	// 2007.02.13 ryoji �ǉ�
void ViewCommander::Command_EXITALLEDITORS(void)
{
	ControlTray::CloseAllEditor(TRUE, GetMainWindow(), TRUE, 0);
	return;
}


// �T�N���G�f�B�^�̑S�I��	// Dec. 27, 2000 JEPRO �ǉ�
void ViewCommander::Command_EXITALL(void)
{
	ControlTray::TerminateApplication(GetMainWindow());	// 2006.12.25 ryoji �����ǉ�
	return;
}


/*!	@brief �ҏW���̓��e��ʖ��ۑ�

	��ɕҏW���̈ꎞ�t�@�C���o�͂Ȃǂ̖ړI�Ɏg�p����D
	���݊J���Ă���t�@�C��(szFilePath)�ɂ͉e�����Ȃ��D

	@retval	TRUE ����I��
	@retval	FALSE �t�@�C���쐬�Ɏ��s

	@author	maru
	@date	2006.12.10 maru �V�K�쐬
*/
bool ViewCommander::Command_PUTFILE(
	LPCWSTR			filename,	// [in] filename �o�̓t�@�C����
	EncodingType	nCharCode,	// [in] nCharCode �����R�[�h�w��
								//  @li CODE_xxxxxxxxxx:�e�핶���R�[�h
								//  @li CODE_AUTODETECT:���݂̕����R�[�h���ێ�
	int				nFlgOpt		// [in] nFlgOpt ����I�v�V����
								//  @li 0x01:�I��͈͂��o�� (��I����Ԃł���t�@�C�����o�͂���)
)
{
	bool bResult = true;
	EncodingType nSaveCharCode = nCharCode;
	if (filename[0] == L'\0') {
		return false;
	}

	if (nSaveCharCode == CODE_AUTODETECT) {
		nSaveCharCode = GetDocument()->GetDocumentEncoding();
	}

	// 2007.09.08 genta EditDoc::FileWrite()�ɂȂ���č����v�J�[�\��
	WaitCursor waitCursor(m_pCommanderView->GetHwnd());

	std::unique_ptr<CodeBase> pcSaveCode(CodeFactory::CreateCodeBase(nSaveCharCode, 0));

	bool bBom = false;
	if (CodeTypeName(nSaveCharCode).UseBom()) {
		bBom = GetDocument()->GetDocumentBomExist();
	}

	if (nFlgOpt & 0x01) {	// �I��͈͂��o��
		try {
			BinaryOutputStream out(to_tchar(filename), true);

			// �I��͈͂̎擾 -> mem
			NativeW mem;
			m_pCommanderView->GetSelectedDataSimple(mem);

			// BOM�ǉ�
			NativeW mem2;
			const NativeW* pConvBuffer;
			if (bBom) {
				NativeW memBom;
				std::unique_ptr<CodeBase> pcUtf16(CodeFactory::CreateCodeBase(CODE_UNICODE, 0));
				pcUtf16->GetBom(memBom._GetMemory());
				mem2.AppendNativeData(memBom);
				mem2.AppendNativeData(mem);
				mem.Clear();
				pConvBuffer = &mem2;
			}else {
				pConvBuffer = &mem;
			}

			// �������ݎ��̃R�[�h�ϊ� -> dst
			Memory dst;
			pcSaveCode->UnicodeToCode(*pConvBuffer, &dst);

			// ����
			if (0 < dst.GetRawLength())
				out.Write(dst.GetRawPtr(), dst.GetRawLength());
		}catch (Error_FileOpen) {
			WarningMessage(
				NULL,
				LS(STR_SAVEAGENT_OTHER_APP),
				to_tchar(filename)
			);
			bResult = false;
		}catch (Error_FileWrite) {
			WarningMessage(
				NULL,
				LS(STR_ERR_DLGEDITVWCMDNW11)
			);
			bResult = false;
		}
	}else {	// �t�@�C���S�̂��o��
		HWND		hwndProgress;
		EditWnd*	pEditWnd = GetEditWindow();

		if (pEditWnd) {
			hwndProgress = pEditWnd->m_statusBar.GetProgressHwnd();
		}else {
			hwndProgress = NULL;
		}
		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_SHOW);
		}

		// �ꎞ�t�@�C���o��
		CodeConvertResult eRet = WriteManager().WriteFile_From_CDocLineMgr(
			GetDocument()->m_docLineMgr,
			SaveInfo(
				to_tchar(filename),
				nSaveCharCode,
				EolType::None,
				bBom
			)
		);
		bResult = (eRet != CodeConvertResult::Failure);
		if (hwndProgress) {
			::ShowWindow(hwndProgress, SW_HIDE);
		}
	}
	return bResult;
}


/*!	@brief �J�[�\���ʒu�Ƀt�@�C����}��

	���݂̃J�[�\���ʒu�Ɏw��̃t�@�C����ǂݍ��ށD

	@param[in] filename ���̓t�@�C����
	@param[in] nCharCode �����R�[�h�w��
		@li	CODE_xxxxxxxxxx:�e�핶���R�[�h
		@li	CODE_AUTODETECT:�O�񕶎��R�[�h�������͎������ʂ̌��ʂɂ��
	@param[in] nFlgOpt ����I�v�V�����i���݂͖���`�D0���w��̂��Ɓj

	@retval	TRUE ����I��
	@retval	FALSE �t�@�C���I�[�v���Ɏ��s

	@author	maru
	@date	2006.12.10 maru �V�K�쐬
*/
bool ViewCommander::Command_INSFILE(
	LPCWSTR filename,
	EncodingType nCharCode,
	int nFlgOpt
	)
{
	FileLoad	fl(m_pCommanderView->m_pTypeData->encoding);
	Eol			eol;
	int			nLineNum = 0;

	DlgCancel*	pDlgCancel = NULL;
	HWND		hwndCancel = NULL;
	HWND		hwndProgress = NULL;
	int			nOldPercent = -1;
	bool		bResult = true;

	if (filename[0] == L'\0') {
		return false;
	}

	// 2007.09.08 genta EditDoc::FileLoad()�ɂȂ���č����v�J�[�\��
	WaitCursor waitCursor(m_pCommanderView->GetHwnd());

	// �͈͑I�𒆂Ȃ�}������I����Ԃɂ��邽��	// 2007.04.29 maru
	bool bBeforeTextSelected = m_pCommanderView->GetSelectionInfo().IsTextSelected();
	LayoutPoint ptFrom;
	if (bBeforeTextSelected) {
		ptFrom = m_pCommanderView->GetSelectionInfo().m_select.GetFrom();
	}


	EncodingType	nSaveCharCode = nCharCode;
	if (nSaveCharCode == CODE_AUTODETECT) {
		EditInfo    fi;
		const MruFile  mru;
		if (mru.GetEditInfo(to_tchar(filename), &fi)) {
				nSaveCharCode = fi.nCharCode;
		}else {
			nSaveCharCode = GetDocument()->GetDocumentEncoding();
		}
	}

	// �����܂ł��ĕ����R�[�h�����肵�Ȃ��Ȃ�ǂ�����������
	if (!IsValidCodeType(nSaveCharCode)) nSaveCharCode = CODE_SJIS;

	try {
		bool bBigFile;
#ifdef _WIN64
		bBigFile = true;
#else
		bBigFile = false;
#endif
		// �t�@�C�����J��
		fl.FileOpen( to_tchar(filename), bBigFile, nSaveCharCode, 0 );

		// �t�@�C���T�C�Y��65KB���z������i���_�C�A���O�\��
		if (0x10000 < fl.GetFileSize()) {
			pDlgCancel = new DlgCancel;
			if ((hwndCancel = pDlgCancel->DoModeless(::GetModuleHandle(NULL), NULL, IDD_OPERATIONRUNNING))) {
				hwndProgress = ::GetDlgItem(hwndCancel, IDC_PROGRESS);
				Progress_SetRange(hwndProgress, 0, 101);
				Progress_SetPos(hwndProgress, 0);
			}
		}

		// ReadLine�̓t�@�C������ �����R�[�h�ϊ����ꂽ1�s��ǂݏo���܂�
		// �G���[����throw Error_FileRead �𓊂��܂�
		NativeW buf;
		while (CodeConvertResult::Failure != fl.ReadLine(&buf, &eol)) {

			const wchar_t*	pLine = buf.GetStringPtr();
			int			nLineLen = buf.GetStringLength();

			++nLineNum;
			Command_INSTEXT(false, pLine, LogicInt(nLineLen), true);

			// �i���_�C�A���O�L��
			if (!pDlgCancel) {
				continue;
			}
			// �������̃��[�U�[������\�ɂ���
			if (!::BlockingHook(pDlgCancel->GetHwnd())) {
				break;
			}
			// ���f�{�^�������`�F�b�N
			if (pDlgCancel->IsCanceled()) {
				break;
			}
			if ((nLineNum & 0xFF) == 0) {
				if (nOldPercent != fl.GetPercent()) {
					Progress_SetPos(hwndProgress, fl.GetPercent() + 1);
					Progress_SetPos(hwndProgress, fl.GetPercent());
					nOldPercent = fl.GetPercent();
				}
				m_pCommanderView->Redraw();
			}
		}
		// �t�@�C���𖾎��I�ɕ��邪�A�����ŕ��Ȃ��Ƃ��̓f�X�g���N�^�ŕ��Ă���
		fl.FileClose();
	}catch (Error_FileOpen) {
		WarningMessage(NULL, LS(STR_GREP_ERR_FILEOPEN), to_tchar(filename));
		bResult = false;
	}catch (Error_FileRead) {
		WarningMessage(NULL, LS(STR_ERR_DLGEDITVWCMDNW12));
		bResult = false;
	} // ��O�����I���

	delete pDlgCancel;

	if (bBeforeTextSelected) {	// �}�����ꂽ������I����Ԃ�
		m_pCommanderView->GetSelectionInfo().SetSelectArea(
			LayoutRange(
				ptFrom,
				GetCaret().GetCaretLayoutPos()
				/*
				m_nCaretPosY, m_nCaretPosX
				*/
			)
		);
		m_pCommanderView->GetSelectionInfo().DrawSelectArea();
	}
	m_pCommanderView->Redraw();
	return bResult;
}

