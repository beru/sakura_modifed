/*!	@file
	@brief �ҏW�E�B���h�E�i�O�g�j�Ǘ��N���X

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro, ao
	Copyright (C) 2001, MIK, Stonee, Misaka, hor, YAZAKI
	Copyright (C) 2002, YAZAKI, genta, hor, aroka, minfu, �S, MIK, ai
	Copyright (C) 2003, genta, MIK, Moca, wmlhq, ryoji, KEITA
	Copyright (C) 2004, genta, Moca, yasu, MIK, novice, Kazika
	Copyright (C) 2005, genta, MIK, Moca, aroka, ryoji
	Copyright (C) 2006, genta, ryoji, aroka, fon, yukihane
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji, nasukoji, Hidetaka Sakai
	Copyright (C) 2010, ryoji, Moca�AUchi
	Copyright (C) 2011, ryoji
	Copyright (C) 2013, Uchi

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
#include <ShlObj.h>

#include "window/CEditWnd.h"
#include "_main/CControlTray.h"
#include "_main/CCommandLine.h"	/// 2003/1/26 aroka
#include "_main/CAppMode.h"
#include "_os/CDropTarget.h"
#include "_os/COsVersionInfo.h"
#include "dlg/CDlgAbout.h"
#include "dlg/CDlgPrintSetting.h"
#include "env/CShareData.h"
#include "env/CSakuraEnvironment.h"
#include "print/CPrintPreview.h"	/// 2002/2/3 aroka
#include "charset/CharPointer.h"
#include "charset/CCodeFactory.h"
#include "charset/CCodeBase.h"
#include "CEditApp.h"
#include "recent/CMRUFile.h"
#include "recent/CMRUFolder.h"
#include "util/module.h"
#include "util/os.h"		// WM_MOUSEWHEEL,WM_THEMECHANGED
#include "util/window.h"
#include "util/shell.h"
#include "util/string_ex2.h"
#include "plugin/CJackManager.h"
#include "CGrepAgent.h"
#include "CMarkMgr.h"
#include "doc/layout/CLayout.h"
#include "debug/CRunningTimer.h"
#include "sakura_rc.h"


//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ��������̂�
//	��`���폜

#ifndef TBSTYLE_ALTDRAG
	#define TBSTYLE_ALTDRAG	0x0400
#endif
#ifndef TBSTYLE_FLAT
	#define TBSTYLE_FLAT	0x0800
#endif
#ifndef TBSTYLE_LIST
	#define TBSTYLE_LIST	0x1000
#endif



#define		YOHAKU_X		4		// �E�B���h�E���̘g�Ǝ��̌��ԍŏ��l
#define		YOHAKU_Y		4		// �E�B���h�E���̘g�Ǝ��̌��ԍŏ��l
//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ��������̂�
//	��`���폜


//	�󋵂ɂ�胁�j���[�̕\����ς���R�}���h���X�g(SetMenuFuncSel�Ŏg�p)
//		2010/5/19	Uchi
//		2012/10/19	syat	�e����Ή��̂��ߒ萔��
struct FuncMenuName {
	EFunctionCode	eFunc;
	int				nNameId[2];		// �I�𕶎���ID
};

static const FuncMenuName	sFuncMenuName[] = {
	{F_RECKEYMACRO,			{F_RECKEYMACRO_REC,				F_RECKEYMACRO_APPE}},
	{F_SAVEKEYMACRO,		{F_SAVEKEYMACRO_REC,			F_SAVEKEYMACRO_APPE}},
	{F_LOADKEYMACRO,		{F_LOADKEYMACRO_REC,			F_LOADKEYMACRO_APPE}},
	{F_EXECKEYMACRO,		{F_EXECKEYMACRO_REC,			F_EXECKEYMACRO_APPE}},
	{F_SPLIT_V,				{F_SPLIT_V_ON,					F_SPLIT_V_OFF}},
	{F_SPLIT_H,				{F_SPLIT_H_ON,					F_SPLIT_H_OFF}},
	{F_SPLIT_VH,			{F_SPLIT_VH_ON,					F_SPLIT_VH_OFF}},
	{F_TAB_CLOSEOTHER,		{F_TAB_CLOSEOTHER_TAB,			F_TAB_CLOSEOTHER_WINDOW}},
	{F_TOPMOST,				{F_TOPMOST_SET,					F_TOPMOST_REL}},
	{F_BIND_WINDOW,			{F_TAB_GROUPIZE,				F_TAB_GROUPDEL}},
	{F_SHOWTOOLBAR,			{F_SHOWTOOLBAR_ON,				F_SHOWTOOLBAR_OFF}},
	{F_SHOWFUNCKEY,			{F_SHOWFUNCKEY_ON,				F_SHOWFUNCKEY_OFF}},
	{F_SHOWTAB,				{F_SHOWTAB_ON,					F_SHOWTAB_OFF}},
	{F_SHOWSTATUSBAR,		{F_SHOWSTATUSBAR_ON,			F_SHOWSTATUSBAR_OFF}},
	{F_SHOWMINIMAP,			{F_SHOWMINIMAP_ON,				F_SHOWMINIMAP_OFF}},
	{F_TOGGLE_KEY_SEARCH,	{F_TOGGLE_KEY_SEARCH_ON,		F_TOGGLE_KEY_SEARCH_OFF}},
};

static void ShowCodeBox(HWND hWnd, EditDoc* pEditDoc)
{
	// �J�[�\���ʒu�̕�������擾
	const Layout*	pLayout;
	LogicInt		nLineLen;
	const EditView* pView = &pEditDoc->m_pEditWnd->GetActiveView();
	const Caret* pCaret = &pView->GetCaret();
	const LayoutMgr* pLayoutMgr = &pEditDoc->m_layoutMgr;
	const wchar_t* pLine = pLayoutMgr->GetLineStr(pCaret->GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout);

	// -- -- -- -- �L�����b�g�ʒu�̕������ -> szCaretChar -- -- -- -- //
	//
	if (pLine) {
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		LogicInt nIdx = pView->LineColumnToIndex(pLayout, pCaret->GetCaretLayoutPos().GetX2());
		if (nIdx < nLineLen) {
			if (nIdx < nLineLen - (pLayout->GetLayoutEol().GetLen() ? 1 : 0)) {
				// �ꎞ�I�ɕ\�����@�̐ݒ��ύX����
				CommonSetting_StatusBar sStatusbar;
				sStatusbar.m_bDispUniInSjis		= false;
				sStatusbar.m_bDispUniInJis		= false;
				sStatusbar.m_bDispUniInEuc		= false;
				sStatusbar.m_bDispUtf8Codepoint	= false;
				sStatusbar.m_bDispSPCodepoint	= false;

				TCHAR szMsg[128];
				TCHAR szCode[CODE_CODEMAX][32];
				wchar_t szChar[3];
				int nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, nIdx);
				memcpy(szChar, &pLine[nIdx], nCharChars * sizeof(wchar_t));
				szChar[nCharChars] = L'\0';
				for (int i=0; i<CODE_CODEMAX; ++i) {
					if (i == CODE_SJIS || i == CODE_JIS || i == CODE_EUC || i == CODE_LATIN1 || i == CODE_UNICODE || i == CODE_UTF8 || i == CODE_CESU8) {
						//auto_sprintf(szCaretChar, _T("%04x"),);
						// �C�ӂ̕����R�[�h����Unicode�֕ϊ�����		2008/6/9 Uchi
						CodeBase* pCode = CodeFactory::CreateCodeBase((ECodeType)i, false);
						CodeConvertResult ret = pCode->UnicodeToHex(&pLine[nIdx], nLineLen - nIdx, szCode[i], &sStatusbar);
						delete pCode;
						if (ret != CodeConvertResult::Complete) {
							// ���܂��R�[�h�����Ȃ�����
							auto_strcpy(szCode[i], _T("-"));
						}
					}
				}
				// �R�[�h�|�C���g���i�T���Q�[�g�y�A���j
				TCHAR szCodeCP[32];
				sStatusbar.m_bDispSPCodepoint = true;
				CodeBase* pCode = CodeFactory::CreateCodeBase(CODE_UNICODE, false);
				CodeConvertResult ret = pCode->UnicodeToHex(&pLine[nIdx], nLineLen - nIdx, szCodeCP, &sStatusbar);
				delete pCode;
				if (ret != CodeConvertResult::Complete) {
					// ���܂��R�[�h�����Ȃ�����
					auto_strcpy(szCodeCP, _T("-"));
				}

				// ���b�Z�[�W�{�b�N�X�\��
				auto_sprintf_s(szMsg, LS(STR_ERR_DLGEDITWND13),
					szChar, szCodeCP, szCode[CODE_SJIS], szCode[CODE_JIS], szCode[CODE_EUC], szCode[CODE_LATIN1], szCode[CODE_UNICODE], szCode[CODE_UTF8], szCode[CODE_CESU8]);
				::MessageBox(hWnd, szMsg, GSTR_APPNAME, MB_OK);
			}
		}
	}
}

//	// ���b�Z�[�W���[�v
//	DWORD MessageLoop_Thread(DWORD pCEditWndObject);

LRESULT CALLBACK CEditWndProc(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	EditWnd* pWnd = (EditWnd*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (pWnd) {
		return pWnd->DispatchEvent(hwnd, uMsg, wParam, lParam);
	}
	return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
}

//	@date 2002.2.17 YAZAKI ShareData�̃C���X�^���X�́AProcess�ɂЂƂ���̂݁B
EditWnd::EditWnd()
	:
	m_hWnd(NULL)
	, m_toolbar(this)			// warning C4355: 'this' : �x�[�X �����o�[�������q���X�g�Ŏg�p����܂����B
	, m_statusBar(this)		// warning C4355: 'this' : �x�[�X �����o�[�������q���X�g�Ŏg�p����܂����B
	, m_pPrintPreview(NULL) //@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
	, m_pDragSourceView(NULL)
	, m_nActivePaneIndex(0)
	, m_nEditViewCount(1)
	, m_nEditViewMaxCount(_countof(m_pEditViewArr))	// ���̂Ƃ���ő�l�͌Œ�
	, m_uMSIMEReconvertMsg(::RegisterWindowMessage(RWM_RECONVERT)) // 20020331 aroka �ĕϊ��Ή� for 95/NT
	, m_uATOKReconvertMsg(::RegisterWindowMessage(MSGNAME_ATOK_RECONVERT))
	, m_bIsActiveApp(false)
	, m_pszLastCaption(NULL)
	, m_pszMenubarMessage(new TCHAR[MENUBAR_MESSAGE_MAX_LEN])
	, m_posSaveAry(NULL)
	, m_nCurrentFocus(0)
	, m_hAccelWine(NULL)
	, m_hAccel(NULL)
	, m_bDragMode(false)
	, m_iconClicked(IconClickStatus::None) // by �S(2)
	, m_nSelectCountMode(SelectCountMode::Toggle)	// �����J�E���g���@�̏����l��SELECT_COUNT_TOGGLE�����ʐݒ�ɏ]��
{
	g_pcEditWnd = this;
}

EditWnd::~EditWnd()
{
	g_pcEditWnd = NULL;

	delete m_pPrintPreview;
	m_pPrintPreview = NULL;

	for (int i=0; i<m_nEditViewMaxCount; ++i) {
		delete m_pEditViewArr[i];
		m_pEditViewArr[i] = NULL;
	}
	m_pEditView = NULL;

	delete m_pEditViewMiniMap;
	m_pEditViewMiniMap = NULL;

	delete m_pViewFont;
	m_pViewFont = NULL;

	delete m_pViewFontMiniMap;
	m_pViewFontMiniMap = NULL;

	delete[] m_pszMenubarMessage;
	delete[] m_pszLastCaption;

	//	Dec. 4, 2002 genta
	// �L�����b�g�̍s���ʒu�\���p�t�H���g
	::DeleteObject(m_hFontCaretPosInfo);

	delete m_pDropTarget;	// 2008.06.20 ryoji
	m_pDropTarget = NULL;

	// �E�B���h�E���ɍ쐬�����A�N�Z�����[�^�e�[�u����j������(Wine�p)
	DeleteAccelTbl();

	m_hWnd = NULL;
}


// �h�L�������g���X�i�F�Z�[�u��
// 2008.02.02 kobake
void EditWnd::OnAfterSave(const SaveInfo& saveInfo)
{
	// �r���[�ĕ`��
	this->Views_RedrawAll();

	// �L���v�V�����̍X�V���s��
	UpdateCaption();

	// �L�����b�g�̍s���ʒu��\������
	GetActiveView().GetCaret().ShowCaretPosInfo();
}

void EditWnd::UpdateCaption()
{
	if (!GetActiveView().GetDrawSwitch()) {
		return;
	}

	// �L���v�V����������̐��� -> pszCap
	wchar_t	pszCap[1024];
	const CommonSetting_Window& setting = GetDllShareData().m_common.m_window;
	const wchar_t* pszFormat = NULL;
	if (!this->IsActiveApp())	pszFormat = to_wchar(setting.m_szWindowCaptionInactive);
	else						pszFormat = to_wchar(setting.m_szWindowCaptionActive);
	SakuraEnvironment::ExpandParameter(
		pszFormat,
		pszCap,
		_countof(pszCap)
	);

	// �L���v�V�����X�V
	::SetWindowText(this->GetHwnd(), to_tchar(pszCap));

	//@@@ From Here 2003.06.13 MIK
	// �^�u�E�B���h�E�̃t�@�C������ʒm
	SakuraEnvironment::ExpandParameter(GetDllShareData().m_common.m_tabBar.m_szTabWndCaption, pszCap, _countof(pszCap));
	this->ChangeFileNameNotify(to_tchar(pszCap), GetListeningDoc()->m_docFile.GetFilePath(), EditApp::getInstance()->m_pGrepAgent->m_bGrepMode);	// 2006.01.28 ryoji �t�@�C�����AGrep���[�h�p�����[�^��ǉ�
	//@@@ To Here 2003.06.13 MIK
}



// �E�B���h�E�����p�̋�`���擾
void EditWnd::_GetWindowRectForInit(Rect* rcResult, int nGroup, const TabGroupInfo& tabGroupInfo)
{
	// �E�B���h�E�T�C�Y�p��
	int	nWinCX, nWinCY;
	//	2004.05.13 Moca m_common.m_eSaveWindowSize��BOOL����enum�ɕς�������
	auto& csWindow = m_pShareData->m_common.m_window;
	if (csWindow.m_eSaveWindowSize != WinSizeMode::Default) {
		nWinCX = csWindow.m_nWinSizeCX;
		nWinCY = csWindow.m_nWinSizeCY;
	}else {
		nWinCX = CW_USEDEFAULT;
		nWinCY = 0;
	}

	// �E�B���h�E�T�C�Y�w��
	EditInfo fi;
	CommandLine::getInstance()->GetEditInfo(&fi);
	if (fi.m_nWindowSizeX >= 0) {
		nWinCX = fi.m_nWindowSizeX;
	}
	if (fi.m_nWindowSizeY >= 0) {
		nWinCY = fi.m_nWindowSizeY;
	}

	// �E�B���h�E�ʒu�w��
	int nWinOX = CW_USEDEFAULT;
	int nWinOY = 0;
	// �E�B���h�E�ʒu�Œ�
	//	2004.05.13 Moca �ۑ������E�B���h�E�ʒu���g���ꍇ�͋��L����������Z�b�g
	if (csWindow.m_eSaveWindowPos != WinSizeMode::Default) {
		nWinOX =  csWindow.m_nWinPosX;
		nWinOY =  csWindow.m_nWinPosY;
	}

	//	2004.05.13 Moca �}���`�f�B�X�v���C�ł͕��̒l���L���Ȃ̂ŁC
	//	���ݒ�̔�����@��ύX�D(���̒l��CW_USEDEFAULT)
	if (fi.m_nWindowOriginX != CW_USEDEFAULT) {
		nWinOX = fi.m_nWindowOriginX;
	}
	if (fi.m_nWindowOriginY != CW_USEDEFAULT) {
		nWinOY = fi.m_nWindowOriginY;
	}

	// �K�v�Ȃ�A�^�u�O���[�v�Ƀt�B�b�g����悤�A�ύX
	if (tabGroupInfo.IsValid()) {
		RECT rcWork, rcMon;
		GetMonitorWorkRect(tabGroupInfo.hwndTop, &rcWork, &rcMon);

		const WINDOWPLACEMENT& wpTop = tabGroupInfo.wpTop;
		nWinCX = wpTop.rcNormalPosition.right  - wpTop.rcNormalPosition.left;
		nWinCY = wpTop.rcNormalPosition.bottom - wpTop.rcNormalPosition.top;
		nWinOX = wpTop.rcNormalPosition.left   + (rcWork.left - rcMon.left);
		nWinOY = wpTop.rcNormalPosition.top    + (rcWork.top - rcMon.top);
	}

	// ����
	rcResult->SetXYWH(nWinOX, nWinOY, nWinCX, nWinCY);
}

HWND EditWnd::_CreateMainWindow(int nGroup, const TabGroupInfo& tabGroupInfo)
{
	// -- -- -- -- �E�B���h�E�N���X�o�^ -- -- -- -- //
	WNDCLASSEX	wc;
	//	Apr. 27, 2000 genta
	//	�T�C�Y�ύX���̂������}���邽��CS_HREDRAW | CS_VREDRAW ���O����
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= CEditWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 32;
	wc.hInstance		= G_AppInstance();
	//	Dec, 2, 2002 genta �A�C�R���ǂݍ��ݕ��@�ύX
	wc.hIcon			= GetAppIcon(G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, false);

	wc.hCursor			= NULL/*LoadCursor(NULL, IDC_ARROW)*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_3DSHADOW + 1)*/;
	wc.lpszMenuName		= NULL;	// MAKEINTRESOURCE(IDR_MENU1);	2010/5/16 Uchi
	wc.lpszClassName	= GSTR_EDITWINDOWNAME;

	//	Dec. 6, 2002 genta
	//	small icon�w��̂��� RegisterClassEx�ɕύX
	wc.cbSize			= sizeof(wc);
	wc.hIconSm			= GetAppIcon(G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, true);
	ATOM atom = RegisterClassEx(&wc);
	if (atom == 0) {
		//	2004.05.13 Moca return NULL��L���ɂ���
		return NULL;
	}

	// ��`�擾
	Rect rc;
	_GetWindowRectForInit(&rc, nGroup, tabGroupInfo);

	// �쐬
	HWND hwndResult = ::CreateWindowEx(
		0,				 	// extended window style
		GSTR_EDITWINDOWNAME,		// pointer to registered class name
		GSTR_EDITWINDOWNAME,		// pointer to window name
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,	// window style
		rc.left,			// horizontal position of window
		rc.top,				// vertical position of window
		rc.Width(),			// window width
		rc.Height(),		// window height
		NULL,				// handle to parent or owner window
		NULL,				// handle to menu or child-window identifier
		G_AppInstance(),		// handle to application instance
		NULL				// pointer to window-creation data
	);
	return hwndResult;
}

void EditWnd::_GetTabGroupInfo(TabGroupInfo* pTabGroupInfo, int& nGroup)
{
	HWND hwndTop = NULL;
	WINDOWPLACEMENT	wpTop = {0};

	// From Here @@@ 2003.05.31 MIK
	// �^�u�E�B���h�E�̏ꍇ�͌���l���w��
	if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd && !m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin) {
		if (nGroup < 0)	// �s���ȃO���[�vID
			nGroup = 0;	// �O���[�v�w�薳���i�ŋ߃A�N�e�B�u�̃O���[�v�ɓ����j
		EditNode* pEditNode = AppNodeGroupHandle(nGroup).GetEditNodeAt(0);	// �O���[�v�̐擪�E�B���h�E�����擾	// 2007.06.20 ryoji
		hwndTop = pEditNode? pEditNode->GetHwnd(): NULL;

		if (hwndTop) {
			//	Sep. 11, 2003 MIK �V�KTAB�E�B���h�E�̈ʒu����ɂ���Ȃ��悤��
			// 2007.06.20 ryoji ��v���C�}�����j�^�܂��̓^�X�N�o�[�𓮂�������ł�����Ȃ��悤��

			wpTop.length = sizeof(wpTop);
			if (::GetWindowPlacement(hwndTop, &wpTop)) {	// ���݂̐擪�E�B���h�E����ʒu���擾
				if (wpTop.showCmd == SW_SHOWMINIMIZED)
					wpTop.showCmd = pEditNode->m_showCmdRestore;
			}else {
				hwndTop = NULL;
			}
		}
	}
	// To Here @@@ 2003.05.31 MIK

	// ����
	pTabGroupInfo->hwndTop = hwndTop;
	pTabGroupInfo->wpTop = wpTop;
}

void EditWnd::_AdjustInMonitor(const TabGroupInfo& tabGroupInfo)
{
	RECT	rcOrg;
	RECT	rcDesktop;
//	int		nWork;

	//	May 01, 2004 genta �}���`���j�^�Ή�
	::GetMonitorWorkRect(GetHwnd(), &rcDesktop);
	::GetWindowRect(GetHwnd(), &rcOrg);

	// 2005.11.23 Moca �}���`���j�^���Ŗ�肪���������ߌv�Z���@�ύX
	// �E�B���h�E�ʒu����
	if (rcOrg.bottom > rcDesktop.bottom) {
		rcOrg.top -= rcOrg.bottom - rcDesktop.bottom;
		rcOrg.bottom = rcDesktop.bottom;	//@@@ 2002.01.08
	}
	if (rcOrg.right > rcDesktop.right) {
		rcOrg.left -= rcOrg.right - rcDesktop.right;
		rcOrg.right = rcDesktop.right;	//@@@ 2002.01.08
	}
	
	if (rcOrg.top < rcDesktop.top) {
		rcOrg.bottom += rcDesktop.top - rcOrg.top;
		rcOrg.top = rcDesktop.top;
	}
	if (rcOrg.left < rcDesktop.left) {
		rcOrg.right += rcDesktop.left - rcOrg.left;
		rcOrg.left = rcDesktop.left;
	}

	// �E�B���h�E�T�C�Y����
	if (rcOrg.bottom > rcDesktop.bottom) {
		//rcOrg.bottom = rcDesktop.bottom - 1;	//@@@ 2002.01.08
		rcOrg.bottom = rcDesktop.bottom;	//@@@ 2002.01.08
	}
	if (rcOrg.right > rcDesktop.right) {
		//rcOrg.right = rcDesktop.right - 1;	//@@@ 2002.01.08
		rcOrg.right = rcDesktop.right;	//@@@ 2002.01.08
	}

	// From Here @@@ 2003.06.13 MIK
	if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd
		&& !m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin
		&& tabGroupInfo.hwndTop
	) {
		// ���݂̐擪�E�B���h�E���� WS_EX_TOPMOST ��Ԃ������p��	// 2007.05.18 ryoji
		DWORD dwExStyle = (DWORD)::GetWindowLongPtr(tabGroupInfo.hwndTop, GWL_EXSTYLE);
		::SetWindowPos(GetHwnd(), (dwExStyle & WS_EX_TOPMOST)? HWND_TOPMOST: HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		// �^�u�E�B���h�E���͌�����ێ�
		// �E�B���h�E�T�C�Y�p��
		// Vista �ȍ~�̏���\���A�j���[�V�������ʂ�}�~����
		if (!IsWinVista_or_later()) {
			if (tabGroupInfo.wpTop.showCmd == SW_SHOWMAXIMIZED) {
				::ShowWindow(GetHwnd(), SW_SHOWMAXIMIZED);
			}else {
				::ShowWindow(GetHwnd(), SW_SHOW);
			}
		}else {
			// ����\���̃A�j���[�V�������ʂ�}�~����

			// �擪�E�B���h�E�̔w��ŉ�ʕ`�悵�Ă����O�ɏo���i�c�[���o�[��r���[�̂������}����j
			// �����ł́A���ƂŐ����ɓK�p�����͂��̃h�L�������g�^�C�v�����ݒ肵�Ĉꎞ�`�悵�Ă����i�r���[�̔z�F�ؑւɂ�邿�����}����j
			// ����ɁA�^�C�v��߂��ĉ�ʂ𖳌����������Ă����i���炩�̌����œr����~�����ꍇ�ɂ͂��Ƃ̃^�C�v�F�ōĕ`�悳���悤�� �� �Ⴆ�΃t�@�C���T�C�Y���傫������x�����o���Ƃ��Ȃǁj
			// �� ���U�@�Ƃ͂����Ȃ���������Ȃ�����������������邱�ƂȂ��Ȍ��ɍς܂�����̂ł������Ă���
			TypeConfigNum typeOld, typeNew(-1);
			typeOld = GetDocument()->m_docType.GetDocumentType();	// ���݂̃^�C�v
			{
				EditInfo ei, mruei;
				CommandLine::getInstance()->GetEditInfo(&ei);
				if (ei.m_szDocType[0] != '\0') {
					typeNew = DocTypeManager().GetDocumentTypeOfExt(ei.m_szDocType);
				}else {
					if (MRUFile().GetEditInfo(ei.m_szPath, &mruei) && 0 < mruei.m_nTypeId) {
						typeNew = DocTypeManager().GetDocumentTypeOfId(mruei.m_nTypeId);
					}
					if (!typeNew.IsValidType()) {
						if (ei.m_szPath[0]) {
							typeNew = DocTypeManager().GetDocumentTypeOfPath(ei.m_szPath);
						}else {
							typeNew = typeOld;
						}
					}
				}
			}
			GetDocument()->m_docType.SetDocumentType(typeNew, true, true);	// ���ݒ�

			// �\�Ȍ����ʕ`��̗l�q�������Ȃ��悤�ꎞ�I�ɐ擪�E�B���h�E�̌��ɔz�u
			::SetWindowPos(GetHwnd(), tabGroupInfo.hwndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

			// �A�j���[�V�������ʂ͈ꎞ�I�� OFF �ɂ���
			ANIMATIONINFO ai = {sizeof(ANIMATIONINFO)};
			::SystemParametersInfo(SPI_GETANIMATION, sizeof(ANIMATIONINFO), &ai, 0);
			int iMinAnimateOld = ai.iMinAnimate;
			ai.iMinAnimate = 0;
			::SystemParametersInfo(SPI_SETANIMATION, sizeof(ANIMATIONINFO), &ai, 0);

			// ��������i�ő剻�̂Ƃ��͎��� ::ShowWindow() �Ŏ�O�ɏo�Ă��܂��̂ŁA�A�j���[�V�����������ʂ͂��邪�N���C�A���g�̈�̂�����͗}������Ȃ��j
			int nCmdShow = (tabGroupInfo.wpTop.showCmd == SW_SHOWMAXIMIZED)? SW_SHOWMAXIMIZED: SW_SHOWNOACTIVATE;
			::ShowWindow(GetHwnd(), nCmdShow);
			::UpdateWindow(GetHwnd());	// ��ʍX�V
			::BringWindowToTop(GetHwnd());
			::ShowWindow(tabGroupInfo.hwndTop , SW_HIDE);	// �ȑO�̐擪�E�B���h�E�͂����ŏ����Ă����Ȃ��Ə�����A�j���[�V������������ꍇ������

			// �A�j���[�V�������ʂ�߂�
			ai.iMinAnimate = iMinAnimateOld;
			::SystemParametersInfo(SPI_SETANIMATION, sizeof(ANIMATIONINFO), &ai, 0);

			// �A�C�h�����O�J�n���ɂ��̎��_�̃^�C�v�ʐݒ�F�ōĕ`�悳���悤�ɂ��Ă���
			GetDocument()->m_docType.SetDocumentType(typeOld, true, true);	// �^�C�v�߂�
			::InvalidateRect(GetHwnd(), NULL, TRUE);	// ��ʖ�����
		}
	}else {
		::SetWindowPos(
			GetHwnd(), 0,
			rcOrg.left, rcOrg.top,
			rcOrg.right - rcOrg.left, rcOrg.bottom - rcOrg.top,
			SWP_NOOWNERZORDER | SWP_NOZORDER
		);

		// �E�B���h�E�T�C�Y�p��
		auto& csWindow = m_pShareData->m_common.m_window;
		if (csWindow.m_eSaveWindowSize != WinSizeMode::Default &&
			csWindow.m_nWinSizeType == SIZE_MAXIMIZED
		) {
			::ShowWindow(GetHwnd(), SW_SHOWMAXIMIZED);
		}else
		// 2004.05.14 Moca �E�B���h�E�T�C�Y�𒼐ڎw�肷��ꍇ�́A�ŏ����\�����󂯓����
		if (csWindow.m_eSaveWindowSize == WinSizeMode::Set &&
			csWindow.m_nWinSizeType == SIZE_MINIMIZED
		) {
			::ShowWindow(GetHwnd(), SW_SHOWMINIMIZED);
		}else {
			::ShowWindow(GetHwnd(), SW_SHOW);
		}
	}
	// To Here @@@ 2003.06.13 MIK
}

/*!
	�쐬

	@date 2002.03.07 genta nDocumentType�ǉ�
	@date 2007.06.26 ryoji nGroup�ǉ�
	@date 2008.04.19 ryoji ����A�C�h�����O���o�p�[���b�^�C�}�[�̃Z�b�g������ǉ�
*/
HWND EditWnd::Create(
	EditDoc*		pEditDoc,
	ImageListMgr*	pIcons,	// [in] Image List
	int				nGroup		// [in] �O���[�vID
	)
{
	MY_RUNNINGTIMER(cRunningTimer, "EditWnd::Create");

	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	m_pShareData = &GetDllShareData();

	m_pEditDoc = pEditDoc;

	for (int i=0; i<_countof(m_pEditViewArr); ++i) {
		m_pEditViewArr[i] = NULL;
	}
	// [0] - [3] �܂ō쐬�E���������Ă������̂�[0]�������B�ق��͕��������܂ŉ������Ȃ�
	m_pEditViewArr[0] = new EditView(this);
	m_pEditView = m_pEditViewArr[0];

	m_pViewFont = new ViewFont(&GetLogfont());

	m_pEditViewMiniMap = new EditView(this);

	m_pViewFontMiniMap = new ViewFont(&GetLogfont(), true);

	auto_memset(m_pszMenubarMessage, _T(' '), MENUBAR_MESSAGE_MAX_LEN);	// null�I�[�͕s�v

	//	Dec. 4, 2002 genta
	InitMenubarMessageFont();

	m_pDropTarget = new DropTarget(this);	// �E�{�^���h���b�v�p	// 2008.06.20 ryoji

	// 2009.01.17 nasukoji	�z�C�[���X�N���[���L����Ԃ��N���A
	ClearMouseState();

	// �E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
	CreateAccelTbl();

	// �E�B���h�E������
	if (m_pShareData->m_nodes.m_nEditArrNum >= MAX_EDITWINDOWS) {	// �ő�l�C��	//@@@ 2003.05.31 MIK
		OkMessage(NULL, LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
		return NULL;
	}

	// �^�u�O���[�v���擾
	TabGroupInfo tabGroupInfo;
	_GetTabGroupInfo(&tabGroupInfo, nGroup);


	// -- -- -- -- �E�B���h�E�쐬 -- -- -- -- //
	HWND hWnd = _CreateMainWindow(nGroup, tabGroupInfo);
	if (!hWnd) {
		return NULL;
	}
	m_hWnd = hWnd;

	// ����A�C�h�����O���o�p�̃[���b�^�C�}�[���Z�b�g����	// 2008.04.19 ryoji
	// �[���b�^�C�}�[�������i����A�C�h�����O���o�j������ MYWM_FIRST_IDLE ���N�����v���Z�X�Ƀ|�X�g����B
	// ���N�����ł̋N����A�C�h�����O���o�ɂ��Ă� ControlTray::OpenNewEditor ���Q��
	::SetTimer(GetHwnd(), IDT_FIRST_IDLE, 0, NULL);

	// �ҏW�E�B���h�E���X�g�ւ̓o�^
	// 2011.01.12 ryoji ���̏����͈ȑO�̓E�B���h�E����������̈ʒu�ɂ�����
	// Vista/7 �ł̏���\���A�j���[�V�����}�~�irev1868�j�Ƃ̂���݂ŁA�E�B���h�E����������鎞�_�Ń^�u�o�[�ɑS�^�u�������Ă��Ȃ��ƌ��ꂵ���̂ł����Ɉړ��B
	// AddEditWndList() �Ŏ��E�B���h�E�Ƀ|�X�g����� MYWM_TAB_WINDOW_NOTIFY(TWNT_ADD) �̓^�u�o�[�쐬��̏���A�C�h�����O���ɏ��������̂œ��ɖ��͖����͂��B
	if (!AppNodeGroupHandle(nGroup).AddEditWndList(GetHwnd())) {	// 2007.06.26 ryoji nGroup�����ǉ�
		OkMessage(GetHwnd(), LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
		::DestroyWindow(GetHwnd());
		m_hWnd = hWnd = NULL;
		return hWnd;
	}

	// �R�����R���g���[��������
	MyInitCommonControls();

	// �C���[�W�A�w���p�Ȃǂ̍쐬
	m_menuDrawer.Create(G_AppInstance(), GetHwnd(), pIcons);
	m_toolbar.Create(pIcons);

	// �v���O�C���R�}���h��o�^����
	RegisterPluginCommand();

	SelectCharWidthCache(CharWidthFontMode::MiniMap, CharWidthCacheMode::Local); // Init
	InitCharWidthCache(m_pViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap);
	SelectCharWidthCache(CharWidthFontMode::Edit, GetLogfontCacheMode());
	InitCharWidthCache(GetLogfont());


	// -- -- -- -- �q�E�B���h�E�쐬 -- -- -- -- //

	// �����t���[���쐬
	m_splitterWnd.Create(G_AppInstance(), GetHwnd(), this);

	// �r���[
	GetView(0).Create(m_splitterWnd.GetHwnd(), GetDocument(), 0, TRUE, false);
	GetView(0).OnSetFocus();

	// �q�E�B���h�E�̐ݒ�
	HWND hWndArr[2];
	hWndArr[0] = GetView(0).GetHwnd();
	hWndArr[1] = NULL;
	m_splitterWnd.SetChildWndArr(hWndArr);

	MY_TRACETIME(cRunningTimer, "View created");


	// -- -- -- -- �e��o�[�쐬 -- -- -- -- //

	// ���C�����j���[
	LayoutMainMenu();

	// �c�[���o�[
	LayoutToolBar();

	// �X�e�[�^�X�o�[
	LayoutStatusBar();

	// �t�@���N�V�����L�[ �o�[
	LayoutFuncKey();

	// �^�u�E�B���h�E
	LayoutTabBar();

	// �~�j�}�b�v
	LayoutMiniMap();

	// �o�[�̔z�u�I��
	EndLayoutBars(FALSE);

	// -- -- -- -- ���̑������Ȃ� -- -- -- -- //

	// ��ʕ\�����O��DispatchEvent��L��������
	::SetWindowLongPtr(GetHwnd(), GWLP_USERDATA, (LONG_PTR)this);

	// �f�X�N�g�b�v����͂ݏo���Ȃ��悤�ɂ���
	_AdjustInMonitor(tabGroupInfo);

	// �h���b�v���ꂽ�t�@�C�����󂯓����
	::DragAcceptFiles(GetHwnd(), TRUE);
	m_pDropTarget->Register_DropTarget(m_hWnd);	// �E�{�^���h���b�v�p	// 2008.06.20 ryoji

	// �A�N�e�B�u���
	m_bIsActiveApp = (::GetActiveWindow() == GetHwnd());	// 2007.03.08 ryoji

	// �G�f�B�^�|�g���C�Ԃł�UI���������̊m�F�iVista UIPI�@�\�j 2007.06.07 ryoji
	if (IsWinVista_or_later()) {
		m_bUIPI = FALSE;
		::SendMessage(m_pShareData->m_handles.m_hwndTray, MYWM_UIPI_CHECK,  (WPARAM)0, (LPARAM)GetHwnd());
		if (!m_bUIPI) {	// �Ԏ����Ԃ�Ȃ�
			TopErrorMessage(GetHwnd(),
				LS(STR_ERR_DLGEDITWND02)
			);
			::DestroyWindow(GetHwnd());
			m_hWnd = hWnd = NULL;
			return hWnd;
		}
	}

	ShareData::getInstance()->SetTraceOutSource(GetHwnd());	// TraceOut()�N�����E�B���h�E�̐ݒ�	// 2006.06.26 ryoji

	// Aug. 29, 2003 wmlhq
	m_nTimerCount = 0;
	// �^�C�}�[���N��	�^�C�}�[��ID�ƊԊu��ύX 20060128 aroka
	if (::SetTimer(GetHwnd(), IDT_EDIT, 500, NULL) == 0) {
		WarningMessage(GetHwnd(), LS(STR_ERR_DLGEDITWND03));
	}
	// �c�[���o�[�̃^�C�}�[�𕪗����� 20060128 aroka
	Timer_ONOFF(true);

	// �f�t�H���g��IME���[�h�ݒ�
	GetDocument()->m_docEditor.SetImeMode(GetDocument()->m_docType.GetDocumentAttribute().m_nImeState);

	return GetHwnd();
}



// �N�����̃t�@�C���I�[�v������
void EditWnd::OpenDocumentWhenStart(
	const LoadInfo& argLoadInfo		// [in]
	)
{
	if (argLoadInfo.filePath.Length()) {
		::ShowWindow(GetHwnd(), SW_SHOW);
		// Oct. 03, 2004 genta �R�[�h�m�F�͐ݒ�Ɉˑ�
		LoadInfo loadInfo = argLoadInfo;
		bool bReadResult = GetDocument()->m_docFileOperation.FileLoadWithoutAutoMacro(&loadInfo);	// �������s�}�N���͌�ŕʂ̏ꏊ�Ŏ��s�����
		if (!bReadResult) {
			// �t�@�C�������ɊJ����Ă���
			if (loadInfo.bOpened) {
				::PostMessage(GetHwnd(), WM_CLOSE, 0, 0);
				// 2004.07.12 Moca return NULL���ƁA���b�Z�[�W���[�v��ʂ炸�ɂ��̂܂ܔj������Ă��܂��A�^�u�̏I��������������
				//	���̌�͐��탋�[�g�Ń��b�Z�[�W���[�v�ɓ�������WM_CLOSE����M���Ē�����CLOSE & DESTROY�ƂȂ�D
				//	���̒��ŕҏW�E�B���h�E�̍폜���s����D
			}
		}
	}
}

void EditWnd::SetDocumentTypeWhenCreate(
	ECodeType		nCharCode,		// [in] �����R�[�h
	bool			bViewMode,		// [in] �r���[���[�h�ŊJ�����ǂ���
	TypeConfigNum	nDocumentType	// [in] �����^�C�v�D-1�̂Ƃ������w�薳���D
	)
{
	//	Mar. 7, 2002 genta �����^�C�v�̋����w��
	//	Jun. 4 ,2004 genta �t�@�C�����w�肪�����Ă��^�C�v�����w���L���ɂ���
	if (nDocumentType.IsValidType()) {
		GetDocument()->m_docType.SetDocumentType(nDocumentType, true);
		//	2002/05/07 YAZAKI �^�C�v�ʐݒ�ꗗ�̈ꎞ�K�p�̃R�[�h�𗬗p
		GetDocument()->m_docType.LockDocumentType();
	}

	// �����R�[�h�̎w��	2008/6/14 Uchi
	if (IsValidCodeType(nCharCode) || nDocumentType.IsValidType()) {
		const TypeConfig& types = GetDocument()->m_docType.GetDocumentAttribute();
		ECodeType eDefaultCharCode = types.m_encoding.m_eDefaultCodetype;
		if (!IsValidCodeType(nCharCode)) {
			nCharCode = eDefaultCharCode;	// ���ڃR�[�h�w�肪�Ȃ���΃^�C�v�w��̃f�t�H���g�����R�[�h���g�p
		}
		if (nCharCode == eDefaultCharCode) {	// �f�t�H���g�����R�[�h�Ɠ��������R�[�h���I�����ꂽ�Ƃ�
			GetDocument()->SetDocumentEncoding(nCharCode, types.m_encoding.m_bDefaultBom);
			GetDocument()->m_docEditor.m_newLineCode = static_cast<EolType>(types.m_encoding.m_eDefaultEoltype);
		}else {
			GetDocument()->SetDocumentEncoding(nCharCode, CodeTypeName(nCharCode).IsBomDefOn());
			GetDocument()->m_docEditor.m_newLineCode = EolType::CRLF;
		}
	}

	//	Jun. 4 ,2004 genta �t�@�C�����w�肪�����Ă��r���[���[�h�����w���L���ɂ���
	AppMode::getInstance()->SetViewMode(bViewMode);

	if (nDocumentType.IsValidType()) {
		// �ݒ�ύX�𔽉f������
		GetDocument()->OnChangeSetting();	// <--- ������ BlockingHook() �Ăяo��������̂ŗ��܂����`�悪�����Ŏ��s�����
	}
}


/*! ���C�����j���[�̔z�u����
	@date 2010/05/16 Uchi
	@date 2012/10/18 syat �e����Ή�
*/
void EditWnd::LayoutMainMenu()
{
	TCHAR		szLabel[300];
	TCHAR		szKey[10];
	CommonSetting_MainMenu*	pMenu = &m_pShareData->m_common.m_mainMenu;
	MainMenu*	mainMenu;
	HWND		hWnd = GetHwnd();
	HMENU		hMenu;
	int 		nCount;
	LPCTSTR		pszName;

	hMenu = ::CreateMenu();
	auto& csKeyBind = m_pShareData->m_common.m_keyBind;
	for (int i=0; i<MAX_MAINMENU_TOP && pMenu->m_nMenuTopIdx[i] >= 0; ++i) {
		nCount = (i >= MAX_MAINMENU_TOP || pMenu->m_nMenuTopIdx[i + 1] < 0 ? pMenu->m_nMainMenuNum : pMenu->m_nMenuTopIdx[i+1])
				- pMenu->m_nMenuTopIdx[i];		// ���j���[���ڐ�
		mainMenu = &pMenu->m_mainMenuTbl[pMenu->m_nMenuTopIdx[i]];
		switch (mainMenu->m_nType) {
		case MainMenuType::Node:
			// ���x�����ݒ肩��Function�R�[�h������Ȃ�X�g�����O�e�[�u������擾 2012/10/18 syat �e����Ή�
			pszName = (mainMenu->m_sName[0] == L'\0' && mainMenu->nFunc != F_NODE)
								? LS(mainMenu->nFunc) : to_tchar(mainMenu->m_sName);
			::AppendMenu(hMenu, MF_POPUP | MF_STRING | (nCount <= 1 ? MF_GRAYED : 0), (UINT_PTR)CreatePopupMenu(), 
				KeyBind::MakeMenuLabel(pszName, to_tchar(mainMenu->m_sKey)));
			break;
		case MainMenuType::Leaf:
			// ���j���[���x���̍쐬
			// 2014.05.04 Moca �v���O�C��/�}�N������u����悤��Funccode2Name���g���悤��
			{
				WCHAR szLabelW[256];
				GetDocument()->m_funcLookup.Funccode2Name(mainMenu->nFunc, szLabelW, 256);
				auto_strncpy(szLabel, to_tchar(szLabelW), _countof(szLabel) - 1);
				szLabel[_countof(szLabel) - 1] = _T('\0');
			}
			auto_strcpy(szKey, to_tchar(mainMenu->m_sKey));
			if (!KeyBind::GetMenuLabel(
				G_AppInstance(),
				csKeyBind.m_nKeyNameArrNum,
				csKeyBind.m_pKeyNameArr,
				mainMenu->nFunc,
				szLabel,
				to_tchar(mainMenu->m_sKey),
				FALSE,
				_countof(szLabel)
				)
			) {
				auto_strcpy(szLabel, _T("?"));
			}
			::AppendMenu(hMenu, MF_STRING, mainMenu->nFunc, szLabel);
			break;
		case MainMenuType::Separator:
			::AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
			break;
		case MainMenuType::Special:
			nCount = 0;
			switch (mainMenu->nFunc) {
			case F_WINDOW_LIST:				// �E�B���h�E���X�g
				EditNode*	pEditNodeArr;
				nCount = AppNodeManager::getInstance()->GetOpenedWindowArr(&pEditNodeArr, TRUE);
				delete [] pEditNodeArr;
				break;
			case F_FILE_USED_RECENTLY:		// �ŋߎg�����t�@�C��
				{
					RecentFile	cRecentFile;
					nCount = cRecentFile.GetViewCount();
				}
				break;
			case F_FOLDER_USED_RECENTLY:	// �ŋߎg�����t�H���_
				{
					RecentFolder	cRecentFolder;
					nCount = cRecentFolder.GetViewCount();
				}
				break;
			case F_CUSTMENU_LIST:			// �J�X�^�����j���[���X�g
				//	�E�N���b�N���j���[
				if (m_pShareData->m_common.m_customMenu.m_nCustMenuItemNumArr[0] > 0) {
					++nCount;
				}
				//	�J�X�^�����j���[
				for (int j=1; j<MAX_CUSTOM_MENU; ++j) {
					if (m_pShareData->m_common.m_customMenu.m_nCustMenuItemNumArr[j] > 0) {
						++nCount;
					}
				}
				break;
			case F_USERMACRO_LIST:			// �o�^�ς݃}�N�����X�g
				for (int j=0; j<MAX_CUSTMACRO; ++j) {
					MacroRec *mp = &m_pShareData->m_common.m_macro.m_macroTable[j];
					if (mp->IsEnabled()) {
						++nCount;
					}
				}
				break;
			case F_PLUGIN_LIST:				// �v���O�C���R�}���h���X�g
				// �v���O�C���R�}���h��񋟂���v���O�C����񋓂���
				{
					const JackManager* pJackManager = JackManager::getInstance();

					Plug::Array plugs = pJackManager->GetPlugs(PP_COMMAND);
					for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
						++nCount;
					}
				}
				break;
			}
			::AppendMenu(hMenu, MF_POPUP | MF_STRING | (nCount <= 0 ? MF_GRAYED : 0), (UINT_PTR)CreatePopupMenu(), 
				KeyBind::MakeMenuLabel(LS(mainMenu->nFunc), to_tchar(mainMenu->m_sKey)));
			break;
		}
	}
	HMENU hMenuOld = ::GetMenu(hWnd);
	SetMenu(hWnd, hMenu);
	if (hMenuOld) {
		DestroyMenu(hMenuOld);
	}

	DrawMenuBar(hWnd);
}

/*! �c�[���o�[�̔z�u����
	@date 2006.12.19 ryoji �V�K�쐬
*/
void EditWnd::LayoutToolBar(void)
{
	if (m_pShareData->m_common.m_window.m_bDispTOOLBAR) {	// �c�[���o�[��\������
		m_toolbar.CreateToolBar();
	}else {
		m_toolbar.DestroyToolBar();
	}
}

/*! �X�e�[�^�X�o�[�̔z�u����
	@date 2006.12.19 ryoji �V�K�쐬
*/
void EditWnd::LayoutStatusBar(void)
{
	if (m_pShareData->m_common.m_window.m_bDispSTATUSBAR) {	// �X�e�[�^�X�o�[��\������
		// �X�e�[�^�X�o�[�쐬
		m_statusBar.CreateStatusBar();
	}else {
		// �X�e�[�^�X�o�[�j��
		m_statusBar.DestroyStatusBar();
	}
}

/*! �t�@���N�V�����L�[�̔z�u����
	@date 2006.12.19 ryoji �V�K�쐬
*/
void EditWnd::LayoutFuncKey(void)
{
	if (m_pShareData->m_common.m_window.m_bDispFUNCKEYWND) {	// �t�@���N�V�����L�[��\������
		if (!m_funcKeyWnd.GetHwnd()) {
			bool bSizeBox;
			if (m_pShareData->m_common.m_window.m_nFUNCKEYWND_Place == 0) {	// �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
				bSizeBox = false;
			}else {
				bSizeBox = true;
				// �X�e�[�^�X�o�[������Ƃ��̓T�C�Y�{�b�N�X��\�����Ȃ�
				if (m_statusBar.GetStatusHwnd()) {
					bSizeBox = false;
				}
			}
			m_funcKeyWnd.Open(G_AppInstance(), GetHwnd(), GetDocument(), bSizeBox);
		}
	}else {
		m_funcKeyWnd.Close();
	}
}

/*! �^�u�o�[�̔z�u����
	@date 2006.12.19 ryoji �V�K�쐬
*/
void EditWnd::LayoutTabBar(void)
{
	if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd) {	// �^�u�o�[��\������
		if (!m_tabWnd.GetHwnd()) {
			m_tabWnd.Open(G_AppInstance(), GetHwnd());
		}else {
			m_tabWnd.UpdateStyle();
		}
	}else {
		m_tabWnd.Close();
		m_tabWnd.SizeBox_ONOFF(false);
	}
}

/*! �~�j�}�b�v�̔z�u����
	@date 2014.07.14 �V�K�쐬
*/
void EditWnd::LayoutMiniMap( void )
{
	if (m_pShareData->m_common.m_window.m_bDispMiniMap) {	// �^�u�o�[��\������
		if (!GetMiniMap().GetHwnd()) {
			GetMiniMap().Create(GetHwnd(), GetDocument(), -1, TRUE, true);
		}
	}else {
		if (GetMiniMap().GetHwnd()) {
			GetMiniMap().Close();
		}
	}
}

/*! �o�[�̔z�u�I������
	@date 2006.12.19 ryoji �V�K�쐬
	@date 2007.03.04 ryoji ����v���r���[���̓o�[���B��
	@date 2011.01.21 ryoji �A�E�g���C����ʂɃS�~���`�悳���̂�}�~����
*/
void EditWnd::EndLayoutBars(BOOL bAdjust/* = TRUE*/)
{
	int nCmdShow = m_pPrintPreview? SW_HIDE: SW_SHOW;
	HWND hwndToolBar = m_toolbar.GetRebarHwnd() ? m_toolbar.GetRebarHwnd(): m_toolbar.GetToolbarHwnd();
	if (hwndToolBar)
		::ShowWindow(hwndToolBar, nCmdShow);
	if (m_statusBar.GetStatusHwnd())
		::ShowWindow(m_statusBar.GetStatusHwnd(), nCmdShow);
	if (m_funcKeyWnd.GetHwnd())
		::ShowWindow(m_funcKeyWnd.GetHwnd(), nCmdShow);
	if (m_tabWnd.GetHwnd())
		::ShowWindow(m_tabWnd.GetHwnd(), nCmdShow);
	if (m_dlgFuncList.GetHwnd() && m_dlgFuncList.IsDocking()) {
		::ShowWindow(m_dlgFuncList.GetHwnd(), nCmdShow);
		// �A�E�g���C�����Ŕw��ɂ��Ă����i�S�~�`��̗}�~��j
		// ���̑΍�ȑO�́A�A�E�g���C�������h�b�L���O���Ă����ԂŁA
		// ���j���[����[�t�@���N�V�����L�[��\��]/[�X�e�[�^�X�o�[��\��]�����s���Ĕ�\���̃o�[���A�E�g���C�������ɕ\��������A
		// ���̌�A�E�B���h�E�̉������E���㉺�h���b�O���ăT�C�Y�ύX����ƃS�~������邱�Ƃ��������B
		::SetWindowPos(m_dlgFuncList.GetHwnd(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	if (bAdjust) {
		RECT rc;
		m_splitterWnd.DoSplit(-1, -1);
		::GetClientRect(GetHwnd(), &rc);
		::SendMessage(GetHwnd(), WM_SIZE, m_nWinSizeType, MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
		::RedrawWindow(GetHwnd(), NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);	// �X�e�[�^�X�o�[�ɕK�v�H

		GetActiveView().SetIMECompFormPos();
	}
}

static inline BOOL MyIsDialogMessage(HWND hwnd, MSG* msg)
{
	if (!hwnd) {
		return FALSE;
	}
	return ::IsDialogMessage(hwnd, msg);
}

// �����v���Z�X��
// ���b�Z�[�W���[�v
// 2004.02.17 Moca GetMessage�̃G���[�`�F�b�N
void EditWnd::MessageLoop(void)
{
	MSG	msg;
	while (GetHwnd()) {
		// ���b�Z�[�W�擾
		int ret = GetMessage(&msg, NULL, 0, 0);
		if (ret == 0) break; // WM_QUIT
		if (ret == -1) break; // GetMessage���s

		// �_�C�A���O���b�Z�[�W
		     if (MyIsDialogMessage(m_pPrintPreview->GetPrintPreviewBarHANDLE_Safe(),	&msg)) {}	// ����v���r���[ ����o�[
		else if (MyIsDialogMessage(m_dlgFind.GetHwnd(),								&msg)) {}	//�u�����v�_�C�A���O
		else if (MyIsDialogMessage(m_dlgFuncList.GetHwnd(),							&msg)) {}	//�u�A�E�g���C���v�_�C�A���O
		else if (MyIsDialogMessage(m_dlgReplace.GetHwnd(),								&msg)) {}	//�u�u���v�_�C�A���O
		else if (MyIsDialogMessage(m_dlgGrep.GetHwnd(),								&msg)) {}	//�uGrep�v�_�C�A���O
		else if (MyIsDialogMessage(m_hokanMgr.GetHwnd(),								&msg)) {}	//�u���͕⊮�v
		else if (m_toolbar.EatMessage(&msg)) { }													// �c�[���o�[
		// �A�N�Z�����[�^
		else {
			if (m_hAccel && TranslateAccelerator(msg.hwnd, m_hAccel, &msg)) {}
			// �ʏ탁�b�Z�[�W
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
}


LRESULT EditWnd::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	int					nRet;
	LPNMHDR				pnmh;
	int					nPane;
	EditInfo*			pfi;
	LPHELPINFO			lphi;
	
	UINT				idCtl;	// �R���g���[����ID
	MEASUREITEMSTRUCT*	lpmis;
	LPDRAWITEMSTRUCT	lpdis;	// ���ڕ`����
	int					nItemWidth;
	int					nItemHeight;
	UINT				uItem;
	LRESULT				lRes;
	TypeConfigNum		typeNew;

	switch (uMsg) {
	case WM_PAINTICON:
		return 0;
	case WM_ICONERASEBKGND:
		return 0;
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam, lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam, lParam);
	case WM_MOUSEWHEEL:
		return OnMouseWheel(wParam, lParam);
	case WM_HSCROLL:
		return OnHScroll(wParam, lParam);
	case WM_VSCROLL:
		return OnVScroll(wParam, lParam);


	case WM_MENUCHAR:
		// ���j���[�A�N�Z�X�L�[�������̏���(WM_MENUCHAR����)
		return m_menuDrawer.OnMenuChar(hwnd, uMsg, wParam, lParam);

	// 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	case WM_SHOWWINDOW:
		if (!wParam) {
			Views_DeleteCompatibleBitmap();
		}
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);

	case WM_MENUSELECT:
		if (!m_statusBar.GetStatusHwnd()) {
			return 1;
		}
		uItem = (UINT) LOWORD(wParam);		// menu item or submenu index
		{
			// ���j���[�@�\�̃e�L�X�g���Z�b�g
			NativeT memWork;

			// �@�\�ɑΉ�����L�[���̎擾(����)
			NativeT** ppcAssignedKeyList;
			auto& csKeyBind = m_pShareData->m_common.m_keyBind;
			int nAssignedKeyNum = KeyBind::GetKeyStrList(
				G_AppInstance(),
				csKeyBind.m_nKeyNameArrNum,
				(KEYDATA*)csKeyBind.m_pKeyNameArr,
				&ppcAssignedKeyList,
				uItem
			);
			if (0 < nAssignedKeyNum) {
				for (int j=0; j<nAssignedKeyNum; ++j) {
					if (j > 0) {
						memWork.AppendString(_T(" , "));
					}
					memWork.AppendNativeData(*ppcAssignedKeyList[j]);
					delete ppcAssignedKeyList[j];
				}
				delete[] ppcAssignedKeyList;
			}
			const TCHAR* pszItemStr = memWork.GetStringPtr();
			m_statusBar.SetStatusText(0, SBT_NOBORDERS, pszItemStr);
		}
		return 0;

	case WM_DRAWITEM:
		idCtl = (UINT) wParam;				// �R���g���[����ID
		lpdis = (DRAWITEMSTRUCT*) lParam;	// ���ڕ`����
		if (IDW_STATUSBAR == idCtl) {
			if (lpdis->itemID == 5) { // 2003.08.26 Moca id������č�悳��Ȃ�����
				int	nColor;
				if (m_pShareData->m_flags.m_bRecordingKeyMacro	// �L�[�{�[�h�}�N���̋L�^��
				 && m_pShareData->m_flags.m_hwndRecordingKeyMacro == GetHwnd()	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
				) {
					nColor = COLOR_BTNTEXT;
				}else {
					nColor = COLOR_3DSHADOW;
				}
				::SetTextColor(lpdis->hDC, ::GetSysColor(nColor));
				::SetBkMode(lpdis->hDC, TRANSPARENT);
				
				// 2003.08.26 Moca �㉺�����ʒu�ɍ��
				TEXTMETRIC tm;
				::GetTextMetrics(lpdis->hDC, &tm);
				int y = (lpdis->rcItem.bottom - lpdis->rcItem.top - tm.tmHeight + 1) / 2 + lpdis->rcItem.top;
				::TextOut(lpdis->hDC, lpdis->rcItem.left, y, _T("REC"), _tcslen(_T("REC")));
				if (COLOR_BTNTEXT == nColor) {
					::TextOut(lpdis->hDC, lpdis->rcItem.left + 1, y, _T("REC"), _tcslen(_T("REC")));
				}
			}
			return 0;
		}else {
			switch (lpdis->CtlType) {
			case ODT_MENU:	// �I�[�i�[�`�惁�j���[
				// ���j���[�A�C�e���`��
				m_menuDrawer.DrawItem(lpdis);
				return TRUE;
			}
		}
		return FALSE;
	case WM_MEASUREITEM:
		idCtl = (UINT) wParam;					// control identifier
		lpmis = (MEASUREITEMSTRUCT*) lParam;	// item-size information
		switch (lpmis->CtlType) {
		case ODT_MENU:	// �I�[�i�[�`�惁�j���[
//			MenuDrawer* pMenuDrawer;
//			pMenuDrawer = (MenuDrawer*)lpmis->itemData;


//			MYTRACE(_T("WM_MEASUREITEM  lpmis->itemID=%d\n"), lpmis->itemID);
			// ���j���[�A�C�e���̕`��T�C�Y���v�Z
			nItemWidth = m_menuDrawer.MeasureItem(lpmis->itemID, &nItemHeight);
			if (0 < nItemWidth) {
				lpmis->itemWidth = nItemWidth;
				lpmis->itemHeight = nItemHeight;
			}
			return TRUE;
		}
		return FALSE;

	case WM_PAINT:
		return OnPaint(hwnd, uMsg, wParam, lParam);

	case WM_PASTE:
		return GetActiveView().GetCommander().HandleCommand(F_PASTE, true, 0, 0, 0, 0);

	case WM_COPY:
		return GetActiveView().GetCommander().HandleCommand(F_COPY, true, 0, 0, 0, 0);

	case WM_HELP:
		lphi = (LPHELPINFO) lParam;
		switch (lphi->iContextType) {
		case HELPINFO_MENUITEM:
			MyWinHelp(hwnd, HELP_CONTEXT, FuncID_To_HelpContextID((EFunctionCode)lphi->iCtrlId));
			break;
		}
		return TRUE;

	case WM_ACTIVATEAPP:
		m_bIsActiveApp = (wParam != 0);	// ���A�v�����A�N�e�B�u���ǂ���

		// �A�N�e�B�u���Ȃ�ҏW�E�B���h�E���X�g�̐擪�Ɉړ�����		// 2007.04.08 ryoji WM_SETFOCUS ����ړ�
		if (m_bIsActiveApp) {
			AppNodeGroupHandle(0).AddEditWndList(GetHwnd());	// ���X�g�ړ�����

			// 2009.01.17 nasukoji	�z�C�[���X�N���[���L����Ԃ��N���A
			ClearMouseState();
		}

		// �^�C�}�[ON/OFF		// 2007.03.08 ryoji WM_ACTIVATE����ړ�
		UpdateCaption();
		m_funcKeyWnd.Timer_ONOFF(m_bIsActiveApp); // 20060126 aroka
		this->Timer_ONOFF(m_bIsActiveApp); // 20060128 aroka

		return 0L;

	case WM_ENABLE:
		// �E�h���b�v�t�@�C���̎󂯓���ݒ�^����	// 2009.01.09 ryoji
		// Note: DragAcceptFiles��K�p�������h���b�v�ɂ��Ă� Enable/Disable �Ŏ����I�Ɏ󂯓���ݒ�^�������؂�ւ��
		if ((BOOL)wParam) {
			m_pDropTarget->Register_DropTarget(m_hWnd);
		}else {
			m_pDropTarget->Revoke_DropTarget();
		}
		return 0L;

	case WM_WINDOWPOSCHANGED:
		// �|�b�v�A�b�v�E�B���h�E�̕\���ؑ֎w�����|�X�g����	// 2007.10.22 ryoji
		// �EWM_SHOWWINDOW�͂��ׂĂ̕\���ؑւŌĂ΂��킯�ł͂Ȃ��̂�WM_WINDOWPOSCHANGED�ŏ���
		//   �i�^�u�O���[�v�����Ȃǂ̐ݒ�ύX����WM_SHOWWINDOW�͌Ă΂�Ȃ��j
		// �E�����ؑւ��ƃ^�u�ؑւɊ����Č��̃^�u�ɖ߂��Ă��܂����Ƃ�����̂Ō�Ő؂�ւ���
		WINDOWPOS* pwp;
		pwp = (WINDOWPOS*)lParam;
		if (pwp->flags & SWP_SHOWWINDOW)
			::PostMessage(hwnd, MYWM_SHOWOWNEDPOPUPS, TRUE, 0);
		else if (pwp->flags & SWP_HIDEWINDOW)
			::PostMessage(hwnd, MYWM_SHOWOWNEDPOPUPS, FALSE, 0);

		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);

	case MYWM_SHOWOWNEDPOPUPS:
		::ShowOwnedPopups(m_hWnd, (BOOL)wParam);	// 2007.10.22 ryoji
		return 0L;

	case WM_SIZE:
//		MYTRACE(_T("WM_SIZE\n"));
		/* WM_SIZE ���� */
		if (SIZE_MINIMIZED == wParam) {
			this->UpdateCaption();
		}
		return OnSize(wParam, lParam);

	// From here 2003.05.31 MIK
	case WM_MOVE:
		// From Here 2004.05.13 Moca �E�B���h�E�ʒu�p��
		//	�Ō�̈ʒu�𕜌����邽�߁C�ړ�����邽�тɋ��L�������Ɉʒu��ۑ�����D
		if (WinSizeMode::Save == m_pShareData->m_common.m_window.m_eSaveWindowPos) {
			if (!::IsZoomed(GetHwnd()) && !::IsIconic(GetHwnd())) {
				// 2005.11.23 Moca ���[�N�G���A���W���Ƃ����̂ŃX�N���[�����W�ɕύX
				// Aero Snap�ŏc�����ő剻�ŏI�����Ď���N������Ƃ��͌��̃T�C�Y�ɂ���K�v������̂ŁA
				// GetWindowRect()�ł͂Ȃ�GetWindowPlacement()�œ������[�N�G���A���W���X�N���[�����W�ɕϊ����ċL������	// 2009.09.02 ryoji
				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				::GetWindowPlacement(GetHwnd(), &wp);	// ���[�N�G���A���W
				RECT rcWin = wp.rcNormalPosition;
				RECT rcWork, rcMon;
				GetMonitorWorkRect(GetHwnd(), &rcWork, &rcMon);
				::OffsetRect(&rcWin, rcWork.left - rcMon.left, rcWork.top - rcMon.top);	// �X�N���[�����W�ɕϊ�
				m_pShareData->m_common.m_window.m_nWinPosX = rcWin.left;
				m_pShareData->m_common.m_window.m_nWinPosY = rcWin.top;
			}
		}
		// To Here 2004.05.13 Moca �E�B���h�E�ʒu�p��
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	// To here 2003.05.31 MIK
	case WM_SYSCOMMAND:
		// �^�u�܂Ƃߕ\���ł͕��铮��̓I�v�V�����w��ɏ]��	// 2006.02.13 ryoji
		//	Feb. 11, 2007 genta �����I�ׂ�悤��(MDI���Ə]������)
		// 2007.02.22 ryoji Alt+F4 �̃f�t�H���g�@�\�Ń��[�h���̓��삪������悤�ɂȂ���
		if (wParam == SC_CLOSE) {
			// ����v���r���[���[�h�ŃE�B���h�E����鑀��̂Ƃ��̓v���r���[�����	// 2007.03.04 ryoji
			if (m_pPrintPreview) {
				PrintPreviewModeONOFF();	// ����v���r���[���[�h�̃I��/�I�t
				return 0L;
			}
			OnCommand(0, (WORD)KeyBind::GetDefFuncCode(VK_F4, _ALT), NULL);
			return 0L;
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
#if 0
	case WM_IME_COMPOSITION:
		if (lParam & GCS_RESULTSTR) {
			// ���b�Z�[�W�̔z��
			return Views_DispatchEvent(hwnd, uMsg, wParam, lParam);
		}else {
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
#endif
	//case WM_KILLFOCUS:
	case WM_CHAR:
	case WM_IME_CHAR:
	case WM_KEYUP:
	case WM_SYSKEYUP:	// 2004.04.28 Moca ALT+�L�[�̃L�[���s�[�g�����̂��ߒǉ�
	case WM_ENTERMENULOOP:
#if 0
	case MYWM_IME_REQUEST:   // �ĕϊ��Ή� by minfu 2002.03.27	20020331 aroka
#endif
		if (GetActiveView().m_nAutoScrollMode) {
			GetActiveView().AutoScrollExit();
		}
		// ���b�Z�[�W�̔z��
		return Views_DispatchEvent(hwnd, uMsg, wParam, lParam);

	case WM_EXITMENULOOP:
//		MYTRACE(_T("WM_EXITMENULOOP\n"));
		if (m_statusBar.GetStatusHwnd()) {
			m_statusBar.SetStatusText(0, SBT_NOBORDERS, _T(""));
		}
		m_menuDrawer.EndDrawMenu();
		// ���b�Z�[�W�̔z��
		return Views_DispatchEvent(hwnd, uMsg, wParam, lParam);

	case WM_SETFOCUS:
//		MYTRACE(_T("WM_SETFOCUS\n"));

		// Aug. 29, 2003 wmlhq & ryoji�t�@�C���̃^�C���X�^���v�̃`�F�b�N���� OnTimer �Ɉڍs
		m_nTimerCount = 9;

		// �r���[�Ƀt�H�[�J�X���ړ�����	// 2007.10.16 ryoji
		if (!m_pPrintPreview && m_pEditView) {
			::SetFocus(GetActiveView().GetHwnd());
		}
		lRes = 0;

//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
		// ����v���r���[���[�h�̂Ƃ��́A�L�[����͑S��PrintPreviewBar�֓]��
		if (m_pPrintPreview) {
			m_pPrintPreview->SetFocusToPrintPreviewBar();
		}
		return lRes;

	case WM_NOTIFY:
		pnmh = (LPNMHDR) lParam;
		//	From Here Feb. 15, 2004 genta 
		//	�X�e�[�^�X�o�[�̃_�u���N���b�N�Ń��[�h�ؑւ��ł���悤�ɂ���
		if (m_statusBar.GetStatusHwnd() && pnmh->hwndFrom == m_statusBar.GetStatusHwnd()) {
			if (pnmh->code == NM_DBLCLK) {
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if (mp->dwItemSpec == 6) {	//	�㏑��/�}��
					GetDocument()->HandleCommand(F_CHGMOD_INS);
				}else if (mp->dwItemSpec == 5) {	//	�}�N���̋L�^�J�n�E�I��
					GetDocument()->HandleCommand(F_RECKEYMACRO);
				}else if (mp->dwItemSpec == 1) {	//	���ʒu���s�ԍ��W�����v
					GetDocument()->HandleCommand(F_JUMP_DIALOG);
				}else if (mp->dwItemSpec == 3) {	//	�����R�[�h���e��R�[�h
					ShowCodeBox(GetHwnd(), GetDocument());
				}else if (mp->dwItemSpec == 4) {	//	�����R�[�h�Z�b�g�������R�[�h�Z�b�g�w��
					GetDocument()->HandleCommand(F_CHG_CHARSET);
				}
			}else if (pnmh->code == NM_RCLICK) {
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if (mp->dwItemSpec == 2) {	//	���͉��s���[�h
					enum eEolExts {
						F_CHGMOD_EOL_NEL = F_CHGMOD_EOL_CR + 1,
						F_CHGMOD_EOL_PS,
						F_CHGMOD_EOL_LS,
					};
					m_menuDrawer.ResetContents();
					HMENU hMenuPopUp = ::CreatePopupMenu();
					m_menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CRLF, 
						LS(F_CHGMOD_EOL_CRLF), _T("C")); // ���͉��s�R�[�h�w��(CRLF)
					m_menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LF,
						LS(F_CHGMOD_EOL_LF), _T("L")); // ���͉��s�R�[�h�w��(LF)
					m_menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CR,
						LS(F_CHGMOD_EOL_CR), _T("R")); // ���͉��s�R�[�h�w��(CR)
					// �g��EOL���L���̎������\��
					if (GetDllShareData().m_common.m_edit.m_bEnableExtEol) {
						m_menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_NEL,
							LS(STR_EDITWND_MENU_NEL), _T(""), TRUE, -2); // ���͉��s�R�[�h�w��(NEL)
						m_menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LS,
							LS(STR_EDITWND_MENU_LS), _T(""), TRUE, -2); // ���͉��s�R�[�h�w��(LS)
						m_menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_PS,
							LS(STR_EDITWND_MENU_PS), _T(""), TRUE, -2); // ���͉��s�R�[�h�w��(PS)
					}
					
					//	mp->pt�̓X�e�[�^�X�o�[�����̍��W�Ȃ̂ŁC�X�N���[�����W�ւ̕ϊ����K�v
					POINT po = mp->pt;
					::ClientToScreen(m_statusBar.GetStatusHwnd(), &po);
					EFunctionCode nId = (EFunctionCode)::TrackPopupMenu(
						hMenuPopUp,
						TPM_CENTERALIGN
						| TPM_BOTTOMALIGN
						| TPM_RETURNCMD
						| TPM_LEFTBUTTON
						,
						po.x,
						po.y,
						0,
						GetHwnd(),
						NULL
					);
					::DestroyMenu(hMenuPopUp);
					EolType nEOLCode = EolType::None;
					switch (nId) {
					case F_CHGMOD_EOL_CRLF: nEOLCode = EolType::CRLF; break;
					case F_CHGMOD_EOL_CR: nEOLCode = EolType::CR; break;
					case F_CHGMOD_EOL_LF: nEOLCode = EolType::LF; break;
					case F_CHGMOD_EOL_NEL: nEOLCode = EolType::NEL; break;
					case F_CHGMOD_EOL_PS: nEOLCode = EolType::PS; break;
					case F_CHGMOD_EOL_LS: nEOLCode = EolType::LS; break;
					default:
						nEOLCode = EolType::Unknown;
					}
					if (nEOLCode != EolType::Unknown) {
						GetActiveView().GetCommander().HandleCommand(F_CHGMOD_EOL, true, (int)nEOLCode, 0, 0, 0);
					}
				}
			}
			return 0L;
		}
		//	To Here Feb. 15, 2004 genta 

		switch (pnmh->code) {
		// 2007.09.08 kobake TTN_NEEDTEXT�̏�����A�ł�W�łɕ����Ė����I�ɏ�������悤�ɂ��܂����B
		//                   ���e�L�X�g��80�����𒴂������Ȃ�TOOLTIPTEXT::lpszText�𗘗p���Ă��������B
		// 2008.11.03 syat   ��`�͈͑I���J�n�̃c�[���`�b�v��80���������Ă����̂�lpszText�ɕύX�B
		case TTN_NEEDTEXT:
			{
				static TCHAR szText[256] = {0};
				// �c�[���`�b�v�e�L�X�g�擾�A�ݒ�
				LPTOOLTIPTEXT lptip = (LPTOOLTIPTEXT)pnmh;
				GetTooltipText(szText, _countof(szText), lptip->hdr.idFrom);
				lptip->lpszText = szText;
			}
			break;

		case TBN_DROPDOWN:
			{
				int	nId;
				nId = CreateFileDropDownMenu(pnmh->hwndFrom);
				if (nId != 0) OnCommand((WORD)0 /*���j���[*/, (WORD)nId, (HWND)0);
			}
			return FALSE;
		//	From Here Jul. 21, 2003 genta
		case NM_CUSTOMDRAW:
			if (pnmh->hwndFrom == m_toolbar.GetToolbarHwnd()) {
				//	�c�[���o�[��Owner Draw
				return m_toolbar.ToolBarOwnerDraw((LPNMCUSTOMDRAW)pnmh);
			}
			break;
		//	To Here Jul. 21, 2003 genta
		}
		return 0L;
	case WM_COMMAND:
		OnCommand(HIWORD(wParam), LOWORD(wParam), (HWND) lParam);
		return 0L;
	case WM_INITMENUPOPUP:
		InitMenu((HMENU)wParam, (UINT)LOWORD(lParam), (BOOL)HIWORD(lParam));
		return 0L;
	case WM_DROPFILES:
		// �t�@�C�����h���b�v���ꂽ
		OnDropFiles((HDROP) wParam);
		return 0L;
	case WM_QUERYENDSESSION:	// OS�̏I��
		if (OnClose(NULL, false)) {
			::DestroyWindow(hwnd);
			return TRUE;
		}else {
			return FALSE;
		}
	case WM_CLOSE:
		if (OnClose(NULL, false)) {
			::DestroyWindow(hwnd);
		}
		return 0L;
	case WM_DESTROY:
		if (m_pShareData->m_flags.m_bRecordingKeyMacro) {					// �L�[�{�[�h�}�N���̋L�^��
			if (m_pShareData->m_flags.m_hwndRecordingKeyMacro == GetHwnd()) {	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
				m_pShareData->m_flags.m_bRecordingKeyMacro = FALSE;			// �L�[�{�[�h�}�N���̋L�^��
				m_pShareData->m_flags.m_hwndRecordingKeyMacro = NULL;		// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
			}
		}

		// �^�C�}�[���폜
		::KillTimer(GetHwnd(), IDT_TOOLBAR);

		// �h���b�v���ꂽ�t�@�C�����󂯓����̂�����
		::DragAcceptFiles(hwnd, FALSE);
		m_pDropTarget->Revoke_DropTarget();	// �E�{�^���h���b�v�p	// 2008.06.20 ryoji

		// �ҏW�E�B���h�E���X�g����̍폜
		AppNodeGroupHandle(GetHwnd()).DeleteEditWndList(GetHwnd());

		if (m_pShareData->m_handles.m_hwndDebug == GetHwnd()) {
			m_pShareData->m_handles.m_hwndDebug = NULL;
		}
		m_hWnd = NULL;


		// �ҏW�E�B���h�E�I�u�W�F�N�g����̃I�u�W�F�N�g�폜�v��
		::PostMessage(m_pShareData->m_handles.m_hwndTray, MYWM_DELETE_ME, 0, 0);

		// Windows �ɃX���b�h�̏I����v�����܂�
		::PostQuitMessage(0);

		return 0L;

	case WM_THEMECHANGED:
		// 2006.06.17 ryoji
		// �r�W���A���X�^�C���^�N���V�b�N�X�^�C�����؂�ւ������c�[���o�[���č쐬����
		// �i�r�W���A���X�^�C��: Rebar �L��A�N���V�b�N�X�^�C��: Rebar �����j
		if (m_toolbar.GetToolbarHwnd()) {
			if (IsVisualStyle() == (!m_toolbar.GetRebarHwnd())) {
				m_toolbar.DestroyToolBar();
				LayoutToolBar();
				EndLayoutBars();
			}
		}
		return 0L;

	case MYWM_UIPI_CHECK:
		// �G�f�B�^�|�g���C�Ԃł�UI���������̊m�F���b�Z�[�W	2007.06.07 ryoji
		m_bUIPI = TRUE;	// �g���C����̕Ԏ����󂯎����
		return 0L;

	case MYWM_CLOSE:
		// �G�f�B�^�ւ̏I���v��
		if (nRet = OnClose(
				(HWND)lParam,
				PM_CLOSE_GREPNOCONFIRM == (PM_CLOSE_GREPNOCONFIRM & wParam)
			)
		) { // Jan. 23, 2002 genta �x���}��
			// �v���O�C���FDocumentClose�C�x���g���s
			Plug::Array plugs;
			WSHIfObj::List params;
			JackManager::getInstance()->GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
			for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
				(*it)->Invoke(&GetActiveView(), params);
			}

			// �v���O�C���FEditorEnd�C�x���g���s
			plugs.clear();
			JackManager::getInstance()->GetUsablePlug(PP_EDITOR_END, 0, &plugs);
			for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
				(*it)->Invoke(&GetActiveView(), params);
			}

			// �^�u�܂Ƃߕ\���ł͕��铮��̓I�v�V�����w��ɏ]��	// 2006.02.13 ryoji
			if ((wParam & PM_CLOSE_EXIT) != PM_CLOSE_EXIT) {	// �S�I���v���łȂ��ꍇ
				// �^�u�܂Ƃߕ\����(����)���c���w��̏ꍇ�A�c�E�B���h�E���P�Ȃ�V�K�G�f�B�^���N�����ďI������
				if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd &&
					!m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin &&
					m_pShareData->m_common.m_tabBar.m_bTab_RetainEmptyWin
				) {
					// ���O���[�v���̎c�E�B���h�E���𒲂ׂ�	// 2007.06.20 ryoji
					int nGroup = AppNodeManager::getInstance()->GetEditNode(GetHwnd())->GetGroup();
					if (AppNodeGroupHandle(nGroup).GetEditorWindowsNum() == 1) {
						EditNode* pEditNode = AppNodeManager::getInstance()->GetEditNode(GetHwnd());
						if (pEditNode)
							pEditNode->m_bClosing = TRUE;	// �����̓^�u�\�����Ă����Ȃ��Ă���
						LoadInfo loadInfo;
						loadInfo.filePath = _T("");
						loadInfo.eCharCode = CODE_NONE;
						loadInfo.bViewMode = false;
						ControlTray::OpenNewEditor(
							G_AppInstance(),
							GetHwnd(),
							loadInfo,
							NULL,
							true
						);
					}
				}
			}
			::DestroyWindow(hwnd);
		}
		return nRet;
	case MYWM_ALLOWACTIVATE:
		::AllowSetForegroundWindow(wParam);
		return 0L;

		
	case MYWM_GETFILEINFO:
		// �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm
		pfi = (EditInfo*)&m_pShareData->m_workBuffer.m_EditInfo_MYWM_GETFILEINFO;

		// �ҏW�t�@�C�������i�[
		GetDocument()->GetEditInfo(pfi);
		return 0L;
	case MYWM_CHANGESETTING:
		// �ݒ�ύX�̒ʒm
		switch ((e_PM_CHANGESETTING_SELECT)lParam) {
		case PM_CHANGESETTING_ALL:
			// �����I������
			SelectLang::ChangeLang(GetDllShareData().m_common.m_window.m_szLanguageDll);
			ShareData::getInstance()->RefreshString();

			// ���C�����j���[	2010/5/16 Uchi
			LayoutMainMenu();

			// Oct 10, 2000 ao
			// �ݒ�ύX���A�c�[���o�[���č쐬����悤�ɂ���i�o�[�̓��e�ύX�����f�j
			m_toolbar.DestroyToolBar();
			LayoutToolBar();
			// Oct 10, 2000 ao �����܂�

			// 2008.10.05 nasukoji	��A�N�e�B�u�ȃE�B���h�E�̃c�[���o�[���X�V����
			// �A�N�e�B�u�ȃE�B���h�E�̓^�C�}�ɂ��X�V����邪�A����ȊO�̃E�B���h�E��
			// �^�C�}���~�����Ă���ݒ�ύX����ƑS���L���ƂȂ��Ă��܂����߁A������
			// �c�[���o�[���X�V����
			if (!m_bIsActiveApp)
				m_toolbar.UpdateToolbar();

			// �t�@���N�V�����L�[���č쐬����i�o�[�̓��e�A�ʒu�A�O���[�v�{�^�����̕ύX�����f�j	// 2006.12.19 ryoji
			m_funcKeyWnd.Close();
			LayoutFuncKey();

			// �^�u�o�[�̕\���^��\���؂�ւ�	// 2006.12.19 ryoji
			LayoutTabBar();

			// �X�e�[�^�X�o�[�̕\���^��\���؂�ւ�	// 2006.12.19 ryoji
			LayoutStatusBar();

			// �����X�N���[���o�[�̕\���^��\���؂�ւ�	// 2006.12.19 ryoji
			{
				bool b1 = (m_pShareData->m_common.m_window.m_bScrollBarHorz == FALSE);
				for (int i=0; i<GetAllViewCount(); ++i) {
					bool b2 = (GetView(i).m_hwndHScrollBar == NULL);
					if (b1 != b2) {		// �����X�N���[���o�[���g��
						GetView(i).DestroyScrollBar();
						GetView(i).CreateScrollBar();
					}
				}
			}

			LayoutMiniMap();

			// �o�[�ύX�ŉ�ʂ�����Ȃ��悤��	// 2006.12.19 ryoji
			EndLayoutBars();

			// �A�N�Z�����[�^�e�[�u�����č쐬����(Wine�p)
			// �E�B���h�E���ɍ쐬�����A�N�Z�����[�^�e�[�u����j������(Wine�p)
			DeleteAccelTbl();
			// �E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
			CreateAccelTbl();
			
			if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd) {
				// �^�u�\���̂܂܃O���[�v������^���Ȃ����ύX����Ă�����^�u���X�V����K�v������
				m_tabWnd.Refresh(false);
			}
			if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd && !m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin) {
				if (AppNodeManager::getInstance()->GetEditNode(GetHwnd())->IsTopInGroup()) {
					if (!::IsWindowVisible(GetHwnd())) {
						// ::ShowWindow(GetHwnd(), SW_SHOWNA) ���Ɣ�\������\���ɐ؂�ւ��Ƃ��� Z-order �����������Ȃ邱�Ƃ�����̂� ::SetWindowPos ���g��
						::SetWindowPos(GetHwnd(), NULL, 0, 0, 0, 0,
										SWP_SHOWWINDOW | SWP_NOACTIVATE
										| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

						// ���̃E�B���h�E�� WS_EX_TOPMOST ��Ԃ�S�E�B���h�E�ɔ��f����	// 2007.05.18 ryoji
						WindowTopMost(((DWORD)::GetWindowLongPtr(GetHwnd(), GWL_EXSTYLE) & WS_EX_TOPMOST)? 1: 2);
					}
				}else {
					if (::IsWindowVisible(GetHwnd())) {
						::ShowWindow(GetHwnd(), SW_HIDE);
					}
				}
			}else {
				if (!::IsWindowVisible(GetHwnd())) {
					// ::ShowWindow(GetHwnd(), SW_SHOWNA) ���Ɣ�\������\���ɐ؂�ւ��Ƃ��� Z-order �����������Ȃ邱�Ƃ�����̂� ::SetWindowPos ���g��
					::SetWindowPos(GetHwnd(), NULL, 0, 0, 0, 0,
									SWP_SHOWWINDOW | SWP_NOACTIVATE
									| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
				}
			}

			//	Aug, 21, 2000 genta
			GetDocument()->m_autoSaveAgent.ReloadAutoSaveParam();

			GetDocument()->OnChangeSetting();	// �r���[�ɐݒ�ύX�𔽉f������
			GetDocument()->m_docType.SetDocumentIcon();	// Sep. 10, 2002 genta �����A�C�R���̍Đݒ�

			break;
		case PM_CHANGESETTING_FONT:
			GetDocument()->OnChangeSetting(true);	// �t�H���g�ŕ��������ς��̂ŁA���C�A�E�g�č\�z
			break;
		case PM_CHANGESETTING_FONTSIZE:
			if ((wParam == -1 && GetLogfontCacheMode() == CharWidthCacheMode::Share)
				|| GetDocument()->m_docType.GetDocumentType().GetIndex() == wParam
			) {
				GetDocument()->OnChangeSetting( false );	// �r���[�ɐݒ�ύX�𔽉f������(���C�A�E�g���̍č쐬���Ȃ�)
			}
			break;
		case PM_CHANGESETTING_TYPE:
			typeNew = DocTypeManager().GetDocumentTypeOfPath(GetDocument()->m_docFile.GetFilePath());
			if (GetDocument()->m_docType.GetDocumentType().GetIndex() == wParam
				|| typeNew.GetIndex() == wParam
			) {
				GetDocument()->OnChangeSetting();

				// �A�E�g���C����͉�ʏ���
				bool bAnalyzed = FALSE;
#if 0
				if (/* �K�v�Ȃ�ύX�����������ɋL�q����i�����p�j */) {
					// �A�E�g���C����͉�ʂ̈ʒu�����݂̐ݒ�ɍ��킹��
					bAnalyzed = m_dlgFuncList.ChangeLayout(OUTLINE_LAYOUT_BACKGROUND);	// �O������̕ύX�ʒm�Ɠ����̈���
				}
#endif
				if (m_dlgFuncList.GetHwnd() && !bAnalyzed) {	// �A�E�g���C�����J���Ă���΍ĉ��
					// SHOW_NORMAL: ��͕��@���ω����Ă���΍ĉ�͂����B�����łȂ���Ε`��X�V�i�ύX���ꂽ�J���[�̓K�p�j�̂݁B
					EFunctionCode nFuncCode = m_dlgFuncList.GetFuncCodeRedraw(m_dlgFuncList.m_nOutlineType);
					GetActiveView().GetCommander().HandleCommand(nFuncCode, true, (LPARAM)ShowDialogType::Normal, 0, 0, 0);
				}
				if (MyGetAncestor(::GetForegroundWindow(), GA_ROOTOWNER2) == GetHwnd())
					::SetFocus(GetActiveView().GetHwnd());	// �t�H�[�J�X��߂�
			}
			break;
		case PM_CHANGESETTING_TYPE2:
			typeNew = DocTypeManager().GetDocumentTypeOfPath(GetDocument()->m_docFile.GetFilePath());
			if (GetDocument()->m_docType.GetDocumentType().GetIndex() == wParam
				|| typeNew.GetIndex() == wParam
			) {
				// index�̂ݍX�V
				GetDocument()->m_docType.SetDocumentTypeIdx();
				// �^�C�v���ύX�ɂȂ����ꍇ�͓K�p����
				if (GetDocument()->m_docType.GetDocumentType().GetIndex() != wParam) {
					::SendMessage(m_hWnd, MYWM_CHANGESETTING, wParam, PM_CHANGESETTING_TYPE);
				}
			}
			break;
		case PM_PRINTSETTING:
			{
				if (m_pPrintPreview) {
					m_pPrintPreview->OnChangeSetting();
				}
			}
			break;
		default:
			break;
		}
		return 0L;
	case MYWM_SAVEEDITSTATE:
		{
			if (m_pPrintPreview) {
				// �ꎞ�I�ɐݒ��߂�
				SelectCharWidthCache(CharWidthFontMode::Edit, CharWidthCacheMode::Neutral);
			}
			// �t�H���g�ύX�O�̍��W�̕ۑ�
			m_posSaveAry = SavePhysPosOfAllView();
			if (m_pPrintPreview) {
				// �ݒ��߂�
				SelectCharWidthCache(CharWidthFontMode::Print, CharWidthCacheMode::Local);
			}
		}
		return 0L; 
	case MYWM_SETACTIVEPANE:
		if ((int)wParam == -1) {
			if (lParam == 0) {
				nPane = m_splitterWnd.GetFirstPane();
			}else {
				nPane = m_splitterWnd.GetLastPane();
			}
			this->SetActivePane(nPane);
		}
		return 0L;
		
	case MYWM_SETCARETPOS:	// �J�[�\���ʒu�ύX�ʒm
		{
			//	2006.07.09 genta LPARAM�ɐV���ȈӖ���ǉ�
			//	bit 0 (MASK 1): (bit 1==0�̂Ƃ�) 0/�I���N���A, 1/�I���J�n�E�ύX
			//	bit 1 (MASK 2): 0: bit 0�̐ݒ�ɏ]���D1:���݂̑I�����b�Ns��Ԃ��p��
			//	�����̎����ł� �ǂ����0�Ȃ̂ŋ��������Ɖ��߂����D
			//	�Ăяo������e_PM_SETCARETPOS_SELECTSTATE�̒l���g�����ƁD
			bool bSelect = ((lParam & 1) != 0);
			if (lParam & 2) {
				// ���݂̏�Ԃ�KEEP
				bSelect = GetActiveView().GetSelectionInfo().m_bSelectingLock;
			}
			
			//	2006.07.09 genta �����������Ȃ�
			/*
			�J�[�\���ʒu�ϊ�
			 �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			��
			 ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
			LogicPoint* ppoCaret = &(m_pShareData->m_workBuffer.m_LogicPoint);
			LayoutPoint ptCaretPos;
			GetDocument()->m_layoutMgr.LogicToLayout(
				*ppoCaret,
				&ptCaretPos
			);
			// ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤��	// 2007.08.22 ryoji
			// Note. ���Ƃ����s�P�ʂ̌��ʒu�Ȃ̂Ń��C�A�E�g�܂�Ԃ��̌��ʒu�𒴂��邱�Ƃ͂Ȃ��B
			//       �I���w��(bSelect==TRUE)�̏ꍇ�ɂ͂ǂ�����̂��Ó����悭�킩��Ȃ����A
			//       2007.08.22���݂ł̓A�E�g���C����̓_�C�A���O���猅�ʒu0�ŌĂяo�����
			//       �p�^�[�������Ȃ��̂Ŏ��p����ɖ��͖����B
			if (!bSelect) {
				const DocLine *pTmpDocLine = GetDocument()->m_docLineMgr.GetLine(ppoCaret->GetY2());
				if (pTmpDocLine) {
					if (pTmpDocLine->GetLengthWithoutEOL() < ppoCaret->x) ptCaretPos.x--;
				}
			}
			//	2006.07.09 genta �I��͈͂��l�����Ĉړ�
			//	MoveCursor�̈ʒu�����@�\������̂ŁC�ŏI�s�ȍ~�ւ�
			//	�ړ��w���̒�����MoveCursor�ɂ܂�����
			GetActiveView().MoveCursorSelecting(ptCaretPos, bSelect, _CARETMARGINRATE / 3);
		}
		return 0L;


	case MYWM_GETCARETPOS:	// �J�[�\���ʒu�擾�v��
		/*
		�J�[�\���ʒu�ϊ�
		 ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		��
		�����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		*/
		{
			LogicPoint* ppoCaret = &(m_pShareData->m_workBuffer.m_LogicPoint);
			GetDocument()->m_layoutMgr.LayoutToLogic(
				GetActiveView().GetCaret().GetCaretLayoutPos(),
				ppoCaret
			);
		}
		return 0L;

	case MYWM_GETLINEDATA:	// �s(���s�P��)�f�[�^�̗v��
	{
		// ���L�f�[�^�F����Write������Read
		// return 0�ȏ�F�s�f�[�^����BwParam�I�t�Z�b�g���������s�f�[�^���B0��EOF��Offset�����傤�ǃo�b�t�@��������
		//       -1�ȉ��F�G���[
		LogicInt	nLineNum = LogicInt(wParam);
		LogicInt	nLineOffset = LogicInt(lParam);
		if (nLineNum < 0 || GetDocument()->m_docLineMgr.GetLineCount() < nLineNum) {
			return -2; // �s�ԍ��s���BLineCount == nLineNum ��EOF�s�Ƃ��ĉ��ŏ���
		}
		LogicInt nLineLen = LogicInt(0);
		const wchar_t* pLine = GetDocument()->m_docLineMgr.GetLine(nLineNum)->GetDocLineStrWithEOL( &nLineLen );
		if (nLineOffset < 0 || nLineLen < nLineOffset) {
			return -3; // �I�t�Z�b�g�ʒu�s��
		}
		if (nLineNum == GetDocument()->m_docLineMgr.GetLineCount()) {
			return 0; // EOF����I��
		}
 		if (!pLine) {
			return -4; // �s���ȃG���[
		}
		if (nLineLen == nLineOffset) {
 			return 0;
 		}
		pLine = GetDocument()->m_docLineMgr.GetLine(LogicInt(wParam))->GetDocLineStrWithEOL( &nLineLen );
		pLine += nLineOffset;
		nLineLen -= nLineOffset;
		size_t nEnd = t_min<size_t>(nLineLen, m_pShareData->m_workBuffer.GetWorkBufferCount<EDIT_CHAR>());
		auto_memcpy( m_pShareData->m_workBuffer.GetWorkBuffer<EDIT_CHAR>(), pLine, nEnd );
		return nLineLen;
	}

	// 2010.05.11 Moca MYWM_ADDSTRINGLEN_W��ǉ� NUL�Z�[�t
	case MYWM_ADDSTRINGLEN_W:
		{
			EDIT_CHAR* pWork = m_pShareData->m_workBuffer.GetWorkBuffer<EDIT_CHAR>();
			size_t addSize = t_min((size_t)wParam, m_pShareData->m_workBuffer.GetWorkBufferCount<EDIT_CHAR>());
			GetActiveView().GetCommander().HandleCommand(F_ADDTAIL_W, true, (LPARAM)pWork, (LPARAM)addSize, 0, 0);
			GetActiveView().GetCommander().HandleCommand(F_GOFILEEND, true, 0, 0, 0, 0);
		}
		return 0L;

	// �^�u�E�B���h�E	//@@@ 2003.05.31 MIK
	case MYWM_TAB_WINDOW_NOTIFY:
		m_tabWnd.TabWindowNotify(wParam, lParam);
		{
			RECT rc;
			::GetClientRect(GetHwnd(), &rc);
			OnSize2(m_nWinSizeType, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ), false);
			GetActiveView().SetIMECompFormPos();
		}
		return 0L;

	// �A�E�g���C��	// 2010.06.06 ryoji
	case MYWM_OUTLINE_NOTIFY:
		m_dlgFuncList.OnOutlineNotify(wParam, lParam);
		return 0L;

	// �o�[�̕\���E��\��	//@@@ 2003.06.10 MIK
	case MYWM_BAR_CHANGE_NOTIFY:
		if (GetHwnd() != (HWND)lParam) {
			switch ((BarChangeNotifyType)wParam) {
			case BarChangeNotifyType::Toolbar:
				LayoutToolBar();	// 2006.12.19 ryoji
				break;
			case BarChangeNotifyType::FuncKey:
				LayoutFuncKey();	// 2006.12.19 ryoji
				break;
			case BarChangeNotifyType::Tab:
				LayoutTabBar();		// 2006.12.19 ryoji
				if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd
					&& !m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin
				) {
					::ShowWindow(GetHwnd(), SW_HIDE);
				}else {
					// ::ShowWindow(hwnd, SW_SHOWNA) ���Ɣ�\������\���ɐ؂�ւ��Ƃ��� Z-order �����������Ȃ邱�Ƃ�����̂� ::SetWindowPos ���g��
					::SetWindowPos(hwnd, NULL, 0,0,0,0,
									SWP_SHOWWINDOW | SWP_NOACTIVATE
									| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
				}
				break;
			case BarChangeNotifyType::StatusBar:
				LayoutStatusBar();		// 2006.12.19 ryoji
				break;
			case BarChangeNotifyType::MiniMap:
				LayoutMiniMap();
				break;
			}
			EndLayoutBars();	// 2006.12.19 ryoji
		}
		return 0L;

	// by �S (2) MYWM_CHECKSYSMENUDBLCLK�͕s�v��, WM_LBUTTONDBLCLK�ǉ�
	case WM_NCLBUTTONDOWN:
		return OnNcLButtonDown(wParam, lParam);

	case WM_NCLBUTTONUP:
		return OnNcLButtonUp(wParam, lParam);

	case WM_LBUTTONDBLCLK:
		return OnLButtonDblClk(wParam, lParam);

#if 0
	case WM_IME_NOTIFY:	// Nov. 26, 2006 genta
		if (wParam == IMN_SETCONVERSIONMODE || wParam == IMN_SETOPENSTATUS) {
			GetActiveView().GetCaret().ShowEditCaret();
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
#endif

	case WM_NCPAINT:
		DefWindowProc(hwnd, uMsg, wParam, lParam);
		if (!m_statusBar.GetStatusHwnd()) {
			PrintMenubarMessage(NULL);
		}
		return 0;

	case WM_NCACTIVATE:
		// �ҏW�E�B���h�E�ؑ֒��i�^�u�܂Ƃߎ��j�̓^�C�g���o�[�̃A�N�e�B�u�^��A�N�e�B�u��Ԃ��ł��邾���ύX���Ȃ��悤�Ɂi�P�j	// 2007.04.03 ryoji
		// �O�ʂɂ���̂��ҏW�E�B���h�E�Ȃ�A�N�e�B�u��Ԃ�ێ�����
		if (m_pShareData->m_flags.m_bEditWndChanging && IsSakuraMainWindow(::GetForegroundWindow())) {
			wParam = TRUE;	// �A�N�e�B�u
		}
		lRes = DefWindowProc(hwnd, uMsg, wParam, lParam);
		if (!m_statusBar.GetStatusHwnd()) {
			PrintMenubarMessage(NULL);
		}
		return lRes;

	case WM_SETTEXT:
		// �ҏW�E�B���h�E�ؑ֒��i�^�u�܂Ƃߎ��j�̓^�C�g���o�[�̃A�N�e�B�u�^��A�N�e�B�u��Ԃ��ł��邾���ύX���Ȃ��悤�Ɂi�Q�j	// 2007.04.03 ryoji
		// �^�C�}�[���g�p���ă^�C�g���̕ύX��x������
		if (m_pShareData->m_flags.m_bEditWndChanging) {
			delete[] m_pszLastCaption;
			m_pszLastCaption = new TCHAR[::_tcslen((LPCTSTR)lParam) + 1];
			::_tcscpy(m_pszLastCaption, (LPCTSTR)lParam);	// �ύX��̃^�C�g�����L�����Ă���
			::SetTimer(GetHwnd(), IDT_CAPTION, 50, NULL);
			return 0L;
		}
		::KillTimer(GetHwnd(), IDT_CAPTION);	// �^�C�}�[���c���Ă�����폜����i�x���^�C�g����j���j
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	case WM_TIMER:
		if (!OnTimer(wParam, lParam))
			return 0L;
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	default:
#if 0
// << 20020331 aroka �ĕϊ��Ή� for 95/NT
		if (uMsg == m_uMSIMEReconvertMsg || uMsg == m_uATOKReconvertMsg) {
			return Views_DispatchEvent(hwnd, uMsg, wParam, lParam);
		}
// >> by aroka
#endif
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

/*! �I�����̏���

	@param hWndFrom [in] �I���v���� Wimdow Handle	// 2013/4/9 Uchi

	@retval TRUE: �I�����ėǂ� / FALSE: �I�����Ȃ�
*/
int	EditWnd::OnClose(HWND hWndActive, bool bGrepNoConfirm)
{
	// �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s
	int nRet = GetDocument()->OnFileClose(bGrepNoConfirm);
	if (!nRet) return nRet;
	// �p�����[�^�Ńn���h����Ⴄ�l�ɂ����̂Ō������폜	2013/4/9 Uchi
	if (hWndActive) {
		// �A�N�e�B�u������E�B���h�E���A�N�e�B�u������
		if (IsSakuraMainWindow(hWndActive)) {
			ActivateFrameWindow(hWndActive);	// �G�f�B�^
		}else {
			::SetForegroundWindow(hWndActive);	// �^�X�N�g���C
		}
	}

#if 0
	// 2005.09.01 ryoji �^�u�܂Ƃߕ\���̏ꍇ�͎��̃E�B���h�E��O�ʂɁi�I�����̃E�B���h�E�������}���j
	if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd
		&& !m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin
	) {
		int i, j;
		EditNode* p = NULL;
		int nCount = AppNodeManager::getInstance()->GetOpenedWindowArr(&p, FALSE);
		if (nCount > 1) {
			for (i=0; i<nCount; ++i) {
				if (p[i].GetHwnd() == GetHwnd())
					break;
			}
			if (i < nCount) {
				for (j=i+1; j<nCount; ++j) {
					if (p[j].m_nGroup == p[i].m_nGroup)
						break;
				}
				if (j >= nCount) {
					for (j=0; j<i; ++j) {
						if (p[j].m_nGroup == p[i].m_nGroup)
							break;
					}
				}
				if (j != i) {
					HWND hwnd = p[j].GetHwnd();
					{
						// 2006.01.28 ryoji
						// �^�u�܂Ƃߕ\���ł��̉�ʂ���\������\���ɕς���Ă�������ꍇ(�^�u�̒��N���b�N����)�A
						// �ȑO�̃E�B���h�E�������������Ɉ�C�ɂ����܂ŏ������i��ł��܂���
						// ���Ƃŉ�ʂ�������̂ŁA�ȑO�̃E�B���h�E��������̂�������Ƃ����҂�
						int iWait = 0;
						while (::IsWindowVisible(hwnd) && iWait++ < 20)
							::Sleep(1);
					}
					if (!::IsWindowVisible(hwnd)) {
						ActivateFrameWindow(hwnd);
					}
				}
			}
		}
		if (p) delete []p;
	}
#endif	// 0

	return nRet;
}


/*! WM_COMMAND����
	@date 2000.11.15 JEPRO // �V���[�g�J�b�g�L�[�����܂������Ȃ��̂ŎE���Ă���������2�s(F_HELP_CONTENTS,F_HELP_SEARCH)���C���E����
	@date 2013.05.09 novice �d�����郁�b�Z�[�W�����폜
*/
void EditWnd::OnCommand(WORD wNotifyCode, WORD wID , HWND hwndCtl)
{
	// �����{�b�N�X����� WM_COMMAND �͂��ׂăR���{�{�b�N�X�ʒm
	// ##### �����{�b�N�X�����̓c�[���o�[���� WindowProc �ɏW�񂷂�ق����X�}�[�g����
	if (m_toolbar.GetSearchHwnd() && hwndCtl == m_toolbar.GetSearchHwnd()) {
		switch (wNotifyCode) {
		case CBN_SETFOCUS:
			m_nCurrentFocus = F_SEARCH_BOX;
			break;
		case CBN_KILLFOCUS:
			m_nCurrentFocus = 0;
			// �t�H�[�J�X���͂��ꂽ�Ƃ��Ɍ����L�[�ɂ��Ă��܂��B
			// �����L�[���[�h���擾
			std::wstring	strText;
			if (m_toolbar.GetSearchKey(strText)) {	// �L�[�����񂪂���
				// �����L�[��o�^
				if (strText.length() < _MAX_PATH) {
					SearchKeywordManager().AddToSearchKeyArr(strText.c_str());
				}
				GetActiveView().m_strCurSearchKey = strText;
				GetActiveView().m_bCurSearchUpdate = true;
				GetActiveView().ChangeCurRegexp();
			}
			break;
		}
		return;	// CBN_SELCHANGE(1) ���A�N�Z�����[�^�ƌ�F����Ȃ��悤�ɂ����Ŕ�����irev1886 �̖��̔��{�΍�j
	}

	switch (wNotifyCode) {
	// ���j���[����̃��b�Z�[�W
	case 0:
	case CMD_FROM_MOUSE: // 2006.05.19 genta �}�E�X����Ăт����ꂽ�ꍇ
		// �E�B���h�E�؂�ւ�
		if (wID - IDM_SELWINDOW >= 0 && wID - IDM_SELWINDOW < m_pShareData->m_nodes.m_nEditArrNum) {
			ActivateFrameWindow(m_pShareData->m_nodes.m_pEditArr[wID - IDM_SELWINDOW].GetHwnd());
		}
		// �ŋߎg�����t�@�C��
		else if (wID - IDM_SELMRU >= 0 && wID - IDM_SELMRU < 999) {
			// �w��t�@�C�����J����Ă��邩���ׂ�
			const MRUFile mru;
			EditInfo checkEditInfo;
			mru.GetEditInfo(wID - IDM_SELMRU, &checkEditInfo);
			LoadInfo loadInfo(checkEditInfo.m_szPath, checkEditInfo.m_nCharCode, false);
			GetDocument()->m_docFileOperation.FileLoad(&loadInfo);	//	Oct.  9, 2004 genta ���ʊ֐���
		}
		// �ŋߎg�����t�H���_
		else if (wID - IDM_SELOPENFOLDER >= 0 && wID - IDM_SELOPENFOLDER < 999) {
			// �t�H���_�擾
			const MRUFolder mruFolder;
			LPCTSTR pszFolderPath = mruFolder.GetPath(wID - IDM_SELOPENFOLDER);

			// Stonee, 2001/12/21 UNC�ł���ΐڑ������݂�
			NetConnect(pszFolderPath);

			//�u�t�@�C�����J���v�_�C�A���O
			LoadInfo loadInfo(_T(""), CODE_AUTODETECT, false);
			DocFileOperation& docOp = GetDocument()->m_docFileOperation;
			std::vector<std::tstring> files;
			if (docOp.OpenFileDialog(GetHwnd(), pszFolderPath, &loadInfo, files)) {
				loadInfo.filePath = files[0].c_str();
				// �J��
				docOp.FileLoad(&loadInfo);

				// �V���ȕҏW�E�B���h�E���N��
				size_t nSize = files.size();
				for (size_t f=1; f<nSize; ++f) {
					loadInfo.filePath = files[f].c_str();
					ControlTray::OpenNewEditor(
						G_AppInstance(),
						GetHwnd(),
						loadInfo,
						NULL,
						true
					);
				}
			}
		}else {
			// ���̑��R�}���h
			// �r���[�Ƀt�H�[�J�X���ړ����Ă���
			if (wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX) {
				::SetFocus(GetActiveView().GetHwnd());
			}

			// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
			//	May 19, 2006 genta ��ʃr�b�g��n��
			//	Jul. 7, 2007 genta ��ʃr�b�g��萔��
			GetDocument()->HandleCommand((EFunctionCode)(wID | 0));
		}
		break;
	// �A�N�Z�����[�^����̃��b�Z�[�W
	case 1:
		{
			// �r���[�Ƀt�H�[�J�X���ړ����Ă���
			if (wID != F_SEARCH_BOX && m_nCurrentFocus == F_SEARCH_BOX)
				::SetFocus(GetActiveView().GetHwnd());
			auto& csKeyBind = m_pShareData->m_common.m_keyBind;
			EFunctionCode nFuncCode = KeyBind::GetFuncCode(
				wID,
				csKeyBind.m_nKeyNameArrNum,
				csKeyBind.m_pKeyNameArr
			);
			GetDocument()->HandleCommand((EFunctionCode)(nFuncCode | FA_FROMKEYBOARD));
		}
		break;
	}
	return;
}



//	�L�[���[�h�F���j���[�o�[����
//	Sept.14, 2000 Jepro note: ���j���[�o�[�̍��ڂ̃L���v�V�����⏇�Ԑݒ�Ȃǂ͈ȉ��ōs���Ă���炵��
//	Sept.16, 2000 Jepro note: �A�C�R���Ƃ̊֘A�t����CShareData_new2.cpp�t�@�C���ōs���Ă���
//	2010/5/16	Uchi	���I�ɍ쐬����l�ɕύX	
void EditWnd::InitMenu(HMENU hMenu, UINT uPos, BOOL fSystemMenu)
{
	int		numMenuItems;
	int		nPos;
	HMENU	hMenuPopUp;

	if (hMenu == ::GetSubMenu(::GetMenu(GetHwnd()), uPos)
		&& !fSystemMenu
	) {
		// ���擾
		const CommonSetting_MainMenu*	pMenu = &m_pShareData->m_common.m_mainMenu;
		const MainMenu*	pMainMenu;
		int		nIdxStr;
		int		nIdxEnd;
		int		nLv;
		std::vector<HMENU>	hSubMenu;
		std::wstring tmpMenuName;
		const wchar_t* pMenuName;

		nIdxStr = pMenu->m_nMenuTopIdx[uPos];
		nIdxEnd = (uPos < MAX_MAINMENU_TOP) ? pMenu->m_nMenuTopIdx[uPos + 1] : -1;
		if (nIdxEnd < 0) {
			nIdxEnd = pMenu->m_nMainMenuNum;
		}

		// ���j���[ ������
		m_menuDrawer.ResetContents();
		numMenuItems = ::GetMenuItemCount(hMenu);
		for (int i=numMenuItems-1; i>=0; --i) {
			::DeleteMenu(hMenu, i, MF_BYPOSITION);
		}

		// ���j���[�쐬
		hSubMenu.push_back(hMenu);
		nLv = 1;
		if (pMenu->m_mainMenuTbl[nIdxStr].m_nType == MainMenuType::Special) {
			nLv = 0;
			--nIdxStr;
		}
		for (int i=nIdxStr+1; i<nIdxEnd; ++i) {
			pMainMenu = &pMenu->m_mainMenuTbl[i];
			if (pMainMenu->m_nLevel != nLv) {
				nLv = pMainMenu->m_nLevel;
				if (hSubMenu.size() < (size_t)nLv) {
					// �ی�
					break;
				}
				hMenu = hSubMenu[nLv-1];
			}
			switch (pMainMenu->m_nType) {
			case MainMenuType::Node:
				hMenuPopUp = ::CreatePopupMenu();
				if (pMainMenu->nFunc != 0 && pMainMenu->m_sName[0] == L'\0') {
					// �X�g�����O�e�[�u������ǂݍ���
					tmpMenuName = LSW(pMainMenu->nFunc);
					if (MAX_MAIN_MENU_NAME_LEN < tmpMenuName.length()) {
						tmpMenuName = tmpMenuName.substr(0, MAX_MAIN_MENU_NAME_LEN);
					}
					pMenuName = tmpMenuName.c_str();
				}else {
					pMenuName = pMainMenu->m_sName;
				}
				m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , 
					pMenuName, pMainMenu->m_sKey);
				if (hSubMenu.size() > (size_t)nLv) {
					hSubMenu[nLv] = hMenuPopUp;
				}else {
					hSubMenu.push_back(hMenuPopUp);
				}
				break;
			case MainMenuType::Leaf:
				InitMenu_Function(hMenu, pMainMenu->nFunc, pMainMenu->m_sName, pMainMenu->m_sKey);
				break;
			case MainMenuType::Separator:
				m_menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
				break;
			case MainMenuType::Special:
				bool	bInList;		// ���X�g��1�ȏ゠��
				bInList = InitMenu_Special(hMenu, pMainMenu->nFunc);
				// ���X�g�������ꍇ�̏���
				if (!bInList) {
					// �������Ɉ͂܂�A�����X�g�Ȃ� �Ȃ�� ���̕��������X�L�b�v
					if ((i == nIdxStr + 1
						  || (pMenu->m_mainMenuTbl[i - 1].m_nType == MainMenuType::Separator 
							&& pMenu->m_mainMenuTbl[i - 1].m_nLevel == pMainMenu->m_nLevel))
						&& i + 1 < nIdxEnd
						&& pMenu->m_mainMenuTbl[i + 1].m_nType == MainMenuType::Separator 
						&& pMenu->m_mainMenuTbl[i + 1].m_nLevel == pMainMenu->m_nLevel) {
						++i;		// �X�L�b�v
					}
				}
				break;
			}
		}
		if (nLv > 0) {
			// ���x�����߂��Ă��Ȃ�
			hMenu = hSubMenu[0];
		}
		// �q�̖����ݒ�SubMenu��Desable
		CheckFreeSubMenu(GetHwnd(), hMenu, uPos);
	}

//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
//	if (m_pPrintPreview)	return;	//	����v���r���[���[�h�Ȃ�r���B�i�����炭�r�����Ȃ��Ă������Ǝv���񂾂��ǁA�O�̂��߁j

	// �@�\�����p�\���ǂ����A�`�F�b�N��Ԃ��ǂ������ꊇ�`�F�b�N
	numMenuItems = ::GetMenuItemCount(hMenu);
	for (nPos=0; nPos<numMenuItems; ++nPos) {
		EFunctionCode	id = (EFunctionCode)::GetMenuItemID(hMenu, nPos);
		// �@�\�����p�\�����ׂ�
		//	Jan.  8, 2006 genta �@�\���L���ȏꍇ�ɂ͖����I�ɍĐݒ肵�Ȃ��悤�ɂ���D
		if (!IsFuncEnable(GetDocument(), m_pShareData, id)) {
			UINT fuFlags = MF_BYCOMMAND | MF_GRAYED;
			::EnableMenuItem(hMenu, id, fuFlags);
		}

		// �@�\���`�F�b�N��Ԃ����ׂ�
		if (IsFuncChecked(GetDocument(), m_pShareData, id)) {
			UINT fuFlags = MF_BYCOMMAND | MF_CHECKED;
			::CheckMenuItem(hMenu, id, fuFlags);
		}
		/* else {
			fuFlags = MF_BYCOMMAND | MF_UNCHECKED;
		}
		*/
	}

	return;
}



/*!	�ʏ�R�}���h(Special�ȊO)�̃��j���[�ւ̒ǉ�
*/
void EditWnd::InitMenu_Function(HMENU hMenu, EFunctionCode eFunc, const wchar_t* pszName, const wchar_t* pszKey)
{
	const wchar_t* psName = NULL;
	// ���j���[���x���̍쐬
	// �J�X�^�����j���[
	if (eFunc == F_MENU_RBUTTON
	  || eFunc >= F_CUSTMENU_1 && eFunc <= F_CUSTMENU_24
	) {
		int j;
		// �E�N���b�N���j���[
		if (eFunc == F_MENU_RBUTTON) {
			j = CUSTMENU_INDEX_FOR_RBUTTONUP;
		}else {
			j = eFunc - F_CUSTMENU_BASE;
		}

		int nFlag = MF_BYPOSITION | MF_STRING | MF_GRAYED;
		if (m_pShareData->m_common.m_customMenu.m_nCustMenuItemNumArr[j] > 0) {
			nFlag = MF_BYPOSITION | MF_STRING;
		}
		WCHAR buf[MAX_CUSTOM_MENU_NAME_LEN + 1];
		m_menuDrawer.MyAppendMenu(hMenu, nFlag,
			eFunc, GetDocument()->m_funcLookup.Custmenu2Name(j, buf, _countof(buf)), pszKey);
	}
	// �}�N��
	else if (eFunc >= F_USERMACRO_0 && eFunc < F_USERMACRO_0 + MAX_CUSTMACRO) {
		MacroRec *mp = &m_pShareData->m_common.m_macro.m_macroTable[eFunc - F_USERMACRO_0];
		if (mp->IsEnabled()) {
			psName = to_wchar(mp->m_szName[0] ? mp->m_szName : mp->m_szFile);
			m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING,
				eFunc, psName, pszKey);
		}else {
			psName = L"-- undefined macro --";
			m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED,
				eFunc, psName, pszKey);
		}
	}
	// �v���O�C���R�}���h
	else if (eFunc >= F_PLUGCOMMAND_FIRST && eFunc < F_PLUGCOMMAND_LAST) {
		WCHAR szLabel[256];
		if (0 < JackManager::getInstance()->GetCommandName( eFunc, szLabel, _countof(szLabel) )) {
			m_menuDrawer.MyAppendMenu(
				hMenu, MF_BYPOSITION | MF_STRING,
				eFunc, szLabel, pszKey,
				TRUE, eFunc
			);
		}else {
			// not found
			psName = L"-- undefined plugin command --";
			m_menuDrawer.MyAppendMenu(
				hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED,
				eFunc, psName, pszKey
			);
		}
	}else {
		switch (eFunc) {
		case F_RECKEYMACRO:
		case F_SAVEKEYMACRO:
		case F_LOADKEYMACRO:
		case F_EXECKEYMACRO:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!m_pShareData->m_flags.m_bRecordingKeyMacro);
			break;
		case F_SPLIT_V:	
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				m_splitterWnd.GetAllSplitRows() == 1);
			break;
		case F_SPLIT_H:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				m_splitterWnd.GetAllSplitCols() == 1);
			break;
		case F_SPLIT_VH:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				m_splitterWnd.GetAllSplitRows() == 1 || m_splitterWnd.GetAllSplitCols() == 1);
			break;
		case F_TAB_CLOSEOTHER:
			SetMenuFuncSel(hMenu, eFunc, pszKey, m_pShareData->m_common.m_tabBar.m_bDispTabWnd);
			break;
		case F_TOPMOST:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				((DWORD)::GetWindowLongPtr(GetHwnd(), GWL_EXSTYLE) & WS_EX_TOPMOST) == 0);
			break;
		case F_BIND_WINDOW:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				(!m_pShareData->m_common.m_tabBar.m_bDispTabWnd 
				|| m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin));
			break;
		case F_SHOWTOOLBAR:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!m_pShareData->m_common.m_window.m_bMenuIcon | !m_toolbar.GetToolbarHwnd());
			break;
		case F_SHOWFUNCKEY:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!m_pShareData->m_common.m_window.m_bMenuIcon | !m_funcKeyWnd.GetHwnd());
			break;
		case F_SHOWTAB:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!m_pShareData->m_common.m_window.m_bMenuIcon | !m_tabWnd.GetHwnd());
			break;
		case F_SHOWSTATUSBAR:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!m_pShareData->m_common.m_window.m_bMenuIcon | !m_statusBar.GetStatusHwnd());
			break;
		case F_SHOWMINIMAP:
			SetMenuFuncSel( hMenu, eFunc, pszKey, 
				!m_pShareData->m_common.m_window.m_bMenuIcon | !GetMiniMap().GetHwnd() );
			break;
		case F_TOGGLE_KEY_SEARCH:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!m_pShareData->m_common.m_window.m_bMenuIcon | !IsFuncChecked(GetDocument(), m_pShareData, F_TOGGLE_KEY_SEARCH));
			break;
		case F_WRAPWINDOWWIDTH:
			{
				LayoutInt ketas;
				WCHAR*	pszLabel;
				EditView::TOGGLE_WRAP_ACTION mode = GetActiveView().GetWrapMode(&ketas);
				if (mode == EditView::TGWRAP_NONE) {
					m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_WRAPWINDOWWIDTH , L"", pszKey);
				}else {
					WCHAR szBuf[60];
					pszLabel = szBuf;
					if (mode == EditView::TGWRAP_FULL) {
						auto_sprintf_s(
							szBuf,
							LSW(STR_WRAP_WIDTH_FULL),	// L"�܂�Ԃ�����: %d ���i�ő�j",
							MAXLINEKETAS
						);
					}else if (mode == EditView::TGWRAP_WINDOW) {
						auto_sprintf_s(
							szBuf,
							LSW(STR_WRAP_WIDTH_WINDOW),	//L"�܂�Ԃ�����: %d ���i�E�[�j",
							GetActiveView().ViewColNumToWrapColNum(
								GetActiveView().GetTextArea().m_nViewColNum
							)
						);
					}else {
						auto_sprintf_s(
							szBuf,
							LSW(STR_WRAP_WIDTH_FIXED),	//L"�܂�Ԃ�����: %d ���i�w��j",
							GetDocument()->m_docType.GetDocumentAttribute().m_nMaxLineKetas
						);
					}
					m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , pszLabel, pszKey);
				}
			}
			break;
		default:
			m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, eFunc, 
				pszName, pszKey);
			break;
		}
	}
}


/*!	Special�R�}���h�̃��j���[�ւ̒ǉ�
*/
bool EditWnd::InitMenu_Special(HMENU hMenu, EFunctionCode eFunc)
{
	bool bInList = false;
	switch (eFunc) {
	case F_WINDOW_LIST:				// �E�B���h�E���X�g
		{
			EditNode* pEditNodeArr;
			int nRowNum = AppNodeManager::getInstance()->GetOpenedWindowArr(&pEditNodeArr, TRUE);
			WinListMenu(hMenu, pEditNodeArr, nRowNum, false);
			bInList = (nRowNum > 0);
			delete [] pEditNodeArr;
		}
		break;
	case F_FILE_USED_RECENTLY:		// �ŋߎg�����t�@�C��
		// MRU���X�g�̃t�@�C���̃��X�g�����j���[�ɂ���
		{
			//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
			const MRUFile mru;
			mru.CreateMenu(hMenu, &m_menuDrawer);	//	�t�@�C�����j���[
			bInList = (mru.MenuLength() > 0);
		}
		break;
	case F_FOLDER_USED_RECENTLY:	// �ŋߎg�����t�H���_
		// �ŋߎg�����t�H���_�̃��j���[���쐬
		{
			//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́AMRUFolder�ɂ��ׂĈ˗�����
			const MRUFolder mruFolder;
			mruFolder.CreateMenu(hMenu, &m_menuDrawer);
			bInList = (mruFolder.MenuLength() > 0);
		}
		break;
	case F_CUSTMENU_LIST:			// �J�X�^�����j���[���X�g
		WCHAR buf[MAX_CUSTOM_MENU_NAME_LEN + 1];
		//	�E�N���b�N���j���[
		if (m_pShareData->m_common.m_customMenu.m_nCustMenuItemNumArr[0] > 0) {
			 m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING,
				 F_MENU_RBUTTON, GetDocument()->m_funcLookup.Custmenu2Name(0, buf, _countof(buf)), L"");
			bInList = true;
		}
		// �J�X�^�����j���[
		for (int j=1; j<MAX_CUSTOM_MENU; ++j) {
			if (m_pShareData->m_common.m_customMenu.m_nCustMenuItemNumArr[j] > 0) {
				 m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING,
			 		F_CUSTMENU_BASE + j, GetDocument()->m_funcLookup.Custmenu2Name(j, buf, _countof(buf)), L"" );
				bInList = true;
			}
		}
		break;
	case F_USERMACRO_LIST:			// �o�^�ς݃}�N�����X�g
		for (int j=0; j<MAX_CUSTMACRO; ++j) {
			MacroRec *mp = &m_pShareData->m_common.m_macro.m_macroTable[j];
			if (mp->IsEnabled()) {
				if (mp->m_szName[0]) {
					m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + j, mp->m_szName, _T(""));
				}else {
					m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + j, mp->m_szFile, _T(""));
				}
				bInList = true;
			}
		}
		break;
	case F_PLUGIN_LIST:				// �v���O�C���R�}���h���X�g
		// �v���O�C���R�}���h��񋟂���v���O�C����񋓂���
		{
			const JackManager* pJackManager = JackManager::getInstance();
			const Plugin* prevPlugin = NULL;
			HMENU hMenuPlugin = 0;

			Plug::Array plugs = pJackManager->GetPlugs(PP_COMMAND);
			for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
				const Plugin* curPlugin = &(*it)->m_plugin;
				if (curPlugin != prevPlugin) {
					// �v���O�C�����ς������v���O�C���|�b�v�A�b�v���j���[��o�^
					hMenuPlugin = ::CreatePopupMenu();
					m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPlugin, curPlugin->m_sName.c_str(), L"");
					prevPlugin = curPlugin;
				}

				// �R�}���h��o�^
				m_menuDrawer.MyAppendMenu(hMenuPlugin, MF_BYPOSITION | MF_STRING,
					(*it)->GetFunctionCode(), to_tchar((*it)->m_sLabel.c_str()), _T(""),
					TRUE, (*it)->GetFunctionCode());
			}
			bInList = (prevPlugin != NULL);
		}
		break;
	}
	return bInList;
}


// ���j���[�o�[�̖�����������	2010/6/18 Uchi
void EditWnd::CheckFreeSubMenu(HWND hWnd, HMENU hMenu, UINT uPos)
{
	int cMenuItems = ::GetMenuItemCount(hMenu);
	if (cMenuItems == 0) {
		// ���������̂Ŗ�����
		::EnableMenuItem(::GetMenu(hWnd), uPos, MF_BYPOSITION | MF_GRAYED);
	}else {
		// ���ʃ��x��������
		CheckFreeSubMenuSub(hMenu, 1);
	}
}

// ���j���[�o�[�̖�����������	2010/6/18 Uchi
void EditWnd::CheckFreeSubMenuSub(HMENU hMenu, int nLv)
{
	int numMenuItems = ::GetMenuItemCount(hMenu);
	for (int nPos=0; nPos<numMenuItems; ++nPos) {
		HMENU hSubMenu = ::GetSubMenu(hMenu, nPos);
		if (hSubMenu) {
			if (::GetMenuItemCount(hSubMenu) == 0) {
				// ���������̂Ŗ�����
				::EnableMenuItem(hMenu, nPos, MF_BYPOSITION | MF_GRAYED);
			}else {
				// ���ʃ��x��������
				CheckFreeSubMenuSub(hSubMenu, nLv + 1);
			}
		}
	}
}


//	�t���O�ɂ��\��������̑I��������B
//		2010/5/19	Uchi
void EditWnd::SetMenuFuncSel(HMENU hMenu, EFunctionCode nFunc, const WCHAR* sKey, bool flag)
{
	const WCHAR* sName = L"";
	for (int i=0; i<_countof(sFuncMenuName); ++i) {
		if (sFuncMenuName[i].eFunc == nFunc) {
			sName = flag ? LSW(sFuncMenuName[i].nNameId[0]) : LSW(sFuncMenuName[i].nNameId[1]);
		}
	}
	assert(auto_strlen(sName));

	m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, nFunc, sName, sKey);
}




STDMETHODIMP EditWnd::DragEnter( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
	if (!pDataObject || !pdwEffect) {
		return E_INVALIDARG;
	}

	// �E�{�^���t�@�C���h���b�v�̏ꍇ������������
	if (!((MK_RBUTTON & dwKeyState) && IsDataAvailable(pDataObject, CF_HDROP))) {
		*pdwEffect = DROPEFFECT_NONE;
		return E_INVALIDARG;
	}

	// ����v���r���[�ł͎󂯕t���Ȃ�
	if (m_pPrintPreview) {
		*pdwEffect = DROPEFFECT_NONE;
		return E_INVALIDARG;
	}

	*pdwEffect &= DROPEFFECT_LINK;
	return S_OK;
}

STDMETHODIMP EditWnd::DragOver(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
	if (!pdwEffect)
		return E_INVALIDARG;
	
	*pdwEffect &= DROPEFFECT_LINK;
	return S_OK;
}

STDMETHODIMP EditWnd::DragLeave(void)
{
	return S_OK;
}

STDMETHODIMP EditWnd::Drop(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
	if (!pDataObject || !pdwEffect)
		return E_INVALIDARG;

	// �t�@�C���h���b�v���A�N�e�B�u�r���[�ŏ�������
	*pdwEffect &= DROPEFFECT_LINK;
	return GetActiveView().PostMyDropFiles(pDataObject);
}

// �t�@�C�����h���b�v���ꂽ
void EditWnd::OnDropFiles(HDROP hDrop)
{
	POINT pt;
	::DragQueryPoint(hDrop, &pt);
	int cFiles = (int)::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	// �t�@�C�����h���b�v�����Ƃ��͕��ĊJ��
	auto& csFile = m_pShareData->m_common.m_file;
	if (csFile.m_bDropFileAndClose) {
		cFiles = 1;
	}
	// ��x�Ƀh���b�v�\�ȃt�@�C����
	if (cFiles > csFile.m_nDropFileNumMax) {
		cFiles = csFile.m_nDropFileNumMax;
	}

	// �A�N�e�B�u�ɂ���	// 2009.08.20 ryoji �����J�n�O�ɖ������ŃA�N�e�B�u��
	ActivateFrameWindow(GetHwnd());

	for (int i=0; i<cFiles; ++i) {
		// �t�@�C���p�X�擾�A�����B
		TCHAR		szFile[_MAX_PATH + 1];
		::DragQueryFile(hDrop, i, szFile, _countof(szFile));
		SakuraEnvironment::ResolvePath(szFile);

		// �w��t�@�C�����J����Ă��邩���ׂ�
		HWND hWndOwner;
		if (ShareData::getInstance()->IsPathOpened(szFile, &hWndOwner)) {
			::SendMessage(hWndOwner, MYWM_GETFILEINFO, 0, 0);
			EditInfo* pfi = (EditInfo*)&m_pShareData->m_workBuffer.m_EditInfo_MYWM_GETFILEINFO;
			// �A�N�e�B�u�ɂ���
			ActivateFrameWindow(hWndOwner);
			// MRU���X�g�ւ̓o�^
			MRUFile mru;
			mru.Add(pfi);
		}else {
			// �ύX�t���O���I�t�ŁA�t�@�C����ǂݍ���ł��Ȃ��ꍇ
			//	2005.06.24 Moca
			if (GetDocument()->IsAcceptLoad()) {
				// �t�@�C���ǂݍ���
				LoadInfo loadInfo(szFile, CODE_AUTODETECT, false);
				GetDocument()->m_docFileOperation.FileLoad(&loadInfo);
			}else {
				// �t�@�C�����h���b�v�����Ƃ��͕��ĊJ��
				if (csFile.m_bDropFileAndClose) {
					// �t�@�C���ǂݍ���
					LoadInfo loadInfo(szFile, CODE_AUTODETECT, false);
					GetDocument()->m_docFileOperation.FileCloseOpen(loadInfo);
				}else {
					// �ҏW�E�B���h�E�̏���`�F�b�N
					if (m_pShareData->m_nodes.m_nEditArrNum >= MAX_EDITWINDOWS) {	// �ő�l�C��	//@@@ 2003.05.31 MIK
						OkMessage(NULL, LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
						return;
					}
					// �V���ȕҏW�E�B���h�E���N��
					LoadInfo loadInfo;
					loadInfo.filePath = szFile;
					loadInfo.eCharCode = CODE_NONE;
					loadInfo.bViewMode = false;
					ControlTray::OpenNewEditor(
						G_AppInstance(),
						GetHwnd(),
						loadInfo
					);
				}
			}
		}
	}
	::DragFinish(hDrop);
	return;
}

/*! WM_TIMER ���� 
	@date 2007.04.03 ryoji �V�K
	@date 2008.04.19 ryoji IDT_FIRST_IDLE �ł� MYWM_FIRST_IDLE �|�X�g������ǉ�
	@date 2013.06.09 novice �R���g���[���v���Z�X�ւ� MYWM_FIRST_IDLE �|�X�g������ǉ�
*/
LRESULT EditWnd::OnTimer(WPARAM wParam, LPARAM lParam)
{
	// �^�C�}�[ ID �ŏ�����U�蕪����
	switch (wParam) {
	case IDT_EDIT:
		OnEditTimer();
		break;
	case IDT_TOOLBAR:
		m_toolbar.OnToolbarTimer();
		break;
	case IDT_CAPTION:
		OnCaptionTimer();
		break;
	case IDT_SYSMENU:
		OnSysMenuTimer();
		break;
	case IDT_FIRST_IDLE:
		m_dlgFuncList.m_bEditWndReady = true;	// �G�f�B�^��ʂ̏�������
		AppNodeGroupHandle(0).PostMessageToAllEditors(MYWM_FIRST_IDLE, ::GetCurrentProcessId(), 0, NULL);	// �v���Z�X�̏���A�C�h�����O�ʒm	// 2008.04.19 ryoji
		::PostMessage(m_pShareData->m_handles.m_hwndTray, MYWM_FIRST_IDLE, (WPARAM)::GetCurrentProcessId(), (LPARAM)0);
		::KillTimer(m_hWnd, wParam);
		break;
	default:
		return 1L;
	}

	return 0L;
}


/*! �L���v�V�����X�V�p�^�C�}�[�̏���
	@date 2007.04.03 ryoji �V�K
*/
void EditWnd::OnCaptionTimer(void)
{
	// �ҏW��ʂ̐ؑցi�^�u�܂Ƃߎ��j���I����Ă�����^�C�}�[���I�����ă^�C�g���o�[���X�V����
	// �܂��ؑ֒��Ȃ�^�C�}�[�p��
	if (!m_pShareData->m_flags.m_bEditWndChanging) {
		::KillTimer(GetHwnd(), IDT_CAPTION);
		::SetWindowText(GetHwnd(), m_pszLastCaption);
	}
}

/*! �V�X�e�����j���[�\���p�^�C�}�[�̏���
	@date 2007.04.03 ryoji �p�����[�^�����ɂ���
	                       �ȑO�̓R�[���o�b�N�֐��ł���Ă���KillTimer()�������ōs���悤�ɂ���
*/
void EditWnd::OnSysMenuTimer(void) // by �S(2)
{
	::KillTimer(GetHwnd(), IDT_SYSMENU);	// 2007.04.03 ryoji

	if (m_iconClicked == IconClickStatus::Clicked) {
		ReleaseCapture();

		// �V�X�e�����j���[�\��
		// 2006.04.21 ryoji �}���`���j�^�Ή��̏C��
		// 2007.05.13 ryoji 0x0313���b�Z�[�W���|�X�g��������ɕύX�iTrackPopupMenu���ƃ��j���[���ڂ̗L���^������Ԃ��s���ɂȂ���΍�j
		RECT R;
		GetWindowRect(GetHwnd(), &R);
		POINT pt;
		pt.x = R.left + GetSystemMetrics(SM_CXFRAME);
		pt.y = R.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
		GetMonitorWorkRect(pt, &R);
		::PostMessage(
			GetHwnd(),
			0x0313, // �E�N���b�N�ŃV�X�e�����j���[��\������ۂɑ��M���郂�m�炵��
			0,
			MAKELPARAM((pt.x > R.left)? pt.x: R.left, (pt.y < R.bottom)? pt.y: R.bottom)
		);
	}
	m_iconClicked = IconClickStatus::None;
}


//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX

// ����v���r���[���[�h�̃I��/�I�t
void EditWnd::PrintPreviewModeONOFF(void)
{
	// 2006.06.17 ryoji Rebar ������΂�����c�[���o�[��������
	HWND hwndToolBar = m_toolbar.GetRebarHwnd() ? m_toolbar.GetRebarHwnd(): m_toolbar.GetToolbarHwnd();

	// ����v���r���[���[�h��
//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
	if (m_pPrintPreview) {
//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
		// ����v���r���[���[�h���������܂��B
		delete m_pPrintPreview;	//	�폜�B
		m_pPrintPreview = NULL;	//	NULL���ۂ��ŁA�v�����g�v���r���[���[�h�����f���邽�߁B

		// �ʏ탂�[�h�ɖ߂�
		::ShowWindow(this->m_splitterWnd.GetHwnd(), SW_SHOW);
		::ShowWindow(hwndToolBar, SW_SHOW);	// 2006.06.17 ryoji
		::ShowWindow(m_statusBar.GetStatusHwnd(), SW_SHOW);
		::ShowWindow(m_funcKeyWnd.GetHwnd(), SW_SHOW);
		::ShowWindow(m_tabWnd.GetHwnd(), SW_SHOW);	//@@@ 2003.06.25 MIK
		::ShowWindow(m_dlgFuncList.GetHwnd(), SW_SHOW);	// 2010.06.25 ryoji

		// ���̑��̃��[�h���X�_�C�A���O���߂�	// 2010.06.25 ryoji
		::ShowWindow(m_dlgFind.GetHwnd(), SW_SHOW);
		::ShowWindow(m_dlgReplace.GetHwnd(), SW_SHOW);
		::ShowWindow(m_dlgGrep.GetHwnd(), SW_SHOW);

		::SetFocus(GetHwnd());

		// ���j���[�𓮓I�ɍ쐬����悤�ɕύX
		//hMenu = ::LoadMenu(G_AppInstance(), MAKEINTRESOURCE(IDR_MENU1));
		//::SetMenu(GetHwnd(), hMenu);
		//::DrawMenuBar(GetHwnd());
		LayoutMainMenu();				// 2010/5/16 Uchi

//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
		::InvalidateRect(GetHwnd(), NULL, TRUE);
	}else {
//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
		// �ʏ탂�[�h���B��
		HMENU hMenu = ::GetMenu(GetHwnd());
		//	Jun. 18, 2001 genta Print Preview�ł̓��j���[���폜
		::SetMenu(GetHwnd(), NULL);
		::DestroyMenu(hMenu);
		::DrawMenuBar(GetHwnd());

		::ShowWindow(this->m_splitterWnd.GetHwnd(), SW_HIDE);
		::ShowWindow(hwndToolBar, SW_HIDE);	// 2006.06.17 ryoji
		::ShowWindow(m_statusBar.GetStatusHwnd(), SW_HIDE);
		::ShowWindow(m_funcKeyWnd.GetHwnd(), SW_HIDE);
		::ShowWindow(m_tabWnd.GetHwnd(), SW_HIDE);	//@@@ 2003.06.25 MIK
		::ShowWindow(m_dlgFuncList.GetHwnd(), SW_HIDE);	// 2010.06.25 ryoji

		// ���̑��̃��[�h���X�_�C�A���O���B��	// 2010.06.25 ryoji
		::ShowWindow(m_dlgFind.GetHwnd(), SW_HIDE);
		::ShowWindow(m_dlgReplace.GetHwnd(), SW_HIDE);
		::ShowWindow(m_dlgGrep.GetHwnd(), SW_HIDE);

//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
		m_pPrintPreview = new PrintPreview(this);
		// ���݂̈���ݒ�
		m_pPrintPreview->SetPrintSetting(
			&m_pShareData->m_printSettingArr[
				GetDocument()->m_docType.GetDocumentAttribute().m_nCurrentPrintSetting]
		);

		// �v�����^�̏����擾�B

		// ���݂̃f�t�H���g�v�����^�̏����擾
		BOOL bRes = m_pPrintPreview->GetDefaultPrinterInfo();
		if (!bRes) {
			TopInfoMessage(GetHwnd(), LS(STR_ERR_DLGEDITWND14));
			return;
		}

		// ����ݒ�̔��f
//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
		m_pPrintPreview->OnChangePrintSetting();
		::InvalidateRect(GetHwnd(), NULL, TRUE);
		::UpdateWindow(GetHwnd() /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);

	}
	return;

}


// WM_SIZE ����
LRESULT EditWnd::OnSize(WPARAM wParam, LPARAM lParam)
{
	return OnSize2(wParam, lParam, true);
}

LRESULT EditWnd::OnSize2( WPARAM wParam, LPARAM lParam, bool bUpdateStatus )
{
	RECT rc;
//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��
//	�ϐ��폜

	int cx = LOWORD(lParam);
	int cy = HIWORD(lParam);
	auto& csWindow = m_pShareData->m_common.m_window;
	
	// �E�B���h�E�T�C�Y�p��
	if (wParam != SIZE_MINIMIZED) {						// �ŏ����͌p�����Ȃ�
		//	2004.05.13 Moca m_eSaveWindowSize�̉��ߒǉ��̂���
		if (csWindow.m_eSaveWindowSize == WinSizeMode::Save) {		// �E�B���h�E�T�C�Y�p�������邩
			if (wParam == SIZE_MAXIMIZED) {					// �ő剻�̓T�C�Y���L�^���Ȃ�
				if (csWindow.m_nWinSizeType != (int)wParam) {
					csWindow.m_nWinSizeType = wParam;
				}
			}else {
				// Aero Snap�̏c�����ő剻��ԂŏI�����Ď���N������Ƃ��͌��̃T�C�Y�ɂ���K�v������̂ŁA
				// GetWindowRect()�ł͂Ȃ�GetWindowPlacement()�œ������[�N�G���A���W���X�N���[�����W�ɕϊ����ċL������	// 2009.09.02 ryoji
				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				::GetWindowPlacement(GetHwnd(), &wp);	// ���[�N�G���A���W
				RECT rcWin = wp.rcNormalPosition;
				RECT rcWork, rcMon;
				GetMonitorWorkRect(GetHwnd(), &rcWork, &rcMon);
				::OffsetRect(&rcWin, rcWork.left - rcMon.left, rcWork.top - rcMon.top);	// �X�N���[�����W�ɕϊ�
				// �E�B���h�E�T�C�Y�Ɋւ���f�[�^���ύX���ꂽ��
				if (csWindow.m_nWinSizeType != (int)wParam ||
					csWindow.m_nWinSizeCX != rcWin.right - rcWin.left ||
					csWindow.m_nWinSizeCY != rcWin.bottom - rcWin.top
				) {
					csWindow.m_nWinSizeType = wParam;
					csWindow.m_nWinSizeCX = rcWin.right - rcWin.left;
					csWindow.m_nWinSizeCY = rcWin.bottom - rcWin.top;
				}
			}
		}

		// ���ɖ߂��Ƃ��̃T�C�Y��ʂ��L��	// 2007.06.20 ryoji
		EditNode* p = AppNodeManager::getInstance()->GetEditNode(GetHwnd());
		if (p) {
			p->m_showCmdRestore = ::IsZoomed(p->GetHwnd())? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
		}
	}

	m_nWinSizeType = wParam;	// �T�C�Y�ύX�̃^�C�v

	// 2006.06.17 ryoji Rebar ������΂�����c�[���o�[��������
	HWND hwndToolBar = m_toolbar.GetRebarHwnd() ? m_toolbar.GetRebarHwnd(): m_toolbar.GetToolbarHwnd();
	int nToolBarHeight = 0;
	if (hwndToolBar) {
		::SendMessage(hwndToolBar, WM_SIZE, wParam, lParam);
		::GetWindowRect(hwndToolBar, &rc);
		nToolBarHeight = rc.bottom - rc.top;
	}
	int nFuncKeyWndHeight = 0;
	if (m_funcKeyWnd.GetHwnd()) {
		::SendMessage(m_funcKeyWnd.GetHwnd(), WM_SIZE, wParam, lParam);
		::GetWindowRect(m_funcKeyWnd.GetHwnd(), &rc);
		nFuncKeyWndHeight = rc.bottom - rc.top;
	}
	//@@@ From Here 2003.05.31 MIK
	//@@@ To Here 2003.05.31 MIK
	bool bMiniMapSizeBox = true;
	if (wParam == SIZE_MAXIMIZED) {
		bMiniMapSizeBox = false;
	}
	int nStatusBarHeight = 0;
	if (m_statusBar.GetStatusHwnd()) {
		::SendMessage(m_statusBar.GetStatusHwnd(), WM_SIZE, wParam, lParam);
		::GetClientRect(m_statusBar.GetStatusHwnd(), &rc);
		//	May 12, 2000 genta
		//	2�J�����ڂɉ��s�R�[�h�̕\����}��
		//	From Here
		int			nStArr[8];
		// 2003.08.26 Moca CR0LF0�p�~�ɏ]���A�K���ɒ���
		// 2004-02-28 yasu ��������o�͎��̏����ɍ��킹��
		// ����ς����ꍇ�ɂ�EditView::ShowCaretPosInfo()�ł̕\�����@���������K�v����D
		// ��pszLabel[3]: �X�e�[�^�X�o�[�����R�[�h�\���̈�͑傫�߂ɂƂ��Ă���
		const TCHAR*	pszLabel[7] = { _T(""), _T("99999 �s 9999 ��"), _T("CRLF"), _T("AAAAAAAAAAAA"), _T("Unicode BOM�t"), _T("REC"), _T("�㏑") };	// Oct. 30, 2000 JEPRO �疜�s���v���	�����R�[�h�g���L���� 2008/6/21	Uchi
		int			nStArrNum = 7;
		//	To Here
		int			nAllWidth = rc.right - rc.left;
		int			nSbxWidth = ::GetSystemMetrics(SM_CXVSCROLL) + ::GetSystemMetrics(SM_CXEDGE); // �T�C�Y�{�b�N�X�̕�
		int			nBdrWidth = ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXEDGE) * 2; // ���E�̕�
		SIZE		sz;
		HDC			hdc;
		int			i;
		// 2004-02-28 yasu
		// ���m�ȕ����v�Z���邽�߂ɁA�\���t�H���g���擾����hdc�ɑI��������B
		hdc = ::GetDC(m_statusBar.GetStatusHwnd());
		HFONT hFont = (HFONT)::SendMessage(m_statusBar.GetStatusHwnd(), WM_GETFONT, 0, 0);
		if (hFont) {
			hFont = (HFONT)::SelectObject(hdc, hFont);
		}
		nStArr[nStArrNum - 1] = nAllWidth;
		if (wParam != SIZE_MAXIMIZED) {
			nStArr[nStArrNum - 1] -= nSbxWidth;
		}
		for (i=nStArrNum-1; i>0; --i) {
			::GetTextExtentPoint32(hdc, pszLabel[i], _tcslen(pszLabel[i]), &sz);
			nStArr[i - 1] = nStArr[i] - (sz.cx + nBdrWidth);
		}

		//	Nov. 8, 2003 genta
		//	������Ԃł͂��ׂĂ̕������u�g����v�����C���b�Z�[�W�G���A�͘g��`�悵�Ȃ��悤�ɂ��Ă���
		//	���߁C���������̘g���ςȕ��Ɏc���Ă��܂��D������ԂŘg��`�悳���Ȃ����邽�߁C
		//	�ŏ��Ɂu�g�����v��Ԃ�ݒ肵����Ńo�[�̕������s���D
		if (bUpdateStatus) {
			m_statusBar.SetStatusText(0, SBT_NOBORDERS, _T(""));
		}

		StatusBar_SetParts(m_statusBar.GetStatusHwnd(), nStArrNum, nStArr);
		if (hFont) {
			::SelectObject(hdc, hFont);
		}
		::ReleaseDC(m_statusBar.GetStatusHwnd(), hdc);

		::UpdateWindow(m_statusBar.GetStatusHwnd());	// 2006.06.17 ryoji �����`��ł���������炷
		::GetWindowRect(m_statusBar.GetStatusHwnd(), &rc);
		nStatusBarHeight = rc.bottom - rc.top;
		bMiniMapSizeBox = false;
	}
	RECT rcClient;
	::GetClientRect(GetHwnd(), &rcClient);

	//@@@ From 2003.05.31 MIK
	// �^�u�E�B���h�E�ǉ��ɔ����C�t�@���N�V�����L�[�\���ʒu������

	// �^�u�E�B���h�E
	int nTabHeightBottom = 0;
	int nTabWndHeight = 0;		//�^�u�E�B���h�E	//@@@ 2003.05.31 MIK
	if (m_tabWnd.GetHwnd()) {
		// �^�u���i��SizeBox/�E�B���h�E���ō������ς��\��������
		TabPosition tabPosition = m_pShareData->m_common.m_tabBar.m_eTabPosition;
		bool bHidden = false;
		if (tabPosition == TabPosition::Top) {
			// �ォ�牺�Ɉړ�����ƃS�~���\�������̂ň�x��\���ɂ���
			if (m_tabWnd.m_eTabPosition != TabPosition::None && m_tabWnd.m_eTabPosition != TabPosition::Top) {
				bHidden = true;
				::ShowWindow( m_tabWnd.GetHwnd(), SW_HIDE );
			}
			m_tabWnd.SizeBox_ONOFF( false );
			::GetWindowRect( m_tabWnd.GetHwnd(), &rc );
			nTabWndHeight = rc.bottom - rc.top;
			if (csWindow.m_nFUNCKEYWND_Place == 0) {
				::MoveWindow(m_tabWnd.GetHwnd(), 0, nToolBarHeight + nFuncKeyWndHeight, cx, nTabWndHeight, TRUE);
			}else {
				::MoveWindow(m_tabWnd.GetHwnd(), 0, nToolBarHeight, cx, nTabWndHeight, TRUE);
			}
			m_tabWnd.OnSize();
			::GetWindowRect( m_tabWnd.GetHwnd(), &rc );
			if (nTabWndHeight != rc.bottom - rc.top) {
				nTabWndHeight = rc.bottom - rc.top;
				if (csWindow.m_nFUNCKEYWND_Place == 0) {
					::MoveWindow( m_tabWnd.GetHwnd(), 0, nToolBarHeight + nFuncKeyWndHeight, cx, nTabWndHeight, TRUE );
				}else {
					::MoveWindow( m_tabWnd.GetHwnd(), 0, nToolBarHeight, cx, nTabWndHeight, TRUE );
				}
			}
		}else if (tabPosition == TabPosition::Bottom) {
			// �ォ�牺�Ɉړ�����ƃS�~���\�������̂ň�x��\���ɂ���
			if (m_tabWnd.m_eTabPosition != TabPosition::None && m_tabWnd.m_eTabPosition != TabPosition::Bottom) {
				bHidden = true;
				ShowWindow( m_tabWnd.GetHwnd(), SW_HIDE );
			}
			bool bSizeBox = true;
			if (m_statusBar.GetStatusHwnd()) {
				bSizeBox = false;
			}
			if (m_funcKeyWnd.GetHwnd()) {
				if (csWindow.m_nFUNCKEYWND_Place == 1 ){
					bSizeBox = false;
				}
			}
			if (wParam == SIZE_MAXIMIZED) {
				bSizeBox = false;
			}
			m_tabWnd.SizeBox_ONOFF( bSizeBox );
			::GetWindowRect( m_tabWnd.GetHwnd(), &rc );
			nTabWndHeight = rc.bottom - rc.top;
			::MoveWindow( m_tabWnd.GetHwnd(), 0,
				cy - nFuncKeyWndHeight - nStatusBarHeight - nTabWndHeight, cx, nTabWndHeight, TRUE );
			m_tabWnd.OnSize();
			::GetWindowRect( m_tabWnd.GetHwnd(), &rc );
			if (nTabWndHeight != rc.bottom - rc.top) {
				nTabWndHeight = rc.bottom - rc.top;
				::MoveWindow( m_tabWnd.GetHwnd(), 0,
					cy - nFuncKeyWndHeight - nStatusBarHeight - nTabWndHeight, cx, nTabWndHeight, TRUE );
			}
			nTabHeightBottom = rc.bottom - rc.top;
			nTabWndHeight = 0;
			bMiniMapSizeBox = false;
		}
		if (bHidden) {
			::ShowWindow( m_tabWnd.GetHwnd(), SW_SHOW );
		}
		m_tabWnd.m_eTabPosition = tabPosition;
	}

	//	2005.04.23 genta �t�@���N�V�����L�[��\���̎��͈ړ����Ȃ�
	if (m_funcKeyWnd.GetHwnd()) {
		if (csWindow.m_nFUNCKEYWND_Place == 0) {
			// �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
			::MoveWindow(
				m_funcKeyWnd.GetHwnd(),
				0,
				nToolBarHeight,
				cx,
				nFuncKeyWndHeight, TRUE);
		}else if (csWindow.m_nFUNCKEYWND_Place == 1) {
			// �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
			::MoveWindow(
				m_funcKeyWnd.GetHwnd(),
				0,
				cy - nFuncKeyWndHeight - nStatusBarHeight,
				cx,
				nFuncKeyWndHeight, TRUE
			);

			bool bSizeBox = true;
			if (m_statusBar.GetStatusHwnd()) {
				bSizeBox = false;
			}
			if (wParam == SIZE_MAXIMIZED) {
				bSizeBox = false;
			}
			m_funcKeyWnd.SizeBox_ONOFF(bSizeBox);
			bMiniMapSizeBox = false;
		}
		::UpdateWindow(m_funcKeyWnd.GetHwnd());	// 2006.06.17 ryoji �����`��ł���������炷
	}

	int nFuncListWidth = 0;
	int nFuncListHeight = 0;
	if (m_dlgFuncList.GetHwnd() && m_dlgFuncList.IsDocking()) {
		::SendMessage(m_dlgFuncList.GetHwnd(), WM_SIZE, wParam, lParam);
		::GetWindowRect(m_dlgFuncList.GetHwnd(), &rc);
		nFuncListWidth = rc.right - rc.left;
		nFuncListHeight = rc.bottom - rc.top;
	}

	DockSideType eDockSideFL = m_dlgFuncList.GetDockSide();
	int nTop = nToolBarHeight + nTabWndHeight;
	if (csWindow.m_nFUNCKEYWND_Place == 0)
		nTop += nFuncKeyWndHeight;
	int nHeight = cy - nToolBarHeight - nFuncKeyWndHeight - nTabWndHeight - nTabHeightBottom - nStatusBarHeight;
	if (m_dlgFuncList.GetHwnd() && m_dlgFuncList.IsDocking()) {
		::MoveWindow(
			m_dlgFuncList.GetHwnd(),
			(eDockSideFL == DockSideType::Right)? cx - nFuncListWidth: 0,
			(eDockSideFL == DockSideType::Bottom)? nTop + nHeight - nFuncListHeight: nTop,
			(eDockSideFL == DockSideType::Left || eDockSideFL == DockSideType::Right)? nFuncListWidth: cx,
			(eDockSideFL == DockSideType::Top || eDockSideFL == DockSideType::Bottom)? nFuncListHeight: nHeight,
			TRUE
		);
		if (eDockSideFL == DockSideType::Right || eDockSideFL == DockSideType::Bottom) {
			bMiniMapSizeBox = false;
		}
	}

	// �~�j�}�b�v
	int nMiniMapWidth = 0;
	if (GetMiniMap().GetHwnd()) {
		nMiniMapWidth = GetDllShareData().m_common.m_window.m_nMiniMapWidth;
		::MoveWindow( m_pEditViewMiniMap->GetHwnd(), 
			(eDockSideFL == DockSideType::Right)? cx - nFuncListWidth - nMiniMapWidth: cx - nMiniMapWidth,
			(eDockSideFL == DockSideType::Top)? nTop + nFuncListHeight: nTop,
			nMiniMapWidth,
			(eDockSideFL == DockSideType::Top || eDockSideFL == DockSideType::Bottom)? nHeight - nFuncListHeight: nHeight,
			TRUE
		);
		GetMiniMap().SplitBoxOnOff(false, false, bMiniMapSizeBox);
	}

	::MoveWindow(
		m_splitterWnd.GetHwnd(),
		(eDockSideFL == DockSideType::Left)? nFuncListWidth: 0,
		(eDockSideFL == DockSideType::Top)? nTop + nFuncListHeight: nTop,	//@@@ 2003.05.31 MIK
		((eDockSideFL == DockSideType::Left || eDockSideFL == DockSideType::Right)? cx - nFuncListWidth: cx) - nMiniMapWidth,
		(eDockSideFL == DockSideType::Top || eDockSideFL == DockSideType::Bottom)? nHeight - nFuncListHeight: nHeight,	//@@@ 2003.05.31 MIK
		TRUE
	);
	//@@@ To 2003.05.31 MIK

	// ����v���r���[���[�h��
//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
	if (!m_pPrintPreview) {
		return 0L;
	}
	return m_pPrintPreview->OnSize(wParam, lParam);
}


// WM_PAINT �`�揈��
LRESULT EditWnd::OnPaint(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
	// ����v���r���[���[�h��
	if (!m_pPrintPreview) {
		PAINTSTRUCT ps;
		::BeginPaint(hwnd, &ps);
		::EndPaint(hwnd, &ps);
		return 0L;
	}
//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
	return m_pPrintPreview->OnPaint(hwnd, uMsg, wParam, lParam);
}

// ����v���r���[ �����X�N���[���o�[���b�Z�[�W���� WM_VSCROLL
LRESULT EditWnd::OnVScroll(WPARAM wParam, LPARAM lParam)
{
	// ����v���r���[���[�h��
	if (!m_pPrintPreview) {
		return 0;
	}
//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
	return m_pPrintPreview->OnVScroll(wParam, lParam);
}

// ����v���r���[ �����X�N���[���o�[���b�Z�[�W����
LRESULT EditWnd::OnHScroll(WPARAM wParam, LPARAM lParam)
{
//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
	// ����v���r���[���[�h��
	if (!m_pPrintPreview) {
		return 0;
	}
	return m_pPrintPreview->OnHScroll(wParam, lParam);
}

LRESULT EditWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	// by �S(2) �L���v�`���[���ĉ����ꂽ���N���C�A���g�ł��������ɗ���
	if (m_iconClicked != IconClickStatus::None)
		return 0;

	m_ptDragPosOrg.x = LOWORD(lParam);	// horizontal position of cursor
	m_ptDragPosOrg.y = HIWORD(lParam);	// vertical position of cursor
	m_bDragMode = true;
	SetCapture(GetHwnd());

	return 0;
}

LRESULT EditWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	// by �S 2002/04/18
	if (m_iconClicked != IconClickStatus::None) {
		if (m_iconClicked == IconClickStatus::Down) {
			m_iconClicked = IconClickStatus::Clicked;
			// by �S(2) �^�C�}�[(ID�͓K���ł�)
			SetTimer(GetHwnd(), IDT_SYSMENU, GetDoubleClickTime(), NULL);
		}
		return 0;
	}

	m_bDragMode = false;
//	MYTRACE(_T("m_bDragMode = FALSE (OnLButtonUp)\n"));
	ReleaseCapture();
	::InvalidateRect(GetHwnd(), NULL, TRUE);
	return 0;
}


/*!	WM_MOUSEMOVE����
	@date 2008.05.05 novice ���������[�N�C��
*/
LRESULT EditWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	// by �S
	if (m_iconClicked != IconClickStatus::None) {
		// by �S(2) ��񉟂��ꂽ������
		if (m_iconClicked == IconClickStatus::Down) {
			POINT P;
			GetCursorPos(&P); // �X�N���[�����W
			if (SendMessage(GetHwnd(), WM_NCHITTEST, 0, P.x | (P.y << 16)) != HTSYSMENU) {
				ReleaseCapture();
				m_iconClicked = IconClickStatus::None;

				if (GetDocument()->m_docFile.GetFilePathClass().IsValidPath()) {
					// 2010.08.22 Moca C:\temp.txt �Ȃǂ�top�̃t�@�C����D&D�ł��Ȃ��o�O�̏C��
					NativeW cmemTitle;
					NativeW cmemDir;
					cmemTitle = to_wchar(GetDocument()->m_docFile.GetFileName());
					cmemDir   = to_wchar(GetDocument()->m_docFile.GetFilePathClass().GetDirPath().c_str());

					IDataObject *DataObject;
					IMalloc *Malloc;
					IShellFolder *Desktop, *Folder;
					LPITEMIDLIST PathID, ItemID;
					SHGetMalloc(&Malloc);
					SHGetDesktopFolder(&Desktop);
					DWORD Eaten, Attribs;
					if (SUCCEEDED(Desktop->ParseDisplayName(0, NULL, cmemDir.GetStringPtr(), &Eaten, &PathID, &Attribs))) {
						Desktop->BindToObject(PathID, NULL, IID_IShellFolder, (void**)&Folder);
						Malloc->Free(PathID);
						if (SUCCEEDED(Folder->ParseDisplayName(0, NULL, cmemTitle.GetStringPtr(), &Eaten, &ItemID, &Attribs))) {
							LPCITEMIDLIST List[1];
							List[0] = ItemID;
							Folder->GetUIObjectOf(0, 1, List, IID_IDataObject, NULL, (void**)&DataObject);
							Malloc->Free(ItemID);
#define DDASTEXT
#ifdef  DDASTEXT
							// �e�L�X�g�ł���������c�֗�
							{
								FORMATETC F;
								F.cfFormat = CF_UNICODETEXT;
								F.ptd      = NULL;
								F.dwAspect = DVASPECT_CONTENT;
								F.lindex   = -1;
								F.tymed    = TYMED_HGLOBAL;

								STGMEDIUM M;
								const wchar_t* pFilePath = to_wchar(GetDocument()->m_docFile.GetFilePath());
								int Len = wcslen(pFilePath);
								M.tymed          = TYMED_HGLOBAL;
								M.pUnkForRelease = NULL;
								M.hGlobal        = GlobalAlloc(GMEM_MOVEABLE, (Len + 1) * sizeof(wchar_t));
								void* p = GlobalLock(M.hGlobal);
								CopyMemory(p, pFilePath, (Len + 1) * sizeof(wchar_t));
								GlobalUnlock(M.hGlobal);

								DataObject->SetData(&F, &M, TRUE);
							}
#endif
							// �ړ��͋֎~
							DWORD R;
							DropSource drop(TRUE);
							DoDragDrop(DataObject, &drop, DROPEFFECT_COPY | DROPEFFECT_LINK, &R);
							DataObject->Release();
						}
						Folder->Release();
					}
					Desktop->Release();
					Malloc->Release();
				}
			}
		}
		return 0;
	}

//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
	if (!m_pPrintPreview) {
		return 0;
	}else {
		return m_pPrintPreview->OnMouseMove(wParam, lParam);
	}
}


LRESULT EditWnd::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
	if (m_pPrintPreview) {
		return m_pPrintPreview->OnMouseWheel(wParam, lParam);
	}
	return Views_DispatchEvent(GetHwnd(), WM_MOUSEWHEEL, wParam, lParam);
}

/** �}�E�X�z�C�[������

	@date 2007.10.16 ryoji OnMouseWheel()���珈�������o��
*/
BOOL EditWnd::DoMouseWheel(WPARAM wParam, LPARAM lParam)
{
//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
	// ����v���r���[���[�h��
	if (!m_pPrintPreview) {
		// 2006.03.26 ryoji by assitance with John �^�u��Ȃ�E�B���h�E�؂�ւ�
		if (m_pShareData->m_common.m_tabBar.m_bChgWndByWheel && m_tabWnd.m_hwndTab) {
			POINT pt;
			pt.x = (short)LOWORD(lParam);
			pt.y = (short)HIWORD(lParam);
			int nDelta = (short)HIWORD(wParam);
			HWND hwnd = ::WindowFromPoint(pt);
			if ((hwnd == m_tabWnd.m_hwndTab || hwnd == m_tabWnd.GetHwnd())) {
				// ���݊J���Ă���ҏW���̃��X�g�𓾂�
				EditNode* pEditNodeArr;
				int nRowNum = AppNodeManager::getInstance()->GetOpenedWindowArr(&pEditNodeArr, TRUE);
				if (nRowNum > 0) {
					// �����̃E�B���h�E�𒲂ׂ�
					int i, j;
					int nGroup = 0;
					for (i=0; i<nRowNum; ++i) {
						if (GetHwnd() == pEditNodeArr[i].GetHwnd()) {
							nGroup = pEditNodeArr[i].m_nGroup;
							break;
						}
					}
					if (i < nRowNum) {
						if (nDelta < 0) {
							// ���̃E�B���h�E
							for (j=i+1; j<nRowNum; ++j) {
								if (nGroup == pEditNodeArr[j].m_nGroup)
									break;
							}
							if (j >= nRowNum) {
								for (j=0; j<i; ++j) {
									if (nGroup == pEditNodeArr[j].m_nGroup)
										break;
								}
							}
						}else {
							// �O�̃E�B���h�E
							for (j=i-1; j>=0; --j) {
								if (nGroup == pEditNodeArr[j].m_nGroup)
									break;
							}
							if (j < 0) {
								for (j=nRowNum-1; j>i; --j) {
									if (nGroup == pEditNodeArr[j].m_nGroup)
										break;
								}
							}
						}

						// ���́ior �O�́j�E�B���h�E���A�N�e�B�u�ɂ���
						if (i != j)
							ActivateFrameWindow(pEditNodeArr[j].GetHwnd());
					}
					delete []pEditNodeArr;
				}
				return TRUE;	// ��������
			}
		}
		return FALSE;	// �������Ȃ�����
	}
	return FALSE;	// �������Ȃ�����
}

/* ����y�[�W�ݒ�
	����v���r���[���ɂ��A�����łȂ��Ƃ��ł��Ă΂��\��������B
*/
BOOL EditWnd::OnPrintPageSetting(void)
{
	// ����ݒ�iCANCEL�������Ƃ��ɔj�����邽�߂̗̈�j
	DlgPrintSetting	dlgPrintSetting;
	BOOL				bRes;
	int					nCurrentPrintSetting;
	int					nLineNumberColumns;

	nCurrentPrintSetting = GetDocument()->m_docType.GetDocumentAttribute().m_nCurrentPrintSetting;
	if (m_pPrintPreview) {
		nLineNumberColumns = GetActiveView().GetTextArea().DetectWidthOfLineNumberArea_calculate(m_pPrintPreview->m_pLayoutMgr_Print); // ����v���r���[���͕����̌��� 2013.5.10 aroka
	}else {
		nLineNumberColumns = 3; // �t�@�C�����j���[����̐ݒ莞�͍ŏ��l 2013.5.10 aroka
	}

	bRes = dlgPrintSetting.DoModal(
		G_AppInstance(),
//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
		GetHwnd(),
		&nCurrentPrintSetting, // ���ݑI�����Ă������ݒ�
		m_pShareData->m_printSettingArr, // ���݂̐ݒ�̓_�C�A���O���ŕێ����� 2013.5.1 aroka
		nLineNumberColumns // �s�ԍ��\���p�Ɍ�����n�� 2013.5.10 aroka
	);

	if (bRes) {
		bool bChangePrintSettingNo = false;
		// ���ݑI������Ă���y�[�W�ݒ�̔ԍ����ύX���ꂽ��
		if (GetDocument()->m_docType.GetDocumentAttribute().m_nCurrentPrintSetting != nCurrentPrintSetting) {
			// �ύX�t���O(�^�C�v�ʐݒ�)
			TypeConfig* type = new TypeConfig();
			DocTypeManager().GetTypeConfig(GetDocument()->m_docType.GetDocumentType(), *type);
			type->m_nCurrentPrintSetting = nCurrentPrintSetting;
			DocTypeManager().SetTypeConfig(GetDocument()->m_docType.GetDocumentType(), *type);
			delete type;
			GetDocument()->m_docType.GetDocumentAttributeWrite().m_nCurrentPrintSetting = nCurrentPrintSetting; // ���̐ݒ�ɂ����f
			AppNodeGroupHandle(0).SendMessageToAllEditors(
				MYWM_CHANGESETTING,
				(WPARAM)GetDocument()->m_docType.GetDocumentType().GetIndex(),
				(LPARAM)PM_CHANGESETTING_TYPE,
				EditWnd::getInstance()->GetHwnd()
			);
			bChangePrintSettingNo = true;
		}

//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
		//	����v���r���[���̂݁B
		if (m_pPrintPreview) {
			// ���݂̈���ݒ�
			// 2013.08.27 ����ݒ�ԍ����ύX���ꂽ���ɑΉ��ł��Ă��Ȃ�����
			if (bChangePrintSettingNo) {
				m_pPrintPreview->SetPrintSetting(&m_pShareData->m_printSettingArr[GetDocument()->m_docType.GetDocumentAttribute().m_nCurrentPrintSetting]);
			}

			// ����v���r���[ �X�N���[���o�[������
			//m_pPrintPreview->InitPreviewScrollBar();

			// ����ݒ�̔��f
			// m_pPrintPreview->OnChangePrintSetting();

			//::InvalidateRect(GetHwnd(), NULL, TRUE);
		}
		AppNodeGroupHandle(0).SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_PRINTSETTING,
			EditWnd::getInstance()->GetHwnd()
		);
	}
//@@@ 2002.01.14 YAZAKI ����v���r���[��PrintPreview�ɓƗ����������Ƃɂ��ύX
	::UpdateWindow(GetHwnd() /* m_pPrintPreview->GetPrintPreviewBarHANDLE() */);
	return bRes;
}

///////////////////////////// by �S

LRESULT EditWnd::OnNcLButtonDown(WPARAM wp, LPARAM lp)
{
	LRESULT Result;
	if (wp == HTSYSMENU) {
		SetCapture(GetHwnd());
		m_iconClicked = IconClickStatus::Down;
		Result = 0;
	}else
		Result = DefWindowProc(GetHwnd(), WM_NCLBUTTONDOWN, wp, lp);

	return Result;
}

LRESULT EditWnd::OnNcLButtonUp(WPARAM wp, LPARAM lp)
{
	LRESULT Result;
	if (m_iconClicked != IconClickStatus::None) {
		// �O�̂���
		ReleaseCapture();
		m_iconClicked = IconClickStatus::None;
		Result = 0;
	}else if (wp == HTSYSMENU) {
		Result = 0;
	}else {
		//	2004.05.23 Moca ���b�Z�[�W�~�X�C��
		//	�t���[���̃_�u���N���b�N����ɃE�B���h�E�T�C�Y
		//	�ύX���[�h�Ȃ��Ă���
		Result = DefWindowProc(GetHwnd(), WM_NCLBUTTONUP, wp, lp);
	}

	return Result;
}

LRESULT EditWnd::OnLButtonDblClk(WPARAM wp, LPARAM lp) // by �S(2)
{
	LRESULT Result;
	if (m_iconClicked != IconClickStatus::None) {
		ReleaseCapture();
		m_iconClicked = IconClickStatus::DoubleClicked;

		SendMessage(GetHwnd(), WM_SYSCOMMAND, SC_CLOSE, 0);

		Result = 0;
	}else {
		// 2004.05.23 Moca ���b�Z�[�W�~�X�C��
		Result = DefWindowProc(GetHwnd(), WM_LBUTTONDBLCLK, wp, lp);
	}

	return Result;
}

// �h���b�v�_�E�����j���[(�J��)	@@@ 2002.06.15 MIK
int	EditWnd::CreateFileDropDownMenu(HWND hwnd)
{
	int			nId;
	HMENU		hMenu;
	HMENU		hMenuPopUp;
	POINT		po;
	RECT		rc;
	int			nIndex;

	// ���j���[�\���ʒu�����߂�	// 2007.03.25 ryoji
	// �� TBN_DROPDOWN ���� NMTOOLBAR::iItem �� NMTOOLBAR::rcButton �ɂ̓h���b�v�_�E�����j���[(�J��)�{�^����
	//    ��������Ƃ��͂ǂ�������������P�ڂ̃{�^����񂪓���悤�Ȃ̂Ń}�E�X�ʒu����{�^���ʒu�����߂�
	::GetCursorPos(&po);
	::ScreenToClient(hwnd, &po);
	nIndex = Toolbar_Hittest(hwnd, &po);
	if (nIndex < 0) {
		return 0;
	}
	Toolbar_GetItemRect(hwnd, nIndex, &rc);
	po.x = rc.left;
	po.y = rc.bottom;
	::ClientToScreen(hwnd, &po);
	GetMonitorWorkRect(po, &rc);
	if (po.x < rc.left)
		po.x = rc.left;
	if (po.y < rc.top)
		po.y = rc.top;

	m_menuDrawer.ResetContents();

	// MRU���X�g�̃t�@�C���̃��X�g�����j���[�ɂ���
	const MRUFile mru;
	hMenu = mru.CreateMenu(&m_menuDrawer);
	if (mru.MenuLength() > 0) {
		m_menuDrawer.MyAppendMenuSep(
			hMenu,
			MF_BYPOSITION | MF_SEPARATOR,
			0,
			NULL,
			FALSE
		);
	}

	// �ŋߎg�����t�H���_�̃��j���[���쐬
	const MRUFolder mruFolder;
	hMenuPopUp = mruFolder.CreateMenu(&m_menuDrawer);
	if (mruFolder.MenuLength() > 0) {
		// �A�N�e�B�u
		m_menuDrawer.MyAppendMenu(
			hMenu,
			MF_BYPOSITION | MF_STRING | MF_POPUP,
			(UINT_PTR)hMenuPopUp,
			LS(F_FOLDER_USED_RECENTLY),
			_T("")
		);
	}else {
		// ��A�N�e�B�u
		m_menuDrawer.MyAppendMenu(
			hMenu,
			MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED,
			(UINT_PTR)hMenuPopUp,
			LS(F_FOLDER_USED_RECENTLY),
			_T("")
		);
	}

	m_menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, FALSE);

	m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, _T(""), _T("N"), FALSE);
	m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW_NEWWINDOW, _T(""), _T("M"), FALSE);
	m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, _T(""), _T("O"), FALSE);

	nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		,
		po.x,
		po.y,
		0,
		GetHwnd(),	// 2009.02.03 ryoji �A�N�Z�X�L�[�L�����̂��� hwnd -> GetHwnd() �ɕύX
		NULL
	);

	::DestroyMenu(hMenu);

	return nId;
}


/*!
	@brief �E�B���h�E�̃A�C�R���ݒ�

	�w�肳�ꂽ�A�C�R�����E�B���h�E�ɐݒ肷��D
	�ȑO�̃A�C�R���͔j������D

	@param hIcon [in] �ݒ肷��A�C�R��
	@param flag [in] �A�C�R����ʁDICON_BIG�܂���ICON_SMALL.
	@author genta
	@date 2002.09.10
*/
void EditWnd::SetWindowIcon(HICON hIcon, int flag)
{
	HICON hOld = (HICON)::SendMessage(GetHwnd(), WM_SETICON, flag, (LPARAM)hIcon);
	if (hOld) {
		::DestroyIcon(hOld);
	}
}

/*!
	�W���A�C�R���̎擾

	@param hIconBig   [out] �傫���A�C�R���̃n���h��
	@param hIconSmall [out] �������A�C�R���̃n���h��

	@author genta
	@date 2002.09.10
	@date 2002.12.02 genta �V�݂������ʊ֐����g���悤��
*/
void EditWnd::GetDefaultIcon(HICON* hIconBig, HICON* hIconSmall) const
{
	*hIconBig   = GetAppIcon(G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, false);
	*hIconSmall = GetAppIcon(G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, true);
}

/*!
	�A�C�R���̎擾
	
	�w�肳�ꂽ�t�@�C�����ɑΉ�����A�C�R��(��E��)���擾���ĕԂ��D
	
	@param szFile     [in] �t�@�C����
	@param hIconBig   [out] �傫���A�C�R���̃n���h��
	@param hIconSmall [out] �������A�C�R���̃n���h��
	
	@retval true �֘A�Â���ꂽ�A�C�R������������
	@retval false �֘A�Â���ꂽ�A�C�R����������Ȃ�����
	
	@author genta
	@date 2002.09.10
*/
bool EditWnd::GetRelatedIcon(const TCHAR* szFile, HICON* hIconBig, HICON* hIconSmall) const
{
	if (szFile && szFile[0] != _T('\0')) {
		TCHAR szExt[_MAX_EXT];
		TCHAR FileType[1024];

		// (.�Ŏn�܂�)�g���q�̎擾
		_tsplitpath(szFile, NULL, NULL, NULL, szExt);
		
		if (ReadRegistry(HKEY_CLASSES_ROOT, szExt, NULL, FileType, _countof(FileType) - 13)) {
			_tcscat(FileType, _T("\\DefaultIcon"));
			if (ReadRegistry(HKEY_CLASSES_ROOT, FileType, NULL, NULL, 0)) {
				// �֘A�Â���ꂽ�A�C�R�����擾����
				SHFILEINFO shfi;
				SHGetFileInfo(szFile, 0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_LARGEICON);
				*hIconBig = shfi.hIcon;
				SHGetFileInfo(szFile, 0, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_SMALLICON);
				*hIconSmall = shfi.hIcon;
				return true;
			}
		}
	}

	// �W���̃A�C�R����Ԃ�
	GetDefaultIcon(hIconBig, hIconSmall);
	return false;
}

/*
	@brief ���j���[�o�[�\���p�t�H���g�̏�����
	
	���j���[�o�[�\���p�t�H���g�̏��������s���D
	
	@date 2002.12.04 EditView�̃R���X�g���N�^����ړ�
*/
void EditWnd::InitMenubarMessageFont(void)
{
	TEXTMETRIC	tm;
	HDC			hdc;
	HFONT		hFontOld;

	// LOGFONT�̏�����
	LOGFONT lf = {0};
	lf.lfHeight			= DpiPointsToPixels(-9);	// 2009.10.01 ryoji ��DPI�Ή��i�|�C���g������Z�o�j
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
	m_hFontCaretPosInfo = ::CreateFontIndirect(&lf);

	hdc = ::GetDC(::GetDesktopWindow());
	hFontOld = (HFONT)::SelectObject(hdc, m_hFontCaretPosInfo);
	::GetTextMetrics(hdc, &tm);
	m_nCaretPosInfoCharWidth = tm.tmAveCharWidth;
	m_nCaretPosInfoCharHeight = tm.tmHeight;
	::SelectObject(hdc, hFontOld);
	::ReleaseDC(::GetDesktopWindow(), hdc);
}

/*
	@brief ���j���[�o�[�Ƀ��b�Z�[�W��\������
	
	���O�Ƀ��j���[�o�[�\���p�t�H���g������������Ă��Ȃ��Ă͂Ȃ�Ȃ��D
	�w��ł��镶�����͍ő�30�o�C�g�D����ȏ�̏ꍇ�͂����؂��ĕ\������D
	
	@author genta
	@date 2002.12.04
*/
void EditWnd::PrintMenubarMessage(const TCHAR* msg)
{
	if (!::GetMenu(GetHwnd()))	// 2007.03.08 ryoji �ǉ�
		return;

	POINT	po, poFrame;
	RECT	rc, rcFrame;
	HFONT	hFontOld;
	int		nStrLen;

	// msg == NULL �̂Ƃ��͈ȑO�� m_pszMenubarMessage �ōĕ`��
	if (msg) {
		int len = _tcslen(msg);
		_tcsncpy(m_pszMenubarMessage, msg, MENUBAR_MESSAGE_MAX_LEN);
		if (len < MENUBAR_MESSAGE_MAX_LEN) {
			auto_memset(m_pszMenubarMessage + len, _T(' '), MENUBAR_MESSAGE_MAX_LEN - len);	//  null�I�[�͕s�v
		}
	}

	HDC hdc = ::GetWindowDC(GetHwnd());
	poFrame.x = 0;
	poFrame.y = 0;
	::ClientToScreen(GetHwnd(), &poFrame);
	::GetWindowRect(GetHwnd(), &rcFrame);
	po.x = rcFrame.right - rcFrame.left;
	po.y = poFrame.y - rcFrame.top;
	hFontOld = (HFONT)::SelectObject(hdc, m_hFontCaretPosInfo);
	nStrLen = MENUBAR_MESSAGE_MAX_LEN;
	rc.left = po.x - nStrLen * m_nCaretPosInfoCharWidth - (::GetSystemMetrics(SM_CXSIZEFRAME) + 2);
	rc.right = rc.left + nStrLen * m_nCaretPosInfoCharWidth + 2;
	rc.top = po.y - m_nCaretPosInfoCharHeight - 2;
	rc.bottom = rc.top + m_nCaretPosInfoCharHeight;
	::SetTextColor(hdc, ::GetSysColor(COLOR_MENUTEXT));
	//	Sep. 6, 2003 genta Windows XP(Luna)�̏ꍇ�ɂ�COLOR_MENUBAR���g��Ȃ��Ă͂Ȃ�Ȃ�
	COLORREF bkColor =
		::GetSysColor(IsWinXP_or_later() ? COLOR_MENUBAR : COLOR_MENU);
	::SetBkColor(hdc, bkColor);
	/*
	int m_pnCaretPosInfoDx[64];	// ������`��p�������z��
	for (i=0; i<_countof(m_pnCaretPosInfoDx); ++i) {
		m_pnCaretPosInfoDx[i] = (m_nCaretPosInfoCharWidth);
	}
	*/
	::ExtTextOut(hdc, rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE,&rc,m_pszMenubarMessage,nStrLen,NULL/*m_pnCaretPosInfoDx*/); // 2007.10.17 kobake �߂�ǂ��̂ō��̂Ƃ���͕����Ԋu�z����g��Ȃ��B
	::SelectObject(hdc, hFontOld);
	::ReleaseDC(GetHwnd(), hdc);
}

/*!
	@brief ���b�Z�[�W�̕\��
	
	�w�肳�ꂽ���b�Z�[�W���X�e�[�^�X�o�[�ɕ\������D
	�X�e�[�^�X�o�[����\���̏ꍇ�̓��j���[�o�[�̉E�[�ɕ\������D
	
	@param msg [in] �\�����郁�b�Z�[�W
	@date 2002.01.26 hor �V�K�쐬
	@date 2002.12.04 genta EditView���ړ�
*/
void EditWnd::SendStatusMessage(const TCHAR* msg)
{
	if (!m_statusBar.GetStatusHwnd()) {
		// ���j���[�o�[��
		PrintMenubarMessage(msg);
	}else {
		// �X�e�[�^�X�o�[��
		m_statusBar.SetStatusText(0, SBT_NOBORDERS, msg);
	}
}

/*! �t�@�C�����ύX�ʒm

	@author MIK
	@date 2003.05.31 �V�K�쐬
	@date 2006.01.28 ryoji �t�@�C�����AGrep���[�h�p�����[�^��ǉ�
*/
void EditWnd::ChangeFileNameNotify(const TCHAR* pszTabCaption, const TCHAR* _pszFilePath, bool bIsGrep)
{
	const TCHAR* pszFilePath = _pszFilePath;
	
	EditNode* p;
	int nIndex;
	
	if (!pszTabCaption) pszTabCaption = _T("");	// �K�[�h
	if (!pszFilePath) pszFilePath = _FT("");	// �K�[�h 2006.01.28 ryoji
	
	RecentEditNode	recentEditNode;
	nIndex = recentEditNode.FindItemByHwnd(GetHwnd());
	if (nIndex != -1) {
		p = recentEditNode.GetItem(nIndex);
		if (p) {
			int	size = _countof(p->m_szTabCaption) - 1;
			_tcsncpy(p->m_szTabCaption, pszTabCaption, size);
			p->m_szTabCaption[size] = _T('\0');

			// 2006.01.28 ryoji �t�@�C�����AGrep���[�h�ǉ�
			size = _countof2(p->m_szFilePath) - 1;
			_tcsncpy(p->m_szFilePath, pszFilePath, size);
			p->m_szFilePath[size] = _T('\0');

			p->m_bIsGrep = bIsGrep;
		}
	}
	recentEditNode.Terminate();

	// �t�@�C�����ύX�ʒm���u���[�h�L���X�g����B
	int nGroup = AppNodeManager::getInstance()->GetEditNode(GetHwnd())->GetGroup();
	AppNodeGroupHandle(nGroup).PostMessageToAllEditors(
		MYWM_TAB_WINDOW_NOTIFY,
		(WPARAM)TabWndNotifyType::Rename,
		(LPARAM)GetHwnd(),
		GetHwnd()
	);

	return;
}

/*! ��Ɏ�O�ɕ\��
	@param top  0:�g�O������ 1:�őO�� 2:�őO�ʉ��� ���̑�:�Ȃɂ����Ȃ�
	@date 2004.09.21 Moca
*/
void EditWnd::WindowTopMost(int top)
{
	if (top == 0) {
		DWORD dwExstyle = (DWORD)::GetWindowLongPtr(GetHwnd(), GWL_EXSTYLE);
		if (dwExstyle & WS_EX_TOPMOST) {
			top = 2; // �őO�ʂł��� -> ����
		}else {
			top = 1;
		}
	}

	HWND hwndInsertAfter;
	switch (top) {
	case 1:
		hwndInsertAfter = HWND_TOPMOST;
		break;
	case 2:
		hwndInsertAfter = HWND_NOTOPMOST;
		break;
	default:
		return;
	}

	::SetWindowPos(GetHwnd(), hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// �^�u�܂Ƃߎ��� WS_EX_TOPMOST ��Ԃ�S�E�B���h�E�œ�������	// 2007.05.18 ryoji
	auto& csTabBar = m_pShareData->m_common.m_tabBar;
	if (m_pShareData->m_common.m_tabBar.m_bDispTabWnd && !m_pShareData->m_common.m_tabBar.m_bDispTabWndMultiWin) {
		hwndInsertAfter = GetHwnd();
		for (int i=0; i<m_pShareData->m_nodes.m_nEditArrNum; ++i) {
			HWND hwnd = m_pShareData->m_nodes.m_pEditArr[i].GetHwnd();
			if (hwnd != GetHwnd() && IsSakuraMainWindow(hwnd)) {
				if (!AppNodeManager::IsSameGroup(GetHwnd(), hwnd))
					continue;
				::SetWindowPos(hwnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				hwndInsertAfter = hwnd;
			}
		}
	}
}


// �^�C�}�[�̍X�V���J�n�^��~����B 20060128 aroka
// �c�[���o�[�\���̓^�C�}�[�ɂ��X�V���Ă��邪�A
// �A�v���̃t�H�[�J�X���O�ꂽ�Ƃ��ɃE�B���h�E����ON/OFF��
//	�Ăяo���Ă��炤���Ƃɂ��A�]�v�ȕ��ׂ��~�������B
void EditWnd::Timer_ONOFF(bool bStart)
{
	if (GetHwnd()) {
		if (bStart) {
			// �^�C�}�[���N��
			if (::SetTimer(GetHwnd(), IDT_TOOLBAR, 300, NULL) == 0) {
				WarningMessage(GetHwnd(), LS(STR_ERR_DLGEDITWND03));
			}
		}else {
			// �^�C�}�[���폜
			::KillTimer(GetHwnd(), IDT_TOOLBAR);
		}
	}
	return;
}

/*!	@brief �E�B���h�E�ꗗ���|�b�v�A�b�v�\��

	@param[in] bMousePos true: �}�E�X�ʒu�Ƀ|�b�v�A�b�v�\������

	@date 2006.03.23 fon OnListBtnClick���x�[�X�ɐV�K�쐬
	@date 2006.05.10 ryoji �|�b�v�A�b�v�ʒu�ύX�A���̑����C��
	@date 2007.02.28 ryoji �t���p�X�w��̃p�����[�^���폜
	@date 2009.06.02 ryoji m_menuDrawer�̏������R��C��
*/
LRESULT EditWnd::PopupWinList(bool bMousePos)
{
	POINT pt;

	// �|�b�v�A�b�v�ʒu���A�N�e�B�u�r���[�̏�ӂɐݒ�
	RECT rc;
	
	if (bMousePos) {
		::GetCursorPos(&pt);	// �}�E�X�J�[�\���ʒu�ɕύX
	}else {
		::GetWindowRect(GetActiveView().GetHwnd(), &rc);
		pt.x = rc.right - 150;
		if (pt.x < rc.left)
			pt.x = rc.left;
		pt.y = rc.top;
	}

	// �E�B���h�E�ꗗ���j���[���|�b�v�A�b�v�\������
	if (m_tabWnd.GetHwnd()) {
		m_tabWnd.TabListMenu(pt);
	}else {
		m_menuDrawer.ResetContents();	// 2009.06.02 ryoji �ǉ�
		EditNode* pEditNodeArr;
		HMENU hMenu = ::CreatePopupMenu();	// 2006.03.23 fon
		int nRowNum = AppNodeManager::getInstance()->GetOpenedWindowArr(&pEditNodeArr, TRUE);
		WinListMenu(hMenu, pEditNodeArr, nRowNum, true);
		// ���j���[��\������
		RECT rcWork;
		GetMonitorWorkRect(pt, &rcWork);	// ���j�^�̃��[�N�G���A
		int nId = ::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
									(pt.x > rcWork.left)? pt.x: rcWork.left,
									(pt.y < rcWork.bottom)? pt.y: rcWork.bottom,
									0, GetHwnd(), NULL);
		delete[] pEditNodeArr;
		::DestroyMenu(hMenu);
		::SendMessage(GetHwnd(), WM_COMMAND, (WPARAM)nId, (LPARAM)NULL);
	}

	return 0L;
}

/*! @brief ���݊J���Ă���ҏW���̃��X�g�����j���[�ɂ��� 
	@date  2006.03.23 fon EditWnd::InitMenu����ړ��B////�������炠��R�����g�B//>�͒ǉ��R�����g�A�E�g�B
	@date 2009.06.02 ryoji �A�C�e�����������Ƃ��̓A�N�Z�X�L�[�� 1-9,A-Z �͈̔͂ōĎg�p����i�]����36����������j
*/
LRESULT EditWnd::WinListMenu(HMENU hMenu, EditNode* pEditNodeArr, int nRowNum, bool bFull)
{
	if (nRowNum > 0) {
		TCHAR szMenu[_MAX_PATH * 2 + 3];
		FileNameManager::getInstance()->TransformFileName_MakeCache();

		NONCLIENTMETRICS met;
		met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
		DCFont dcFont(met.lfMenuFont, GetHwnd());
		for (int i=0; i<nRowNum; ++i) {
			// �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm
			::SendMessage(pEditNodeArr[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0);
////	From Here Oct. 4, 2000 JEPRO commented out & modified	�J���Ă���t�@�C�������킩��悤�ɗ����Ƃ͈����1���琔����
			const EditInfo*	pfi = (EditInfo*)&m_pShareData->m_workBuffer.m_EditInfo_MYWM_GETFILEINFO;
			FileNameManager::getInstance()->GetMenuFullLabel_WinList(szMenu, _countof(szMenu), pfi, pEditNodeArr[i].m_nId, i, dcFont.GetHDC());
			m_menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + pEditNodeArr[i].m_nIndex, szMenu, _T(""));
			if (GetHwnd() == pEditNodeArr[i].GetHwnd()) {
				::CheckMenuItem(hMenu, IDM_SELWINDOW + pEditNodeArr[i].m_nIndex, MF_BYCOMMAND | MF_CHECKED);
			}
		}
	}
	return 0L;
}

// 2007.09.08 kobake �ǉ�
// �c�[���`�b�v�̃e�L�X�g���擾
void EditWnd::GetTooltipText(TCHAR* wszBuf, size_t nBufCount, int nID) const
{
	// �@�\������̎擾 -> tmp -> wszBuf
	WCHAR tmp[256];
	size_t nLen;
	GetDocument()->m_funcLookup.Funccode2Name(nID, tmp, _countof(tmp));
	nLen = _wcstotcs(wszBuf, tmp, nBufCount);

	// �@�\�ɑΉ�����L�[���̎擾(����)
	auto& csKeyBind = m_pShareData->m_common.m_keyBind;
	NativeT** ppcAssignedKeyList;
	int nAssignedKeyNum = KeyBind::GetKeyStrList(
		G_AppInstance(),
		csKeyBind.m_nKeyNameArrNum,
		csKeyBind.m_pKeyNameArr,
		&ppcAssignedKeyList,
		nID
	);

	// wszBuf�֌���
	if (0 < nAssignedKeyNum) {
		for (int j=0; j<nAssignedKeyNum; ++j) {
			const TCHAR* pszKey = ppcAssignedKeyList[j]->GetStringPtr();
			int nKeyLen = _tcslen(pszKey);
			if (nLen + 9 + nKeyLen < nBufCount) {
				_tcscat_s(wszBuf, nBufCount, _T("\n        "));
				_tcscat_s(wszBuf, nBufCount, pszKey);
				nLen += 9 + nKeyLen;
			}
			delete ppcAssignedKeyList[j];
		}
		delete[] ppcAssignedKeyList;
	}
}


/*! �^�C�}�[�̏���
	@date 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�ShareData����MenuDrawer�ֈړ��������Ƃɂ��C���B
	@date 2003.08.29 wmlhq, ryoji nTimerCount�̓���
	@date 2006.01.28 aroka �c�[���o�[�X�V�� OnToolbarTimer�Ɉړ�����
	@date 2007.04.03 ryoji �p�����[�^�����ɂ���
*/
void EditWnd::OnEditTimer(void)
{
	//static	int	nLoopCount = 0; // wmlhq m_nTimerCount�Ɉڍs
	// �^�C�}�[�̌Ăяo���Ԋu�� 500ms�ɕύX�B300*10��500*6�ɂ���B 20060128 aroka
	IncrementTimerCount(6);

	// 2006.01.28 aroka �c�[���o�[�X�V�֘A�� OnToolbarTimer�Ɉړ������B
	
	//	Aug. 29, 2003 wmlhq, ryoji
	if (m_nTimerCount == 0 && !GetCapture()) { 
		// �t�@�C���̃^�C���X�^���v�̃`�F�b�N����
		GetDocument()->m_autoReloadAgent.CheckFileTimeStamp();

#if 0	// 2011.02.11 ryoji �����֎~�̊Ď���p�~�i����������Ȃ�u�X�V�̊Ď��v�t���ł͂Ȃ��ʃI�v�V�����ɂ��Ăق����j
		// �t�@�C�������\�̃`�F�b�N����
		if (GetDocument()->m_autoReloadAgent._ToDoChecking()) {
			bool bOld = GetDocument()->m_docLocker.IsDocWritable();
			GetDocument()->m_docLocker.CheckWritable(false);
			if (bOld != GetDocument()->m_docLocker.IsDocWritable()) {
				this->UpdateCaption();
			}
		}
#endif
	}

	GetDocument()->m_autoSaveAgent.CheckAutoSave();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �r���[�Ǘ�                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	EditView�̉�ʃo�b�t�@���폜
	@date 2007.09.09 Moca �V�K�쐬
*/
void EditWnd::Views_DeleteCompatibleBitmap()
{
	// EditView�Q�֓]������
	for (int i=0; i<GetAllViewCount(); ++i) {
		if (GetView(i).GetHwnd()) {
			GetView(i).DeleteCompatibleBitmap();
		}
	}
}

LRESULT EditWnd::Views_DispatchEvent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_ENTERMENULOOP:
	case WM_EXITMENULOOP:
		for (int i=0; i<GetAllViewCount(); ++i) {
			GetView(i).DispatchEvent(hwnd, msg, wParam, lParam);
		}
		return 0L;
	default:
		return GetActiveView().DispatchEvent(hwnd, msg, wParam, lParam);
	}
}

/*
	�����w���B2�ڈȍ~�̃r���[�����
	@param nViewCount  �����̃r���[���܂߂��r���[�̍��v�v����
*/
bool EditWnd::CreateEditViewBySplit(int nViewCount)
{
	if (m_nEditViewMaxCount < nViewCount) {
		return false;
	}
	if (GetAllViewCount() < nViewCount) {
		for (int i=GetAllViewCount(); i<nViewCount; ++i) {
			assert(!m_pEditViewArr[i]);
			m_pEditViewArr[i] = new EditView(this);
			m_pEditViewArr[i]->Create(m_splitterWnd.GetHwnd(), GetDocument(), i, FALSE, false);
		}
		m_nEditViewCount = nViewCount;

		std::vector<HWND> hWndArr;
		hWndArr.reserve(nViewCount + 1);
		for (int i=0; i<nViewCount; ++i) {
			hWndArr.push_back(GetView(i).GetHwnd());
		}
		hWndArr.push_back(NULL);

		m_splitterWnd.SetChildWndArr(&hWndArr[0]);
	}
	return true;
}

/*
	�r���[�̍ď�����
	@date 2010.04.10 EditDoc::InitAllView����ړ�
*/
void EditWnd::InitAllViews()
{
	// �擪�փJ�[�\�����ړ�
	for (int i=0; i<GetAllViewCount(); ++i) {
		//	Apr. 1, 2001 genta
		// �ړ������̏���
		GetView(i).m_pHistory->Flush();

		// ���݂̑I��͈͂��I����Ԃɖ߂�
		GetView(i).GetSelectionInfo().DisableSelectArea(false);

		GetView(i).OnChangeSetting();
		GetView(i).GetCaret().MoveCursor(LayoutPoint(0, 0), true);
		GetView(i).GetCaret().m_nCaretPosX_Prev = LayoutInt(0);
	}
	GetMiniMap().OnChangeSetting();
}


void EditWnd::Views_RedrawAll()
{
	// �A�N�e�B�u�ȊO���ĕ`�悵�Ă���c
	for (int v=0; v<GetAllViewCount(); ++v) {
		if (m_nActivePaneIndex != v) {
			GetView(v).RedrawAll();
		}
	}
	GetMiniMap().RedrawAll();
	// �A�N�e�B�u���ĕ`��
	GetActiveView().RedrawAll();
}

void EditWnd::Views_Redraw()
{
	// �A�N�e�B�u�ȊO���ĕ`�悵�Ă���c
	for (int v=0; v<GetAllViewCount(); ++v) {
		if (m_nActivePaneIndex != v)
			GetView(v).Redraw();
	}
	GetMiniMap().Redraw();
	// �A�N�e�B�u���ĕ`��
	GetActiveView().Redraw();
}


// �A�N�e�B�u�ȃy�C����ݒ�
void  EditWnd::SetActivePane(int nIndex)
{
	assert_warning(nIndex < GetAllViewCount());
	DEBUG_TRACE(_T("EditWnd::SetActivePane %d\n"), nIndex);

	// �A�N�e�B�u�ȃr���[��؂�ւ���
	int nOldIndex = m_nActivePaneIndex;
	m_nActivePaneIndex = nIndex;
	m_pEditView = m_pEditViewArr[m_nActivePaneIndex];

	// �t�H�[�J�X���ړ�����	// 2007.10.16 ryoji
	GetView(nOldIndex).GetCaret().m_cUnderLine.CaretUnderLineOFF(true);	//	2002/05/11 YAZAKI
	if (::GetActiveWindow() == GetHwnd()
		&& ::GetFocus() != GetActiveView().GetHwnd()
	) {
		// ::SetFocus()�Ńt�H�[�J�X��؂�ւ���
		::SetFocus(GetActiveView().GetHwnd());
	}else {
		// 2010.04.08 ryoji
		// �N���Ɠ����ɃG�f�B�b�g�{�b�N�X�Ƀt�H�[�J�X�̂���_�C�A���O��\������Ɠ��Y�G�f�B�b�g�{�b�N�X��
		// �L�����b�g���\������Ȃ����(*1)���C������̂��߁A�����I�Ȑ؂�ւ�������̂̓A�N�e�B�u�y�C����
		// �؂�ւ��Ƃ������ɂ����B�� EditView::OnKillFocus()�͎��X���b�h�̃L�����b�g��j������̂�
		// (*1) -GREPDLG�I�v�V�����ɂ��GREP�_�C�A���O�\����J�t�@�C���㎩�����s�}�N���ł�InputBox�\��
		if (m_nActivePaneIndex != nOldIndex) {
			// �A�N�e�B�u�łȂ��Ƃ���::SetFocus()����ƃA�N�e�B�u�ɂȂ��Ă��܂�
			// �i�s���Ȃ���ɂȂ�j�̂œ����I�ɐ؂�ւ��邾���ɂ���
			GetView(nOldIndex).OnKillFocus();
			GetActiveView().OnSetFocus();
		}
	}

	GetActiveView().RedrawAll();	// �t�H�[�J�X�ړ����̍ĕ`��

	m_splitterWnd.SetActivePane(nIndex);

	if (m_dlgFind.GetHwnd()) {		//�u�����v�_�C�A���O
		// ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX
		m_dlgFind.ChangeView((LPARAM)&GetActiveView());
	}
	if (m_dlgReplace.GetHwnd()) {	//�u�u���v�_�C�A���O
		// ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX
		m_dlgReplace.ChangeView((LPARAM)&GetActiveView());
	}
	if (m_hokanMgr.GetHwnd()) {	//�u���͕⊮�v�_�C�A���O
		m_hokanMgr.Hide();
		// ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX
		m_hokanMgr.ChangeView((LPARAM)&GetActiveView());
	}
	if (m_dlgFuncList.GetHwnd()) {	//�u�A�E�g���C���v�_�C�A���O	20060201 aroka
		// ���[�h���X���F���݈ʒu�\���̑ΏۂƂȂ�r���[�̕ύX
		m_dlgFuncList.ChangeView((LPARAM)&GetActiveView());
	}

	return;
}

/** ���ׂẴy�C���̕`��X�C�b�`��ݒ肷��

	@param bDraw [in] �`��X�C�b�`�̐ݒ�l

	@date 2008.06.08 ryoji �V�K�쐬
*/
bool EditWnd::SetDrawSwitchOfAllViews(bool bDraw)
{
	bool bDrawSwitchOld = GetActiveView().GetDrawSwitch();
	for (int i=0; i<GetAllViewCount(); ++i) {
		GetView(i).SetDrawSwitch(bDraw);
	}
	GetMiniMap().SetDrawSwitch( bDraw );
	return bDrawSwitchOld;
}


/** ���ׂẴy�C����Redraw����

	�X�N���[���o�[�̏�ԍX�V�̓p�����[�^�Ńt���O���� or �ʊ֐��ɂ����ق��������H
	@date 2007.07.22 ryoji �X�N���[���o�[�̏�ԍX�V��ǉ�

	@param pViewExclude [in] Redraw���珜�O����r���[
	@date 2008.06.08 ryoji pViewExclude �p�����[�^�ǉ�
*/
void EditWnd::RedrawAllViews(EditView* pViewExclude)
{
	for (int i=0; i<GetAllViewCount(); ++i) {
		EditView* pView = &GetView(i);
		if (pView == pViewExclude)
			continue;
		if (i == m_nActivePaneIndex) {
			pView->RedrawAll();
		}else {
			pView->Redraw();
			pView->AdjustScrollBars();
		}
	}
	GetMiniMap().Redraw();
	GetMiniMap().AdjustScrollBars();
}


void EditWnd::Views_DisableSelectArea(bool bRedraw)
{
	for (int i=0; i<GetAllViewCount(); ++i) {
		if (GetView(i).GetSelectionInfo().IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			GetView(i).GetSelectionInfo().DisableSelectArea(true);
		}
	}
}


// ���ׂẴy�C���ŁA�s�ԍ��\���ɕK�v�ȕ����Đݒ肷��i�K�v�Ȃ�ĕ`�悷��j
BOOL EditWnd::DetectWidthOfLineNumberAreaAllPane(bool bRedraw)
{
	if (GetAllViewCount() == 1) {
		return GetActiveView().GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
	}
	// �ȉ�2,4��������

	if (GetActiveView().GetTextArea().DetectWidthOfLineNumberArea(bRedraw)) {
		// ActivePane�Ōv�Z������A�Đݒ�E�ĕ`�悪�K�v�Ɣ�������
		if (m_splitterWnd.GetAllSplitCols() == 2) {
			GetView(m_nActivePaneIndex^1).GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
		}else {
			// �\������Ă��Ȃ��̂ōĕ`�悵�Ȃ�
			GetView(m_nActivePaneIndex^1).GetTextArea().DetectWidthOfLineNumberArea(false);
		}
		if (m_splitterWnd.GetAllSplitRows() == 2) {
			GetView(m_nActivePaneIndex^2).GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
			if (m_splitterWnd.GetAllSplitCols() == 2) {
				GetView((m_nActivePaneIndex^1)^2).GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
			}
		}else {
			GetView(m_nActivePaneIndex^2).GetTextArea().DetectWidthOfLineNumberArea(false);
			GetView((m_nActivePaneIndex^1)^2).GetTextArea().DetectWidthOfLineNumberArea(false);
		}
		return TRUE;
	}
	return FALSE;
}


/** �E�[�Ő܂�Ԃ�
	@param nViewColNum	[in] �E�[�Ő܂�Ԃ��y�C���̔ԍ�
	@retval �܂�Ԃ���ύX�������ǂ���
	@date 2008.06.08 ryoji �V�K�쐬
*/
bool EditWnd::WrapWindowWidth(int nPane)
{
	// �E�[�Ő܂�Ԃ�
	LayoutInt nWidth = GetView(nPane).ViewColNumToWrapColNum(GetView(nPane).GetTextArea().m_nViewColNum);
	if (GetDocument()->m_layoutMgr.GetMaxLineKetas() != nWidth) {
		ChangeLayoutParam(false, GetDocument()->m_layoutMgr.GetTabSpace(), nWidth);
		ClearViewCaretPosInfo();
		return true;
	}
	return false;
}

/** �܂�Ԃ����@�֘A�̍X�V
	@retval ��ʍX�V�������ǂ���
	@date 2008.06.10 ryoji �V�K�쐬
*/
BOOL EditWnd::UpdateTextWrap(void)
{
	// ���̊֐��̓R�}���h���s���Ƃɏ����̍ŏI�i�K�ŗ��p����
	// �i�A���h�D�o�^���S�r���[�X�V�̃^�C�~���O�j
	if (GetDocument()->m_nTextWrapMethodCur == TextWrappingMethod::WindowWidth) {
		bool bWrap = WrapWindowWidth(0);	// �E�[�Ő܂�Ԃ�
		if (bWrap) {
			// WrapWindowWidth() �Œǉ������X�V���[�W�����ŉ�ʍX�V����
			for (int i=0; i<GetAllViewCount(); ++i) {
				::UpdateWindow(GetView(i).GetHwnd());
			}
			if (GetMiniMap().GetHwnd()) {
				::UpdateWindow( GetMiniMap().GetHwnd() );
			}
		}
		return bWrap;	// ��ʍX�V���܂�Ԃ��ύX
	}
	return FALSE;	// ��ʍX�V���Ȃ�����
}

/*!	���C�A�E�g�p�����[�^�̕ύX

	��̓I�ɂ̓^�u���Ɛ܂�Ԃ��ʒu��ύX����D
	���݂̃h�L�������g�̃��C�A�E�g�݂̂�ύX���C���ʐݒ�͕ύX���Ȃ��D

	@date 2005.08.14 genta �V�K�쐬
	@date 2008.06.18 ryoji ���C�A�E�g�ύX�r���̓J�[�\���ړ��̉�ʃX�N���[���������Ȃ��i��ʂ̂�����}�~�j
*/
void EditWnd::ChangeLayoutParam(bool bShowProgress, LayoutInt nTabSize, LayoutInt nMaxLineKetas)
{
	HWND hwndProgress = NULL;
	if (bShowProgress && this) {
		hwndProgress = this->m_statusBar.GetProgressHwnd();
		// Status Bar���\������Ă��Ȃ��Ƃ���m_hwndProgressBar == NULL
	}

	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_SHOW);
	}

	// ���W�̕ۑ�
	LogicPointEx* posSave = SavePhysPosOfAllView();

	// ���C�A�E�g�̍X�V
	GetDocument()->m_layoutMgr.ChangeLayoutParam(nTabSize, nMaxLineKetas);
	ClearViewCaretPosInfo();

	// ���W�̕���
	// ���C�A�E�g�ύX�r���̓J�[�\���ړ��̉�ʃX�N���[���������Ȃ�	// 2008.06.18 ryoji
	const bool bDrawSwitchOld = SetDrawSwitchOfAllViews(false);
	RestorePhysPosOfAllView(posSave);
	SetDrawSwitchOfAllViews(bDrawSwitchOld);

	for (int i=0; i<GetAllViewCount(); ++i) {
		if (GetView(i).GetHwnd()) {
			InvalidateRect(GetView(i).GetHwnd(), NULL, TRUE);
			GetView(i).AdjustScrollBars();	// 2008.06.18 ryoji
		}
	}
	if (GetMiniMap().GetHwnd()) {
		InvalidateRect(GetMiniMap().GetHwnd(), NULL, TRUE);
		GetMiniMap().AdjustScrollBars();
	}
	GetActiveView().GetCaret().ShowCaretPosInfo();	// 2009.07.25 ryoji

	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_HIDE);
	}
}


/*!
	���C�A�E�g�̕ύX�ɐ旧���āC�S�Ă�View�̍��W�𕨗����W�ɕϊ����ĕۑ�����D

	@return �f�[�^��ۑ������z��ւ̃|�C���^

	@note �擾�����l�̓��C�A�E�g�ύX���EditWnd::RestorePhysPosOfAllView�֓n���D
	�n���Y���ƃ��������[�N�ƂȂ�D

	@date 2005.08.11 genta  �V�K�쐬
	@date 2007.09.06 kobake �߂�l��LogicPoint*�ɕύX
	@date 2011.12.28 LogicPoint��LogicPointEx�ɕύX�B���s���E���ł����A�ł���悤��
*/
LogicPointEx* EditWnd::SavePhysPosOfAllView()
{
	const int NUM_OF_VIEW = GetAllViewCount();
	const int NUM_OF_POS = 6;
	
	LogicPointEx* pptPosArray = new LogicPointEx[NUM_OF_VIEW * NUM_OF_POS];
	
	for (int i=0; i<NUM_OF_VIEW; ++i) {
		LayoutPoint tmp = LayoutPoint(LayoutInt(0), GetView(i).m_pTextArea->GetViewTopLine());
		const Layout* layoutLine = GetDocument()->m_layoutMgr.SearchLineByLayoutY(tmp.GetY2());
		if (layoutLine) {
			LogicInt nLineCenter = layoutLine->GetLogicOffset() + layoutLine->GetLengthWithoutEOL() / 2;
			pptPosArray[i * NUM_OF_POS + 0].x = nLineCenter;
			pptPosArray[i * NUM_OF_POS + 0].y = layoutLine->GetLogicLineNo();
		}else {
			pptPosArray[i * NUM_OF_POS + 0].x = LogicInt(0);
			pptPosArray[i * NUM_OF_POS + 0].y = LogicInt(0);
		}
		pptPosArray[i * NUM_OF_POS + 0].ext = LayoutInt(0);
		if (GetView(i).GetSelectionInfo().m_selectBgn.GetFrom().y >= 0) {
			GetDocument()->m_layoutMgr.LayoutToLogicEx(
				GetView(i).GetSelectionInfo().m_selectBgn.GetFrom(),
				&pptPosArray[i * NUM_OF_POS + 1]
			);
		}
		if (GetView(i).GetSelectionInfo().m_selectBgn.GetTo().y >= 0) {
			GetDocument()->m_layoutMgr.LayoutToLogicEx(
				GetView(i).GetSelectionInfo().m_selectBgn.GetTo(),
				&pptPosArray[i * NUM_OF_POS + 2]
			);
		}
		if (GetView(i).GetSelectionInfo().m_select.GetFrom().y >= 0) {
			GetDocument()->m_layoutMgr.LayoutToLogicEx(
				GetView(i).GetSelectionInfo().m_select.GetFrom(),
				&pptPosArray[i * NUM_OF_POS + 3]
			);
		}
		if (GetView(i).GetSelectionInfo().m_select.GetTo().y >= 0) {
			GetDocument()->m_layoutMgr.LayoutToLogicEx(
				GetView(i).GetSelectionInfo().m_select.GetTo(),
				&pptPosArray[i * NUM_OF_POS + 4]
			);
		}
		GetDocument()->m_layoutMgr.LayoutToLogicEx(
			GetView(i).GetCaret().GetCaretLayoutPos(),
			&pptPosArray[i * NUM_OF_POS + 5]
		);
	}
	return pptPosArray;
}


/*!	���W�̕���

	EditWnd::SavePhysPosOfAllView�ŕۑ������f�[�^�����ɍ��W�l���Čv�Z����D

	@date 2005.08.11 genta  �V�K�쐬
	@date 2007.09.06 kobake ������LogicPoint*�ɕύX
	@date 2011.12.28 LogicPoint��LogicPointEx�ɕύX�B���s���E���ł����A�ł���悤��
*/
void EditWnd::RestorePhysPosOfAllView(LogicPointEx* pptPosArray)
{
	const int NUM_OF_VIEW = GetAllViewCount();
	const int NUM_OF_POS = 6;

	for (int i=0; i<NUM_OF_VIEW; ++i) {
		LayoutPoint tmp;
		GetDocument()->m_layoutMgr.LogicToLayoutEx(
			pptPosArray[i * NUM_OF_POS + 0],
			&tmp
		);
		GetView(i).m_pTextArea->SetViewTopLine(tmp.GetY2());

		if (GetView(i).GetSelectionInfo().m_selectBgn.GetFrom().y >= 0) {
			GetDocument()->m_layoutMgr.LogicToLayoutEx(
				pptPosArray[i * NUM_OF_POS + 1],
				GetView(i).GetSelectionInfo().m_selectBgn.GetFromPointer()
			);
		}
		if (GetView(i).GetSelectionInfo().m_selectBgn.GetTo().y >= 0) {
			GetDocument()->m_layoutMgr.LogicToLayoutEx(
				pptPosArray[i * NUM_OF_POS + 2],
				GetView(i).GetSelectionInfo().m_selectBgn.GetToPointer()
			);
		}
		if (GetView(i).GetSelectionInfo().m_select.GetFrom().y >= 0) {
			GetDocument()->m_layoutMgr.LogicToLayoutEx(
				pptPosArray[i * NUM_OF_POS + 3],
				GetView(i).GetSelectionInfo().m_select.GetFromPointer()
			);
		}
		if (GetView(i).GetSelectionInfo().m_select.GetTo().y >= 0) {
			GetDocument()->m_layoutMgr.LogicToLayoutEx(
				pptPosArray[i * NUM_OF_POS + 4],
				GetView(i).GetSelectionInfo().m_select.GetToPointer()
			);
		}
		LayoutPoint ptPosXY;
		GetDocument()->m_layoutMgr.LogicToLayoutEx(
			pptPosArray[i * NUM_OF_POS + 5],
			&ptPosXY
		);
		GetView(i).GetCaret().MoveCursor(ptPosXY, false); // 2013.06.05 bScroll��true=>falase
		GetView(i).GetCaret().m_nCaretPosX_Prev = GetView(i).GetCaret().GetCaretLayoutPos().GetX2();

		LayoutInt nLeft = LayoutInt(0);
		if (GetView(i).GetTextArea().m_nViewColNum < GetView(i).GetRightEdgeForScrollBar()) {
			nLeft = GetView(i).GetRightEdgeForScrollBar() - GetView(i).GetTextArea().m_nViewColNum;
		}
		if (nLeft < GetView(i).GetTextArea().GetViewLeftCol()) {
			GetView(i).GetTextArea().SetViewLeftCol(nLeft);
		}

		GetView(i).GetCaret().ShowEditCaret();
	}
	GetActiveView().GetCaret().ShowCaretPosInfo();
	delete[] pptPosArray;
}

/*!
	@brief �}�E�X�̏�Ԃ��N���A����i�z�C�[���X�N���[���L����Ԃ��N���A�j

	@note �z�C�[������ɂ��y�[�W�X�N���[���E���X�N���[���Ή��̂��߂ɒǉ��B
		  �y�[�W�X�N���[���E���X�N���[������t���O��OFF����B

	@date 2009.01.17 nasukoji	�V�K�쐬
*/
void EditWnd::ClearMouseState(void)
{
	SetPageScrollByWheel(FALSE);		// �z�C�[������ɂ��y�[�W�X�N���[���L��
	SetHScrollByWheel(FALSE);			// �z�C�[������ɂ�鉡�X�N���[���L��
}

/*! �E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
	@date 2009.08.15 Hidetaka Sakai, nasukoji
	@date 2013.10.19 novice ���L�������̑����Wine���s���菈�����Ăяo��

	@note Wine�ł͕ʃv���Z�X�ō쐬�����A�N�Z�����[�^�e�[�u�����g�p���邱�Ƃ��ł��Ȃ��B
	      IsWine()�ɂ��v���Z�X���ɃA�N�Z�����[�^�e�[�u�����쐬�����悤�ɂȂ�
	      ���߁A�V���[�g�J�b�g�L�[��J�[�\���L�[������ɏ��������悤�ɂȂ�B
*/
void EditWnd::CreateAccelTbl(void)
{
	if (IsWine()) {
		auto& csKeyBind = m_pShareData->m_common.m_keyBind;
		m_hAccelWine = KeyBind::CreateAccerelator(
			csKeyBind.m_nKeyNameArrNum,
			csKeyBind.m_pKeyNameArr
		);

		if (!m_hAccelWine) {
			ErrorMessage(
				NULL,
				LS(STR_ERR_DLGEDITWND01)
			);
		}
	}

	m_hAccel = m_hAccelWine ? m_hAccelWine : m_pShareData->m_handles.m_hAccel;
}

/*! �E�B���h�E���ɍ쐬�����A�N�Z�����[�^�e�[�u����j������
	@datet 2009.08.15 Hidetaka Sakai, nasukoji
*/
void EditWnd::DeleteAccelTbl(void)
{
	m_hAccel = NULL;

	if (m_hAccelWine) {
		::DestroyAcceleratorTable(m_hAccelWine);
		m_hAccelWine = NULL;
	}
}

// �v���O�C���R�}���h���G�f�B�^�ɓo�^����
void EditWnd::RegisterPluginCommand(int idCommand)
{
	Plug* plug = JackManager::getInstance()->GetCommandById(idCommand);
	RegisterPluginCommand(plug);
}

// �v���O�C���R�}���h���G�f�B�^�ɓo�^����i�ꊇ�j
void EditWnd::RegisterPluginCommand()
{
	const Plug::Array& plugs = JackManager::getInstance()->GetPlugs(PP_COMMAND);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		RegisterPluginCommand(*it);
	}
}

// �v���O�C���R�}���h���G�f�B�^�ɓo�^����
void EditWnd::RegisterPluginCommand(Plug* plug)
{
	int iBitmap = MenuDrawer::TOOLBAR_ICON_PLUGCOMMAND_DEFAULT - 1;
	if (!plug->m_sIcon.empty()) {
		iBitmap = m_menuDrawer.m_pIcons->Add(to_tchar(plug->m_plugin.GetFilePath(to_tchar(plug->m_sIcon.c_str())).c_str()));
	}

	m_menuDrawer.AddToolButton(iBitmap, plug->GetFunctionCode());
}


const LOGFONT& EditWnd::GetLogfont(bool bTempSetting)
{
	if (bTempSetting && GetDocument()->m_blfCurTemp) {
		return GetDocument()->m_lfCur;
	}
	bool bUseTypeFont = GetDocument()->m_docType.GetDocumentAttribute().m_bUseTypeFont;
	if (bUseTypeFont) {
		return GetDocument()->m_docType.GetDocumentAttribute().m_lf;
	}
	return m_pShareData->m_common.m_view.m_lf;
}

int EditWnd::GetFontPointSize(bool bTempSetting)
{
	if (bTempSetting && GetDocument()->m_blfCurTemp) {
		return GetDocument()->m_nPointSizeCur;
	}
	bool bUseTypeFont = GetDocument()->m_docType.GetDocumentAttribute().m_bUseTypeFont;
	if (bUseTypeFont) {
		return GetDocument()->m_docType.GetDocumentAttribute().m_nPointSize;
	}
	return m_pShareData->m_common.m_view.m_nPointSize;
}
CharWidthCacheMode EditWnd::GetLogfontCacheMode()
{
	if (GetDocument()->m_blfCurTemp) {
		return CharWidthCacheMode::Local;
	}
	bool bUseTypeFont = GetDocument()->m_docType.GetDocumentAttribute().m_bUseTypeFont;
	if (bUseTypeFont) {
		return CharWidthCacheMode::Local;
	}
	return CharWidthCacheMode::Share;
}


void EditWnd::ClearViewCaretPosInfo()
{
	for (int v=0; v<GetAllViewCount(); ++v) {
		GetView(v).GetCaret().ClearCaretPosInfoCache();
	}
}
