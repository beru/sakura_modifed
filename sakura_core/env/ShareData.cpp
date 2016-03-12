/*!	@file
	@brief �v���Z�X�ԋ��L�f�[�^�ւ̃A�N�Z�X

	@author Norio Nakatani
	@date 1998/05/26  �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta, MIK
	Copyright (C) 2001, jepro, genta, asa-o, MIK, YAZAKI, hor
	Copyright (C) 2002, genta, ai, Moca, MIK, YAZAKI, hor, KK, aroka
	Copyright (C) 2003, Moca, aroka, MIK, genta, wmlhq, sui
	Copyright (C) 2004, Moca, novice, genta, isearch, MIK
	Copyright (C) 2005, Moca, MIK, genta, ryoji, ���, aroka
	Copyright (C) 2006, aroka, ryoji, genta
	Copyright (C) 2007, ryoji, genta, maru
	Copyright (C) 2008, ryoji, Uchi, nasukoji
	Copyright (C) 2009, nasukoji, ryoji
	Copyright (C) 2011, nasukoji
	Copyright (C) 2012, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "env/ShareData_IO.h"
#include "env/SakuraEnvironment.h"
#include "doc/DocListener.h" // LoadInfo
#include "_main/ControlTray.h"
#include "_main/CommandLine.h"
#include "_main/Mutex.h"
#include "charset/CodePage.h"
#include "debug/RunningTimer.h"
#include "recent/MRUFile.h"
#include "recent/MRUFolder.h"
#include "util/module.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "util/os.h"
#include "DataProfile.h"
#include "sakura_rc.h"

struct ARRHEAD {
	int nLength;
	int nItemNum;
};

const unsigned int uShareDataVersion = N_SHAREDATA_VERSION;

// CShareData_new2.cpp�Ɠ���
//@@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�ShareData����CMenuDrawer�ֈړ�
ShareData::ShareData()
{
	m_hFileMap   = NULL;
	m_pShareData = NULL;
	m_pvTypeSettings = NULL;
}

/*!
	���L�������̈悪����ꍇ�̓v���Z�X�̃A�h���X��Ԃ���
	���łɃ}�b�v����Ă���t�@�C�� �r���[���A���}�b�v����B
*/
ShareData::~ShareData()
{
	if (m_pShareData) {
		// �v���Z�X�̃A�h���X��Ԃ��� ���łɃ}�b�v����Ă���t�@�C�� �r���[���A���}�b�v���܂�
		SetDllShareData(NULL);
		::UnmapViewOfFile(m_pShareData);
		m_pShareData = NULL;
	}
	if (m_hFileMap) {
		CloseHandle(m_hFileMap);
	}
	if (m_pvTypeSettings) {
		for (int i=0; i<(int)m_pvTypeSettings->size(); ++i) {
			delete (*m_pvTypeSettings)[i];
			(*m_pvTypeSettings)[i] = NULL;
		}
		delete m_pvTypeSettings;
		m_pvTypeSettings = NULL;
	}
}


static Mutex g_cMutexShareWork( FALSE, GSTR_MUTEX_SAKURA_SHAREWORK );
 
Mutex& ShareData::GetMutexShareWork(){
	return g_cMutexShareWork;
}

//! ShareData�N���X�̏���������
/*!
	ShareData�N���X�𗘗p����O�ɕK���Ăяo�����ƁB

	@retval true ����������
	@retval false ���������s

	@note ���ɑ��݂��鋤�L�������̃o�[�W���������̃G�f�B�^���g�����̂�
	�قȂ�ꍇ�͒v���I�G���[��h�����߂�false��Ԃ��܂��BProcess::Initialize()
	��Init()�Ɏ��s����ƃ��b�Z�[�W���o���ăG�f�B�^�̋N���𒆎~���܂��B
*/
bool ShareData::InitShareData()
{
	MY_RUNNINGTIMER(runningTimer, "ShareData::InitShareData");

	m_hwndTraceOutSource = NULL;	// 2006.06.26 ryoji

	// �t�@�C���}�b�s���O�I�u�W�F�N�g
	{
		std::tstring strProfileName = to_tchar(CommandLine::getInstance()->GetProfileName());
		std::tstring strShareDataName = GSTR_SHAREDATA;
		strShareDataName += strProfileName;
		m_hFileMap = ::CreateFileMapping(
			INVALID_HANDLE_VALUE,	// Sep. 6, 2003 wmlhq
			NULL,
			PAGE_READWRITE | SEC_COMMIT,
			0,
			sizeof(DllSharedData),
			strShareDataName.c_str()
		);
	}
	if (!m_hFileMap) {
		::MessageBox(
			NULL,
			_T("CreateFileMapping()�Ɏ��s���܂���"),
			_T("�\�����ʃG���["),
			MB_OK | MB_APPLMODAL | MB_ICONSTOP
		);
		return false;
	}

	if (GetLastError() != ERROR_ALREADY_EXISTS) {
		// �I�u�W�F�N�g�����݂��Ă��Ȃ������ꍇ
		// �t�@�C���̃r���[�� �Ăяo�����v���Z�X�̃A�h���X��ԂɃ}�b�v���܂�
		m_pShareData = (DllSharedData*)::MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);
		CreateTypeSettings();
		SetDllShareData(m_pShareData);

		// 2007.05.19 ryoji ���s�t�@�C���t�H���_->�ݒ�t�@�C���t�H���_�ɕύX
		TCHAR szIniFolder[_MAX_PATH];
		m_pShareData->fileNameManagement.iniFolder.bInit = false;
		GetInidir(szIniFolder);
		AddLastChar(szIniFolder, _MAX_PATH, _T('\\'));

		m_pShareData->vStructureVersion = uShareDataVersion;
		m_pShareData->nSize = sizeof(*m_pShareData);

		// 2004.05.13 Moca ���\�[�X���琻�i�o�[�W�����̎擾
		GetAppVersionInfo(NULL, VS_VERSION_INFO,
			&m_pShareData->version.dwProductVersionMS, &m_pShareData->version.dwProductVersionLS);

		m_pShareData->flags.bEditWndChanging = FALSE;		// �ҏW�E�B���h�E�ؑ֒�	// 2007.04.03 ryoji
		m_pShareData->flags.bRecordingKeyMacro = FALSE;	// �L�[�{�[�h�}�N���̋L�^��
		m_pShareData->flags.hwndRecordingKeyMacro = NULL;	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E

		m_pShareData->nodes.nSequences = 0;				// �E�B���h�E�A��
		m_pShareData->nodes.nNonameSequences = 0;
		m_pShareData->nodes.nGroupSequences = 0;			// �^�u�O���[�v�A��		// 2007.06.20 ryoji
		m_pShareData->nodes.nEditArrNum = 0;

		m_pShareData->handles.hwndTray = NULL;
		m_pShareData->handles.hAccel = NULL;
		m_pShareData->handles.hwndDebug = NULL;

		for (int i=0; i<_countof(m_pShareData->dwCustColors); ++i) {
			m_pShareData->dwCustColors[i] = RGB( 255, 255, 255 );
		}

//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
		MruFile mru;
		mru.ClearAll();
//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́AMruFolder�ɂ��ׂĈ˗�����
		MruFolder mruFolder;
		mruFolder.ClearAll();

// From Here Sept. 19, 2000 JEPRO �R�����g�A�E�g�ɂȂ��Ă������߂̃u���b�N�𕜊������̉����R�����g�A�E�g
// MS �S�V�b�N�W���X�^�C��10pt�ɐݒ�
//		// LOGFONT�̏�����
		LOGFONT lf = {0};
		lf.lfHeight			= DpiPointsToPixels(-10);	// 2009.10.01 ryoji ��DPI�Ή��i�|�C���g������Z�o�j
		lf.lfWidth			= 0;
		lf.lfEscapement		= 0;
		lf.lfOrientation	= 0;
		lf.lfWeight			= 400;
		lf.lfItalic			= 0x0;
		lf.lfUnderline		= 0x0;
		lf.lfStrikeOut		= 0x0;
		lf.lfCharSet		= 0x80;
		lf.lfOutPrecision	= 0x3;
		lf.lfClipPrecision	= 0x2;
		lf.lfQuality		= 0x1;
		lf.lfPitchAndFamily	= 0x31;
		_tcscpy(lf.lfFaceName, _T("�l�r �S�V�b�N"));

		// LoadShareData�Ńt�H���g���ς��\��������̂ŁA�����ł͕s�v // 2013.04.08 aroka
		//InitCharWidthCacheCommon();								// 2008/5/17 Uchi

		// �L�[���[�h�w���v�̃t�H���g ai 02/05/21 Add S
		LOGFONT lfIconTitle;	// �G�N�X�v���[���̃t�@�C�����\���Ɏg�p�����t�H���g
		::SystemParametersInfo(
			SPI_GETICONTITLELOGFONT,				// system parameter to query or set
			sizeof(LOGFONT),						// depends on action to be taken
			(PVOID)&lfIconTitle,					// depends on action to be taken
			0										// user profile update flag
		);
		// ai 02/05/21 Add E
		INT nIconPointSize = lfIconTitle.lfHeight >= 0 ? lfIconTitle.lfHeight : DpiPixelsToPoints(-lfIconTitle.lfHeight, 10);	// �t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj
// To Here Sept. 19,2000

		// [�S��]�^�u
		{
			CommonSetting_General& general = m_pShareData->common.general;

			general.nMRUArrNum_MAX = 15;			// �t�@�C���̗���MAX	//Oct. 14, 2000 JEPRO �������₵��(10��15)
			general.nOPENFOLDERArrNum_MAX = 15;		// �t�H���_�̗���MAX	//Oct. 14, 2000 JEPRO �������₵��(10��15)

			general.nCaretType = 0;					// �J�[�\���̃^�C�v 0=win 1=dos
			general.bIsINSMode = true;				// �}���^�㏑�����[�h
			general.bIsFreeCursorMode = false;		// �t���[�J�[�\�����[�h��	//Oct. 29, 2000 JEPRO �u�Ȃ��v�ɕύX

			general.bStopsBothEndsWhenSearchWord = false;	// �P��P�ʂňړ�����Ƃ��ɁA�P��̗��[�Ŏ~�܂邩
			general.bStopsBothEndsWhenSearchParagraph = false;	// �P��P�ʂňړ�����Ƃ��ɁA�P��̗��[�Ŏ~�܂邩

			general.bCloseAllConfirm = false;		// [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F����		// 2006.12.25 ryoji
			general.bExitConfirm = false;			// �I�����̊m�F������
			general.nRepeatedScrollLineNum = LayoutInt(3);	// �L�[���s�[�g���̃X�N���[���s��
			general.nRepeatedScroll_Smooth = false;	// �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩
			general.nPageScrollByWheel = 0;			// �L�[/�}�E�X�{�^�� + �z�C�[���X�N���[���Ńy�[�W�X�N���[������	// 2009.01.17 nasukoji
			general.nHorizontalScrollByWheel = 0;	// �L�[/�}�E�X�{�^�� + �z�C�[���X�N���[���ŉ��X�N���[������		// 2009.01.17 nasukoji

			general.bUseTaskTray = true;			// �^�X�N�g���C�̃A�C�R�����g��
#ifdef _DEBUG
			general.bStayTaskTray = false;			// �^�X�N�g���C�̃A�C�R�����풓
#else
			general.bStayTaskTray = true;			// �^�X�N�g���C�̃A�C�R�����풓
#endif
			general.wTrayMenuHotKeyCode = L'Z';		// �^�X�N�g���C���N���b�N���j���[ �L�[
			general.wTrayMenuHotKeyMods = HOTKEYF_ALT | HOTKEYF_CONTROL;	// �^�X�N�g���C���N���b�N���j���[ �L�[

			general.bDispExitingDialog = false;		// �I���_�C�A���O��\������

			general.bNoCaretMoveByActivation = false;	// �}�E�X�N���b�N�ɂăA�N�e�B�x�[�g���ꂽ���̓J�[�\���ʒu���ړ����Ȃ� 2007.10.02 nasukoji (add by genta)
		}

		// [�E�B���h�E]�^�u
		{
			CommonSetting_Window& window = m_pShareData->common.window;

			window.bDispToolBar = true;				// ����E�B���h�E���J�����Ƃ��c�[���o�[��\������
			window.bDispStatusBar = true;			// ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������
			window.bDispFuncKeyWnd = false;			// ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������
			window.bDispMiniMap = false;			// �~�j�}�b�v��\������
			window.nFuncKeyWnd_Place = 1;			// �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
			window.nFuncKeyWnd_GroupNum = 4;		// 2002/11/04 Moca �t�@���N�V�����L�[�̃O���[�v�{�^����
			window.nMiniMapFontSize = -1;
			window.nMiniMapQuality = NONANTIALIASED_QUALITY;
			window.nMiniMapWidth = 150;

			window.bSplitterWndHScroll = true;	// 2001/06/20 asa-o �����E�B���h�E�̐����X�N���[���̓������Ƃ�
			window.bSplitterWndVScroll = true;	// 2001/06/20 asa-o �����E�B���h�E�̐����X�N���[���̓������Ƃ�

			// 2001/06/14 asa-o �⊮�ƃL�[���[�h�w���v�̓^�C�v�ʂɈړ������̂ō폜
			//	2004.05.13 Moca �E�B���h�E�T�C�Y�Œ�w��ǉ��ɔ����w����@�ύX
			window.eSaveWindowSize = WinSizeMode::Save;	// �E�B���h�E�T�C�Y�p��
			window.nWinSizeType = SIZE_RESTORED;
			window.nWinSizeCX = CW_USEDEFAULT;
			window.nWinSizeCY = 0;

			window.bScrollBarHorz = true;					// �����X�N���[���o�[���g��
			//	2004.05.13 Moca �E�B���h�E�ʒu
			window.eSaveWindowPos = WinSizeMode::Default;	// �E�B���h�E�ʒu�Œ�E�p��
			window.nWinPosX = CW_USEDEFAULT;
			window.nWinPosY = 0;

			window.nRulerHeight = 13;						// ���[���[�̍���
			window.nRulerBottomSpace = 0;					// ���[���[�ƃe�L�X�g�̌���
			window.nRulerType = 0;							// ���[���[�̃^�C�v
			window.nLineNumRightSpace = 0;					// �s�ԍ��̉E�̌���
			window.nVertLineOffset = -1;					// 2005.11.10 Moca �w�茅�c��
			window.bUseCompatibleBMP = true;				// 2007.09.09 Moca ��ʃL���b�V�����g��	// 2009.06.09 ryoji FALSE->TRUE

			window.bMenuIcon = true;						// ���j���[�ɃA�C�R����\������

			//	Apr. 05, 2003 genta �E�B���h�E�L���v�V�����̏����l
			//	Aug. 16, 2003 genta $N(�t�@�C�����ȗ��\��)���f�t�H���g�ɕύX
			_tcscpy( window.szWindowCaptionActive, 
				_T("${w?$h$:�A�E�g�v�b�g$:${I?$f$n$:$N$n$}$}${U?(�X�V)$} -")
				_T(" $A $V ${R?(�r���[���[�h)$:(�㏑���֎~)$}${M?  �y�L�[�}�N���̋L�^���z$} $<profile>") );
			_tcscpy( window.szWindowCaptionInactive, 
				_T("${w?$h$:�A�E�g�v�b�g$:$f$n$}${U?(�X�V)$} -")
				_T(" $A $V ${R?(�r���[���[�h)$:(�㏑���֎~)$}${M?  �y�L�[�}�N���̋L�^���z$} $<profile>") );
		}
		
		// [�^�u�o�[]�^�u
		{
			CommonSetting_TabBar& tabBar = m_pShareData->common.tabBar;

			tabBar.bDispTabWnd = false;				// �^�u�E�B���h�E�\��	//@@@ 2003.05.31 MIK
			tabBar.bDispTabWndMultiWin = false;		// �^�u�E�B���h�E�\��	//@@@ 2003.05.31 MIK
			wcscpy(	//@@@ 2003.06.13 MIK
				tabBar.szTabWndCaption,
				L"${w?�yGrep�z$h$:�y�A�E�g�v�b�g�z$:$f$n$}${U?(�X�V)$}${R?(�r���[���[�h)$:(�㏑���֎~)$}${M?�y�L�[�}�N���̋L�^���z$}"
			);
			tabBar.bSameTabWidth = false;			// �^�u�𓙕��ɂ���			//@@@ 2006.01.28 ryoji
			tabBar.bDispTabIcon = false;			// �^�u�ɃA�C�R����\������	//@@@ 2006.01.28 ryoji
			tabBar.dispTabClose = DispTabCloseType::No;	// �^�u�ɕ���{�^����\������	//@@@ 2012.04.14 syat
			tabBar.bSortTabList = true;				// �^�u�ꗗ���\�[�g����		//@@@ 2006.05.10 ryoji
			tabBar.bTab_RetainEmptyWin = true;		// �Ō�̃t�@�C��������ꂽ�Ƃ�(����)���c��	// 2007.02.11 genta
			tabBar.bTab_CloseOneWin = false;		// �^�u���[�h�ł��E�B���h�E�̕���{�^���Ō��݂̃t�@�C���̂ݕ���	// 2007.02.11 genta
			tabBar.bTab_ListFull = false;			// �^�u�ꗗ���t���p�X�\������	//@@@ 2007.02.28 ryoji
			tabBar.bChgWndByWheel = false;			// �}�E�X�z�C�[���ŃE�B���h�E�ؑ�	//@@@ 2006.03.26 ryoji
			tabBar.bNewWindow = false;				// �O������N������Ƃ��͐V�����E�B���h�E�ŊJ��
			tabBar.bTabMultiLine = false;			// �^�u���i
			tabBar.eTabPosition = TabPosition::Top;	// �^�u�ʒu

			tabBar.lf = lfIconTitle;
			tabBar.nPointSize = nIconPointSize;
			tabBar.nTabMaxWidth = 200;
			tabBar.nTabMinWidth = 60;
			tabBar.nTabMinWidthOnMulti = 100;
		}

		// [�ҏW]�^�u
		{
			CommonSetting_Edit& edit = m_pShareData->common.edit;

			edit.bAddCRLFWhenCopy = false;			// �܂�Ԃ��s�ɉ��s��t���ăR�s�[

			edit.bUseOLE_DragDrop = true;			// OLE�ɂ��h���b�O & �h���b�v���g��
			edit.bUseOLE_DropSource = true;			// OLE�ɂ��h���b�O���ɂ��邩
			edit.bSelectClickedURL = true;			// URL���N���b�N���ꂽ��I�����邩
			edit.bCopyAndDisablSelection = false;	// �R�s�[������I������
			edit.bEnableNoSelectCopy = true;		// �I���Ȃ��ŃR�s�[���\�ɂ���			// 2007.11.18 ryoji
			edit.bEnableLineModePaste = true;		// ���C�����[�h�\��t�����\�ɂ���		// 2007.10.08 ryoji
			edit.bConvertEOLPaste = false;			// ���s�R�[�h��ϊ����ē\��t���� 		// 2009.02.28 salarm
			edit.bEnableExtEol = false;
			edit.bBoxSelectLock = true;

			edit.bNotOverWriteCRLF = true;			// ���s�͏㏑�����Ȃ�
			edit.bOverWriteFixMode = false;			// �������ɍ��킹�ăX�y�[�X���l�߂�

			edit.bOverWriteBoxDelete = false;
			edit.eOpenDialogDir = OPENDIALOGDIR_CUR;
			auto_strcpy(edit.openDialogSelDir, _T("%Personal%\\"));
			edit.bAutoColumnPaste = true;			// ��`�R�s�[�̃e�L�X�g�͏�ɋ�`�\��t��
		}

		// [�t�@�C��]�^�u
		{
			CommonSetting_File& file = m_pShareData->common.file;

			// �t�@�C���̔r������
			file.nFileShareMode = FileShareMode::DenyWrite;	// �t�@�C���̔r�����䃂�[�h
			file.bCheckFileTimeStamp = true;			// �X�V�̊Ď�
			file.nAutoloadDelay = 0;					// �����Ǎ����x��
			file.bUneditableIfUnwritable = true;		// �㏑���֎~���o���͕ҏW�֎~�ɂ���

			// �t�@�C���̕ۑ�
			file.bEnableUnmodifiedOverwrite = false;	// ���ύX�ł��㏑�����邩

			//�u���O��t���ĕۑ��v�Ńt�@�C���̎�ނ�[���[�U�w��]�̂Ƃ��̃t�@�C���ꗗ�\��	// �t�@�C���ۑ��_�C�A���O�̃t�B���^�ݒ�	// 2006.11.16 ryoji
			file.bNoFilterSaveNew = true;		// �V�K����ۑ����͑S�t�@�C���\��
			file.bNoFilterSaveFile = true;		// �V�K�ȊO����ۑ����͑S�t�@�C���\��

			// �t�@�C���I�[�v��
			file.bDropFileAndClose = false;		// �t�@�C�����h���b�v�����Ƃ��͕��ĊJ��
			file.nDropFileNumMax = 8;			// ��x�Ƀh���b�v�\�ȃt�@�C����
			file.bRestoreCurPosition = true;	// �J�[�\���ʒu����	//	Oct. 27, 2000 genta
			file.bRestoreBookmarks = true;		// �u�b�N�}�[�N����	//2002.01.16 hor
			file.bAutoMimeDecode = false;		// �t�@�C���ǂݍ��ݎ���MIME�̃f�R�[�h���s����	//Jul. 13, 2001 JEPRO
			file.bQueryIfCodeChange = true;		// �O��ƈقȂ镶���R�[�h�̎��ɖ₢���킹���s����	Oct. 03, 2004 genta
			file.bAlertIfFileNotExist = false;	// �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������	Oct. 09, 2004 genta
			file.bAlertIfLargeFile = false;		// �J�����Ƃ����t�@�C�����傫���ꍇ�Ɍx������
			file.nAlertFileSize = 10;			// �x�����n�߂�t�@�C���T�C�Y�iMB�P�ʁj
		}

		// [�o�b�N�A�b�v]�^�u
		{
			CommonSetting_Backup& backup = m_pShareData->common.backup;

			backup.bBackUp = false;										// �o�b�N�A�b�v�̍쐬
			backup.bBackUpDialog = true;									// �o�b�N�A�b�v�̍쐬�O�Ɋm�F
			backup.bBackUpFolder = false;								// �w��t�H���_�Ƀo�b�N�A�b�v���쐬����
			backup.szBackUpFolder[0] = L'\0';							// �o�b�N�A�b�v���쐬����t�H���_
			backup.nBackUpType = 2;										// �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.*
			backup.nBackUpType_Opt1 = BKUP_YEAR | BKUP_MONTH | BKUP_DAY;	// �o�b�N�A�b�v�t�@�C�����F���t
			backup.nBackUpType_Opt2 = ('b' << 16 ) + 10;					// �o�b�N�A�b�v�t�@�C�����F�A�Ԃ̐��Ɛ擪����
			backup.nBackUpType_Opt3 = 5;									// �o�b�N�A�b�v�t�@�C�����FOption3
			backup.nBackUpType_Opt4 = 0;									// �o�b�N�A�b�v�t�@�C�����FOption4
			backup.nBackUpType_Opt5 = 0;									// �o�b�N�A�b�v�t�@�C�����FOption5
			backup.nBackUpType_Opt6 = 0;									// �o�b�N�A�b�v�t�@�C�����FOption6
			backup.bBackUpDustBox = false;								// �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞��	//@@@ 2001.12.11 add MIK
			backup.bBackUpPathAdvanced = false;							// 20051107 aroka �o�b�N�A�b�v��t�H���_���ڍאݒ肷��
			backup.szBackUpPathAdvanced[0] = _T('\0');					// 20051107 aroka �o�b�N�A�b�v���쐬����t�H���_�̏ڍאݒ�
		}

		// [����]�^�u
		{
			CommonSetting_Format& format = m_pShareData->common.format;

			// ���o���L��
			wcscpy( format.szMidashiKigou, L"�P�Q�R�S�T�U�V�W�X�O�i(�m[�u�w�y�������������������������E��������@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]���O�l�ܘZ������\���Q��" );
			// ���p��
			wcscpy( format.szInyouKigou, L"> " );		// ���p��

			/*
				�����w��q�̈Ӗ���Windows SDK��GetDateFormat(), GetTimeFormat()���Q�Ƃ̂���
			*/

			format.nDateFormatType = 0;	//���t�����̃^�C�v
			_tcscpy( format.szDateFormat, _T("yyyy\'�N\'M\'��\'d\'��(\'dddd\')\'") );	//���t����
			format.nTimeFormatType = 0;	//���������̃^�C�v
			_tcscpy( format.szTimeFormat, _T("tthh\'��\'mm\'��\'ss\'�b\'")  );			//��������
		}

		// [����]�^�u
		{
			CommonSetting_Search& search = m_pShareData->common.search;

			search.searchOption.Reset();			// �����I�v�V����
			search.bConsecutiveAll = 0;				// �u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
			search.bSelectedArea = false;			// �I��͈͓��u��
			search.bNotifyNotFound = true;			// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��

			search.bGrepSubFolder = true;			// Grep: �T�u�t�H���_������
			search.nGrepOutputLineType = 1;			// Grep: �s���o��/�Y������/�ۃ}�b�`�s ���o��
			search.nGrepOutputStyle = 1;			// Grep: �o�͌`��
			search.bGrepOutputFileOnly = false;
			search.bGrepOutputBaseFolder = false;
			search.bGrepSeparateFolder = false;
			search.bGrepBackup = true;

			search.bGrepDefaultFolder = false;		// Grep: �t�H���_�̏����l���J�����g�t�H���_�ɂ���
			search.nGrepCharSet = CODE_AUTODETECT;	// Grep: �����R�[�h�Z�b�g
			search.bGrepRealTimeView = false;		// 2003.06.28 Moca Grep���ʂ̃��A���^�C���\��
			search.bCaretTextForSearch = true;		// 2006.08.23 ryoji �J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ���
			search.bInheritKeyOtherView = true;
			search.szRegexpLib[0] = _T('\0');		// 2007.08.12 genta ���K�\��DLL
			search.bGTJW_Return = true;				// �G���^�[�L�[�Ń^�O�W�����v
			search.bGTJW_DoubleClick = true;		// �_�u���N���b�N�Ń^�O�W�����v

			search.bGrepExitConfirm = false;		// Grep���[�h�ŕۑ��m�F���邩

			search.bAutoCloseDlgFind = true;		// �����_�C�A���O�������I�ɕ���
			search.bSearchAll		= false;		// �����^�u���^�u�b�N�}�[�N  �擪�i�����j����Č��� 2002.01.26 hor
			search.bAutoCloseDlgReplace = true;		// �u�� �_�C�A���O�������I�ɕ���

			search.nTagJumpMode = 1;				//�^�O�W�����v���[�h
			search.nTagJumpModeKeyword = 3;			//�^�O�W�����v���[�h
		}

		// [�L�[���蓖��]�^�u
		{
			//	Jan. 30, 2005 genta �֐��Ƃ��ēƗ�
			//	2007.11.04 genta �߂�l�`�F�b�N�Dfalse�Ȃ�N�����f�D
			if (!InitKeyAssign(m_pShareData)) {
				return false;
			}
		}

		// [�J�X�^�����j���[]�^�u
		{
			CommonSetting_CustomMenu& customMenu = m_pShareData->common.customMenu;

			for (int i=0; i<MAX_CUSTOM_MENU; ++i) {
				customMenu.szCustMenuNameArr[i][0] = '\0';
				customMenu.nCustMenuItemNumArr[i] = 0;
				for (int j=0; j<MAX_CUSTOM_MENU_ITEMS; ++j) {
					customMenu.nCustMenuItemFuncArr[i][j] = F_0;
					customMenu.nCustMenuItemKeyArr [i][j] = '\0';
				}
				customMenu.bCustMenuPopupArr[i] = true;
			}
			customMenu.szCustMenuNameArr[CUSTMENU_INDEX_FOR_TABWND][0] = '\0';	//@@@ 2003.06.13 MIK

			InitPopupMenu( m_pShareData );
		}

		// [�c�[���o�[]�^�u
		{
			// Jan. 30, 2005 genta �֐��Ƃ��ēƗ�
			InitToolButtons( m_pShareData );
		}

		// [�����L�[���[�h]�^�u
		{
			InitKeyword(m_pShareData);
		}

		// [�x��]�^�u
		{
			CommonSetting_Helper& helper = m_pShareData->common.helper;

			helper.lf = lfIconTitle;
			helper.nPointSize = nIconPointSize;	// �t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj ���Â��o�[�W��������̈ڍs���l�����Ė����l�ŏ�����	// 2009.10.01 ryoji

			helper.szExtHelp[0] = L'\0';		// �O���w���v�P
			helper.szExtHtmlHelp[0] = L'\0';	// �O��HTML�w���v
		
			helper.szMigemoDll[0] = L'\0';		// migemo dll
			helper.szMigemoDict[0] = L'\0';		// migemo dict

			helper.bHtmlHelpIsSingle = true;	// HtmlHelp�r���[�A�͂ЂƂ�

			helper.bHokanKey_RETURN	= true;		// VK_RETURN �⊮����L�[���L��/����
			helper.bHokanKey_TAB	= false;	// VK_TAB   �⊮����L�[���L��/����
			helper.bHokanKey_RIGHT	= true;		// VK_RIGHT �⊮����L�[���L��/����
			helper.bHokanKey_SPACE	= false;	// VK_SPACE �⊮����L�[���L��/����

			helper.bUseHokan = false;			// ���͕⊮�@�\���g�p����
		}

		// [�A�E�g���C��]�^�u
		{
			CommonSetting_OutLine& outline = m_pShareData->common.outline;

			outline.nOutlineDockSet = 0;				// �A�E�g���C����͂̃h�b�L���O�ʒu�p�����@
			outline.bOutlineDockSync = true;			// �A�E�g���C����͂̃h�b�L���O�ʒu�𓯊�����
			outline.bOutlineDockDisp = false;			// �A�E�g���C����͕\���̗L��
			outline.eOutlineDockSide = DockSideType::Float;	// �A�E�g���C����̓h�b�L���O�z�u
			outline.cxOutlineDockLeft		=	0;		// �A�E�g���C���̍��h�b�L���O��
			outline.cyOutlineDockTop		=	0;		// �A�E�g���C���̏�h�b�L���O��
			outline.cxOutlineDockRight		=	0;		// �A�E�g���C���̉E�h�b�L���O��
			outline.cyOutlineDockBottom		=	0;		// �A�E�g���C���̉��h�b�L���O��
			outline.nDockOutline = OutlineType::Text;
			outline.bAutoCloseDlgFuncList = false;		// �A�E�g���C�� �_�C�A���O�������I�ɕ���					//Nov. 18, 2000 JEPRO TRUE��FALSE �ɕύX
			outline.bMarkUpBlankLineEnable	=	false;	// �A�E�g���C���_�C�A���O�Ńu�b�N�}�[�N�̋�s�𖳎�			2002.02.08 aroka,hor
			outline.bFunclistSetFocusOnJump	=	false;	// �A�E�g���C���_�C�A���O�ŃW�����v������t�H�[�J�X���ڂ�	2002.02.08 hor

			InitFileTree( &outline.fileTree );
			outline.fileTreeDefIniName = _T("_sakurafiletree.ini");
		}

		// [�t�@�C�����e��r]�^�u
		{
			CommonSetting_Compare& compare = m_pShareData->common.compare;

			compare.bCompareAndTileHorz = true;		// ������r��A���E�ɕ��ׂĕ\��
		}

		// [�r���[]�^�u
		{
			CommonSetting_View& view = m_pShareData->common.view;

			view.lf = lf;
			view.nPointSize = 0;	// �t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj ���Â��o�[�W��������̈ڍs���l�����Ė����l�ŏ�����	// 2009.10.01 ryoji

			view.bFontIs_FixedPitch = true;				// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
		}

		// [�}�N��]�^�u
		{
			CommonSetting_Macro& macro = m_pShareData->common.macro;

			macro.szKeyMacroFileName[0] = _T('\0');	// �L�[���[�h�}�N���̃t�@�C���� //@@@ 2002.1.24 YAZAKI

			// From Here Sep. 14, 2001 genta
			// Macro�o�^�̏�����
			MacroRec *mptr = macro.macroTable;
			for (int i=0; i<MAX_CUSTMACRO; ++i, ++mptr) {
				mptr->szName[0] = L'\0';
				mptr->szFile[0] = L'\0';
				mptr->bReloadWhenExecute = false;
			}
			// To Here Sep. 14, 2001 genta

			_tcscpy( macro.szMACROFOLDER, szIniFolder );	// �}�N���p�t�H���_

			macro.nMacroOnOpened = -1;		// �I�[�v���㎩�����s�}�N���ԍ�			//@@@ 2006.09.01 ryoji
			macro.nMacroOnTypeChanged = -1;	// �^�C�v�ύX�㎩�����s�}�N���ԍ�		//@@@ 2006.09.01 ryoji
			macro.nMacroOnSave = -1;		// �ۑ��O�������s�}�N���ԍ� 			//@@@ 2006.09.01 ryoji
			macro.nMacroCancelTimer = 10;	// �}�N����~�_�C�A���O�\���҂�����(�b)	// 2011.08.04 syat
		}

		// [�t�@�C�����\��]�^�u
		{
			CommonSetting_FileName& fileName = m_pShareData->common.fileName;

			fileName.bTransformShortPath = true;
			fileName.nTransformShortMaxWidth = 100; // 100'x'��

			for (int i=0; i<MAX_TRANSFORM_FILENAME; ++i) {
				fileName.szTransformFileNameFrom[i][0] = _T('\0');
				fileName.szTransformFileNameTo[i][0] = _T('\0');
			}
			_tcscpy( fileName.szTransformFileNameFrom[0], _T("%DeskTop%\\") );
			_tcscpy( fileName.szTransformFileNameTo[0],   _T("�f�X�N�g�b�v\\") );
			_tcscpy( fileName.szTransformFileNameFrom[1], _T("%Personal%\\") );
			_tcscpy( fileName.szTransformFileNameTo[1],   _T("�}�C�h�L�������g\\") );
			_tcscpy( fileName.szTransformFileNameFrom[2], _T("%Cache%\\Content.IE5\\") );
			_tcscpy( fileName.szTransformFileNameTo[2],   _T("IE�L���b�V��\\") );
			_tcscpy( fileName.szTransformFileNameFrom[3], _T("%TEMP%\\") );
			_tcscpy( fileName.szTransformFileNameTo[3],   _T("TEMP\\") );
			_tcscpy( fileName.szTransformFileNameFrom[4], _T("%Common DeskTop%\\") );
			_tcscpy( fileName.szTransformFileNameTo[4],   _T("���L�f�X�N�g�b�v\\") );
			_tcscpy( fileName.szTransformFileNameFrom[5], _T("%Common Documents%\\") );
			_tcscpy( fileName.szTransformFileNameTo[5],   _T("���L�h�L�������g\\") );
			_tcscpy( fileName.szTransformFileNameFrom[6], _T("%AppData%\\") );		// 2007.05.19 ryoji �ǉ�
			_tcscpy( fileName.szTransformFileNameTo[6],   _T("�A�v���f�[�^\\") );	// 2007.05.19 ryoji �ǉ�
			fileName.nTransformFileNameArrNum = 7;
		}

		// [���̑�]�^�u
		{
			CommonSetting_Others& others = m_pShareData->common.others;

			::SetRect( &others.rcOpenDialog, 0, 0, 0, 0 );		// �u�J���v�_�C�A���O�̃T�C�Y�ƈʒu
			::SetRect( &others.rcCompareDialog, 0, 0, 0, 0 );
			::SetRect( &others.rcDiffDialog, 0, 0, 0, 0 );
			::SetRect( &others.rcFavoriteDialog, 0, 0, 0, 0 );
			::SetRect( &others.rcTagJumpDialog, 0, 0, 0, 0 );
		}

		// [�X�e�[�^�X�o�[]�^�u
		{
			CommonSetting_StatusBar& statusbar = m_pShareData->common.statusBar;

			// �\�������R�[�h�̎w��		2008/6/21	Uchi
			statusbar.bDispUniInSjis		= false;	// SJIS�ŕ����R�[�h�l��Unicode�ŕ\������
			statusbar.bDispUniInJis			= false;	// JIS�ŕ����R�[�h�l��Unicode�ŕ\������
			statusbar.bDispUniInEuc			= false;	// EUC�ŕ����R�[�h�l��Unicode�ŕ\������
			statusbar.bDispUtf8Codepoint	= true;		// UTF-8���R�[�h�|�C���g�ŕ\������
			statusbar.bDispSPCodepoint		= true;		// �T���Q�[�g�y�A���R�[�h�|�C���g�ŕ\������
			statusbar.bDispSelCountByByte	= false;	// �I�𕶎����𕶎��P�ʂł͂Ȃ��o�C�g�P�ʂŕ\������
		}

		// [�v���O�C��]�^�u
		{
			CommonSetting_Plugin& plugin = m_pShareData->common.plugin;

			plugin.bEnablePlugin			= false;	// �v���O�C�����g�p����
			for (int nPlugin=0; nPlugin<MAX_PLUGIN; ++nPlugin) {
				plugin.pluginTable[nPlugin].szName[0] = L'\0';	// �v���O�C����
				plugin.pluginTable[nPlugin].szId[0]	= L'\0';	// �v���O�C��ID
				plugin.pluginTable[nPlugin].state = PLS_NONE;	// �v���O�C�����
			}
		}

		// [���C�����j���[]�^�u
		{
			DataProfile	profile;
			std::vector<std::wstring> data;
			profile.SetReadingMode();
			profile.ReadProfileRes( MAKEINTRESOURCE(IDR_MENU1), MAKEINTRESOURCE(ID_RC_TYPE_INI), &data );

			ShareData_IO::IO_MainMenu( profile, &data, m_pShareData->common.mainMenu, false );
		}

		{
			InitTypeConfigs( m_pShareData, *m_pvTypeSettings );
		}

		{
			/* printSettingArr[0]��ݒ肵�āA�c���1�`7�ɃR�s�[����B
				�K�v�ɂȂ�܂Œx�点�邽�߂ɁAPrint�ɁAShareData�𑀍삷�錠����^����B
				YAZAKI.
			*/
			{
				/*
					2006.08.16 Moca �������P�ʂ� PrintSetting�ɕύX�BShareData�ɂ͈ˑ����Ȃ��B
				*/
				TCHAR szSettingName[64];
				int i = 0;
				auto_sprintf( szSettingName, _T("����ݒ� %d"), i + 1 );
				Print::SettingInitialize( m_pShareData->printSettingArr[0], szSettingName );	//	���������߁B
			}
			for (int i=1; i<MAX_PrintSettingARR; ++i) {
				m_pShareData->printSettingArr[i] = m_pShareData->printSettingArr[0];
				auto_sprintf( m_pShareData->printSettingArr[i].szPrintSettingName, _T("����ݒ� %d"), i + 1 );	// ����ݒ�̖��O
			}
		}

		{
			m_pShareData->searchKeywords.searchKeys.clear();
			m_pShareData->searchKeywords.replaceKeys.clear();
			m_pShareData->searchKeywords.grepFiles.clear();
			m_pShareData->searchKeywords.grepFiles.push_back(_T("*.*"));
			m_pShareData->searchKeywords.grepFolders.clear();

			// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
			m_pShareData->tagJump.tagJumpNum = 0;
			// 2004.06.22 Moca �^�O�W�����v�̐擪
			m_pShareData->tagJump.tagJumpTop = 0;
			// From Here 2005.04.03 MIK �L�[���[�h�w��^�O�W�����v��History�ۊ�
			m_pShareData->tagJump.aTagJumpKeywords.clear();
			m_pShareData->tagJump.bTagJumpICase = FALSE;
			m_pShareData->tagJump.bTagJumpAnyWhere = FALSE;
			// To Here 2005.04.03 MIK 

			m_pShareData->history.m_aExceptMRU.clear();

			_tcscpy( m_pShareData->history.m_szIMPORTFOLDER, szIniFolder );	// �ݒ�C���|�[�g�p�t�H���_

			m_pShareData->history.m_aCommands.clear();
			m_pShareData->history.m_aCurDirs.clear();

			m_pShareData->nExecFlgOpt = 1;	// �O���R�}���h���s�́u�W���o�͂𓾂�v	// 2006.12.03 maru �I�v�V�����̊g���̂���

			m_pShareData->nDiffFlgOpt = 0;	// DIFF�����\��	//@@@ 2002.05.27 MIK

			m_pShareData->szTagsCmdLine[0] = _T('\0');	// CTAGS	//@@@ 2003.05.12 MIK
			m_pShareData->nTagsOpt = 0;	/* CTAGS */	//@@@ 2003.05.12 MIK

			m_pShareData->bLineNumIsCRLF_ForJump = true;	// �w��s�փW�����v�́u���s�P�ʂ̍s�ԍ��v���u�܂�Ԃ��P�ʂ̍s�ԍ��v��
		}
	}else {
		// �I�u�W�F�N�g�����łɑ��݂���ꍇ
		// �t�@�C���̃r���[�� �Ăяo�����v���Z�X�̃A�h���X��ԂɃ}�b�v���܂�
		m_pShareData = (DllSharedData*)::MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);
		SetDllShareData(m_pShareData);

		SelectCharWidthCache(CharWidthFontMode::Edit, CharWidthCacheMode::Share);
		InitCharWidthCache(m_pShareData->common.view.lf);	// 2008/5/15 Uchi

		// From Here Oct. 27, 2000 genta
		//	2014.01.08 Moca �T�C�Y�`�F�b�N�ǉ�
		if (m_pShareData->vStructureVersion != uShareDataVersion
			|| m_pShareData->nSize != sizeof(*m_pShareData)
		) {
			// ���̋��L�f�[�^�̈�͎g���Ȃ��D
			// �n���h�����������
			SetDllShareData(NULL);
			::UnmapViewOfFile(m_pShareData);
			m_pShareData = NULL;
			return false;
		}
		// To Here Oct. 27, 2000 genta

	}
	return true;
}

static
void ConvertLangString(wchar_t* pBuf, size_t chBufSize, std::wstring& org, std::wstring& to)
{
	NativeW mem;
	mem.SetString(pBuf);
	mem.Replace(org.c_str(), to.c_str());
	auto_strncpy(pBuf, mem.GetStringPtr(), chBufSize);
	pBuf[chBufSize - 1] = L'\0';
}

static
void ConvertLangString(char* pBuf, size_t chBufSize, std::wstring& org, std::wstring& to)
{
	NativeA mem;
	mem.SetString(pBuf);
	mem.Replace_j(to_achar(org.c_str()), to_achar(to.c_str()));
	auto_strncpy(pBuf, mem.GetStringPtr(), chBufSize);
	pBuf[chBufSize - 1] = '\0';
}

static
void ConvertLangValueImpl(
	wchar_t* pBuf,
	size_t chBufSize,
	int nStrId,
	std::vector<std::wstring>& values,
	int& index,
	bool setValues,
	bool bUpdate
	)
{
	if (setValues) {
		if (bUpdate) {
			values.push_back(std::wstring(LSW(nStrId)));
		}
		return;
	}
	std::wstring to = LSW(nStrId);
	ConvertLangString(pBuf, chBufSize, values[index], to);
	++index;
}

static
void ConvertLangValueImpl(
	char* pBuf,
	size_t chBufSize,
	int nStrId,
	std::vector<std::wstring>& values,
	int& index,
	bool setValues,
	bool bUpdate
	)
{
	if (setValues) {
		if (bUpdate) {
			values.push_back(std::wstring(LSW(nStrId)));
		}
		return;
	}
	std::wstring to = LSW(nStrId);
	ConvertLangString(pBuf, chBufSize, values[index], to);
	++index;
}

#define ConvertLangValue(buf, id)  ConvertLangValueImpl(buf, _countof(buf), id, values, index, bSetValues, true);
#define ConvertLangValue2(buf, id) ConvertLangValueImpl(buf, _countof(buf), id, values, index, bSetValues, false);


/*!
	���ۉ��Ή��̂��߂̕������ύX����

	1. 1��ڌĂяo���AsetValues��true�ɂ��āAvalues�ɋ��ݒ�̌��ꕶ�����ǂݍ���
	2. SelectLang�Ăяo��
	3. 2��ڌĂяo���Avalues���g���ĐV�ݒ�̌���ɏ�������
*/
void ShareData::ConvertLangValues(std::vector<std::wstring>& values, bool bSetValues)
{
	DllSharedData& shareData = *m_pShareData;
	int index = 0;
	int indexBackup;
	CommonSetting& common = shareData.common;
	ConvertLangValue(common.tabBar.szTabWndCaption, STR_TAB_CAPTION_OUTPUT);
	ConvertLangValue(common.tabBar.szTabWndCaption, STR_TAB_CAPTION_GREP);
	indexBackup = index;
	ConvertLangValue(common.tabBar.szTabWndCaption, STR_CAPTION_ACTIVE_OUTPUT);
	ConvertLangValue(common.tabBar.szTabWndCaption, STR_CAPTION_ACTIVE_UPDATE);
	ConvertLangValue(common.tabBar.szTabWndCaption, STR_CAPTION_ACTIVE_VIEW);
	ConvertLangValue(common.tabBar.szTabWndCaption, STR_CAPTION_ACTIVE_OVERWRITE);
	ConvertLangValue(common.tabBar.szTabWndCaption, STR_CAPTION_ACTIVE_KEYMACRO);
	index = indexBackup;
	ConvertLangValue2(common.window.szWindowCaptionActive, STR_CAPTION_ACTIVE_OUTPUT);
	ConvertLangValue2(common.window.szWindowCaptionActive, STR_CAPTION_ACTIVE_UPDATE);
	ConvertLangValue2(common.window.szWindowCaptionActive, STR_CAPTION_ACTIVE_VIEW);
	ConvertLangValue2(common.window.szWindowCaptionActive, STR_CAPTION_ACTIVE_OVERWRITE);
	ConvertLangValue2(common.window.szWindowCaptionActive, STR_CAPTION_ACTIVE_KEYMACRO);
	index = indexBackup;
	ConvertLangValue2(common.window.szWindowCaptionInactive, STR_CAPTION_ACTIVE_OUTPUT);
	ConvertLangValue2(common.window.szWindowCaptionInactive, STR_CAPTION_ACTIVE_UPDATE);
	ConvertLangValue2(common.window.szWindowCaptionInactive, STR_CAPTION_ACTIVE_VIEW);
	ConvertLangValue2(common.window.szWindowCaptionInactive, STR_CAPTION_ACTIVE_OVERWRITE);
	ConvertLangValue2(common.window.szWindowCaptionInactive, STR_CAPTION_ACTIVE_KEYMACRO);
	ConvertLangValue(common.format.szDateFormat, STR_DATA_FORMAT);
	ConvertLangValue(common.format.szTimeFormat, STR_TIME_FORMAT);
	indexBackup = index;
	for (int i=0; i<common.fileName.nTransformFileNameArrNum; ++i) {
		index = indexBackup;
		ConvertLangValue(common.fileName.szTransformFileNameTo[i], STR_TRANSNAME_COMDESKTOP);
		ConvertLangValue(common.fileName.szTransformFileNameTo[i], STR_TRANSNAME_COMDOC);
		ConvertLangValue(common.fileName.szTransformFileNameTo[i], STR_TRANSNAME_DESKTOP);
		ConvertLangValue(common.fileName.szTransformFileNameTo[i], STR_TRANSNAME_MYDOC);
		ConvertLangValue(common.fileName.szTransformFileNameTo[i], STR_TRANSNAME_IE);
		ConvertLangValue(common.fileName.szTransformFileNameTo[i], STR_TRANSNAME_TEMP);
		ConvertLangValue(common.fileName.szTransformFileNameTo[i], STR_TRANSNAME_APPDATA);
		if (bSetValues) {
			break;
		}
	}
	indexBackup = index;
	for (int i=0; i<MAX_PrintSettingARR; ++i) {
		index = indexBackup;
		ConvertLangValue(shareData.printSettingArr[i].szPrintSettingName, STR_PRINT_SET_NAME);
		if (bSetValues) {
			break;
		}
	}
	assert(m_pvTypeSettings);
	indexBackup = index;
	ConvertLangValue(shareData.typeBasis.szTypeName, STR_TYPE_NAME_BASIS);
	for (int i=0; i<(int)GetTypeSettings().size(); ++i) {
		index = indexBackup;
		TypeConfig& type = *(GetTypeSettings()[i]);
		ConvertLangValue2(type.szTypeName, STR_TYPE_NAME_BASIS);
		ConvertLangValue(type.szTypeName, STR_TYPE_NAME_RICHTEXT);
		ConvertLangValue(type.szTypeName, STR_TYPE_NAME_TEXT);
		ConvertLangValue(type.szTypeName, STR_TYPE_NAME_DOS);
		ConvertLangValue(type.szTypeName, STR_TYPE_NAME_ASM);
		ConvertLangValue(type.szTypeName, STR_TYPE_NAME_INI);
		index = indexBackup;
		ConvertLangValue2(shareData.typesMini[i].szTypeName, STR_TYPE_NAME_BASIS);
		ConvertLangValue2(shareData.typesMini[i].szTypeName, STR_TYPE_NAME_RICHTEXT);
		ConvertLangValue2(shareData.typesMini[i].szTypeName, STR_TYPE_NAME_TEXT);
		ConvertLangValue2(shareData.typesMini[i].szTypeName, STR_TYPE_NAME_DOS);
		ConvertLangValue2(shareData.typesMini[i].szTypeName, STR_TYPE_NAME_ASM);
		ConvertLangValue2(shareData.typesMini[i].szTypeName, STR_TYPE_NAME_INI);
		if (bSetValues) {
			break;
		}
	}
}

/*!
	@brief	�w��t�@�C�����J����Ă��邩���ׂ�
	
	�w��̃t�@�C�����J����Ă���ꍇ�͊J���Ă���E�B���h�E�̃n���h����Ԃ�

	@retval	true ���łɊJ���Ă���
	@retval	false �J���Ă��Ȃ�����
*/
bool ShareData::IsPathOpened(const TCHAR* pszPath, HWND* phwndOwner)
{
	*phwndOwner = NULL;

	// 2007.10.01 genta ���΃p�X���΃p�X�ɕϊ�
	// �ϊ����Ȃ���IsPathOpened�Ő��������ʂ�����ꂸ�C
	// ����t�@�C���𕡐��J�����Ƃ�����D
	TCHAR szBuf[_MAX_PATH];
	if (GetLongFileName(pszPath, szBuf)) {
		pszPath = szBuf;
	}

	// ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ�
	if (AppNodeGroupHandle(0).GetEditorWindowsNum() == 0) {
		return false;
	}
	
	for (int i=0; i<m_pShareData->nodes.nEditArrNum; ++i) {
		if (IsSakuraMainWindow(m_pShareData->nodes.pEditArr[i].hWnd)) {
			// �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm
			::SendMessage(m_pShareData->nodes.pEditArr[i].hWnd, MYWM_GETFILEINFO, 1, 0);
			EditInfo* pfi = (EditInfo*)&m_pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;

			// ����p�X�̃t�@�C�������ɊJ����Ă��邩
			if (_tcsicmp(pfi->szPath, pszPath) == 0) {
				*phwndOwner = m_pShareData->nodes.pEditArr[i].hWnd;
				return true;
			}
		}
	}
	return false;
}

/*!
	@brief	�w��t�@�C�����J����Ă��邩���ׂA���d�I�[�v�����̕����R�[�h�Փ˂��m�F

	�������łɊJ���Ă���΃A�N�e�B�u�ɂ��āA�E�B���h�E�̃n���h����Ԃ��B
	����ɁA�����R�[�h���قȂ�Ƃ��̃��[�j���O����������B
	���������ɎU��΂������d�I�[�v���������W��������̂��ړI�B

	@retval	�J����Ă���ꍇ�͊J���Ă���E�B���h�E�̃n���h��

	@note	CEditDoc::FileLoad�ɐ旧���Ď��s����邱�Ƃ����邪�A
			CEditDoc::FileLoad��������s�����K�v�����邱�Ƃɒ��ӁB
			(�t�H���_�w��̏ꍇ��CEditDoc::FileLoad�����ڎ��s�����ꍇ�����邽��)

	@retval	TRUE ���łɊJ���Ă���
	@retval	FALSE �J���Ă��Ȃ�����

	@date 2007.03.12 maru �V�K�쐬
*/
bool ShareData::ActiveAlreadyOpenedWindow(const TCHAR* pszPath, HWND* phwndOwner, EncodingType nCharCode)
{
	if (IsPathOpened(pszPath, phwndOwner)) {
		
		// �����R�[�h�̈�v�m�F
		::SendMessage(*phwndOwner, MYWM_GETFILEINFO, 0, 0);
		EditInfo* pfi = (EditInfo*)&m_pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;
		if (nCharCode != CODE_AUTODETECT) {
			TCHAR szCpNameCur[100];
			CodePage::GetNameLong(szCpNameCur, pfi->nCharCode);
			TCHAR szCpNameNew[100];
			CodePage::GetNameLong(szCpNameNew, pfi->nCharCode);
			if (szCpNameCur[0] && szCpNameNew[0]) {
				if (nCharCode != pfi->nCharCode) {
					TopWarningMessage(*phwndOwner,
						LS(STR_ERR_CSHAREDATA20),
						pszPath,
						szCpNameCur,
						szCpNameNew
					);
				}
			}else {
				TopWarningMessage(*phwndOwner,
					LS(STR_ERR_CSHAREDATA21),
					pszPath,
					pfi->nCharCode,
					szCpNameCur[0] == NULL ? LS(STR_ERR_CSHAREDATA22) : szCpNameCur,
					nCharCode,
					szCpNameNew[0] == NULL ? LS(STR_ERR_CSHAREDATA22) : szCpNameNew
				);
			}
		}

		// �J���Ă���E�B���h�E���A�N�e�B�u�ɂ���
		ActivateFrameWindow(*phwndOwner);

		// MRU���X�g�ւ̓o�^
		MruFile().Add(pfi);
		return true;
	}else {
		return false;
	}

}


/*!
	�A�E�g�v�b�g�E�B���h�E�ɏo��(�����t)

	�A�E�g�v�b�g�E�B���h�E��������΃I�[�v������
	@param lpFmt [in] �����w�蕶����(wchar_t��)
	@date 2010.02.22 Moca auto_vsprintf���� tchar_vsnprintf_s �ɕύX.��������Ƃ��͐؂�l�߂���
*/
void ShareData::TraceOut(LPCTSTR lpFmt, ...)
{
	if (!OpenDebugWindow(m_hwndTraceOutSource, false)) {
		return;
	}
	
	va_list argList;
	va_start(argList, lpFmt);
	int ret = tchar_vsnprintf_s(m_pShareData->workBuffer.GetWorkBuffer<WCHAR>(), 
		m_pShareData->workBuffer.GetWorkBufferCount<WCHAR>(),
		to_wchar(lpFmt), argList);
	va_end(argList);
	if (ret == -1) {
		// �؂�l�߂�ꂽ
		ret = auto_strlen(m_pShareData->workBuffer.GetWorkBuffer<WCHAR>());
	}else if (ret < 0) {
		// �ی�R�[�h:�󂯑���wParam��size_t�ŕ����Ȃ��̂���
		ret = 0;
	}
	DWORD_PTR dwMsgResult;
	::SendMessageTimeout(m_pShareData->handles.hwndDebug, MYWM_ADDSTRINGLEN_W, ret, 0,
		SMTO_NORMAL, 10000, &dwMsgResult);
}

/*!
	�A�E�g�v�b�g�E�B���h�E�ɏo��(������w��)

	�����ꍇ�͕������đ���
	�A�E�g�v�b�g�E�B���h�E��������΃I�[�v������
	@param  pStr  �o�͂��镶����
	@param  len   pStr�̕�����(�I�[NUL���܂܂Ȃ�) -1�Ŏ����v�Z
	@date 2010.05.11 Moca �V��
*/
void ShareData::TraceOutString(const wchar_t* pStr, int len)
{
	if (!OpenDebugWindow(m_hwndTraceOutSource, false)) {
		return;
	}
	if (len == -1) {
		len = wcslen(pStr);
	}
	// workBuffer���肬��ł����Ȃ�����ǁA�O�̂���\0�I�[�ɂ��邽�߂ɗ]�T���Ƃ�
	// -1 ��� 8,4�o�C�g���E�̂ق����R�s�[�������͂��Ȃ̂ŁA-4�ɂ���
	const int buffLen = (int)m_pShareData->workBuffer.GetWorkBufferCount<WCHAR>() - 4;
	wchar_t*  pOutBuffer = m_pShareData->workBuffer.GetWorkBuffer<WCHAR>();
	int outPos = 0;
	if (len == 0) {
		// 0�̂Ƃ��͉����ǉ����Ȃ����A�J�[�\���ړ�����������
		LockGuard<Mutex> guard( ShareData::GetMutexShareWork() );
		pOutBuffer[0] = L'\0';
		::SendMessage(m_pShareData->handles.hwndDebug, MYWM_ADDSTRINGLEN_W, 0, 0);
	}else {
		while (outPos < len) {
			int outLen = buffLen;
			if (len - outPos < buffLen) {
				// �c��S��
				outLen = len - outPos;
			}
			// ���܂肪1�����ȏ゠��
			if (outPos + outLen < len) {
				// CRLF(\r\n)��UTF-16����������Ȃ��悤��
				if ((pStr[outPos + outLen - 1] == WCODE::CR && pStr[outPos + outLen] == WCODE::LF)
					|| (IsUtf16SurrogHi(pStr[outPos + outLen - 1]) && IsUtf16SurrogLow(pStr[outPos + outLen]))
				) {
					--outLen;
				}
			}
			LockGuard<Mutex> guard( ShareData::GetMutexShareWork() );
			wmemcpy(pOutBuffer, pStr + outPos, outLen);
			pOutBuffer[outLen] = L'\0';
			DWORD_PTR dwMsgResult;
			if (::SendMessageTimeout(m_pShareData->handles.hwndDebug, MYWM_ADDSTRINGLEN_W, outLen, 0,
				SMTO_NORMAL, 10000, &dwMsgResult) == 0
			) {
				// �G���[���^�C���A�E�g
				break;
			}
			outPos += outLen;
		}
	}
}

/*
	�f�o�b�O�E�B���h�E��\��
	@param hwnd �V�K�E�B���h�E�̂Ƃ��̃f�o�b�O�E�B���h�E�̏����O���[�v
	@param bAllwaysActive �\���ς݃E�B���h�E�ł��A�N�e�B�u�ɂ���
	@return true:�\���ł����B�܂��͂��łɕ\������Ă���B
	@date 2010.05.11 Moca TraceOut���番��
*/
bool ShareData::OpenDebugWindow(HWND hwnd, bool bAllwaysActive)
{
	bool ret = true;
	if (!m_pShareData->handles.hwndDebug
		|| !IsSakuraMainWindow(m_pShareData->handles.hwndDebug)
	) {
		// 2007.06.26 ryoji
		// �A�E�g�v�b�g�E�B���h�E���쐬���Ɠ����O���[�v�ɍ쐬���邽�߂� m_hwndTraceOutSource ���g���Ă��܂�
		// �im_hwndTraceOutSource �� CEditWnd::Create() �ŗ\�ߐݒ�j
		// ������ƕs���D�����ǁATraceOut() �̈����ɂ��������N�������w�肷��̂��D�D�D
		// 2010.05.11 Moca m_hwndTraceOutSource�͈ˑR�Ƃ��Ďg���Ă��܂��������ɂ��܂���
		LoadInfo loadInfo;
		loadInfo.filePath = _T("");
		// CODE_SJIS->CODE_UNICODE	2008/6/8 Uchi
		// CODE_UNICODE->CODE_NONE	2010.05.11 Moca �f�t�H���g�����R�[�h�Őݒ�ł���悤�ɖ��w��ɕύX
		loadInfo.eCharCode = CODE_NONE;
		loadInfo.bViewMode = false;
		ret = ControlTray::OpenNewEditor(
			NULL,
			hwnd,
			loadInfo,
			_T("-DEBUGMODE"),
			true
		);
		// 2001/06/23 N.Nakatani �����o��܂ŃE�G�C�g��������悤�ɏC��
		// �A�E�g�v�b�g�E�B���h�E���o����܂�5�b���炢�҂B
		// Jun. 25, 2001 genta OpenNewEditor�̓����@�\�𗘗p����悤�ɕύX
		bAllwaysActive = true; // �V����������Ƃ���active
	}
	// �J���Ă���E�B���h�E���A�N�e�B�u�ɂ���
	if (ret && bAllwaysActive) {
		ActivateFrameWindow(m_pShareData->handles.hwndDebug);
	}
	return ret;
}

// ini�t�@�C���̕ۑ��悪���[�U�ʐݒ�t�H���_���ǂ���	// 2007.05.25 ryoji
bool ShareData::IsPrivateSettings(void) {
	return m_pShareData->fileNameManagement.iniFolder.bWritePrivate;
}


/*
	ShareData::CheckMRUandOPENFOLDERList
	MRU��OPENFOLDER���X�g�̑��݃`�F�b�N�Ȃ�
	���݂��Ȃ��t�@�C����t�H���_��MRU��OPENFOLDER���X�g����폜����

	@note ���݂͎g���Ă��Ȃ��悤���B
	@par History
	2001.12.26 �폜�����B�iYAZAKI�j
	
*/
/*!	idx�Ŏw�肵���}�N���t�@�C�����i�t���p�X�j���擾����D

	@param pszPath [in]	�p�X���̏o�͐�D�����݂̂�m�肽���Ƃ���NULL������D
	@param idx [in]		�}�N���ԍ�
	@param nBufLen [in]	pszPath�Ŏw�肳�ꂽ�o�b�t�@�̃o�b�t�@�T�C�Y

	@retval >0 : �p�X���̒����D
	@retval  0 : �G���[�C���̃}�N���͎g���Ȃ��C�t�@�C�������w�肳��Ă��Ȃ��D
	@retval <0 : �o�b�t�@�s���D�K�v�ȃo�b�t�@�T�C�Y�� -(�߂�l)+1

	@author YAZAKI
	@date 2003.06.08 Moca ���[�J���ϐ��ւ̃|�C���^��Ԃ��Ȃ��悤�Ɏd�l�ύX
	@date 2003.06.14 genta �����񒷁C�|�C���^�̃`�F�b�N��ǉ�
	@date 2003.06.24 Moca idx��-1�̂Ƃ��A�L�[�}�N���̃t���p�X��Ԃ�.
	
	@note idx�͐��m�Ȃ��̂łȂ���΂Ȃ�Ȃ��B(�����Ő������`�F�b�N���s���Ă��Ȃ�)
*/
int ShareData::GetMacroFilename(int idx, TCHAR* pszPath, int nBufLen)
{
	if (idx != -1 && !m_pShareData->common.macro.macroTable[idx].IsEnabled()) {
		return 0;
	}
	const TCHAR* pszFile;

	if (idx == -1) {
		pszFile = _T("RecKey.mac");
	}else {
		pszFile = m_pShareData->common.macro.macroTable[idx].szFile;
	}
	if (pszFile[0] == _T('\0')) {	// �t�@�C����������
		if (pszPath) {
			pszPath[0] = _T('\0');
		}
		return 0;
	}
	const TCHAR* ptr = pszFile;
	int nLen = _tcslen(ptr); // Jul. 21, 2003 genta wcslen�Ώۂ�����Ă������߃}�N�����s���ł��Ȃ�

	if (!_IS_REL_PATH(pszFile)	// ��΃p�X
		|| m_pShareData->common.macro.szMACROFOLDER[0] == _T('\0')	// �t�H���_�w��Ȃ�
	) {
		if (!pszPath || nBufLen <= nLen) {
			return -nLen;
		}
		_tcscpy(pszPath, pszFile);
		return nLen;
	}else {	// �t�H���_�w�肠��
		// ���΃p�X����΃p�X
		int nFolderSep = AddLastChar(m_pShareData->common.macro.szMACROFOLDER, _countof2(m_pShareData->common.macro.szMACROFOLDER), _T('\\'));
		TCHAR* pszDir;

		// 2003.06.24 Moca �t�H���_�����΃p�X�Ȃ���s�t�@�C������̃p�X
		// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
		if (_IS_REL_PATH(m_pShareData->common.macro.szMACROFOLDER)) {
			TCHAR szDir[_MAX_PATH + _countof2(m_pShareData->common.macro.szMACROFOLDER)];
			GetInidirOrExedir(szDir, m_pShareData->common.macro.szMACROFOLDER);
			pszDir = szDir;
		}else {
			pszDir = m_pShareData->common.macro.szMACROFOLDER;
		}

		int nDirLen = _tcslen(pszDir);
		int nAllLen = nDirLen + nLen + (nFolderSep == -1 ? 1 : 0);
		if (!pszPath || nBufLen <= nAllLen) {
			return -nAllLen;
		}

		_tcscpy(pszPath, pszDir);
		TCHAR* ptr = pszPath + nDirLen;
		if (nFolderSep == -1) {
			*ptr++ = _T('\\');
		}
		_tcscpy(ptr, pszFile);
		return nAllLen;
	}

}

/*!	idx�Ŏw�肵���}�N����bReloadWhenExecute���擾����B
	idx�͐��m�Ȃ��̂łȂ���΂Ȃ�Ȃ��B
	YAZAKI
*/
bool ShareData::BeReloadWhenExecuteMacro(int idx)
{
	if (!m_pShareData->common.macro.macroTable[idx].IsEnabled()) {
		return false;
	}
	return m_pShareData->common.macro.macroTable[idx].bReloadWhenExecute;
}


/*!	@brief ���L������������/�c�[���o�[

	�c�[���o�[�֘A�̏���������

	@author genta
	@date 2005.01.30 genta ShareData::Init()���番���D
		����ݒ肵�Ȃ��ň�C�Ƀf�[�^�]������悤�ɁD
*/
void ShareData::InitToolButtons(DllSharedData* pShareData)
{
	// �c�[���o�[�{�^���\����
	// Sept. 16, 2000 JEPRO
	// CShareData_new2.cpp�łł��邾���n���ƂɏW�܂�悤�ɃA�C�R���̏��Ԃ�啝�ɓ���ւ����̂ɔ����ȉ��̏����ݒ�l��ύX
	// 2010.06.26 Moca ���e�� CMenuDrawer::FindToolbarNoFromCommandId �̖߂�l�ł�
	static const int DEFAULT_TOOL_BUTTONS[] = {
		1,	// �V�K�쐬
		25,		// �t�@�C�����J��(DropDown)
		3,		// �㏑���ۑ�		// Sept. 16, 2000 JEPRO 3��11�ɕύX	//Oct. 25, 2000 11��3
		4,		// ���O��t���ĕۑ�	// Sept. 19, 2000 JEPRO �ǉ�
		0,

		33,	// ���ɖ߂�(Undo)	// Sept. 16, 2000 JEPRO 7��19�ɕύX	//Oct. 25, 2000 19��33
		34,	// ��蒼��(Redo)	// Sept. 16, 2000 JEPRO 8��20�ɕύX	//Oct. 25, 2000 20��34
		0,

		87,	// �ړ�����: �O��	// Dec. 24, 2000 JEPRO �ǉ�
		88,	// �ړ�����: ����	// Dec. 24, 2000 JEPRO �ǉ�
		0,

		225,	// ����		// Sept. 16, 2000 JEPRO 9��22�ɕύX	//Oct. 25, 2000 22��225
		226,	// ��������	// Sept. 16, 2000 JEPRO 16��23�ɕύX	//Oct. 25, 2000 23��226
		227,	// �O������	// Sept. 16, 2000 JEPRO 17��24�ɕύX	//Oct. 25, 2000 24��227
		228,	// �u��		// Oct. 7, 2000 JEPRO �ǉ�
		229,	// �����}�[�N�̃N���A	// Sept. 16, 2000 JEPRO 41��25�ɕύX(Oct. 7, 2000 25��26)	//Oct. 25, 2000 25��229
		230,	// Grep		// Sept. 16, 2000 JEPRO 14��31�ɕύX	//Oct. 25, 2000 31��230
		232,	// �A�E�g���C�����	// Dec. 24, 2000 JEPRO �ǉ�
		0,

		264,	// �^�C�v�ʐݒ�ꗗ	// Sept. 16, 2000 JEPRO �ǉ�
		265,	// �^�C�v�ʐݒ�		// Sept. 16, 2000 JEPRO 18��36�ɕύX	//Oct. 25, 2000 36��265
		266,	// ���ʐݒ�			// Sept. 16, 2000 JEPRO 10��37�ɕύX �������u�ݒ�v���p�e�B�V�[�g�v����ύX	//Oct. 25, 2000 37��266
		0,		// Oct. 8, 2000 jepro ���s�̂��߂ɒǉ�
		346,	// �R�}���h�ꗗ	// Oct. 8, 2000 JEPRO �ǉ�
	};

	// �c�[���o�[�A�C�R�����̍ő�l�𒴂��Ȃ����߂̂��܂��Ȃ�
	// �ő�l�𒴂��Ē�`���悤�Ƃ���Ƃ����ŃR���p�C���G���[�ɂȂ�܂��D
	char dummy[_countof(DEFAULT_TOOL_BUTTONS) < MAX_TOOLBAR_BUTTON_ITEMS ? 1 : 0];
	dummy[0] = 0;

	memcpy_raw(
		pShareData->common.toolBar.nToolBarButtonIdxArr,
		DEFAULT_TOOL_BUTTONS,
		sizeof(DEFAULT_TOOL_BUTTONS)
	);

	// �c�[���o�[�{�^���̐�
	pShareData->common.toolBar.nToolBarButtonNum = _countof(DEFAULT_TOOL_BUTTONS);
	pShareData->common.toolBar.bToolBarIsFlat = !IsVisualStyle();			// �t���b�g�c�[���o�[�ɂ���^���Ȃ�	// 2006.06.23 ryoji �r�W���A���X�^�C���ł͏����l���m�[�}���ɂ���
	
}


/*!	@brief ���L������������/�|�b�v�A�b�v���j���[

	�|�b�v�A�b�v���j���[�̏���������

	@date 2005.01.30 genta ShareData::Init()���番���D
*/
void ShareData::InitPopupMenu(DllSharedData* pShareData)
{
	// �J�X�^�����j���[ �K��l
	
	CommonSetting_CustomMenu& menu = m_pShareData->common.customMenu;

	// �E�N���b�N���j���[
	int n = 0;
	menu.nCustMenuItemFuncArr[0][n] = F_UNDO;
	menu.nCustMenuItemKeyArr [0][n] = 'U';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_REDO;
	menu.nCustMenuItemKeyArr [0][n] = 'R';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_0;
	menu.nCustMenuItemKeyArr [0][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_CUT;
	menu.nCustMenuItemKeyArr [0][n] = 'T';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_COPY;
	menu.nCustMenuItemKeyArr [0][n] = 'C';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_PASTE;
	menu.nCustMenuItemKeyArr [0][n] = 'P';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_DELETE;
	menu.nCustMenuItemKeyArr [0][n] = 'D';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_0;
	menu.nCustMenuItemKeyArr [0][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_COPY_CRLF;	// Nov. 9, 2000 JEPRO �uCRLF���s�ŃR�s�[�v��ǉ�
	menu.nCustMenuItemKeyArr [0][n] = 'L';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_COPY_ADDCRLF;
	menu.nCustMenuItemKeyArr [0][n] = 'H';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_PASTEBOX;	// Nov. 9, 2000 JEPRO �u��`�\��t���v�𕜊�
	menu.nCustMenuItemKeyArr [0][n] = 'X';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_0;
	menu.nCustMenuItemKeyArr [0][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_SELECTALL;
	menu.nCustMenuItemKeyArr [0][n] = 'A';
	++n;

	menu.nCustMenuItemFuncArr[0][n] = F_0;		// Oct. 3, 2000 JEPRO �ȉ��Ɂu�^�O�W�����v�v�Ɓu�^�O�W�����v�o�b�N�v��ǉ�
	menu.nCustMenuItemKeyArr [0][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_TAGJUMP;
	menu.nCustMenuItemKeyArr [0][n] = 'G';		// Nov. 9, 2000 JEPRO �u�R�s�[�v�ƃo�b�e�B���O���Ă����A�N�Z�X�L�[��ύX(T��G)
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_TAGJUMPBACK;
	menu.nCustMenuItemKeyArr [0][n] = 'B';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_0;		// Oct. 15, 2000 JEPRO �ȉ��Ɂu�I��͈͓��S�s�R�s�[�v�Ɓu���p���t���R�s�[�v��ǉ�
	menu.nCustMenuItemKeyArr [0][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_COPYLINES;
	menu.nCustMenuItemKeyArr [0][n] = '@';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_COPYLINESASPASSAGE;
	menu.nCustMenuItemKeyArr [0][n] = '.';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_0;
	menu.nCustMenuItemKeyArr [0][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_COPYPATH;
	menu.nCustMenuItemKeyArr [0][n] = '\\';
	++n;
	menu.nCustMenuItemFuncArr[0][n] = F_PROPERTY_FILE;
	menu.nCustMenuItemKeyArr [0][n] = 'F';		// Nov. 9, 2000 JEPRO �u��蒼���v�ƃo�b�e�B���O���Ă����A�N�Z�X�L�[��ύX(R��F)
	++n;
	menu.nCustMenuItemNumArr[0] = n;

	// �J�X�^�����j���[�P
	menu.nCustMenuItemNumArr[1] = 7;
	menu.nCustMenuItemFuncArr[1][0] = F_FILEOPEN;
	menu.nCustMenuItemKeyArr [1][0] = 'O';		// Sept. 14, 2000 JEPRO �ł��邾���W���ݒ�l�ɍ��킹��悤�ɕύX (F��O)
	menu.nCustMenuItemFuncArr[1][1] = F_FILESAVE;
	menu.nCustMenuItemKeyArr [1][1] = 'S';
	menu.nCustMenuItemFuncArr[1][2] = F_NEXTWINDOW;
	menu.nCustMenuItemKeyArr [1][2] = 'N';		// Sept. 14, 2000 JEPRO �ł��邾���W���ݒ�l�ɍ��킹��悤�ɕύX (O��N)
	menu.nCustMenuItemFuncArr[1][3] = F_TOLOWER;
	menu.nCustMenuItemKeyArr [1][3] = 'L';
	menu.nCustMenuItemFuncArr[1][4] = F_TOUPPER;
	menu.nCustMenuItemKeyArr [1][4] = 'U';
	menu.nCustMenuItemFuncArr[1][5] = F_0;
	menu.nCustMenuItemKeyArr [1][5] = '\0';
	menu.nCustMenuItemFuncArr[1][6] = F_WINCLOSE;
	menu.nCustMenuItemKeyArr [1][6] = 'C';

	// �^�u���j���[	//@@@ 2003.06.14 MIK
	n = 0;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILESAVE;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'S';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILESAVEAS_DIALOG;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'A';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILECLOSE;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'R';	// 2007.06.26 ryoji B -> R
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILECLOSE_OPEN;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'L';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_WINCLOSE;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'C';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_FILE_REOPEN;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'W';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_0;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_GROUPCLOSE;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'G';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_CLOSEOTHER;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'O';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_CLOSELEFT;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'H';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_CLOSERIGHT;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'M';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_0;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '\0';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_MOVERIGHT;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '0';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_MOVELEFT;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '1';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_SEPARATE;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'E';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_JOINTNEXT;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'X';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_JOINTPREV;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = 'V';
	++n;
	// TODO: loop
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_1;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '1';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_2;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '2';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_3;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '3';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_4;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '4';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_5;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '5';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_6;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '6';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_7;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '7';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_8;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '8';
	++n;
	menu.nCustMenuItemFuncArr[CUSTMENU_INDEX_FOR_TABWND][n] = F_TAB_9;
	menu.nCustMenuItemKeyArr [CUSTMENU_INDEX_FOR_TABWND][n] = '9';
	++n;
	menu.nCustMenuItemNumArr[CUSTMENU_INDEX_FOR_TABWND] = n;
}

// ����I����ɋ��L���������̕�������X�V����
void ShareData::RefreshString()
{

	RefreshKeyAssignString(m_pShareData);
}

void ShareData::CreateTypeSettings()
{
	if (!m_pvTypeSettings) {
		m_pvTypeSettings = new std::vector<TypeConfig*>();
	}
}

std::vector<TypeConfig*>& ShareData::GetTypeSettings()
{
	return *m_pvTypeSettings;
}


void ShareData::InitFileTree( FileTree* setting )
{
	setting->bProject = true;
	for (int i=0; i<(int)_countof(setting->items); ++i) {
		FileTreeItem& item = setting->items[i];
		item.eFileTreeItemType = FileTreeItemType::Grep;
		item.szTargetPath = _T("");
		item.szLabelName = _T("");
		item.szTargetPath = _T("");
		item.nDepth = 0;
		item.szTargetFile = _T("");
		item.bIgnoreHidden = true;
		item.bIgnoreReadOnly = false;
		item.bIgnoreSystem = false;
	}
	setting->nItemCount = 1;
	setting->items[0].szTargetPath = _T(".");
	setting->items[0].szTargetFile = _T("*.*");
}
