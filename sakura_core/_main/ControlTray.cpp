
/*!	@file
	@brief �풓��
	
	�^�X�N�g���C�A�C�R���̊Ǘ��C�^�X�N�g���C���j���[�̃A�N�V�����C
	MRU�A�L�[���蓖�āA���ʐݒ�A�ҏW�E�B���h�E�̊Ǘ��Ȃ�

	@author Norio Nakatani
	@date 1998/05/13 �V�K�쐬
	@date 2001/06/03 N.Nakatani grep�P��P�ʂŌ�������������Ƃ��̂��߂ɃR�}���h���C���I�v�V�����̏����ǉ�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, Stonee, jepro, genta, aroka, hor, YAZAKI
	Copyright (C) 2002, MIK, Moca, genta, YAZAKI, towest
	Copyright (C) 2003, MIK, Moca, KEITA, genta, aroka
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#define ID_HOTKEY_TRAYMENU	0x1234

#include <HtmlHelp.h>
#include "ControlTray.h"
#include "PropertyManager.h"
#include "typeprop/DlgTypeList.h"
#include "debug/RunningTimer.h"
#include "dlg/DlgOpenFile.h"
#include "dlg/DlgAbout.h"		// Nov. 21, 2000 JEPROtest
#include "plugin/PluginManager.h"
#include "plugin/JackManager.h"
#include "io/TextStream.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/string_ex2.h"
#include "env/ShareData.h"
#include "env/ShareData_IO.h"
#include "env/SakuraEnvironment.h"
#include "env/HelpManager.h"
#include "doc/DocListener.h" // LoadInfo,EditInfo
#include "recent/MRUFile.h"
#include "recent/MRUFolder.h"
#include "_main/CommandLine.h"
#include "sakura_rc.h"

#define IDT_EDITCHECK 2
// 3�b
#define IDT_EDITCHECK_INTERVAL 3000
/////////////////////////////////////////////////////////////////////////
static LRESULT CALLBACK ControlTrayWndProc(HWND, UINT, WPARAM, LPARAM);

static ControlTray* g_pControlTray;

// Stonee, 2001/03/21
// Stonee, 2001/07/01  ���d�N�����ꂽ�ꍇ�͑O��̃_�C�A���O��O�ʂɏo���悤�ɂ����B
void ControlTray::DoGrep()
{
	// Stonee, 2001/06/30
	// �O��̃_�C�A���O������ΑO�ʂ� (suggested by genta)
	if (::IsWindow(dlgGrep.GetHwnd())) {
		::OpenIcon(dlgGrep.GetHwnd());
		::BringWindowToTop(dlgGrep.GetHwnd());
		return;
	}

	auto& searchKeywords = pShareData->searchKeywords;
	auto& csSearch = pShareData->common.search;
	if (0 < searchKeywords.searchKeys.size()
		&& nCurSearchKeySequence < csSearch.nSearchKeySequence
	) {
		dlgGrep.strText = searchKeywords.searchKeys[0];
	}
	if (0 < searchKeywords.grepFiles.size()) {
		_tcscpy(dlgGrep.szFile, searchKeywords.grepFiles[0]);		// �����t�@�C��
	}
	if (0 < searchKeywords.grepFolders.size()) {
		_tcscpy(dlgGrep.szFolder, searchKeywords.grepFolders[0]);	// �����t�H���_
	}

	// Grep�_�C�A���O�̕\��
	INT_PTR nRet = dlgGrep.DoModal(hInstance, NULL, _T(""));
	if (!nRet || !GetTrayHwnd()) {
		return;
	}
	nCurSearchKeySequence = csSearch.nSearchKeySequence;
	DoGrepCreateWindow(hInstance, pShareData->handles.hwndTray, dlgGrep);
}

void ControlTray::DoGrepCreateWindow(HINSTANCE hinst, HWND msgParent, DlgGrep& dlgGrep)
{
	// ======= Grep�̎��s =============
	// Grep���ʃE�B���h�E�̕\��

	NativeT mWork1;
	NativeT mWork2;
	NativeT mWork3;
	mWork1.SetString(dlgGrep.strText.c_str());
	mWork2.SetString(dlgGrep.szFile);
	mWork3.SetString(dlgGrep.szFolder);
	mWork1.Replace(_T("\""), _T("\"\""));
	mWork2.Replace(_T("\""), _T("\"\""));
	mWork3.Replace(_T("\""), _T("\"\""));

	// -GREPMODE -GKEY="1" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
	NativeT cmdLine;
	cmdLine.AppendStringLiteral(_T("-GREPMODE -GKEY=\""));
	cmdLine.AppendStringT(mWork1.GetStringPtr());
	cmdLine.AppendStringLiteral(_T("\" -GFILE=\""));
	cmdLine.AppendStringT(mWork2.GetStringPtr());
	cmdLine.AppendStringLiteral(_T("\" -GFOLDER=\""));
	cmdLine.AppendStringT(mWork3.GetStringPtr());
	cmdLine.AppendStringLiteral(_T("\" -GCODE="));
	TCHAR szTemp[20];
	auto_sprintf_s(szTemp, _T("%d"), dlgGrep.nGrepCharSet);
	cmdLine.AppendString(szTemp);

	// GOPT�I�v�V����
	TCHAR pOpt[64] = _T("");
	if (dlgGrep.bSubFolder			) _tcscat(pOpt, _T("S"));	// �T�u�t�H���_�������������
	if (dlgGrep.searchOption.bLoHiCase	) _tcscat(pOpt, _T("L"));	// �p�啶���Ɖp����������ʂ���
	if (dlgGrep.searchOption.bRegularExp) _tcscat(pOpt, _T("R"));	// ���K�\��
	if (dlgGrep.nGrepOutputLineType == 1) _tcscat(pOpt, _T("P"));	// �s���o�͂���
	if (dlgGrep.nGrepOutputLineType == 2) _tcscat(pOpt, _T("N"));	// �ۃq�b�g�s���o�͂��� 2014.09.23
	if (dlgGrep.searchOption.bWordOnly	) _tcscat(pOpt, _T("W"));	// �P��P�ʂŒT��
	if (dlgGrep.nGrepOutputStyle == 1	) _tcscat(pOpt, _T("1"));	// Grep: �o�͌`��
	if (dlgGrep.nGrepOutputStyle == 2	) _tcscat(pOpt, _T("2"));	// Grep: �o�͌`��
	if (dlgGrep.nGrepOutputStyle == 3	) _tcscat(pOpt, _T("3"));
	if (dlgGrep.bGrepOutputFileOnly		) _tcscat(pOpt, _T("F"));
	if (dlgGrep.bGrepOutputBaseFolder	) _tcscat(pOpt, _T("B"));
	if (dlgGrep.bGrepSeparateFolder		) _tcscat(pOpt, _T("D"));
	if (pOpt[0] != _T('\0')) {
		cmdLine.AppendStringLiteral(_T(" -GOPT="));
		cmdLine.AppendString(pOpt);
	}

	// �V�K�ҏW�E�B���h�E�̒ǉ� ver 0
	LoadInfo loadInfo;
	loadInfo.filePath = _T("");
	loadInfo.eCharCode = CODE_NONE;
	loadInfo.bViewMode = false;
	OpenNewEditor(
		hinst,
		msgParent,
		loadInfo,
		cmdLine.GetStringPtr(),
		false,
		NULL,
		GetDllShareData().common.tabBar.bNewWindow
	);
}


// �E�B���h�E�v���V�[�W������
static LRESULT CALLBACK ControlTrayWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	ControlTray* pApp;
	switch (uMsg) {
	case WM_CREATE:
		pApp = (ControlTray*)g_pControlTray;
		return pApp->DispatchEvent(hwnd, uMsg, wParam, lParam);
	default:
		// Modified by KEITA for WIN64 2003.9.6
		// RELPRINT(_T("dispatch\n"));
		pApp = (ControlTray*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (pApp) {
			return pApp->DispatchEvent(hwnd, uMsg, wParam, lParam);
		}
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}


/////////////////////////////////////////////////////////////////////////////
// ControlTray
// @date 2002.2.17 YAZAKI ShareData�̃C���X�^���X�́AProcess�ɂЂƂ���̂݁B
ControlTray::ControlTray()
// Apr. 24, 2001 genta
	:
	pPropertyManager(nullptr),
	hInstance(NULL),
	hWnd(NULL),
	bCreatedTrayIcon(false),	// �g���C�ɃA�C�R���������
	nCurSearchKeySequence(-1),
	uCreateTaskBarMsg(::RegisterWindowMessage(TEXT("TaskbarCreated")))
{
	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	pShareData = &GetDllShareData();

	// �A�N�Z�����[�^�e�[�u���쐬
	CreateAccelTbl();

	bUseTrayMenu = false;

	return;
}


ControlTray::~ControlTray()
{
	delete pPropertyManager;
	return;
}

/////////////////////////////////////////////////////////////////////////////
// ControlTray �����o�֐�


// �쐬
HWND ControlTray::Create(HINSTANCE hInstance)
{
	MY_RUNNINGTIMER(runningTimer, "ControlTray::Create");

	// �������N���X�̃E�B���h�E�����ɑ��݂��Ă�����A���s
	hInstance = hInstance;
	std::tstring strProfileName = to_tchar(CommandLine::getInstance().GetProfileName());
	std::tstring strCEditAppName = GSTR_CEDITAPP;
	strCEditAppName += strProfileName;
	HWND hwndWork = ::FindWindow(strCEditAppName.c_str(), strCEditAppName.c_str());
	if (hwndWork) {
		return NULL;
	}

	// �E�B���h�E�N���X�o�^
	WNDCLASS wc;
	{
		wc.style			=	CS_HREDRAW |
								CS_VREDRAW |
								CS_DBLCLKS |
								CS_BYTEALIGNCLIENT |
								CS_BYTEALIGNWINDOW;
		wc.lpfnWndProc		= ControlTrayWndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= hInstance;
		wc.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName		= NULL;
		wc.lpszClassName	= strCEditAppName.c_str();
		ATOM atom = RegisterClass(&wc);
		if (atom == 0) {
			ErrorMessage(NULL, LS(STR_TRAY_CREATE));
		}
	}
	g_pControlTray = this;

	// �E�B���h�E�쐬 (WM_CREATE�ŁAGetHwnd() �� HWND ���i�[�����)
	::CreateWindow(
		strCEditAppName.c_str(),			// pointer to registered class name
		strCEditAppName.c_str(),			// pointer to window name
		WS_OVERLAPPEDWINDOW/*WS_VISIBLE *//*| WS_CHILD *//* | WS_CLIPCHILDREN*/	,	// window style
		CW_USEDEFAULT,						// horizontal position of window
		0,									// vertical position of window
		100,								// window width
		100,								// window height
		NULL,								// handle to parent or owner window
		NULL,								// handle to menu or child-window identifier
		hInstance,						// handle to application instance
		NULL								// pointer to window-creation data
	);

	// �őO�ʂɂ���i�g���C����̃|�b�v�A�b�v�E�B���h�E���őO�ʂɂȂ�悤�Ɂj
	::SetWindowPos(
		GetTrayHwnd(),
		HWND_TOPMOST,
		0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE
	);
	
	// �^�X�N�g���C�A�C�R���쐬
	hIcons.Create(hInstance);	// Oct. 16, 2000 genta
	menuDrawer.Create(SelectLang::getLangRsrcInstance(), GetTrayHwnd(), &hIcons);
	if (GetTrayHwnd()) {
		CreateTrayIcon(GetTrayHwnd());
	}

	pPropertyManager = new PropertyManager();
	pPropertyManager->Create(GetTrayHwnd(), &hIcons, &menuDrawer);

	auto_strcpy(szLanguageDll, pShareData->common.window.szLanguageDll);

	return GetTrayHwnd();
}

// �^�X�N�g���C�ɃA�C�R����o�^����
bool ControlTray::CreateTrayIcon(HWND hWnd)
{
	// �^�X�N�g���C�̃A�C�R�������
	if (pShareData->common.general.bUseTaskTray) {	// �^�X�N�g���C�̃A�C�R�����g��
		// Dec. 02, 2002 genta
		HICON hIcon = GetAppIcon(hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true);
// From Here Jan. 12, 2001 JEPRO �g���C�A�C�R���Ƀ|�C���g����ƃo�[�W����no.���\�������悤�ɏC��
//			TrayMessage(GetTrayHwnd(), NIM_ADD, 0,  hIcon, GSTR_APPNAME);
		// �o�[�W�������
		// UR version no.��ݒ� (cf. dlgAbout.cpp)
		TCHAR	pszTips[64 + _MAX_PATH];
		// 2004.05.13 Moca �o�[�W�����ԍ��́A�v���Z�X���ƂɎ擾����
		DWORD dwVersionMS, dwVersionLS;
		GetAppVersionInfo(
			NULL,
			VS_VERSION_INFO,
			&dwVersionMS,
			&dwVersionLS
		);

		std::wstring profname;
		if (CommandLine::getInstance().GetProfileName()[0] != L'\0') {
			profname = L" ";
			profname += CommandLine::getInstance().GetProfileName();
		}
		auto_snprintf_s(
			pszTips,
			_countof(pszTips),
			_T("%ts %d.%d.%d.%d%ls"),		//Jul. 06, 2001 jepro UR �͂����t���Ȃ��Ȃ����̂�Y��Ă���
			GSTR_APPNAME,
			HIWORD(dwVersionMS),
			LOWORD(dwVersionMS),
			HIWORD(dwVersionLS),
			LOWORD( dwVersionLS ),
			profname.c_str()
		);
		TrayMessage(GetTrayHwnd(), NIM_ADD, 0,  hIcon, pszTips);
// To Here Jan. 12, 2001
		bCreatedTrayIcon = true;	// �g���C�ɃA�C�R���������
	}
	return true;
}


// ���b�Z�[�W���[�v
void ControlTray::MessageLoop(void)
{
	// �����v���Z�X��
	MSG	msg;
	int ret;
	
	// 2004.02.17 Moca GetMessage�̃G���[�`�F�b�N
	while (GetTrayHwnd() && (ret = ::GetMessage(&msg, NULL, 0, 0 )) != 0) {
		if (ret == -1) {
			break;
		}
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	return;

}


// �^�X�N�g���C�̃A�C�R���Ɋւ��鏈��
BOOL ControlTray::TrayMessage(
	HWND hDlg,
	DWORD dwMessage,
	UINT uID,
	HICON hIcon,
	const TCHAR* pszTip
	)
{
	NOTIFYICONDATA	tnd;
	tnd.cbSize				= sizeof_raw(tnd);
	tnd.hWnd				= hDlg;
	tnd.uID					= uID;
	tnd.uFlags				= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	tnd.uCallbackMessage	= MYWM_NOTIFYICON;
	tnd.hIcon				= hIcon;
	if (pszTip) {
		lstrcpyn(tnd.szTip, pszTip, _countof(tnd.szTip));
	}else {
		tnd.szTip[0] = _T('\0');
	}
	BOOL res = Shell_NotifyIcon(dwMessage, &tnd);
	if (hIcon) {
		DestroyIcon(hIcon);
	}
	return res;
}


// ���b�Z�[�W����
// @@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
LRESULT ControlTray::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	int			nId;
	HWND		hwndWork;
	LPHELPINFO	lphi;

	EditNode*	pEditNodeArr;
	static HWND	hwndHtmlHelp;

	static WORD		wHotKeyMods;
	static WORD		wHotKeyCode;
	LPMEASUREITEMSTRUCT	lpmis;	// ���ڃT�C�Y���
	LPDRAWITEMSTRUCT	lpdis;	// ���ڕ`����
	int					nItemWidth;
	int					nItemHeight;
	static bool			bLDClick = false;	// ���_�u���N���b�N�������� 03/02/20 ai

	switch (uMsg) {
	case WM_MENUCHAR:
		// ���j���[�A�N�Z�X�L�[�������̏���(WM_MENUCHAR����)
		return menuDrawer.OnMenuChar(hwnd, uMsg, wParam, lParam);
	case WM_DRAWITEM:
		lpdis = (DRAWITEMSTRUCT*) lParam;	// ���ڕ`����
		switch (lpdis->CtlType) {
		case ODT_MENU:	// �I�[�i�[�`�惁�j���[
			// ���j���[�A�C�e���`��
			menuDrawer.DrawItem(lpdis);
			return TRUE;
		}
		return FALSE;
	case WM_MEASUREITEM:
		lpmis = (MEASUREITEMSTRUCT*) lParam;	// item-size information
		switch (lpmis->CtlType) {
		case ODT_MENU:	// �I�[�i�[�`�惁�j���[
			// ���j���[�A�C�e���̕`��T�C�Y���v�Z
			nItemWidth = menuDrawer.MeasureItem(lpmis->itemID, &nItemHeight);
			if (0 < nItemWidth) {
				lpmis->itemWidth = nItemWidth;
				lpmis->itemHeight = nItemHeight;
			}
			return TRUE;
		}
		return FALSE;
	case WM_EXITMENULOOP:
		menuDrawer.EndDrawMenu();
		break;

	// �^�X�N�g���C���N���b�N���j���[�ւ̃V���[�g�J�b�g�L�[�o�^
	case WM_HOTKEY:
		{
			int		idHotKey = (int) wParam;				// identifier of hot key
			UINT	fuModifiers = (UINT) LOWORD(lParam);	// key-modifier flags
			UINT	uVirtKey = (UINT) HIWORD(lParam);		// virtual-key code
			TCHAR	szClassName[100];
			TCHAR	szText[256];

			hwndWork = ::GetForegroundWindow();
			szClassName[0] = L'\0';
			::GetClassName(hwndWork, szClassName, _countof(szClassName) - 1);
			::GetWindowText(hwndWork, szText, _countof(szText) - 1);
			if (_tcscmp(szText, LS(STR_PROPCOMMON)) == 0) {
				return -1;
			}

			if (idHotKey == ID_HOTKEY_TRAYMENU
				&& (wHotKeyMods) == fuModifiers
				&& wHotKeyCode == uVirtKey
			) {
				// Jan. 1, 2003 AROKA
				// �^�X�N�g���C���j���[�̕\���^�C�~���O��LBUTTONDOWN��LBUTTONUP�ɕύX�������Ƃɂ��
				::PostMessage(GetTrayHwnd(), MYWM_NOTIFYICON, 0, WM_LBUTTONUP);
			}
		}
		return 0;

	case WM_TIMER:
		// �^�C�}���b�Z�[�W
		if (wParam == IDT_EDITCHECK) {
			// 2010.08.26 �E�B���h�E���݊m�F�B�������E�B���h�E�𖕏�����
			bool bDelete = false;
			bool bDelFound;
			auto& nodes = pShareData->nodes;
			do {
				bDelFound = false;
				for (int i=0; i<nodes.nEditArrNum; ++i) {
					HWND target = nodes.pEditArr[i].GetHwnd();
					if (!IsSakuraMainWindow(target)) {
						AppNodeGroupHandle(nodes.pEditArr[i].nGroup).DeleteEditWndList(target);
						bDelete = bDelFound = true;
						// 1�폜�������蒼��
						break;
					}
				}
			}while (bDelFound);
			if (bDelete && nodes.nEditArrNum == 0) {
				PostMessage(hwnd, MYWM_DELETE_ME, 0, 0);
			}
		}
		return 0;

	case MYWM_UIPI_CHECK:
		// �G�f�B�^�|�g���C�Ԃł�UI���������̊m�F���b�Z�[�W 	 2007.06.07 ryoji
		::SendMessage((HWND)lParam, MYWM_UIPI_CHECK,  (WPARAM)0, (LPARAM)0);	// �Ԏ���Ԃ�
		return 0L;

	case MYWM_HTMLHELP:
		{
			TCHAR* pWork = pShareData->workBuffer.GetWorkBuffer<TCHAR>();
			// szHtmlFile�擾
			TCHAR szHtmlHelpFile[1024];
			_tcscpy_s(szHtmlHelpFile, pWork);
			size_t nLen = _tcslen(szHtmlHelpFile);
			// Jul. 6, 2001 genta HtmlHelp�̌Ăяo�����@�ύX
			hwndHtmlHelp = OpenHtmlHelp(
				NULL,
				szHtmlHelpFile,
				HH_DISPLAY_TOPIC,
				(DWORD_PTR)0,
				true
			);
			HH_AKLINK link;
			link.cbStruct		= sizeof_raw(link);
			link.fReserved		= FALSE;
			link.pszKeywords	= to_tchar(&pWork[nLen + 1]);
			link.pszUrl			= NULL;
			link.pszMsgText		= NULL;
			link.pszMsgTitle	= NULL;
			link.pszWindow		= NULL;
			link.fIndexOnFail	= TRUE;
			// Jul. 6, 2001 genta HtmlHelp�̌Ăяo�����@�ύX
			hwndHtmlHelp = OpenHtmlHelp(
				NULL,
				szHtmlHelpFile,
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link,
				false
			);
		}
		return (LRESULT)hwndHtmlHelp;

	// �ҏW�E�B���h�E�I�u�W�F�N�g����̃I�u�W�F�N�g�폜�v��
	case MYWM_DELETE_ME:
		{
			auto& csGeneral = pShareData->common.general;
			// �^�X�N�g���C�̃A�C�R�����풓���Ȃ��A�܂��́A�g���C�ɃA�C�R��������Ă��Ȃ�
			if (!(csGeneral.bStayTaskTray && csGeneral.bUseTaskTray) || !bCreatedTrayIcon) {
				// ���݊J���Ă���ҏW���̃��X�g
				size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
				if (0 < nRowNum) {
					delete[] pEditNodeArr;
				}
				// �ҏW�E�B���h�E�̐���0�ɂȂ�����I��
				if (nRowNum == 0) {
					::SendMessage(hwnd, WM_CLOSE, 0, 0);
				}
			}
		}
		return 0;

	case WM_CREATE:
		{
			auto& csGeneral = pShareData->common.general;
			hWnd = hwnd;
			hwndHtmlHelp = NULL;
			// Modified by KEITA for WIN64 2003.9.6
			::SetWindowLongPtr(GetTrayHwnd(), GWLP_USERDATA, (LONG_PTR)this);

			// �^�X�N�g���C���N���b�N���j���[�ւ̃V���[�g�J�b�g�L�[�o�^
			wHotKeyMods = 0;
			if (csGeneral.wTrayMenuHotKeyMods & HOTKEYF_SHIFT) {
				wHotKeyMods |= MOD_SHIFT;
			}
			if (csGeneral.wTrayMenuHotKeyMods & HOTKEYF_CONTROL) {
				wHotKeyMods |= MOD_CONTROL;
			}
			if (csGeneral.wTrayMenuHotKeyMods & HOTKEYF_ALT) {
				wHotKeyMods |= MOD_ALT;
			}
			wHotKeyCode = csGeneral.wTrayMenuHotKeyCode;
			if (wHotKeyCode != 0) {
				::RegisterHotKey(
					GetTrayHwnd(),
					ID_HOTKEY_TRAYMENU,
					wHotKeyMods,
					wHotKeyCode
				);
			}

			// 2006.07.09 ryoji �Ō�̕��ŃV���b�g�_�E������A�v���P�[�V�����ɂ���
			BOOL (WINAPI *pfnSetProcessShutdownParameters)(DWORD dwLevel, DWORD dwFlags);
			HINSTANCE hDll = ::GetModuleHandle(_T("KERNEL32"));
			if (hDll) {
				*(FARPROC*)&pfnSetProcessShutdownParameters = ::GetProcAddress(hDll, "SetProcessShutdownParameters");
				if (pfnSetProcessShutdownParameters) {
					pfnSetProcessShutdownParameters(0x180, 0);
				}
			}

			// 2010.08.26 �E�B���h�E���݊m�F
			::SetTimer(hwnd, IDT_EDITCHECK, IDT_EDITCHECK_INTERVAL, NULL);
		}
		return 0L;

//	case WM_QUERYENDSESSION:
	case WM_HELP:
		lphi = (LPHELPINFO) lParam;
		switch (lphi->iContextType) {
		case HELPINFO_MENUITEM:
			MyWinHelp(hwnd, HELP_CONTEXT, FuncID_To_HelpContextID((EFunctionCode)lphi->iCtrlId ));
			break;
		}
		return TRUE;
	case WM_COMMAND:
		OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND) lParam);
		return 0L;

//		case MYWM_SETFILEINFO:
//			return 0L;
	case MYWM_CHANGESETTING:
		if ((e_PM_CHANGESETTING_SELECT)lParam == PM_CHANGESETTING_ALL) {
			{
				auto& csWindow = GetDllShareData().common.window;
				bool bChangeLang = auto_strcmp(csWindow.szLanguageDll, szLanguageDll) != 0;
				auto_strcpy(szLanguageDll, csWindow.szLanguageDll);
				std::vector<std::wstring> values;
				if (bChangeLang) {
					ShareData::getInstance().ConvertLangValues(values, true);
				}
				// �����I������
				SelectLang::ChangeLang(csWindow.szLanguageDll);
				if (bChangeLang) {
					ShareData::getInstance().ConvertLangValues(values, false);
				}
			}

			::UnregisterHotKey(GetTrayHwnd(), ID_HOTKEY_TRAYMENU);
			// �^�X�N�g���C���N���b�N���j���[�ւ̃V���[�g�J�b�g�L�[�o�^
			wHotKeyMods = 0;
			auto& csGeneral = pShareData->common.general;
			if (csGeneral.wTrayMenuHotKeyMods & HOTKEYF_SHIFT) {
				wHotKeyMods |= MOD_SHIFT;
			}
			if (csGeneral.wTrayMenuHotKeyMods & HOTKEYF_CONTROL) {
				wHotKeyMods |= MOD_CONTROL;
			}
			if (csGeneral.wTrayMenuHotKeyMods & HOTKEYF_ALT) {
				wHotKeyMods |= MOD_ALT;
			}
			wHotKeyCode = csGeneral.wTrayMenuHotKeyCode;
			::RegisterHotKey(
				GetTrayHwnd(),
				ID_HOTKEY_TRAYMENU,
				wHotKeyMods,
				wHotKeyCode
			);

//@@			// ���L�f�[�^�̕ۑ�
//@@			pShareData->SaveShareData();

			// �A�N�Z�����[�^�e�[�u���̍č쐬
			// �A�N�Z�����[�^�e�[�u���j��
			DeleteAccelTbl();
			// �A�N�Z�����[�^�e�[�u���쐬
			CreateAccelTbl();
		}
		return 0L;

	case MYWM_SET_TYPESETTING:
		{
			int nIdx = (int)wParam;
			if (0 <= nIdx && pShareData->nTypesCount) {
				TypeConfig& type = pShareData->workBuffer.typeConfig;
				if (nIdx == 0) {
					pShareData->typeBasis = type;
					pShareData->typeBasis.nIdx = 0;
				}
				*(ShareData::getInstance().GetTypeSettings()[nIdx]) = type;
				ShareData::getInstance().GetTypeSettings()[nIdx]->nIdx = nIdx;
				auto& typeMini = pShareData->typesMini[nIdx];
				auto_strcpy(typeMini.szTypeName, type.szTypeName);
				auto_strcpy(typeMini.szTypeExts, type.szTypeExts);
				typeMini.id = type.id;
				typeMini.encoding = type.encoding;
			}else {
				return FALSE;
			}
		}
		return TRUE;
	case MYWM_GET_TYPESETTING:
		{
			int nIdx = (int)wParam;
			if (0 <= nIdx && pShareData->nTypesCount) {
				pShareData->workBuffer.typeConfig = *(ShareData::getInstance().GetTypeSettings()[nIdx]);
			}else {
				return FALSE;
			}
		}
		return TRUE;
	case MYWM_ADD_TYPESETTING:
		{
			int nInsert = (int)wParam;
			// "����"�̑O�ɂ͓���Ȃ�
			if (0 < nInsert && nInsert <= pShareData->nTypesCount && nInsert < MAX_TYPES) {
				std::vector<TypeConfig*>& types = ShareData::getInstance().GetTypeSettings();
				TypeConfig* type = new TypeConfig();
				*type = *types[0]; // ��{���R�s�[
				type->nIdx = nInsert;
				type->id = (::GetTickCount() & 0x3fffffff) + nInsert * 0x10000;
				// �������O�̂��̂��������炻�̎��ɂ���
				int nAddNameNum = nInsert + 1;
				auto_sprintf_s(type->szTypeName, LS(STR_TRAY_TYPE_NAME), nAddNameNum); 
				for (int k=1; k<pShareData->nTypesCount; ++k) {
					if (auto_strcmp(types[k]->szTypeName, type->szTypeName) == 0) {
						++nAddNameNum;
						auto_sprintf_s(type->szTypeName, LS(STR_TRAY_TYPE_NAME), nAddNameNum); 
						k = 0;
					}
				}
				auto_strcpy(type->szTypeExts, _T(""));
				type->nRegexKeyMagicNumber = RegexKeyword::GetNewMagicNumber();
				types.resize(pShareData->nTypesCount + 1);
				int nTypeSizeOld = pShareData->nTypesCount;
				++pShareData->nTypesCount;
				for (int i=nTypeSizeOld; nInsert<i; --i) {
					types[i] = types[i-1];
					types[i]->nIdx = i;
					pShareData->typesMini[i] = pShareData->typesMini[i-1];
				}
				types[nInsert] = type;
				auto& typeMini = pShareData->typesMini[nInsert];
				auto_strcpy(typeMini.szTypeName, type->szTypeName);
				auto_strcpy(typeMini.szTypeExts, type->szTypeExts);
				typeMini.id = type->id;
				typeMini.encoding = type->encoding;
			}else {
				return FALSE;
			}
		}
		return TRUE;
	case MYWM_DEL_TYPESETTING:
		{
			int nDelPos = (int)wParam;
			if (0 < nDelPos && nDelPos < pShareData->nTypesCount && 1 < pShareData->nTypesCount) {
				int nTypeSizeOld = pShareData->nTypesCount;
				auto& types = ShareData::getInstance().GetTypeSettings();
				delete types[nDelPos];
				for (int i=nDelPos; i<nTypeSizeOld-1; ++i) {
					types[i] = types[i + 1];
					types[i]->nIdx = i;
					pShareData->typesMini[i] = pShareData->typesMini[i + 1];
				}
				types.resize(pShareData->nTypesCount - 1);
				pShareData->nTypesCount--;
				pShareData->typesMini[nTypeSizeOld - 1].szTypeName[0] = 0;
				pShareData->typesMini[nTypeSizeOld - 1].szTypeExts[0] = 0;
				pShareData->typesMini[nTypeSizeOld - 1].id = 0;
			}else {
				return FALSE;
			}
		}
		return TRUE;

	case MYWM_NOTIFYICON:
//			MYTRACE(_T("MYWM_NOTIFYICON\n"));
		switch (lParam) {
// �L�[���[�h�F�g���C�E�N���b�N���j���[�ݒ�
// From Here Oct. 12, 2000 JEPRO ���E�Ƃ����ꏈ���ɂȂ��Ă����̂�ʁX�ɏ�������悤�ɕύX
		case WM_RBUTTONUP:	// Dec. 24, 2002 towest UP�ɕύX
			::SetActiveWindow(GetTrayHwnd());
			::SetForegroundWindow(GetTrayHwnd());
			// �|�b�v�A�b�v���j���[(�g���C�E�{�^��)
			nId = CreatePopUpMenu_R();
			switch (nId) {
			case F_HELP_CONTENTS:
				// �w���v�ڎ�
				ShowWinHelpContents(GetTrayHwnd());	// �ڎ���\������
				break;
			case F_HELP_SEARCH:
				// �w���v�L�[���[�h����
				MyWinHelp(GetTrayHwnd(), HELP_KEY, (ULONG_PTR)_T(""));	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
				break;
			case F_EXTHELP1:
				// �O���w���v�P
				do {
					if (HelpManager().ExtWinHelpIsSet()) {	// ���ʐݒ�̂݊m�F
						break;
					}else {
						ErrorBeep();
					}
				}while (::MYMESSAGEBOX(
						NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST,
						GSTR_APPNAME,
						LS(STR_TRAY_EXTHELP1)
					) == IDYES
				);// do-while

				break;
			case F_EXTHTMLHELP:
				// �O��HTML�w���v
				{
//						CEditView::Command_ExtHTMLHelp();
				}
				break;
			case F_TYPE_LIST:	// �^�C�v�ʐݒ�ꗗ
				{
					DlgTypeList			dlgTypeList;
					DlgTypeList::Result	result;
					result.documentType = TypeConfigNum(0);
					result.bTempChange = false;
					if (dlgTypeList.DoModal(G_AppInstance(), GetTrayHwnd(), &result)) {
						// �^�C�v�ʐݒ�
						PluginManager::getInstance().LoadAllPlugin();
						pPropertyManager->OpenPropertySheetTypes(NULL, -1, result.documentType);
						PluginManager::getInstance().UnloadAllPlugin();
					}
				}
				break;
			case F_OPTION:	// ���ʐݒ�
				{
					PluginManager::getInstance().LoadAllPlugin();
					{
						// �A�C�R���̓o�^
						const auto& plugs = JackManager::getInstance().GetPlugs(PP_COMMAND);
						menuDrawer.pIcons->ResetExtend();
						for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
							int iBitmap = MenuDrawer::TOOLBAR_ICON_PLUGCOMMAND_DEFAULT - 1;
							const Plug* plug = *it;
							if (!plug->sIcon.empty()) {
								iBitmap = menuDrawer.pIcons->Add(
									to_tchar(plug->plugin.GetFilePath(to_tchar(plug->sIcon.c_str())).c_str()) );
							}
							menuDrawer.AddToolButton( iBitmap, plug->GetFunctionCode() );
						}
					}
					pPropertyManager->OpenPropertySheet(NULL, -1, true);
					PluginManager::getInstance().UnloadAllPlugin();
				}
				break;
			case F_ABOUT:
				// �o�[�W�������
				{
					DlgAbout dlgAbout;
					dlgAbout.DoModal(hInstance, GetTrayHwnd());
				}
				break;
//				case IDM_EXITALL:
			case F_EXITALL:	// Dec. 26, 2000 JEPRO F_�ɕύX
				// �T�N���G�f�B�^�̑S�I��
				ControlTray::TerminateApplication(GetTrayHwnd());	// 2006.12.25 ryoji �����ǉ�
				break;
			default:
				break;
			}
			return 0L;
// To Here Oct. 12, 2000

		case WM_LBUTTONDOWN:
			// Mar. 29, 2003 genta �O�̂��߃t���O�N���A
			bLDClick = false;
			return 0L;
		case WM_LBUTTONUP:	// Dec. 24, 2002 towest UP�ɕύX
//				MYTRACE(_T("WM_LBUTTONDOWN\n"));
			// 03/02/20 ���_�u���N���b�N��̓��j���[��\�����Ȃ� ai Start
			if (bLDClick) {
				bLDClick = false;
				return 0L;
			}
			// 03/02/20 ai End
			::SetActiveWindow(GetTrayHwnd());
			::SetForegroundWindow(GetTrayHwnd());
			// �|�b�v�A�b�v���j���[(�g���C���{�^��)
			nId = CreatePopUpMenu_L();
			switch (nId) {
			case F_FILENEW:	// �V�K�쐬
				// �V�K�ҏW�E�B���h�E�̒ǉ�
				OnNewEditor(false);
				break;
			case F_FILEOPEN:	// �J��
				{
					// �t�@�C���I�[�v���_�C�A���O�̏�����
					LoadInfo loadInfo;
					loadInfo.filePath = _T("");
					loadInfo.eCharCode = CODE_AUTODETECT;	// �����R�[�h��������
					loadInfo.bViewMode = false;
					// 2013.03.21 novice �J�����g�f�B���N�g���ύX(MRU�͎g�p���Ȃ�)
					DlgOpenFile dlgOpenFile;
					dlgOpenFile.Create(
						hInstance,
						NULL,
						_T("*.*"),
						SakuraEnvironment::GetDlgInitialDir(true).c_str(),
						MruFile().GetPathList(),
						MruFolder().GetPathList()	// OPENFOLDER���X�g�̃t�@�C���̃��X�g
					);
					std::vector<std::tstring> files;
					if (!dlgOpenFile.DoModalOpenDlg(&loadInfo, &files)) {
						break;
					}
					if (!GetTrayHwnd()) {
						break;
					}
						
					// �V���ȕҏW�E�B���h�E���N��
					size_t nSize = files.size();
					for (size_t f=0; f<nSize; ++f) {
						loadInfo.filePath = files[f].c_str();
						ControlTray::OpenNewEditor(
							hInstance,
							GetTrayHwnd(),
							loadInfo,
							NULL,
							true,
							NULL,
							pShareData->common.tabBar.bNewWindow
						);
					}
				}
				break;
			case F_GREP_DIALOG:
				// Grep
				DoGrep();  // Stonee, 2001/03/21  Grep��ʊ֐���
				break;
			case F_FILESAVEALL:	// Jan. 24, 2005 genta �S�ď㏑���ۑ�
				AppNodeGroupHandle(0).PostMessageToAllEditors(
					WM_COMMAND,
					MAKELONG(F_FILESAVE_QUIET, 0),
					(LPARAM)0,
					NULL
				);
				break;
			case F_EXITALLEDITORS:	// Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)	// 2007.02.13 ryoji ��F_EXITALLEDITORS
				// �ҏW�̑S�I��
				ControlTray::CloseAllEditor(true, GetTrayHwnd(), true, 0);	// 2006.12.25, 2007.02.13 ryoji �����ǉ�
				break;
			case F_EXITALL:	// Dec. 26, 2000 JEPRO F_�ɕύX
				// �T�N���G�f�B�^�̑S�I��
				ControlTray::TerminateApplication(GetTrayHwnd());	// 2006.12.25 ryoji �����ǉ�
				break;
			default:
				if (nId - IDM_SELWINDOW >= 0
					&& nId - IDM_SELWINDOW < pShareData->nodes.nEditArrNum
				) {
					hwndWork = pShareData->nodes.pEditArr[nId - IDM_SELWINDOW].GetHwnd();

					// �A�N�e�B�u�ɂ���
					ActivateFrameWindow(hwndWork);
				}else if (nId - IDM_SELMRU >= 0 && nId-IDM_SELMRU < 999) {

					// �V�����ҏW�E�B���h�E���J��
					// From Here Oct. 27, 2000 genta	�J�[�\���ʒu�𕜌����Ȃ��@�\
					const MruFile mru;
					EditInfo openEditInfo;
					mru.GetEditInfo(nId - IDM_SELMRU, &openEditInfo);

					if (pShareData->common.file.GetRestoreCurPosition()) {
						ControlTray::OpenNewEditor2(hInstance, GetTrayHwnd(), openEditInfo, false);
					}else {
						LoadInfo loadInfo;
						loadInfo.filePath = openEditInfo.szPath;
						loadInfo.eCharCode = openEditInfo.nCharCode;
						loadInfo.bViewMode = false;
						ControlTray::OpenNewEditor(
							hInstance,
							GetTrayHwnd(),
							loadInfo,
							NULL,
							false,
							NULL,
							pShareData->common.tabBar.bNewWindow
						);

					}
					// To Here Oct. 27, 2000 genta
				}else if (
					nId - IDM_SELOPENFOLDER >= 0
					&& nId - IDM_SELOPENFOLDER  < 999
				) {
					// MRU���X�g�̃t�@�C���̃��X�g
					const MruFile mru;
					std::vector<LPCTSTR> vMRU = mru.GetPathList();

					// OPENFOLDER���X�g�̃t�@�C���̃��X�g
					const MruFolder mruFolder;
					std::vector<LPCTSTR> vOPENFOLDER = mruFolder.GetPathList();

					// Stonee, 2001/12/21 UNC�ł���ΐڑ������݂�
					NetConnect(mruFolder.GetPath(nId - IDM_SELOPENFOLDER));

					// �t�@�C���I�[�v���_�C�A���O�̏�����
					DlgOpenFile	dlgOpenFile;
					dlgOpenFile.Create(
						hInstance,
						NULL,
						_T("*.*"),
						vOPENFOLDER[nId - IDM_SELOPENFOLDER],
						vMRU,
						vOPENFOLDER
					);
					LoadInfo loadInfo(_T(""), CODE_AUTODETECT, false);
					std::vector<std::tstring> files;
					if (!dlgOpenFile.DoModalOpenDlg(&loadInfo, &files)) {
						break;
					}
					if (!GetTrayHwnd()) {
						break;
					}

					// �V���ȕҏW�E�B���h�E���N��
					size_t nSize = files.size();
					for (size_t f=0; f<nSize; ++f) {
						loadInfo.filePath = files[f].c_str();
						ControlTray::OpenNewEditor(
							hInstance,
							GetTrayHwnd(),
							loadInfo,
							NULL,
							true,
							NULL,
							pShareData->common.tabBar.bNewWindow
						);
					}
				}
				break;
			}
			return 0L;
		case WM_LBUTTONDBLCLK:
			bLDClick = true;		// 03/02/20 ai
			// �V�K�ҏW�E�B���h�E�̒ǉ�
			OnNewEditor(pShareData->common.tabBar.bNewWindow);
			// Apr. 1, 2003 genta ���̌�ŕ\�����ꂽ���j���[�͕���
			::PostMessage(GetTrayHwnd(), WM_CANCELMODE, 0, 0);
			return 0L;
		case WM_RBUTTONDBLCLK:
			return 0L;
		}
		break;

	case WM_QUERYENDSESSION:
		// ���ׂẴE�B���h�E����� // Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
		if (CloseAllEditor(false, GetTrayHwnd(), true, 0)) {	// 2006.12.25, 2007.02.13 ryoji �����ǉ�
			// Jan. 31, 2000 genta
			// ���̎��_�ł�Windows�̏I�����m�肵�Ă��Ȃ��̂ŏ풓�������ׂ��ł͂Ȃ��D
			//::DestroyWindow(hwnd);
			return TRUE;
		}else {
			return FALSE;
		}
	case WM_CLOSE:
		// ���ׂẴE�B���h�E����� 	Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
		if (CloseAllEditor(false, GetTrayHwnd(), true, 0)) {	// 2006.12.25, 2007.02.13 ryoji �����ǉ�
			::DestroyWindow(hwnd);
		}
		return 0L;

	// From Here Jan. 31, 2000 genta	Windows�I�����̌㏈���D
	// Windows�I������WM_CLOSE���Ă΂�Ȃ���CDestroyWindow��
	// �Ăяo���K�v���Ȃ��D�܂��C���b�Z�[�W���[�v�ɖ߂�Ȃ��̂�
	// ���b�Z�[�W���[�v�̌��̏����������Ŋ���������K�v������D
	case WM_ENDSESSION:
		// ����Windows�̏I�������f���ꂽ�̂Ȃ牽�����Ȃ�
		if (wParam != FALSE) {
			OnDestroy();	// 2006.07.09 ryoji WM_DESTROY �Ɠ�������������i�g���C�A�C�R���̔j���Ȃǂ�NT�n�ł͕K�v�j
		}
		return 0;	// �������̃v���Z�X�ɐ��䂪�߂邱�Ƃ͂Ȃ�
	// To Here Jan. 31, 2000 genta
	case WM_DESTROY:
		OnDestroy();

		// Windows �ɃX���b�h�̏I����v�����܂��B
		::PostQuitMessage(0);
		return 0L;
	case MYWM_ALLOWACTIVATE:
		::AllowSetForegroundWindow(wParam);
		return 0L;
	default:
// << 20010412 by aroka
// Apr. 24, 2001 genta RegisterWindowMessage���g���悤�ɏC��
		if (uMsg == uCreateTaskBarMsg) {
			/* TaskTray Icon�̍ēo�^��v�����郁�b�Z�[�W�D
				Explorer���ċN�������Ƃ��ɑ��o�����D*/
			CreateTrayIcon(GetTrayHwnd()) ;
		}
		break;	// default
// >> by aroka
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


// WM_COMMAND���b�Z�[�W����
void ControlTray::OnCommand(WORD wNotifyCode, WORD wID , HWND hwndCtl)
{
	switch (wNotifyCode) {
	// ���j���[����̃��b�Z�[�W
	case 0:
		break;
	}
	return;
}

/*!
	@brief �V�K�E�B���h�E���쐬����

	@author genta
	@date 2003.05.30 �V�K�쐬
	@date 2013.03.21 novice MRU�͎g�p���Ȃ�
*/
void ControlTray::OnNewEditor(bool bNewWindow)
{
	// �V�K�E�B���h�E�ŊJ���I�v�V�����́A�^�u�o�[���O���[�v����O��Ƃ���
	auto& csTabBar = pShareData->common.tabBar;
	bNewWindow = bNewWindow
				 && csTabBar.bDispTabWnd
				 && !csTabBar.bDispTabWndMultiWin;

	// �ҏW�E�B���h�E���J��
	LoadInfo loadInfo;
	loadInfo.filePath = _T("");
	loadInfo.eCharCode = CODE_NONE;
	loadInfo.bViewMode = false;
	std::tstring tstrCurDir = SakuraEnvironment::GetDlgInitialDir(true);
	OpenNewEditor(
		hInstance,
		GetTrayHwnd(),
		loadInfo,
		NULL,
		false,
		tstrCurDir.c_str(),
		bNewWindow
	);
}

/*!
	�V�K�ҏW�E�B���h�E�̒ǉ� ver 0

	@date 2000.10.24 genta WinExec -> CreateProcess�D�����@�\��t��
	@date 2002.02.17 YAZAKI ShareData�̃C���X�^���X�́AProcess�ɂЂƂ���̂݁B
	@date 2003.05.30 genta �O���v���Z�X�N�����̃J�����g�f�B���N�g���w����\�ɁD
	@date 2007.06.26 ryoji �V�K�ҏW�E�B���h�E�� hWndParent �Ɠ����O���[�v���w�肵�ċN������
	@date 2008.04.19 ryoji MYWM_FIRST_IDLE �҂���ǉ�
	@date 2008.05.05 novice GetModuleHandle(NULL)��NULL�ɕύX
*/
bool ControlTray::OpenNewEditor(
	HINSTANCE			hInstance,			// [in] �C���X�^���XID (���͖��g�p)
	HWND				hWndParent,			// [in] �e�E�B���h�E�n���h���D�G���[���b�Z�[�W�\���p
	const LoadInfo&		loadInfo,			// [in]
	const TCHAR*		szCmdLineOption,	// [in] �ǉ��̃R�}���h���C���I�v�V����
	bool				sync,				// [in] true�Ȃ�V�K�G�f�B�^�̋N���܂őҋ@����
	const TCHAR*		pszCurDir,			// [in] �V�K�G�f�B�^�̃J�����g�f�B���N�g��(NULL��)
	bool				bNewWindow			// [in] �V�K�G�f�B�^��V�����E�B���h�E�ŊJ��
	)
{
	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	DllSharedData& shareData = GetDllShareData();

	// �ҏW�E�B���h�E�̏���`�F�b�N
	if (shareData.nodes.nEditArrNum >= MAX_EDITWINDOWS) {	// �ő�l�C��	//@@@ 2003.05.31 MIK
		OkMessage(NULL, LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
		return false;
	}

	// -- -- -- -- �R�}���h���C��������𐶐� -- -- -- -- //
	CommandLineString cmdLineBuf;

	// �A�v���P�[�V�����p�X
	TCHAR szEXE[MAX_PATH + 1];
	::GetModuleFileName(NULL, szEXE, _countof(szEXE));
	cmdLineBuf.AppendF(_T("\"%ts\""), szEXE);

	// �t�@�C����
	if (loadInfo.filePath.c_str()[0] != _T('\0')) {
		cmdLineBuf.AppendF(_T(" \"%ts\""), loadInfo.filePath.c_str());
	}

	// �R�[�h�w��
	if (IsValidCodeType(loadInfo.eCharCode)) {
		cmdLineBuf.AppendF(_T(" -CODE=%d"), loadInfo.eCharCode);
	}

	// �r���[���[�h�w��
	if (loadInfo.bViewMode) {
		cmdLineBuf.AppendF(_T(" -R"));
	}

	// �O���[�vID
	if (!bNewWindow) {	// �V�K�G�f�B�^���E�B���h�E�ŊJ��
		// �O���[�vID��e�E�B���h�E����擾
		HWND hwndAncestor = MyGetAncestor(hWndParent, GA_ROOTOWNER2);	// 2007.10.22 ryoji GA_ROOTOWNER -> GA_ROOTOWNER2
		int nGroup = AppNodeManager::getInstance().GetEditNode(hwndAncestor)->GetGroup();
		if (nGroup > 0) {
			cmdLineBuf.AppendF(_T(" -GROUP=%d"), nGroup);
		}
	}else {
		// �󂢂Ă���O���[�vID���g�p����
		cmdLineBuf.AppendF(_T(" -GROUP=%d"), AppNodeManager::getInstance().GetFreeGroupId());
	}

	if (CommandLine::getInstance().IsSetProfile()) {
		cmdLineBuf.AppendF( _T(" -PROF=\"%ls\""), CommandLine::getInstance().GetProfileName() );
	}

	// �ǉ��̃R�}���h���C���I�v�V����
	TCHAR szResponseFile[_MAX_PATH] = _T("");
	struct ResponseFileDeleter {
		LPCTSTR fileName;
		ResponseFileDeleter()
			:
			fileName(NULL)
		{}
		~ResponseFileDeleter() {
			if (fileName && fileName[0]) {
				::DeleteFile(fileName);
				fileName = NULL;
			}
		}
	};
	ResponseFileDeleter respDeleter;
	if (szCmdLineOption) {
		// Grep�Ȃǂœ��肫��Ȃ��ꍇ�̓��X�|���X�t�@�C���𗘗p����
		if (cmdLineBuf.max_size() < cmdLineBuf.size() + auto_strlen(szCmdLineOption)) {
			TCHAR szIniDir[_MAX_PATH];
			GetInidir(szIniDir);
			LPTSTR pszTempFile = _ttempnam(szIniDir, _T("skr_resp"));
			if (!pszTempFile) {
				ErrorMessage(hWndParent, LS(STR_TRAY_RESPONSEFILE));
				return false;
			}
			auto_strcpy(szResponseFile, pszTempFile);
			free(pszTempFile);
			TextOutputStream output(szResponseFile);
			if (!output) {
				ErrorMessage(hWndParent, LS(STR_TRAY_RESPONSEFILE));
				return false;
			}
			respDeleter.fileName = szResponseFile;
			// �o��
			output.WriteString(to_wchar(szCmdLineOption));
			output.Close();
			sync = true;
			cmdLineBuf.AppendF(_T(" -@=\"%ts\""), szResponseFile);
		}else {
			cmdLineBuf.AppendF(_T(" %ts"), szCmdLineOption);
		}
	}
	// -- -- -- -- �v���Z�X���� -- -- -- -- //

	// �����ȃf�B���N�g���̂Ƃ���NULL�ɕύX
	if (pszCurDir) {
		DWORD attr = GetFileAttributes(pszCurDir);
		if ((attr != -1) && (attr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
		}else {
			pszCurDir = NULL;
		}
	}

	// �v���Z�X�̋N��
	PROCESS_INFORMATION p;
	STARTUPINFO s;

	s.cb = sizeof_raw(s);
	s.lpReserved = NULL;
	s.lpDesktop = NULL;
	s.lpTitle = NULL;

	s.dwFlags = STARTF_USESHOWWINDOW;
	s.wShowWindow = SW_SHOWDEFAULT;
	s.cbReserved2 = 0;
	s.lpReserved2 = NULL;

	// May 30, 2003 genta �J�����g�f�B���N�g���w����\��
	// �G�f�B�^�v���Z�X���N��
	DWORD dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;
#ifdef _DEBUG
//	dwCreationFlag |= DEBUG_PROCESS; // 2007.09.22 kobake �f�o�b�O�p�t���O
#endif
	TCHAR szCmdLine[1024]; _tcscpy_s(szCmdLine, cmdLineBuf.c_str());
	BOOL bCreateResult = CreateProcess(
		szEXE,					// ���s�\���W���[���̖��O
		szCmdLine,				// �R�}���h���C���̕�����
		NULL,					// �Z�L�����e�B�L�q�q
		NULL,					// �Z�L�����e�B�L�q�q
		FALSE,					// �n���h���̌p���I�v�V����
		dwCreationFlag,			// �쐬�̃t���O
		NULL,					// �V�������u���b�N
		pszCurDir,				// �J�����g�f�B���N�g���̖��O
		&s,						// �X�^�[�g�A�b�v���
		&p						// �v���Z�X���
	);
	if (!bCreateResult) {
		// ���s
		TCHAR* pMsg;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_IGNORE_INSERTS |
						FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR)&pMsg,
						0,
						NULL
		);
		ErrorMessage(
			hWndParent,
			LS(STR_TRAY_CREATEPROC1),
			szEXE,
			pMsg
		);
		::LocalFree((HLOCAL)pMsg);	// �G���[���b�Z�[�W�o�b�t�@�����
		return false;
	}

	bool bRet = true;
	if (sync) {
		// �N�������v���Z�X�����S�ɗ����オ��܂ł�����Ƒ҂D
		int nResult = WaitForInputIdle(p.hProcess, 10000);	// �ő�10�b�ԑ҂�
		if (nResult != 0) {
			ErrorMessage(
				hWndParent,
				LS(STR_TRAY_CREATEPROC2),
				szEXE
			);
			bRet = false;
		}
	}else {
		// �^�u�܂Ƃߎ��͋N�������v���Z�X�������オ��܂ł��΂炭�^�C�g���o�[���A�N�e�B�u�ɕۂ�	// 2007.02.03 ryoji
		if (shareData.common.tabBar.bDispTabWnd
			&& !shareData.common.tabBar.bDispTabWndMultiWin
		) {
			WaitForInputIdle(p.hProcess, 3000);
			sync = true;
		}
	}

	// MYWM_FIRST_IDLE ���͂��܂ł�����Ƃ����]���ɑ҂�	// 2008.04.19 ryoji
	// Note. �N����v���Z�X���������������� COM �֐��iSHGetFileInfo API �Ȃǂ��܂ށj�����s����ƁA
	//       ���̎��_�� COM �̓����@�\�������� WaitForInputIdle �͏I�����Ă��܂��\��������i�炵���j�B
	if (sync && bRet) {
		for (int i=0; i<200; ++i) {
			MSG msg;
			DWORD dwExitCode;
			if (::PeekMessage(&msg, 0, MYWM_FIRST_IDLE, MYWM_FIRST_IDLE, PM_REMOVE)) {
				if (msg.message == WM_QUIT) {	// �w��͈͊O�ł� WM_QUIT �͎��o�����
					::PostQuitMessage(msg.wParam);
					break;
				}
				// �Ď��Ώۃv���Z�X����̃��b�Z�[�W�Ȃ甲����
				// �����łȂ���Δj�����Ď������o��
				if (msg.wParam == p.dwProcessId) {
					break;
				}
			}
			if (::GetExitCodeProcess(p.hProcess, &dwExitCode) && dwExitCode != STILL_ACTIVE) {
				break;	// �Ď��Ώۃv���Z�X���I������
			}
			::Sleep(10);
		}
	}

	CloseHandle(p.hThread);
	CloseHandle(p.hProcess);

	return bRet;
}


/*!	�V�K�ҏW�E�B���h�E�̒ǉ� ver 2:

	@date Oct. 24, 2000 genta create.
	@date Feb. 25, 2012 novice -CODE/-R��OpenNewEditor���ŏ�������̂ō폜
*/
bool ControlTray::OpenNewEditor2(
	HINSTANCE		hInstance,
	HWND			hWndParent,
	const EditInfo&	editInfo,
	bool			bViewMode,
	bool			sync,
	bool			bNewWindow			// [in] �V�K�G�f�B�^��V�����E�B���h�E�ŊJ��
	)
{

	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	DllSharedData& shareData = GetDllShareData();

	// �ҏW�E�B���h�E�̏���`�F�b�N
	if (shareData.nodes.nEditArrNum >= MAX_EDITWINDOWS) {	// �ő�l�C��	//@@@ 2003.05.31 MIK
		OkMessage(NULL, LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
		return false;
	}

	// �ǉ��̃R�}���h���C���I�v�V����
	CommandLineString cmdLine;
	if (editInfo.ptCursor.x >= 0			) cmdLine.AppendF(_T(" -X=%d"), editInfo.ptCursor.x + 1);
	if (editInfo.ptCursor.y >= 0			) cmdLine.AppendF(_T(" -Y=%d"), editInfo.ptCursor.y + 1);
	if (editInfo.nViewLeftCol >= 0) cmdLine.AppendF(_T(" -VX=%d"), editInfo.nViewLeftCol + 1);
	if (editInfo.nViewTopLine >= 0) cmdLine.AppendF(_T(" -VY=%d"), editInfo.nViewTopLine + 1);
	LoadInfo loadInfo;
	loadInfo.filePath = editInfo.szPath;
	loadInfo.eCharCode = editInfo.nCharCode;
	loadInfo.bViewMode = bViewMode;
	return OpenNewEditor(
		hInstance,
		hWndParent,
		loadInfo,
		cmdLine.c_str(),
		sync,
		NULL,
		bNewWindow
	);
}
// To Here Oct. 24, 2000 genta



void ControlTray::ActiveNextWindow(HWND hwndParent)
{
	// ���݊J���Ă���ҏW���̃��X�g�𓾂�
	EditNode*	pEditNodeArr;
	size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
	if (nRowNum > 0) {
		// �����̃E�B���h�E�𒲂ׂ�
		int nGroup = 0;
		size_t i;
		for (i=0; i<nRowNum; ++i) {
			if (hwndParent == pEditNodeArr[i].GetHwnd()) {
				nGroup = pEditNodeArr[i].nGroup;
				break;
			}
		}
		if (i < nRowNum) {
			// �O�̃E�B���h�E
			int j;
			for (j=(int)i-1; j>=0; --j) {
				if (nGroup == pEditNodeArr[j].nGroup) {
					break;
				}
			}
			if (j < 0) {
				for (j=nRowNum-1; j>i; --j) {
					if (nGroup == pEditNodeArr[j].nGroup) {
						break;
					}
				}
			}
			// �O�̃E�B���h�E���A�N�e�B�u�ɂ���
			HWND hwndWork = pEditNodeArr[j].GetHwnd();
			ActivateFrameWindow(hwndWork);
			// �Ō�̃y�C�����A�N�e�B�u�ɂ���
			::PostMessage(hwndWork, MYWM_SETACTIVEPANE, (WPARAM) - 1, 1);
		}
		delete[] pEditNodeArr;
	}
}

void ControlTray::ActivePrevWindow(HWND hwndParent)
{
	// ���݊J���Ă���ҏW���̃��X�g�𓾂�
	EditNode* pEditNodeArr;
	size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
	if (nRowNum > 0) {
		// �����̃E�B���h�E�𒲂ׂ�
		int nGroup = 0;
		size_t i;
		for (i=0; i<nRowNum; ++i) {
			if (hwndParent == pEditNodeArr[i].GetHwnd()) {
				nGroup = pEditNodeArr[i].nGroup;
				break;
			}
		}
		if (i < nRowNum) {
			// ���̃E�B���h�E
			size_t j;
			for (j=i+1; j<nRowNum; ++j) {
				if (nGroup == pEditNodeArr[j].nGroup) {
					break;
				}
			}
			if (j >= nRowNum) {
				for (j=0; j<i; ++j) {
					if (nGroup == pEditNodeArr[j].nGroup) {
						break;
					}
				}
			}
			// ���̃E�B���h�E���A�N�e�B�u�ɂ���
			HWND hwndWork = pEditNodeArr[j].GetHwnd();
			ActivateFrameWindow(hwndWork);
			// �ŏ��̃y�C�����A�N�e�B�u�ɂ���
			::PostMessage(hwndWork, MYWM_SETACTIVEPANE, (WPARAM) - 1, 0);
		}
		delete[] pEditNodeArr;
	}
}



/*!	�T�N���G�f�B�^�̑S�I��

	@date 2002.2.17 YAZAKI ShareData�̃C���X�^���X�́AProcess�ɂЂƂ���̂݁B
	@date 2006.12.25 ryoji �����̕ҏW�E�B���h�E�����Ƃ��̊m�F�i�����ǉ��j
*/
void ControlTray::TerminateApplication(
	HWND hWndFrom	// [in] �Ăяo�����̃E�B���h�E�n���h��
	)
{
	DllSharedData& shareData = GetDllShareData();	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�

	// ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ�
	if (shareData.common.general.bExitConfirm) {	// �I�����̊m�F
		if (0 < AppNodeGroupHandle(0).GetEditorWindowsNum()) {
			if (::MYMESSAGEBOX(
					hWndFrom,
					MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION,
					GSTR_APPNAME,
					LS(STR_TRAY_EXITALL)
				) != IDYES
			) {
				return;
			}
		}
	}
	// �u���ׂẴE�B���h�E�����v�v��	// Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
	bool bCheckConfirm = shareData.common.general.bExitConfirm;	// 2006.12.25 ryoji �I���m�F�ς݂Ȃ炻��ȏ�͊m�F���Ȃ�
	if (CloseAllEditor(bCheckConfirm, hWndFrom, true, 0)) {	// 2006.12.25, 2007.02.13 ryoji �����ǉ�
		::PostMessage(shareData.handles.hwndTray, WM_CLOSE, 0, 0);
	}
	return;
}


/*!	���ׂẴE�B���h�E�����

	@date Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
	@date 2002.2.17 YAZAKI ShareData�̃C���X�^���X�́AProcess�ɂЂƂ���̂݁B
	@date 2006.12.25 ryoji �����̕ҏW�E�B���h�E�����Ƃ��̊m�F�i�����ǉ��j
	@date 2007.02.13 ryoji �u�ҏW�̑S�I���v����������(bExit)��ǉ�
	@date 2007.06.20 ryoji nGroup������ǉ�
*/
bool ControlTray::CloseAllEditor(
	bool	bCheckConfirm,	// [in] [���ׂĕ���]�m�F�I�v�V�����ɏ]���Ė₢���킹�����邩�ǂ���
	HWND	hWndFrom,		// [in] �Ăяo�����̃E�B���h�E�n���h��
	bool	bExit,			// [in] true: �ҏW�̑S�I�� / false: ���ׂĕ���
	int		nGroup			// [in] �O���[�vID
	)
{
	EditNode* pWndArr;
	size_t n = AppNodeManager::getInstance().GetOpenedWindowArr(&pWndArr, false);
	if (n == 0) {
		return true;
	}
	
	// �S�ҏW�E�B���h�E�֏I���v�����o��
	bool bRes = AppNodeGroupHandle(nGroup).RequestCloseEditor(pWndArr, n, bExit, bCheckConfirm, hWndFrom);	// 2007.02.13 ryoji bExit�������p��
	delete[] pWndArr;
	return bRes;
}


// �|�b�v�A�b�v���j���[(�g���C���{�^��)
int	ControlTray::CreatePopUpMenu_L(void)
{
	// �{���̓Z�}�t�H�ɂ��Ȃ��Ƃ���
	if (bUseTrayMenu) {
		return -1;
	}
	bUseTrayMenu = true;

	menuDrawer.ResetContents();
	FileNameManager::getInstance().TransformFileName_MakeCache();

	// ���\�[�X���g��Ȃ��悤��
	HMENU hMenuTop = ::CreatePopupMenu();
	HMENU hMenu = ::CreatePopupMenu();
	menuDrawer.MyAppendMenu(hMenuTop, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenu, L"TrayL", L"");

	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, _T(""), _T("N"), false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, _T(""), _T("O"), false);

	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_GREP_DIALOG, _T(""), _T("G"), false);
	menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);

	// MRU���X�g�̃t�@�C���̃��X�g�����j���[�ɂ���
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	const MruFile mru;
	HMENU hMenuPopUp = mru.CreateMenu(menuDrawer);	// �t�@�C�����j���[
	int nEnable = (mru.MenuLength() > 0 ? 0 : MF_GRAYED);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP | nEnable, (UINT_PTR)hMenuPopUp , LS(F_FILE_RCNTFILE_SUBMENU), _T("F"));

	// �ŋߎg�����t�H���_�̃��j���[���쐬
//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́AMruFolder�ɂ��ׂĈ˗�����
	const MruFolder mruFolder;
	hMenuPopUp = mruFolder.CreateMenu(menuDrawer);
	nEnable = (mruFolder.MenuLength() > 0 ? 0 : MF_GRAYED);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP| nEnable, (UINT_PTR)hMenuPopUp, LS(F_FILE_RCNTFLDR_SUBMENU), _T("D"));

	menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILESAVEALL, _T(""), _T("Z"), false);	// Jan. 24, 2005 genta

	// ���݊J���Ă���ҏW���̃��X�g�����j���[�ɂ���
	int j = 0;
	for (int i=0; i<pShareData->nodes.nEditArrNum; ++i) {
		if (IsSakuraMainWindow(pShareData->nodes.pEditArr[i].GetHwnd())) {
			++j;
		}
	}

	if (j > 0) {
		menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);

		NONCLIENTMETRICS met;
		met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, met.cbSize);
		DCFont dcFont(met.lfMenuFont);

		j = 0;
		TCHAR szMenu[100 + MAX_PATH * 2];	// Jan. 19, 2001 genta
		for (int i=0; i<pShareData->nodes.nEditArrNum; ++i) {
			if (IsSakuraMainWindow(pShareData->nodes.pEditArr[i].GetHwnd())) {
				// �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm
				::SendMessage(pShareData->nodes.pEditArr[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0);
				EditInfo* pfi = (EditInfo*)&pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;

				// ���j���[���x���B1����A�N�Z�X�L�[��U��
				FileNameManager::getInstance().GetMenuFullLabel_WinList(
					szMenu,
					_countof(szMenu),
					pfi,
					pShareData->nodes.pEditArr[i].nId,
					i,
					dcFont.GetHDC()
				);
				menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + i, szMenu, _T(""), false);
				++j;
			}
		}
	}
	menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_EXITALLEDITORS, _T(""), _T("Q"), false);	// Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)	//Feb. 18, 2001 JEPRO �A�N�Z�X�L�[�ύX(L��Q)	// 2006.10.21 ryoji �\��������ύX	// 2007.02.13 ryoji ��F_EXITALLEDITORS
	if (j == 0) {
		::EnableMenuItem(hMenu, F_EXITALLEDITORS, MF_BYCOMMAND | MF_GRAYED);	// Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)	// 2007.02.13 ryoji ��F_EXITALLEDITORS
		::EnableMenuItem(hMenu, F_FILESAVEALL, MF_BYCOMMAND | MF_GRAYED);	// Jan. 24, 2005 genta
	}

	// Jun. 9, 2001 genta �\�t�g�E�F�A������
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, _T(""), _T("X"), false);	// Dec. 26, 2000 JEPRO F_�ɕύX

	POINT po;
	po.x = 0;
	po.y = 0;
	::GetCursorPos(&po);
	po.y -= 4;

	RECT rc;
	rc.left = 0;
	rc.right = 0;
	rc.top = 0;
	rc.bottom = 0;

	::SetForegroundWindow(GetTrayHwnd());
	int nId = ::TrackPopupMenu(
		hMenu,
		TPM_BOTTOMALIGN
		| TPM_RIGHTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		GetTrayHwnd(),
		&rc
	);
	::PostMessage(GetTrayHwnd(), WM_USER + 1, 0, 0);
	::DestroyMenu(hMenuTop);
//	MYTRACE(_T("nId=%d\n"), nId);

	bUseTrayMenu = false;

	return nId;
}

// �L�[���[�h�F�g���C�E�N���b�N���j���[����
// Oct. 12, 2000 JEPRO �|�b�v�A�b�v���j���[(�g���C���{�^��) ���Q�l�ɂ��ĐV���ɒǉ���������

// �|�b�v�A�b�v���j���[(�g���C�E�{�^��)
int	ControlTray::CreatePopUpMenu_R(void)
{
	// �{���̓Z�}�t�H�ɂ��Ȃ��Ƃ���
	if (bUseTrayMenu) {
		return -1;
	}
	bUseTrayMenu = true;

	menuDrawer.ResetContents();

	// ���\�[�X���g��Ȃ��悤��
	HMENU hMenuTop = ::CreatePopupMenu();
	HMENU hMenu = ::CreatePopupMenu();
	menuDrawer.MyAppendMenu(hMenuTop, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenu, L"TrayR", L"");

	// �g���C�E�N���b�N�́u�w���v�v���j���[
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_HELP_CONTENTS , _T(""), _T("O"), false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_HELP_SEARCH , _T(""), _T("S"), false);	// Nov. 25, 2000 JEPRO �u�g�s�b�N�́v���u�L�[���[�h�v�ɕύX
	menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_TYPE_LIST, _T(""), _T("L"), false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_OPTION, _T(""), _T("C"), false);
	menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_ABOUT, _T(""), _T("A"), false);	// Dec. 25, 2000 JEPRO F_�ɕύX
	menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);
	// Jun. 18, 2001 genta �\�t�g�E�F�A������
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_EXITALL, _T(""), _T("X"), false);

	POINT po;
	po.x = 0;
	po.y = 0;
	::GetCursorPos(&po);
	po.y -= 4;

	RECT rc;
	rc.left = 0;
	rc.right = 0;
	rc.top = 0;
	rc.bottom = 0;

	::SetForegroundWindow(GetTrayHwnd());
	int nId = ::TrackPopupMenu(
		hMenu,
		TPM_BOTTOMALIGN
		| TPM_RIGHTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		GetTrayHwnd(),
		&rc
	);
	::PostMessage(GetTrayHwnd(), WM_USER + 1, 0, 0);
	::DestroyMenu(hMenuTop);
//	MYTRACE(_T("nId=%d\n"), nId);

	bUseTrayMenu = false;

	return nId;
}

/*! �A�N�Z�����[�^�e�[�u���쐬
	@date 2013.04.20 novice ���ʏ������֐���
*/
void ControlTray::CreateAccelTbl(void)
{
	auto& csKeyBind = pShareData->common.keyBind;
	pShareData->handles.hAccel = KeyBind::CreateAccerelator(
		csKeyBind.nKeyNameArrNum,
		csKeyBind.pKeyNameArr
	);

	if (!pShareData->handles.hAccel) {
		ErrorMessage(
			NULL,
			LS(STR_TRAY_ACCELTABLE)
		);
	}
}

/*! �A�N�Z�����[�^�e�[�u���j��
	@date 2013.04.20 novice ���ʏ������֐���
*/
void ControlTray::DeleteAccelTbl(void)
{
	if (pShareData->handles.hAccel) {
		::DestroyAcceleratorTable(pShareData->handles.hAccel);
		pShareData->handles.hAccel = NULL;
	}
}

/*!
	@brief WM_DESTROY ����
	@date 2006.07.09 ryoji �V�K�쐬
*/
void ControlTray::OnDestroy()
{
	HWND hwndExitingDlg = 0;

	if (!GetTrayHwnd()) {
		return;	// ���ɔj������Ă���
	}

	// �z�b�g�L�[�̔j��
	::UnregisterHotKey(GetTrayHwnd(), ID_HOTKEY_TRAYMENU);

	// 2006.07.09 ryoji ���L�f�[�^�ۑ��� CControlProcess::Terminate() ����ړ�
	//
	// �u�^�X�N�g���C�ɏ풓���Ȃ��v�ݒ�ŃG�f�B�^��ʁiNormal Process�j�𗧂��グ���܂�
	// �Z�b�V�����I������悤�ȏꍇ�ł����L�f�[�^�ۑ����s���Ȃ������蒆�f����邱�Ƃ�
	// �����悤�A�����ŃE�B���h�E���j�������O�ɕۑ�����
	//

	// �I���_�C�A���O��\������
	if (pShareData->common.general.bDispExitingDialog) {
		// �I�����_�C�A���O�̕\��
		hwndExitingDlg = ::CreateDialog(
			hInstance,
			MAKEINTRESOURCE(IDD_EXITING),
			GetTrayHwnd()/*::GetDesktopWindow()*/,
			ExitingDlgProc
		);
		::ShowWindow(hwndExitingDlg, SW_SHOW);
	}

	// ���L�f�[�^�̕ۑ�
	ShareData_IO::SaveShareData();

	// �I���_�C�A���O��\������
	if (pShareData->common.general.bDispExitingDialog) {
		// �I�����_�C�A���O�̔j��
		::DestroyWindow(hwndExitingDlg);
	}

	if (bCreatedTrayIcon) {	// �g���C�ɃA�C�R���������
		TrayMessage(GetTrayHwnd(), NIM_DELETE, 0, NULL, NULL);
	}

	// �A�N�Z�����[�^�e�[�u���̍폜
	DeleteAccelTbl();

	hWnd = NULL;
}

/*!
	@brief �I���_�C�A���O�p�v���V�[�W��
	@date 2006.07.02 ryoji CControlProcess ����ړ�
*/
INT_PTR CALLBACK ControlTray::ExitingDlgProc(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam		// second message parameter
	)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		return TRUE;
	}
	return FALSE;
}

