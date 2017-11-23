/*!	@file
	@brief �풓��

	�^�X�N�g���C�A�C�R���̊Ǘ��C�^�X�N�g���C���j���[�̃A�N�V�����C
	MRU�A�L�[���蓖�āA���ʐݒ�A�ҏW�E�B���h�E�̊Ǘ��Ȃ�
*/

#pragma once

#include <Windows.h>
#include "uiparts/MenuDrawer.h"
#include "uiparts/ImageListMgr.h"
#include "dlg/DlgGrep.h"

struct LoadInfo;
struct EditInfo;
struct DllSharedData;
class PropertyManager;

// �풓���̊Ǘ�
/*!
	�^�X�N�g���C�A�C�R���̊Ǘ��C�^�X�N�g���C���j���[�̃A�N�V�����C
	MRU�A�L�[���蓖�āA���ʐݒ�A�ҏW�E�B���h�E�̊Ǘ��Ȃ�
*/
class ControlTray {
public:
	/*
	||  Constructors
	*/
	ControlTray();
	~ControlTray();

	/*
	|| �����o�֐�
	*/
	HWND Create(HINSTANCE);	// �쐬
	bool CreateTrayIcon(HWND);
	LRESULT DispatchEvent(HWND, UINT, WPARAM, LPARAM);	// ���b�Z�[�W����
	void MessageLoop(void);	// ���b�Z�[�W���[�v
	void OnDestroy(void);		// WM_DESTROY ����
	int	CreatePopUpMenu_L(void);	// �|�b�v�A�b�v���j���[(�g���C���{�^��)
	int	CreatePopUpMenu_R(void);	// �|�b�v�A�b�v���j���[(�g���C�E�{�^��)
	void CreateAccelTbl(void);	// �A�N�Z�����[�^�e�[�u���쐬
	void DeleteAccelTbl(void);	// �A�N�Z�����[�^�e�[�u���j��

	// �E�B���h�E�Ǘ�
	static bool OpenNewEditor(							// �V�K�ҏW�E�B���h�E�̒ǉ� ver 0
		HINSTANCE			hInstance,					// [in] �C���X�^���XID (���͖��g�p)
		HWND				hWndParent,					// [in] �e�E�B���h�E�n���h���D�G���[���b�Z�[�W�\���p
		const LoadInfo&		loadInfo,					// [in]
		const TCHAR*		szCmdLineOption	= NULL,		// [in] �ǉ��̃R�}���h���C���I�v�V����
		bool				sync			= false,	// [in] true�Ȃ�V�K�G�f�B�^�̋N���܂őҋ@����
		const TCHAR*		pszCurDir		= NULL,		// [in] �V�K�G�f�B�^�̃J�����g�f�B���N�g��
		bool				bNewWindow		= false		// [in] �V�K�G�f�B�^���E�B���h�E�ŊJ��
	);
	static bool OpenNewEditor2(						// �V�K�ҏW�E�B���h�E�̒ǉ� ver 1
		HINSTANCE		hInstance,
		HWND			hWndParent,
		const EditInfo&	editInfo,
		bool			bViewMode,
		bool			sync		= false,
		bool			bNewWindow	= false
	);
	static void ActiveNextWindow(HWND hwndParent);
	static void ActivePrevWindow(HWND hwndParent);

	static bool CloseAllEditor(bool bCheckConfirm, HWND hWndFrom, bool bExit, int nGroup);	// ���ׂẴE�B���h�E�����
	static void TerminateApplication(HWND hWndFrom);	// �T�N���G�f�B�^�̑S�I��

public:
	HWND GetTrayHwnd() const { return hWnd; }

	/*
	|| �����w���p�n
	*/
	static void DoGrepCreateWindow(HINSTANCE hinst, HWND, DlgGrep& dlgGrep);
protected:
	void DoGrep();
	BOOL TrayMessage(HWND, DWORD, UINT, HICON, const TCHAR*);	// �^�X�N�g���C�̃A�C�R���Ɋւ��鏈��
	void OnCommand(WORD, WORD, HWND);	// WM_COMMAND���b�Z�[�W����
	void OnNewEditor(bool); // �V�K�E�B���h�E�쐬����

	static INT_PTR CALLBACK ExitingDlgProc(	// �I���_�C�A���O�p�v���V�[�W��
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam		// second message parameter
	);

	/*
	|| �����o�ϐ�
	*/
private:
	MenuDrawer			menuDrawer;
	PropertyManager*	pPropertyManager;
	bool				bUseTrayMenu;			// �g���C���j���[�\����
	HINSTANCE			hInstance;
	HWND				hWnd;
	bool				bCreatedTrayIcon;		// �g���C�ɃA�C�R���������

	DllSharedData*		pShareData;
	DlgGrep				dlgGrep;
	int					nCurSearchKeySequence;

	ImageListMgr		hIcons;

	UINT				uCreateTaskBarMsg;	// RegisterMessage�œ�����Message ID�̕ۊǏꏊ

	TCHAR				szLanguageDll[MAX_PATH];
};

