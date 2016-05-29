// 2007.09.28 kobake Common����
/*
	Copyright (C) 2008, kobake

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
#pragma once

#include "KeywordSetMgr.h"
#include "func/KeyBind.h"
#include "func/FuncLookup.h" // MacroRec
#include "io/File.h" // ShareMode

// Apr. 05, 2003 genta WindowCaption�p�̈�i�ϊ��O�j�̒���
static const uint32_t MAX_CAPTION_CONF_LEN = 256;

static const uint32_t MAX_DATETIMEFOREMAT_LEN	= 100;
static const uint32_t MAX_CUSTOM_MENU			=  25;
static const uint32_t MAX_CUSTOM_MENU_NAME_LEN	=  32;
static const uint32_t MAX_CUSTOM_MENU_ITEMS		=  48;
static const uint32_t MAX_TOOLBAR_BUTTON_ITEMS	= 512;	// �c�[���o�[�ɓo�^�\�ȃ{�^���ő吔	
static const uint32_t MAX_TOOLBAR_ICON_X		=  32;	// �A�C�R��BMP�̌���
static const uint32_t MAX_TOOLBAR_ICON_Y		=  15;	// �A�C�R��BMP�̒i��
static const uint32_t MAX_TOOLBAR_ICON_COUNT	= MAX_TOOLBAR_ICON_X * MAX_TOOLBAR_ICON_Y; // =480
// Oct. 22, 2000 JEPRO �A�C�R���̍ő�o�^����128���₵��(256��384)	
// 2010/3/14 Uchi �A�C�R���̍ő�o�^����32���₵��(384��416)
// 2010/6/26 syat �A�C�R���̍ő�o�^����15�i�ɑ��₵��(416��480)

// ���łƈႢ�Abool�^�g����悤�ɂ��Ă���܂� by kobake

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �S��                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_General {
	// Jul. 3, 2000 genta
	// �A�N�Z�X�֐�(�Ȉ�)
	// int���r�b�g�P�ʂɕ������Ďg��
	// ��4bit��CaretType�ɓ��ĂĂ���(�����̗\��ő��߂Ɏ���Ă���)
	int		GetCaretType(void) const { return nCaretType & 0xf; }
	void	SetCaretType(const int f) { nCaretType &= ~0xf; nCaretType |= f & 0xf; }

	// �J�[�\��
	int		nCaretType;							// �J�[�\���̃^�C�v 0=win 1=dos 
	bool	bIsINSMode;							// �}���^�㏑�����[�h
	bool	bIsFreeCursorMode;					// �t���[�J�[�\�����[�h��
	bool	bStopsBothEndsWhenSearchWord;		// �P��P�ʂňړ�����Ƃ��ɁA�P��̗��[�Ŏ~�܂邩
	bool	bStopsBothEndsWhenSearchParagraph;	// �i���P�ʂňړ�����Ƃ��ɁA�i���̗��[�Ŏ~�܂邩
	bool	bNoCaretMoveByActivation;			// �}�E�X�N���b�N�ɂăA�N�e�B�x�[�g���ꂽ���̓J�[�\���ʒu���ړ����Ȃ�  2007.10.02 nasukoji (add by genta)

	// �X�N���[��
	int		nRepeatedScrollLineNum;			// �L�[���s�[�g���̃X�N���[���s��
	bool	nRepeatedScroll_Smooth;			// �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩
	int		nPageScrollByWheel;				// �L�[/�}�E�X�{�^�� + �z�C�[���X�N���[���Ńy�[�WUP/DOWN����	// 2009.01.17 nasukoji
	int		nHorizontalScrollByWheel;		// �L�[/�}�E�X�{�^�� + �z�C�[���X�N���[���ŉ��X�N���[������		// 2009.01.17 nasukoji

	// �^�X�N�g���C
	bool	bUseTaskTray;					// �^�X�N�g���C�̃A�C�R�����g��
	bool	bStayTaskTray;					// �^�X�N�g���C�̃A�C�R�����풓
	WORD	wTrayMenuHotKeyCode;			// �^�X�N�g���C���N���b�N���j���[ �L�[
	WORD	wTrayMenuHotKeyMods;			// �^�X�N�g���C���N���b�N���j���[ �L�[

	// ����
	size_t	nMRUArrNum_MAX;					// �t�@�C���̗���MAX
	size_t	nOPENFOLDERArrNum_MAX;			// �t�H���_�̗���MAX

	// �m�[�J�e�S��
	bool	bCloseAllConfirm;				// [���ׂĕ���]�ő��ɕҏW�p�̃E�B���h�E������Ίm�F����	// 2006.12.25 ryoji
	bool	bExitConfirm;					// �I�����̊m�F������

	// INI���ݒ�̂�
	bool	bDispExitingDialog;				// �I���_�C�A���O��\������
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �E�B���h�E                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// 2004.05.13 Moca
// �E�B���h�E�T�C�Y�E�ʒu�̐�����@
enum class WinSizeMode {
	Default		= 0,	// �w��Ȃ�
	Save		= 1,	// �p��(�ۑ�)
	Set			= 2		// ���ڎw��(�Œ�)
};

struct CommonSetting_Window {
	// ��{�ݒ�
	bool			bDispToolBar;				// ����E�B���h�E���J�����Ƃ��c�[���o�[��\������
	bool			bDispStatusBar;				// ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������
	bool			bDispFuncKeyWnd;			// ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������
	bool			bDispMiniMap;				// �~�j�}�b�v��\������
	bool			bMenuIcon;					// ���j���[�ɃA�C�R����\������ (�A�C�R���t�����j���[)
	bool			bScrollBarHorz;				// �����X�N���[���o�[���g��
	bool			bUseCompatibleBMP;			// �č��p�݊��r�b�g�}�b�v���g�� 2007.09.09 Moca

	// �ʒu�Ƒ傫���̐ݒ�
	WinSizeMode		eSaveWindowSize;			// �E�B���h�E�T�C�Y�p���E�Œ� WinSizeMode�ɏ����� 2004.05.13 Moca
	int				nWinSizeType;				// �傫���̎w��
	int				nWinSizeCX;					// ���ڎw�� ��
	int				nWinSizeCY;					// ���ڎw�� ����
	WinSizeMode		eSaveWindowPos;				// �E�B���h�E�ʒu�p���E�Œ� WinSizeMode�ɏ����� 2004.05.13 Moca
	int				nWinPosX;					// ���ڎw�� X���W
	int				nWinPosY;					// ���ڎw�� Y���W

	// �t�@���N�V�����L�[
	int				nFuncKeyWnd_Place;			// �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
	int				nFuncKeyWnd_GroupNum;		// 2002/11/04 Moca �t�@���N�V�����L�[�̃O���[�v�{�^����

	// ���[���[�E�s�ԍ�
	int				nRulerHeight;				// ���[���[����
	int				nRulerBottomSpace;			// ���[���[�ƃe�L�X�g�̌���
	int				nRulerType;					// ���[���[�̃^�C�v $$$���g�p���ۂ�
	int				nLineNumRightSpace;			// �s�ԍ��̉E�̃X�y�[�X Sep. 18, 2002 genta

	// �����E�B���h�E
	bool			bSplitterWndHScroll;		// �����E�B���h�E�̐����X�N���[���̓������Ƃ� 2001/06/20 asa-o
	bool			bSplitterWndVScroll;		// �����E�B���h�E�̐����X�N���[���̓������Ƃ� 2001/06/20 asa-o

	// �^�C�g���o�[
	TCHAR			szWindowCaptionActive[MAX_CAPTION_CONF_LEN];	// �^�C�g���o�[(�A�N�e�B�u��)
	TCHAR			szWindowCaptionInactive[MAX_CAPTION_CONF_LEN];	// �^�C�g���o�[(��A�N�e�B�u��)

	// INI���ݒ�̂�
	int				nVertLineOffset;			// �c���̕`����W�I�t�Z�b�g 2005.11.10 Moca

	// ����I��
	TCHAR			szLanguageDll[MAX_PATH];	// ����DLL�t�@�C����

	// �~�j�}�b�v
	int				nMiniMapFontSize;
	int				nMiniMapQuality;
	int				nMiniMapWidth;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �^�u�o�[                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// ����{�^��
enum class DispTabCloseType {
	No			= 0, // �Ȃ�
	Always		= 1, // ��ɕ\��
	Auto		= 2  // �����\��
};

enum class TabPosition {
	Top,
	Bottom,
	Left,
	Right,
	None = -1,
};

struct CommonSetting_TabBar {
	bool		bDispTabWnd;					// �^�u�E�B���h�E�\������	//@@@ 2003.05.31 MIK
	bool		bDispTabWndMultiWin;			// �^�u���܂Ƃ߂Ȃ�	//@@@ 2003.05.31 MIK
	bool		bTab_RetainEmptyWin;			// �Ō�̕���������ꂽ�Ƃ�(����)���c��
	bool		bTab_CloseOneWin;				// �^�u���[�h�ł��E�B���h�E�̕���{�^���Ō��݂̃t�@�C���̂ݕ���
	bool		bNewWindow;						// �O������N������Ƃ��͐V�����E�B���h�E�ŊJ��
	bool		bTabMultiLine;					// �^�u���i
	TabPosition	eTabPosition;					// �^�u�ʒu

	wchar_t		szTabWndCaption[MAX_CAPTION_CONF_LEN];	// �^�u�E�B���h�E�L���v�V����	//@@@ 2003.06.13 MIK
	bool		bSameTabWidth;					// �^�u�𓙕��ɂ���			//@@@ 2006.01.28 ryoji
	bool		bDispTabIcon;					// �^�u�ɃA�C�R����\������	//@@@ 2006.01.28 ryoji
	DispTabCloseType	dispTabClose;			// �^�u�ɕ���{�^����\������	//@@@ 2012.04.14 syat
	bool		bSortTabList;					// �^�u�ꗗ���\�[�g����	//@@@ 2006.03.23 fon
	bool		bTab_ListFull;					// �^�u�ꗗ���t���p�X�\������	//@@@ 2007.02.28 ryoji

	bool		bChgWndByWheel;					// �}�E�X�z�C�[���ŃE�B���h�E�؂�ւ�	//@@@ 2006.03.26 ryoji

	LOGFONT		lf;								// �^�u�t�H���g // 2011.12.01 Moca
	INT			nPointSize;						// �t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj
	int			nTabMaxWidth;					// �^�u���̍ő�l
	int			nTabMinWidth;					// �^�u���̍ŏ��l
	int			nTabMinWidthOnMulti;			// �^�u���̍ŏ��l(�^�u���i��)
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �ҏW                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// �t�@�C���_�C�A���O�̏����ʒu
enum EOpenDialogDir {
	OPENDIALOGDIR_CUR, // �J�����g�t�H���_
	OPENDIALOGDIR_MRU, // �ŋߎg�����t�H���_
	OPENDIALOGDIR_SEL, // �w��t�H���_
};

struct CommonSetting_Edit {
	// �R�s�[
	bool	bAddCRLFWhenCopy;			// �܂�Ԃ��s�ɉ��s��t���ăR�s�[
	bool	bEnableNoSelectCopy;		// �I���Ȃ��ŃR�s�[���\�ɂ��� 2007.11.18 ryoji
	bool	bCopyAndDisablSelection;	// �R�s�[������I������
	bool	bEnableLineModePaste;		// ���C�����[�h�\��t�����\�ɂ���  2007.10.08 ryoji
	bool	bConvertEOLPaste;			// ���s�R�[�h��ϊ����ē\��t����  2009.2.28 salarm

	// �h���b�O���h���b�v
	bool	bUseOLE_DragDrop;			// OLE�ɂ��h���b�O & �h���b�v���g��
	bool	bUseOLE_DropSource;			// OLE�ɂ��h���b�O���ɂ��邩

	// �㏑�����[�h
	bool	bNotOverWriteCRLF;			// ���s�͏㏑�����Ȃ�
	bool	bOverWriteFixMode;			// �������ɍ��킹�ăX�y�[�X���l�߂�
	bool	bOverWriteBoxDelete;		// �㏑�����[�h�ł̋�`���͂őI��͈͂��폜����

	// �N���b�J�u��URL
	bool	bJumpSingleClickURL;		// URL�̃V���O���N���b�N��Jump $$$���g�p
	bool	bSelectClickedURL;			// URL���N���b�N���ꂽ��I�����邩

	EOpenDialogDir	eOpenDialogDir;		// �t�@�C���_�C�A���O�̏����ʒu
	SFilePath	openDialogSelDir;		// �w��t�H���_

	bool	bEnableExtEol;				// NEL,PS,LS�����s�R�[�h�Ƃ��ė��p����
	bool	bBoxSelectLock;				// (��`�I��)�ړ��Ń��b�N����

	// (�_�C�A���O���ږ���)
	bool	bAutoColumnPaste;			// ��`�R�s�[�̃e�L�X�g�͏�ɋ�`�\��t��
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �t�@�C��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_File {
public:
	// �J�[�\���ʒu�𕜌����邩�ǂ���  Oct. 27, 2000 genta
	bool	GetRestoreCurPosition() const		{ return bRestoreCurPosition; }
	void	SetRestoreCurPosition(bool i)		{ bRestoreCurPosition = i; }

	// �u�b�N�}�[�N�𕜌����邩�ǂ���  2002.01.16 hor
	bool	GetRestoreBookmarks() const			{ return bRestoreBookmarks; }
	void	SetRestoreBookmarks(bool i)			{ bRestoreBookmarks = i; }

	// �t�@�C���ǂݍ��ݎ���MIME��decode���s����  Nov. 12, 2000 genta
	bool	GetAutoMIMEdecode() const			{ return bAutoMimeDecode; }
	void	SetAutoMIMEdecode(bool i)			{ bAutoMimeDecode = i; }

	// �O��ƕ����R�[�h���قȂ�Ƃ��ɖ₢���킹���s��  Oct. 03, 2004 genta
	bool	GetQueryIfCodeChange() const		{ return bQueryIfCodeChange; }
	void	SetQueryIfCodeChange(bool i)		{ bQueryIfCodeChange = i; }
	
	// �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������  Oct. 09, 2004 genta
	bool	GetAlertIfFileNotExist() const		{ return bAlertIfFileNotExist; }
	void	SetAlertIfFileNotExist(bool i)		{ bAlertIfFileNotExist = i; }

public:
	// �t�@�C���̔r�����䃂�[�h
	FileShareMode	nFileShareMode;				// �t�@�C���̔r�����䃂�[�h
	bool		bCheckFileTimeStamp;		// �X�V�̊Ď�
	int 		nAutoloadDelay;				// �����Ǎ����x��
	bool		bUneditableIfUnwritable;	// �㏑���֎~���o���͕ҏW�֎~�ɂ���

	// �t�@�C���̕ۑ�
	bool	bEnableUnmodifiedOverwrite;		// ���ύX�ł��㏑�����邩

	//�u���O��t���ĕۑ��v�Ńt�@�C���̎�ނ�[���[�U�[�w��]�̂Ƃ��̃t�@�C���ꗗ�\��
	// �t�@�C���ۑ��_�C�A���O�̃t�B���^�ݒ�	// 2006.11.16 ryoji
	bool	bNoFilterSaveNew;				// �V�K����ۑ����͑S�t�@�C���\��
	bool	bNoFilterSaveFile;				// �V�K�ȊO����ۑ����͑S�t�@�C���\��

	// �t�@�C���I�[�v��
	bool	bDropFileAndClose;				// �t�@�C�����h���b�v�����Ƃ��͕��ĊJ��
	int		nDropFileNumMax;				// ��x�Ƀh���b�v�\�ȃt�@�C����
	bool	bRestoreCurPosition;			// �t�@�C�����J�����Ƃ��J�[�\���ʒu�𕜌����邩
	bool	bRestoreBookmarks;				// �u�b�N�}�[�N�𕜌����邩�ǂ��� 2002.01.16 hor
	bool	bAutoMimeDecode;				// �t�@�C���ǂݍ��ݎ���MIME��decode���s����
	bool	bQueryIfCodeChange;				// �O��ƕ����R�[�h���قȂ�Ƃ��ɖ₢���킹���s�� Oct. 03, 2004 genta
	bool	bAlertIfFileNotExist;			// �J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������ Oct. 09, 2004 genta
	bool	bAlertIfLargeFile;				// �J�����Ƃ����t�@�C���T�C�Y���傫���ꍇ�Ɍx������
	int		nAlertFileSize;					// �x�����n�߂�t�@�C���T�C�Y(MB)
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �o�b�N�A�b�v                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// Aug. 15, 2000 genta
// Backup Flags
enum EBackupOptionFlag {
	BKUP_YEAR		= 32,
	BKUP_MONTH		= 16,
	BKUP_DAY		= 8,
	BKUP_HOUR		= 4,
	BKUP_MIN		= 2,
	BKUP_SEC		= 1,

	// Aug. 21, 2000 genta
	BKUP_AUTO		= 64,
};

struct CommonSetting_Backup {
public:
	// Aug. 15, 2000 genta
	// Backup�ݒ�̃A�N�Z�X�֐�
	int		GetBackupType(void) const			{ return nBackUpType; }
	void	SetBackupType(int n)				{ nBackUpType = n; }

	bool	GetBackupOpt(EBackupOptionFlag flag) const			{ return (flag & nBackUpType_Opt1) == flag; }
	void	SetBackupOpt(EBackupOptionFlag flag, bool value)	{ nBackUpType_Opt1 = value ? (flag | nBackUpType_Opt1) :  ((~flag) & nBackUpType_Opt1); }

	// �o�b�N�A�b�v��
	int		GetBackupCount(void) const			{ return nBackUpType_Opt2 & 0xffff; }
	void	SetBackupCount(int value)			{ nBackUpType_Opt2 = (nBackUpType_Opt2 & 0xffff0000) | (value & 0xffff); }

	// �o�b�N�A�b�v�̊g���q�擪����(1����)
	TCHAR	GetBackupExtChar(void) const		{ return (TCHAR)((nBackUpType_Opt2 >> 16) & 0xff) ; }
	void	SetBackupExtChar(int value)			{ nBackUpType_Opt2 = (nBackUpType_Opt2 & 0xff00ffff) | ((value & 0xff) << 16); }

	// Aug. 21, 2000 genta
	// ����Backup
	bool	IsAutoBackupEnabled(void) const		{ return GetBackupOpt(BKUP_AUTO); }
	void	EnableAutoBackup(bool flag)			{ SetBackupOpt(BKUP_AUTO, flag); }
	
	int		GetAutoBackupInterval(void) const	{ return nBackUpType_Opt3; }
	void	SetAutoBackupInterval(int i)		{ nBackUpType_Opt3 = i; }

	// Backup�ڍאݒ�̃A�N�Z�X�֐�
	int		GetBackupTypeAdv(void) const { return nBackUpType_Opt4; }
	void	SetBackupTypeAdv(int n) { nBackUpType_Opt4 = n; }

public:
	bool		bBackUp;					// �ۑ����Ƀo�b�N�A�b�v���쐬����
	bool		bBackUpDialog;				// �o�b�N�A�b�v�̍쐬�O�Ɋm�F
	bool		bBackUpFolder;				// �w��t�H���_�Ƀo�b�N�A�b�v���쐬����
	bool		bBackUpFolderRM;			// �w��t�H���_�Ƀo�b�N�A�b�v���쐬����(�����[�o�u�����f�B�A�̂�)
	SFilePath	szBackUpFolder;				// �o�b�N�A�b�v���쐬����t�H���_
	int 		nBackUpType;				// �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.*
	int 		nBackUpType_Opt1;			// �o�b�N�A�b�v�t�@�C�����F�I�v�V����1
	int 		nBackUpType_Opt2;			// �o�b�N�A�b�v�t�@�C�����F�I�v�V����2
	int 		nBackUpType_Opt3;			// �o�b�N�A�b�v�t�@�C�����F�I�v�V����3
	int 		nBackUpType_Opt4;			// �o�b�N�A�b�v�t�@�C�����F�I�v�V����4
	int 		nBackUpType_Opt5;			// �o�b�N�A�b�v�t�@�C�����F�I�v�V����5
	int 		nBackUpType_Opt6;			// �o�b�N�A�b�v�t�@�C�����F�I�v�V����6
	bool		bBackUpDustBox;				// �o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞��	//@@@ 2001.12.11 add MIK
	bool		bBackUpPathAdvanced;		// �o�b�N�A�b�v��t�H���_���ڍאݒ肷�� 20051107 aroka
	SFilePath	szBackUpPathAdvanced;		// �o�b�N�A�b�v���쐬����t�H���_�̏ڍאݒ� 20051107 aroka
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Format {
	// ���t����
	int			nDateFormatType;							// ���t�����̃^�C�v
	TCHAR		szDateFormat[MAX_DATETIMEFOREMAT_LEN];		// ���t����

	// ��������
	int			nTimeFormatType;							// ���������̃^�C�v
	TCHAR		szTimeFormat[MAX_DATETIMEFOREMAT_LEN];		// ��������

	// ���o���L��
	wchar_t		szMidashiKigou[256];						// ���o���L��

	// ���p��
	wchar_t		szInyouKigou[32];							// ���p��
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Search {
	
	int				nSearchKeySequence;			// �����V�[�P���X(���ۑ�)
	SearchOption	searchOption;				// �����^�u��  ����

	int				nReplaceKeySequence;		// �u����V�[�P���X(���ۑ�)
	bool			bConsecutiveAll;			//�u���ׂĒu���v�͒u���̌J�Ԃ�	// 2007.01.16 ryoji
	bool			bNotifyNotFound;			// �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\��
	bool			bSelectedArea;				// �u��  �I��͈͓��u��

	bool			bGrepSubFolder;				// Grep: �T�u�t�H���_������
	int				nGrepOutputLineType;		// Grep: �s���o��/�Y������/�ۃ}�b�`�s ���o��
	int				nGrepOutputStyle;			// Grep: �o�͌`��
	bool			bGrepDefaultFolder;			// Grep: �t�H���_�̏����l���J�����g�t�H���_�ɂ���
	EncodingType	nGrepCharSet;				// Grep: �����R�[�h�Z�b�g // 2002/09/20 Moca Add
	bool			bGrepOutputFileOnly;		// Grep: �t�@�C�����ŏ��̂݌���
	bool			bGrepOutputBaseFolder;		// Grep: �x�[�X�t�H���_�\��
	bool			bGrepSeparateFolder;		// Grep: �t�H���_���ɕ\��
	bool			bGrepBackup;				// Grep: �o�b�N�A�b�v�쐬
	
	bool			bCaretTextForSearch;		// �J�[�\���ʒu�̕�������f�t�H���g�̌���������ɂ��� 2006.08.23 ryoji
	bool			bInheritKeyOtherView;		// ���E�O�����ő��̃r���[�̌��������������p��
	TCHAR			szRegexpLib[_MAX_PATH];		// �g�p���鐳�K�\��DLL  2007.08.22 genta

	// Grep
	bool			bGrepExitConfirm;			// Grep���[�h�ŕۑ��m�F���邩
	bool			bGrepRealTimeView;			// Grep���ʂ̃��A���^�C���\�� 2003.06.16 Moca

	bool			bGTJW_Return;				// �G���^�[�L�[�Ń^�O�W�����v
	bool			bGTJW_DoubleClick;			// �_�u���N���b�N�Ń^�O�W�����v

	// �����E�u���_�C�A���O
	bool			bAutoCloseDlgFind;			// �����_�C�A���O�������I�ɕ���
	bool			bAutoCloseDlgReplace;		// �u�� �_�C�A���O�������I�ɕ���
	bool			bSearchAll;					// �擪�i�����j����Č��� 2002.01.26 hor

	int				nTagJumpMode;				// �^�O�W�����v���[�h(0-3)
	int				nTagJumpModeKeyword;		// �^�O�W�����v���[�h(0-3)

	// INI���ݒ�̂�
	bool			bUseCaretKeyword;			// �L�����b�g�ʒu�̒P�����������		// 2006.03.24 fon
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �L�[���蓖��                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_KeyBind {
	// �L�[���蓖��
	int					nKeyNameArrNum;					// �L�[���蓖�ĕ\�̗L���f�[�^��
	KeyData				pKeyNameArr[100 + 1];			// �L�[���蓖�ĕ\ �����蓖�ăL�[�R�[�h�p�Ƀ_�~�[��ǉ�
	BYTE				keyToKeyNameArr[256 + 10];		// �L�[�R�[�h�����蓖�ĕ\�C���f�b�N�X // 2012.11.25 aroka
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �J�X�^�����j���[                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_CustomMenu {
	WCHAR			szCustMenuNameArr   [MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_NAME_LEN + 1];
	int				nCustMenuItemNumArr [MAX_CUSTOM_MENU];
	EFunctionCode	nCustMenuItemFuncArr[MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
	KEYCODE			nCustMenuItemKeyArr [MAX_CUSTOM_MENU][MAX_CUSTOM_MENU_ITEMS];
	bool			bCustMenuPopupArr   [MAX_CUSTOM_MENU];
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �c�[���o�[                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_ToolBar {
	int		nToolBarButtonNum;								// �c�[���o�[�{�^���̐�
	int		nToolBarButtonIdxArr[MAX_TOOLBAR_BUTTON_ITEMS];	// �c�[���o�[�{�^���\����
	bool	bToolBarIsFlat;									// �t���b�g�c�[���o�[�ɂ���^���Ȃ�
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �����L�[���[�h                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_SpecialKeyword {
	// �����L�[���[�h�ݒ�
	KeywordSetMgr		keywordSetMgr;					// �����L�[���[�h
	char				szKeywordSetDir[MAX_PATH];		// �����L�[���[�h�t�@�C���̃f�B���N�g��
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �x��                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Helper {
	// ���͕⊮�@�\
	bool		bHokanKey_RETURN;				// VK_RETURN	�⊮����L�[���L��/����
	bool		bHokanKey_TAB;					// VK_TAB		�⊮����L�[���L��/����
	bool		bHokanKey_RIGHT;				// VK_RIGHT		�⊮����L�[���L��/����
	bool		bHokanKey_SPACE;				// VK_SPACE		�⊮����L�[���L��/���� $$$�قږ��g�p

	// �O���w���v�̐ݒ�
	TCHAR		szExtHelp[_MAX_PATH];			// �O���w���v�P

	// �O��HTML�w���v�̐ݒ�
	TCHAR		szExtHtmlHelp[_MAX_PATH];		// �O��HTML�w���v
	bool		bHtmlHelpIsSingle;				// HtmlHelp�r���[�A�͂ЂƂ� (�r���[�A�𕡐��N�����Ȃ�)

	// migemo�ݒ�
	TCHAR		szMigemoDll[_MAX_PATH];			// migemo dll
	TCHAR		szMigemoDict[_MAX_PATH];		// migemo dict

	// �L�[���[�h�w���v
	LOGFONT		lf;								// �L�[���[�h�w���v�̃t�H���g���		// ai 02/05/21 Add
	INT			nPointSize;						// �L�[���[�h�w���v�̃t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj	// 2009.10.01 ryoji

	// INI���ݒ�̂�
	bool		bUseHokan;						// ���͕⊮�@�\���g�p����
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �}�N��                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Macro {
	TCHAR		szKeyMacroFileName[MAX_PATH];	// �L�[�{�[�h�}�N���̃t�@�C����
	MacroRec	macroTable[MAX_CUSTMACRO];		// �L�[���蓖�ėp�}�N���e�[�u��	 Sep. 14, 2001 genta
	SFilePath	szMACROFOLDER;					// �}�N���p�t�H���_
	int			nMacroOnOpened;					// �I�[�v���㎩�����s�}�N���ԍ�		@@@ 2006.09.01 ryoji
	int			nMacroOnTypeChanged;			// �^�C�v�ύX�㎩�����s�}�N���ԍ�	@@@ 2006.09.01 ryoji
	int			nMacroOnSave;					// �ۑ��O�������s�}�N���ԍ�			@@@ 2006.09.01 ryoji
	int			nMacroCancelTimer;				// �}�N����~�_�C�A���O�\���҂�����	@@@ 2011.08.04 syat
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �t�@�C�����\��                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_FileName {
	bool	bTransformShortPath;											// �t�@�C�����̏ȗ��\�L
	int		nTransformShortMaxWidth;										// �t�@�C�����̏ȗ��\�L�̍ő咷
	int		nTransformFileNameArrNum;										// �t�@�C�����̊ȈՕ\���o�^��
	TCHAR	szTransformFileNameFrom[MAX_TRANSFORM_FILENAME][_MAX_PATH];		// �t�@�C�����̊ȈՕ\���ϊ��O������
	TCHAR	szTransformFileNameTo[MAX_TRANSFORM_FILENAME][_MAX_PATH];		// �t�@�C�����̊ȈՕ\���ϊ��㕶����	//@@@ 2003.04.08 MIK
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �A�E�g���C��                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �h�b�L���O�z�u
enum class DockSideType {
	Float,				// �t���[�e�B���O
	Left,				// ���h�b�L���O
	Top,				// ��h�b�L���O
	Right,				// �E�h�b�L���O
	Bottom,				// ���h�b�L���O
	Undockable = -1,	// �h�b�L���O�֎~
};

enum class FileTreeItemType {
	Grep,
	File,
	Folder
};

struct FileTreeItem {
public:
	FileTreeItemType eFileTreeItemType;
	SFilePath	szTargetPath;					// �t�H���_or�t�@�C���p�X
	StaticString<TCHAR,_MAX_PATH> szLabelName;	// ���x����(""�̂Ƃ��̓t�@�C�������g��)
	int  nDepth;	// �K�w

	// Grep�^�C�vTreeItem
	StaticString<TCHAR,_MAX_PATH>	szTargetFile;	// �t�@�C���ꗗ
	bool		bIgnoreHidden;		// �B���t�@�C��������
	bool		bIgnoreReadOnly;	// �ǂݎ���p�t�@�C��������
	bool		bIgnoreSystem;		// �V�X�e���t�@�C��������

	FileTreeItem()
		: eFileTreeItemType(FileTreeItemType::Grep)
		, nDepth(0)
		, bIgnoreHidden(true)
		, bIgnoreReadOnly(false)
		, bIgnoreSystem(false)
		{}
};

struct FileTree {
	bool		bProject;			// �v���W�F�N�g�t�@�C�����[�h
	SFilePath	szProjectIni;		// �f�t�H���gini�p�X
	int			nItemCount;			// �t�@�C���p�X��
	FileTreeItem	items[20];		// �c���[�A�C�e��
};


struct CommonSetting_OutLine {
	// 20060201 aroka �A�E�g���C��/�g�s�b�N���X�g �̈ʒu�ƃT�C�Y���L��
	bool		bRememberOutlineWindowPos;	// �A�E�g���C��/�g�s�b�N���X�g �̈ʒu�ƃT�C�Y���L������
	int			widthOutlineWindow;			// �A�E�g���C��/�g�s�b�N���X�g �̃T�C�Y(��)
	int			heightOutlineWindow;		// �A�E�g���C��/�g�s�b�N���X�g �̃T�C�Y(����)
	int			xOutlineWindowPos;			// �A�E�g���C��/�g�s�b�N���X�g �̈ʒu(X���W)
	int			yOutlineWindowPos;			// �A�E�g���C��/�g�s�b�N���X�g �̈ʒu(Y���W)

	int			nOutlineDockSet;			// �A�E�g���C����͂̃h�b�L���O�ʒu�p�����@(0:���ʐݒ�, 1:�^�C�v�ʐݒ�)
	bool		bOutlineDockSync;			// �A�E�g���C����͂̃h�b�L���O�ʒu�𓯊�����
	bool		bOutlineDockDisp;			// �A�E�g���C����͕\���̗L��
	DockSideType	eOutlineDockSide;		// �A�E�g���C����̓h�b�L���O�z�u
	int			cxOutlineDockLeft;			// �A�E�g���C���̍��h�b�L���O��
	int			cyOutlineDockTop;			// �A�E�g���C���̏�h�b�L���O��
	int			cxOutlineDockRight;			// �A�E�g���C���̉E�h�b�L���O��
	int			cyOutlineDockBottom;		// �A�E�g���C���̉��h�b�L���O��
	enum class OutlineType nDockOutline;	// �A�E�g���C���^�C�v

	// IDD_FUNCLIST (�c�[�� - �A�E�g���C�����)
	bool		bAutoCloseDlgFuncList;		// �A�E�g���C���_�C�A���O�������I�ɕ���
	bool		bFunclistSetFocusOnJump;	// �t�H�[�J�X���ڂ� 2002.02.08 hor
	bool		bMarkUpBlankLineEnable;	// ��s�𖳎����� 2002.02.08 aroka,hor

	FileTree	fileTree;					// �t�@�C���c���[�ݒ�
	SFilePath	fileTreeDefIniName;			// �t�@�C���c���[�ݒ�̃f�t�H���g�t�@�C����(GUI�Ȃ�)
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �t�@�C�����e��r                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Compare {
	// �t�@�C�����e��r�_�C�A���O
	bool		bCompareAndTileHorz;		// ������r��A���E�ɕ��ׂĕ\��
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �r���[                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_View {
	// INI���ݒ�̂�
	LOGFONT		lf;						// ���݂̃t�H���g���
	bool		bFontIs_FixedPitch;		// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
	INT			nPointSize;				// �t�H���g�T�C�Y�i1/10�|�C���g�P�ʁj	// 2009.10.01 ryoji
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���̑�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
struct CommonSetting_Others {
	// INI���ݒ�̂�
	RECT		rcOpenDialog;				//�u�J���v�_�C�A���O�̃T�C�Y�ƈʒu
	RECT		rcCompareDialog;			//�u�t�@�C����r�v�_�C�A���O�{�b�N�X�̃T�C�Y�ƈʒu
	RECT		rcDiffDialog;				//�uDIFF�����\���v�_�C�A���O�{�b�N�X�̃T�C�Y�ƈʒu
	RECT		rcFavoriteDialog;			//�u�����Ƃ��C�ɓ���̊Ǘ��v�_�C�A���O�{�b�N�X�̃T�C�Y�ƈʒu
	RECT		rcTagJumpDialog;			//�u�_�C���N�g�^�O�W�����v���ꗗ�v�_�C�A���O�{�b�N�X�̃T�C�Y�ƈʒu
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �X�e�[�^�X�o�[                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// 2008/6/21	Uchi
struct CommonSetting_StatusBar {
	// �\�������R�[�h�̎w��
	bool		bDispUniInSjis;				// SJIS�ŕ����R�[�h�l��Unicode�ŕ\������
	bool		bDispUniInJis;				// JIS�ŕ����R�[�h�l��Unicode�ŕ\������
	bool		bDispUniInEuc;				// EUC�ŕ����R�[�h�l��Unicode�ŕ\������
	bool		bDispUtf8Codepoint;			// UTF-8���R�[�h�|�C���g�ŕ\������
	bool		bDispSPCodepoint;			// �T���Q�[�g�y�A���R�[�h�|�C���g�ŕ\������
	bool		bDispSelCountByByte;		// �I�𕶎����𕶎��P�ʂł͂Ȃ��o�C�g�P�ʂŕ\������
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �v���O�C��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �v���O�C�����
enum EPluginState {
	PLS_NONE,			// �v���O�C���e�[�u���ɓo�^���Ȃ�
	PLS_INSTALLED,		// �ǉ����ꂽ
	PLS_UPDATED,		// �X�V���ꂽ
	PLS_STOPPED,		// ��~���Ă���
	PLS_LOADED,			// �ǂݍ��܂ꂽ
	PLS_DELETED			// �폜���ꂽ
};

struct PluginRec {
	WCHAR			szId[MAX_PLUGIN_ID];		// �v���O�C��ID
	WCHAR			szName[MAX_PLUGIN_NAME];	// �v���O�C���t�H���_/�ݒ�t�@�C����
	EPluginState	state;						// �v���O�C����ԁB�ݒ�t�@�C���ɕۑ�������������̂݁B
	int 			nCmdNum;					// �v���O�C�� �R�}���h�̐�	// 2010/7/3 Uchi
};

struct CommonSetting_Plugin {
	bool			bEnablePlugin;				// �v���O�C�����g�p���邩�ǂ���
	PluginRec		pluginTable[MAX_PLUGIN];	// �v���O�C���e�[�u��
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���C�����j���[                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// ���C�����j���[���
enum class MainMenuType {
	Node,			// Node
	Leaf,			// �@�\�R�}���h
	Separator,		// ��ؐ�
	Special,		// ����@�\�R�}���h
}; 

class MainMenu {
public:
	MainMenuType	type;		// ���
	EFunctionCode	nFunc;		// Function
	WCHAR			sKey[2];	// �A�N�Z�X�L�[
	WCHAR			sName[MAX_MAIN_MENU_NAME_LEN + 1];	// ���O
	int 			nLevel;		// ���x��
};

struct CommonSetting_MainMenu {
	int				nVersion;							// ���C�����j���[�o�[�W����
	int				nMenuTopIdx[MAX_MAINMENU_TOP];		// ���C�����j���[�g�b�v���x��
	int 			nMainMenuNum;						// ���C�����j���[�f�[�^�̐�
	MainMenu		mainMenuTbl[MAX_MAINMENU];			// ���C�����j���[�f�[�^
	bool 			bMainMenuKeyParentheses;			// �A�N�Z�X�L�[��()�t�ŕ\��
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                                                             //
//                          �܂Ƃ�                             //
//                                                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ���ʐݒ�
struct CommonSetting {
	CommonSetting_General			general;			// �S��
	CommonSetting_Window			window;				// �E�B���h�E
	CommonSetting_TabBar			tabBar;				// �^�u�o�[
	CommonSetting_Edit				edit;				// �ҏW
	CommonSetting_File				file;				// �t�@�C��
	CommonSetting_Backup			backup;				// �o�b�N�A�b�v
	CommonSetting_Format			format;				// ����
	CommonSetting_Search			search;				// ����
	CommonSetting_KeyBind			keyBind;			// �L�[���蓖��
	//
	CommonSetting_CustomMenu		customMenu;			// �J�X�^�����j���[
	CommonSetting_ToolBar			toolBar;			// �c�[���o�[
	CommonSetting_SpecialKeyword	specialKeyword;		// �����L�[���[�h
	CommonSetting_Helper			helper;				// �x��
	CommonSetting_Macro				macro;				// �}�N��
	CommonSetting_FileName			fileName;			// �t�@�C�����\��
	//
	CommonSetting_OutLine			outline;			// �A�E�g���C��
	CommonSetting_Compare			compare;			// �t�@�C�����e��r
	CommonSetting_View				view;				// �r���[
	CommonSetting_Others			others;				// ���̑�

	//
	CommonSetting_StatusBar			statusBar;			// �X�e�[�^�X�o�[		// 2008/6/21 Uchi
	CommonSetting_Plugin			plugin;				// �v���O�C�� 2009/11/30 syat
	CommonSetting_MainMenu			mainMenu;			// ���C�����j���[		// 2010/5/15 Uchi
};

