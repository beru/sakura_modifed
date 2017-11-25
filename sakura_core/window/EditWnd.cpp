// �ҏW�E�B���h�E�i�O�g�j�Ǘ��N���X

#include "StdAfx.h"
#include <ShlObj.h>

#include "window/EditWnd.h"
#include "_main/ControlTray.h"
#include "_main/CommandLine.h"
#include "_main/AppMode.h"
#include "_os/DropTarget.h"
#include "_os/OsVersionInfo.h"
#include "dlg/DlgAbout.h"
#include "dlg/DlgPrintSetting.h"
#include "env/ShareData.h"
#include "env/SakuraEnvironment.h"
#include "print/PrintPreview.h"
#include "charset/CharPointer.h"
#include "charset/CodeFactory.h"
#include "charset/CodeBase.h"
#include "EditApp.h"
#include "recent/MRUFile.h"
#include "recent/MRUFolder.h"
#include "util/module.h"
#include "util/os.h"		// WM_MOUSEWHEEL,WM_THEMECHANGED
#include "util/window.h"
#include "util/shell.h"
#include "util/string_ex2.h"
#include "plugin/JackManager.h"
#include "GrepAgent.h"
#include "MarkMgr.h"
#include "doc/layout/Layout.h"
#include "debug/RunningTimer.h"
#include "sakura_rc.h"


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

//	�󋵂ɂ�胁�j���[�̕\����ς���R�}���h���X�g(SetMenuFuncSel�Ŏg�p)
struct FuncMenuName {
	EFunctionCode	eFunc;
	int				nNameId[2];		// �I�𕶎���ID
};

static const FuncMenuName	gFuncMenuName[] = {
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
	size_t nLineLen;
	const EditView* pView = &pEditDoc->pEditWnd->GetActiveView();
	const Caret* pCaret = &pView->GetCaret();
	const LayoutMgr* pLayoutMgr = &pEditDoc->layoutMgr;
	const wchar_t* pLine = pLayoutMgr->GetLineStr(pCaret->GetCaretLayoutPos().y, &nLineLen, &pLayout);

	// -- -- -- -- �L�����b�g�ʒu�̕������ -> szCaretChar -- -- -- -- //
	//
	if (pLine) {
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
		size_t nIdx = pView->LineColumnToIndex(pLayout, pCaret->GetCaretLayoutPos().x);
		if (nIdx < nLineLen) {
			if (nIdx < nLineLen - (pLayout->GetLayoutEol().GetLen() ? 1 : 0)) {
				// �ꎞ�I�ɕ\�����@�̐ݒ��ύX����
				CommonSetting_StatusBar sStatusbar;
				sStatusbar.bDispUniInSjis		= false;
				sStatusbar.bDispUniInJis		= false;
				sStatusbar.bDispUniInEuc		= false;
				sStatusbar.bDispUtf8Codepoint	= false;
				sStatusbar.bDispSPCodepoint	= false;

				TCHAR szMsg[128];
				TCHAR szCode[CODE_CODEMAX][32];
				wchar_t szChar[3];
				size_t nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, nIdx);
				memcpy(szChar, &pLine[nIdx], nCharChars * sizeof(wchar_t));
				szChar[nCharChars] = L'\0';
				for (size_t i=0; i<CODE_CODEMAX; ++i) {
					if (i == CODE_SJIS || i == CODE_JIS || i == CODE_EUC || i == CODE_LATIN1 || i == CODE_UNICODE || i == CODE_UTF8 || i == CODE_CESU8) {
						// �C�ӂ̕����R�[�h����Unicode�֕ϊ�����
						CodeBase* pCode = CodeFactory::CreateCodeBase((EncodingType)i, false);
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
				sStatusbar.bDispSPCodepoint = true;
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

EditWnd::EditWnd()
	:
	hWnd(NULL)
	, toolbar(*this)
	, statusBar(*this)
	, pPrintPreview(nullptr)
	, pDragSourceView(nullptr)
	, nActivePaneIndex(0)
	, nEditViewCount(1)
	, nEditViewMaxCount(_countof(pEditViewArr))	// ���̂Ƃ���ő�l�͌Œ�
	, uMSIMEReconvertMsg(::RegisterWindowMessage(RWM_RECONVERT))
	, uATOKReconvertMsg(::RegisterWindowMessage(MSGNAME_ATOK_RECONVERT))
	, bIsActiveApp(false)
	, pszLastCaption(NULL)
	, pszMenubarMessage(new TCHAR[MENUBAR_MESSAGE_MAX_LEN])
	, posSaveAry(nullptr)
	, nCurrentFocus(0)
	, hAccelWine(NULL)
	, hAccel(NULL)
	, bDragMode(false)
	, iconClicked(IconClickStatus::None)
	, nSelectCountMode(SelectCountMode::Toggle)	// �����J�E���g���@�̏����l��SELECT_COUNT_TOGGLE�����ʐݒ�ɏ]��
{
	g_pcEditWnd = this;
}

EditWnd::~EditWnd()
{
	g_pcEditWnd = nullptr;

	delete pPrintPreview;
	pPrintPreview = nullptr;

	for (int i=0; i<nEditViewMaxCount; ++i) {
		delete pEditViewArr[i];
		pEditViewArr[i] = nullptr;
	}
	pEditView = nullptr;

	delete pEditViewMiniMap;
	pEditViewMiniMap = nullptr;

	delete pViewFont;
	pViewFont = nullptr;

	delete pViewFontMiniMap;
	pViewFontMiniMap = nullptr;

	delete[] pszMenubarMessage;
	delete[] pszLastCaption;

	// �L�����b�g�̍s���ʒu�\���p�t�H���g
	::DeleteObject(hFontCaretPosInfo);

	delete pDropTarget;
	pDropTarget = nullptr;

	// �E�B���h�E���ɍ쐬�����A�N�Z�����[�^�e�[�u����j������(Wine�p)
	DeleteAccelTbl();

	hWnd = NULL;
}


// �h�L�������g���X�i�F�Z�[�u��
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
	const CommonSetting_Window& setting = GetDllShareData().common.window;
	const wchar_t* pszFormat = NULL;
	if (!this->IsActiveApp())	pszFormat = to_wchar(setting.szWindowCaptionInactive);
	else						pszFormat = to_wchar(setting.szWindowCaptionActive);
	SakuraEnvironment::ExpandParameter(
		pszFormat,
		pszCap,
		_countof(pszCap)
	);

	// �L���v�V�����X�V
	::SetWindowText(this->GetHwnd(), to_tchar(pszCap));

	// �^�u�E�B���h�E�̃t�@�C������ʒm
	SakuraEnvironment::ExpandParameter(GetDllShareData().common.tabBar.szTabWndCaption, pszCap, _countof(pszCap));
	this->ChangeFileNameNotify(
		to_tchar(pszCap),
		GetListeningDoc()->docFile.GetFilePath(),
		EditApp::getInstance().pGrepAgent->bGrepMode
	);
}



// �E�B���h�E�����p�̋�`���擾
void EditWnd::_GetWindowRectForInit(Rect* rcResult, int nGroup, const TabGroupInfo& tabGroupInfo)
{
	// �E�B���h�E�T�C�Y�p��
	int	nWinCX, nWinCY;
	auto& csWindow = pShareData->common.window;
	if (csWindow.eSaveWindowSize != WinSizeMode::Default) {
		nWinCX = csWindow.nWinSizeCX;
		nWinCY = csWindow.nWinSizeCY;
	}else {
		nWinCX = CW_USEDEFAULT;
		nWinCY = 0;
	}

	// �E�B���h�E�T�C�Y�w��
	EditInfo fi = CommandLine::getInstance().GetEditInfo();
	if (fi.nWindowSizeX >= 0) {
		nWinCX = fi.nWindowSizeX;
	}
	if (fi.nWindowSizeY >= 0) {
		nWinCY = fi.nWindowSizeY;
	}

	// �E�B���h�E�ʒu�w��
	int nWinOX = CW_USEDEFAULT;
	int nWinOY = 0;
	// �E�B���h�E�ʒu�Œ�
	if (csWindow.eSaveWindowPos != WinSizeMode::Default) {
		nWinOX =  csWindow.nWinPosX;
		nWinOY =  csWindow.nWinPosY;
	}

	if (fi.nWindowOriginX != CW_USEDEFAULT) {
		nWinOX = fi.nWindowOriginX;
	}
	if (fi.nWindowOriginY != CW_USEDEFAULT) {
		nWinOY = fi.nWindowOriginY;
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
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= CEditWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 32;
	wc.hInstance		= G_AppInstance();
	wc.hIcon			= GetAppIcon(G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, false);

	wc.hCursor			= NULL/*LoadCursor(NULL, IDC_ARROW)*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_3DSHADOW + 1)*/;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= GSTR_EDITWINDOWNAME;

	wc.cbSize			= sizeof(wc);
	wc.hIconSm			= GetAppIcon(G_AppInstance(), ICON_DEFAULT_APP, FN_APP_ICON, true);
	ATOM atom = RegisterClassEx(&wc);
	if (atom == 0) {
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

	// �^�u�E�B���h�E�̏ꍇ�͌���l���w��
	if (pShareData->common.tabBar.bDispTabWnd && !pShareData->common.tabBar.bDispTabWndMultiWin) {
		if (nGroup < 0)	// �s���ȃO���[�vID
			nGroup = 0;	// �O���[�v�w�薳���i�ŋ߃A�N�e�B�u�̃O���[�v�ɓ����j
		EditNode* pEditNode = AppNodeGroupHandle(nGroup).GetEditNodeAt(0);	// �O���[�v�̐擪�E�B���h�E�����擾
		hwndTop = pEditNode? pEditNode->GetHwnd(): NULL;

		if (hwndTop) {
			wpTop.length = sizeof(wpTop);
			if (::GetWindowPlacement(hwndTop, &wpTop)) {	// ���݂̐擪�E�B���h�E����ʒu���擾
				if (wpTop.showCmd == SW_SHOWMINIMIZED)
					wpTop.showCmd = pEditNode->showCmdRestore;
			}else {
				hwndTop = NULL;
			}
		}
	}

	// ����
	pTabGroupInfo->hwndTop = hwndTop;
	pTabGroupInfo->wpTop = wpTop;
}

void EditWnd::_AdjustInMonitor(const TabGroupInfo& tabGroupInfo)
{
	RECT	rcOrg;
	RECT	rcDesktop;
//	int		nWork;

	::GetMonitorWorkRect(GetHwnd(), &rcDesktop);
	::GetWindowRect(GetHwnd(), &rcOrg);

	// �E�B���h�E�ʒu����
	if (rcOrg.bottom > rcDesktop.bottom) {
		rcOrg.top -= rcOrg.bottom - rcDesktop.bottom;
		rcOrg.bottom = rcDesktop.bottom;
	}
	if (rcOrg.right > rcDesktop.right) {
		rcOrg.left -= rcOrg.right - rcDesktop.right;
		rcOrg.right = rcDesktop.right;
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
		rcOrg.bottom = rcDesktop.bottom;
	}
	if (rcOrg.right > rcDesktop.right) {
		rcOrg.right = rcDesktop.right;
	}

	if (pShareData->common.tabBar.bDispTabWnd
		&& !pShareData->common.tabBar.bDispTabWndMultiWin
		&& tabGroupInfo.hwndTop
	) {
		// ���݂̐擪�E�B���h�E���� WS_EX_TOPMOST ��Ԃ������p��
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
			typeOld = GetDocument().docType.GetDocumentType();	// ���݂̃^�C�v
			{
				EditInfo ei = CommandLine::getInstance().GetEditInfo();
				if (ei.szDocType[0] != '\0') {
					typeNew = DocTypeManager().GetDocumentTypeOfExt(ei.szDocType);
				}else {
					EditInfo mruei;
					if (MruFile().GetEditInfo(ei.szPath, &mruei) && 0 < mruei.nTypeId) {
						typeNew = DocTypeManager().GetDocumentTypeOfId(mruei.nTypeId);
					}
					if (!typeNew.IsValidType()) {
						if (ei.szPath[0]) {
							typeNew = DocTypeManager().GetDocumentTypeOfPath(ei.szPath);
						}else {
							typeNew = typeOld;
						}
					}
				}
			}
			GetDocument().docType.SetDocumentType(typeNew, true, true);	// ���ݒ�

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
			GetDocument().docType.SetDocumentType(typeOld, true, true);	// �^�C�v�߂�
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
		auto& csWindow = pShareData->common.window;
		if (csWindow.eSaveWindowSize != WinSizeMode::Default &&
			csWindow.nWinSizeType == SIZE_MAXIMIZED
		) {
			::ShowWindow(GetHwnd(), SW_SHOWMAXIMIZED);
		}else
		// �E�B���h�E�T�C�Y�𒼐ڎw�肷��ꍇ�́A�ŏ����\�����󂯓����
		if (csWindow.eSaveWindowSize == WinSizeMode::Set &&
			csWindow.nWinSizeType == SIZE_MINIMIZED
		) {
			::ShowWindow(GetHwnd(), SW_SHOWMINIMIZED);
		}else {
			::ShowWindow(GetHwnd(), SW_SHOW);
		}
	}
}

/*!
	�쐬
*/
HWND EditWnd::Create(
	EditDoc*		pEditDoc,
	ImageListMgr*	pIcons,	// [in] Image List
	int				nGroup		// [in] �O���[�vID
	)
{
	MY_RUNNINGTIMER(runningTimer, "EditWnd::Create");

	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	pShareData = &GetDllShareData();

	this->pEditDoc = pEditDoc;

	for (size_t i=0; i<_countof(pEditViewArr); ++i) {
		pEditViewArr[i] = nullptr;
	}
	// [0] - [3] �܂ō쐬�E���������Ă������̂�[0]�������B�ق��͕��������܂ŉ������Ȃ�
	pEditViewArr[0] = new EditView(*this);
	pEditView = pEditViewArr[0];

	pViewFont = new ViewFont(&GetLogfont());

	pEditViewMiniMap = new EditView(*this);

	pViewFontMiniMap = new ViewFont(&GetLogfont(), true);

	auto_memset(pszMenubarMessage, _T(' '), MENUBAR_MESSAGE_MAX_LEN);	// null�I�[�͕s�v

	InitMenubarMessageFont();

	pDropTarget = new DropTarget(this);	// �E�{�^���h���b�v�p

	// �z�C�[���X�N���[���L����Ԃ��N���A
	ClearMouseState();

	// �E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
	CreateAccelTbl();

	// �E�B���h�E������
	if (pShareData->nodes.nEditArrNum >= MAX_EDITWINDOWS) {	// �ő�l�C��
		OkMessage(NULL, LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
		return NULL;
	}

	// �^�u�O���[�v���擾
	TabGroupInfo tabGroupInfo;
	_GetTabGroupInfo(&tabGroupInfo, nGroup);


	// -- -- -- -- �E�B���h�E�쐬 -- -- -- -- //
	hWnd = _CreateMainWindow(nGroup, tabGroupInfo);
	if (!hWnd) {
		return NULL;
	}

	// ����A�C�h�����O���o�p�̃[���b�^�C�}�[���Z�b�g����
	// �[���b�^�C�}�[�������i����A�C�h�����O���o�j������ MYWM_FIRST_IDLE ���N�����v���Z�X�Ƀ|�X�g����B
	// ���N�����ł̋N����A�C�h�����O���o�ɂ��Ă� ControlTray::OpenNewEditor ���Q��
	::SetTimer(GetHwnd(), IDT_FIRST_IDLE, 0, NULL);

	// �ҏW�E�B���h�E���X�g�ւ̓o�^
	if (!AppNodeGroupHandle(nGroup).AddEditWndList(GetHwnd())) {
		OkMessage(GetHwnd(), LS(STR_MAXWINDOW), MAX_EDITWINDOWS);
		::DestroyWindow(GetHwnd());
		hWnd = NULL;
		return hWnd;
	}

	// �R�����R���g���[��������
	MyInitCommonControls();

	// �C���[�W�A�w���p�Ȃǂ̍쐬
	menuDrawer.Create(G_AppInstance(), GetHwnd(), pIcons);
	toolbar.Create(pIcons);

	// �v���O�C���R�}���h��o�^����
	RegisterPluginCommand();

	SelectCharWidthCache(CharWidthFontMode::MiniMap, CharWidthCacheMode::Local); // Init
	InitCharWidthCache(pViewFontMiniMap->GetLogfont(), CharWidthFontMode::MiniMap);
	SelectCharWidthCache(CharWidthFontMode::Edit, GetLogfontCacheMode());
	InitCharWidthCache(GetLogfont());

	// -- -- -- -- �q�E�B���h�E�쐬 -- -- -- -- //

	// �����t���[���쐬
	splitterWnd.Create(G_AppInstance(), GetHwnd(), this);

	// �r���[
	GetView(0).Create(splitterWnd.GetHwnd(), GetDocument(), 0, TRUE, false);
	GetView(0).OnSetFocus();

	// �q�E�B���h�E�̐ݒ�
	HWND hWndArr[2];
	hWndArr[0] = GetView(0).GetHwnd();
	hWndArr[1] = NULL;
	splitterWnd.SetChildWndArr(hWndArr);

	MY_TRACETIME(runningTimer, "View created");

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
	pDropTarget->Register_DropTarget(hWnd);	// �E�{�^���h���b�v�p

	// �A�N�e�B�u���
	bIsActiveApp = (::GetActiveWindow() == GetHwnd());

	// �G�f�B�^�|�g���C�Ԃł�UI���������̊m�F�iVista UIPI�@�\�j
	if (IsWinVista_or_later()) {
		bUIPI = FALSE;
		::SendMessage(pShareData->handles.hwndTray, MYWM_UIPI_CHECK,  (WPARAM)0, (LPARAM)GetHwnd());
		if (!bUIPI) {	// �Ԏ����Ԃ�Ȃ�
			TopErrorMessage(GetHwnd(),
				LS(STR_ERR_DLGEDITWND02)
			);
			::DestroyWindow(GetHwnd());
			hWnd = NULL;
			return hWnd;
		}
	}

	ShareData::getInstance().SetTraceOutSource(GetHwnd());	// TraceOut()�N�����E�B���h�E�̐ݒ�

	nTimerCount = 0;
	if (::SetTimer(GetHwnd(), IDT_EDIT, 500, NULL) == 0) {
		WarningMessage(GetHwnd(), LS(STR_ERR_DLGEDITWND03));
	}
	Timer_ONOFF(true);

	// �f�t�H���g��IME���[�h�ݒ�
	GetDocument().docEditor.SetImeMode(GetDocument().docType.GetDocumentAttribute().nImeState);

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
		bool bReadResult = GetDocument().docFileOperation.FileLoadWithoutAutoMacro(&loadInfo);	// �������s�}�N���͌�ŕʂ̏ꏊ�Ŏ��s�����
		if (!bReadResult) {
			// �t�@�C�������ɊJ����Ă���
			if (loadInfo.bOpened) {
				::PostMessage(GetHwnd(), WM_CLOSE, 0, 0);
				// return NULL���ƁA���b�Z�[�W���[�v��ʂ炸�ɂ��̂܂ܔj������Ă��܂��A�^�u�̏I��������������
				//	���̌�͐��탋�[�g�Ń��b�Z�[�W���[�v�ɓ�������WM_CLOSE����M���Ē�����CLOSE & DESTROY�ƂȂ�D
				//	���̒��ŕҏW�E�B���h�E�̍폜���s����D
			}
		}
	}
}

void EditWnd::SetDocumentTypeWhenCreate(
	EncodingType	nCharCode,		// [in] �����R�[�h
	bool			bViewMode,		// [in] �r���[���[�h�ŊJ�����ǂ���
	TypeConfigNum	nDocumentType	// [in] �����^�C�v�D-1�̂Ƃ������w�薳���D
	)
{
	auto& docType = GetDocument().docType;
	if (nDocumentType.IsValidType()) {
		docType.SetDocumentType(nDocumentType, true);
		// �^�C�v�ʐݒ�ꗗ�̈ꎞ�K�p�̃R�[�h�𗬗p
		docType.LockDocumentType();
	}

	// �����R�[�h�̎w��
	if (IsValidCodeType(nCharCode) || nDocumentType.IsValidType()) {
		const TypeConfig& types = docType.GetDocumentAttribute();
		EncodingType eDefaultCharCode = types.encoding.eDefaultCodetype;
		if (!IsValidCodeType(nCharCode)) {
			nCharCode = eDefaultCharCode;	// ���ڃR�[�h�w�肪�Ȃ���΃^�C�v�w��̃f�t�H���g�����R�[�h���g�p
		}
		if (nCharCode == eDefaultCharCode) {	// �f�t�H���g�����R�[�h�Ɠ��������R�[�h���I�����ꂽ�Ƃ�
			GetDocument().SetDocumentEncoding(nCharCode, types.encoding.bDefaultBom);
			GetDocument().docEditor.newLineCode = static_cast<EolType>(types.encoding.eDefaultEoltype);
		}else {
			GetDocument().SetDocumentEncoding(nCharCode, CodeTypeName(nCharCode).IsBomDefOn());
			GetDocument().docEditor.newLineCode = EolType::CRLF;
		}
	}

	AppMode::getInstance().SetViewMode(bViewMode);

	if (nDocumentType.IsValidType()) {
		// �ݒ�ύX�𔽉f������
		GetDocument().OnChangeSetting();	// <--- ������ BlockingHook() �Ăяo��������̂ŗ��܂����`�悪�����Ŏ��s�����
	}
}


/*! ���C�����j���[�̔z�u���� */
void EditWnd::LayoutMainMenu()
{
	TCHAR		szLabel[300];
	TCHAR		szKey[10];
	CommonSetting_MainMenu*	pMenu = &pShareData->common.mainMenu;
	MainMenu*	mainMenu;
	HWND		hWnd = GetHwnd();
	HMENU		hMenu;
	int 		nCount;
	LPCTSTR		pszName;

	hMenu = ::CreateMenu();
	auto& csKeyBind = pShareData->common.keyBind;
	for (int i=0; i<MAX_MAINMENU_TOP && pMenu->nMenuTopIdx[i] >= 0; ++i) {
		nCount = (i >= MAX_MAINMENU_TOP || pMenu->nMenuTopIdx[i + 1] < 0 ? pMenu->nMainMenuNum : pMenu->nMenuTopIdx[i+1])
				- pMenu->nMenuTopIdx[i];		// ���j���[���ڐ�
		mainMenu = &pMenu->mainMenuTbl[pMenu->nMenuTopIdx[i]];
		switch (mainMenu->type) {
		case MainMenuType::Node:
			// ���x�����ݒ肩��Function�R�[�h������Ȃ�X�g�����O�e�[�u������擾
			pszName = (mainMenu->sName[0] == L'\0' && mainMenu->nFunc != F_NODE)
								? LS(mainMenu->nFunc) : to_tchar(mainMenu->sName);
			::AppendMenu(hMenu, MF_POPUP | MF_STRING | (nCount <= 1 ? MF_GRAYED : 0), (UINT_PTR)CreatePopupMenu(), 
				KeyBind::MakeMenuLabel(pszName, to_tchar(mainMenu->sKey)));
			break;
		case MainMenuType::Leaf:
			// ���j���[���x���̍쐬
			{
				wchar_t szLabelW[256];
				GetDocument().funcLookup.Funccode2Name(mainMenu->nFunc, szLabelW, 256);
				auto_strncpy(szLabel, to_tchar(szLabelW), _countof(szLabel) - 1);
				szLabel[_countof(szLabel) - 1] = _T('\0');
			}
			auto_strcpy(szKey, to_tchar(mainMenu->sKey));
			if (!KeyBind::GetMenuLabel(
				G_AppInstance(),
				csKeyBind.nKeyNameArrNum,
				csKeyBind.pKeyNameArr,
				mainMenu->nFunc,
				szLabel,
				to_tchar(mainMenu->sKey),
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
				nCount = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
				delete[] pEditNodeArr;
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
				if (pShareData->common.customMenu.nCustMenuItemNumArr[0] > 0) {
					++nCount;
				}
				//	�J�X�^�����j���[
				for (int j=1; j<MAX_CUSTOM_MENU; ++j) {
					if (pShareData->common.customMenu.nCustMenuItemNumArr[j] > 0) {
						++nCount;
					}
				}
				break;
			case F_USERMACRO_LIST:			// �o�^�ς݃}�N�����X�g
				for (int j=0; j<MAX_CUSTMACRO; ++j) {
					MacroRec *mp = &pShareData->common.macro.macroTable[j];
					if (mp->IsEnabled()) {
						++nCount;
					}
				}
				break;
			case F_PLUGIN_LIST:				// �v���O�C���R�}���h���X�g
				// �v���O�C���R�}���h��񋟂���v���O�C����񋓂���
				{
					auto& jackManager = JackManager::getInstance();

					Plug::Array plugs = jackManager.GetPlugs(PP_COMMAND);
					for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
						++nCount;
					}
				}
				break;
			}
			::AppendMenu(hMenu, MF_POPUP | MF_STRING | (nCount <= 0 ? MF_GRAYED : 0), (UINT_PTR)CreatePopupMenu(), 
				KeyBind::MakeMenuLabel(LS(mainMenu->nFunc), to_tchar(mainMenu->sKey)));
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

/*! �c�[���o�[�̔z�u���� */
void EditWnd::LayoutToolBar(void)
{
	if (pShareData->common.window.bDispToolBar) {	// �c�[���o�[��\������
		toolbar.CreateToolBar();
	}else {
		toolbar.DestroyToolBar();
	}
}

/*! �X�e�[�^�X�o�[�̔z�u���� */
void EditWnd::LayoutStatusBar(void)
{
	if (pShareData->common.window.bDispStatusBar) {	// �X�e�[�^�X�o�[��\������
		// �X�e�[�^�X�o�[�쐬
		statusBar.CreateStatusBar();
	}else {
		// �X�e�[�^�X�o�[�j��
		statusBar.DestroyStatusBar();
	}
}

/*! �t�@���N�V�����L�[�̔z�u���� */
void EditWnd::LayoutFuncKey(void)
{
	if (pShareData->common.window.bDispFuncKeyWnd) {	// �t�@���N�V�����L�[��\������
		if (!funcKeyWnd.GetHwnd()) {
			bool bSizeBox;
			if (pShareData->common.window.nFuncKeyWnd_Place == 0) {	// �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
				bSizeBox = false;
			}else {
				bSizeBox = true;
				// �X�e�[�^�X�o�[������Ƃ��̓T�C�Y�{�b�N�X��\�����Ȃ�
				if (statusBar.GetStatusHwnd()) {
					bSizeBox = false;
				}
			}
			funcKeyWnd.Open(G_AppInstance(), GetHwnd(), &GetDocument(), bSizeBox);
		}
	}else {
		funcKeyWnd.Close();
	}
}

/*! �^�u�o�[�̔z�u���� */
void EditWnd::LayoutTabBar(void)
{
	if (pShareData->common.tabBar.bDispTabWnd) {	// �^�u�o�[��\������
		if (!tabWnd.GetHwnd()) {
			tabWnd.Open(G_AppInstance(), GetHwnd());
		}else {
			tabWnd.UpdateStyle();
		}
	}else {
		tabWnd.Close();
		tabWnd.SizeBox_ONOFF(false);
	}
}

/*! �~�j�}�b�v�̔z�u���� */
void EditWnd::LayoutMiniMap( void )
{
	if (pShareData->common.window.bDispMiniMap) {	// �^�u�o�[��\������
		if (!GetMiniMap().GetHwnd()) {
			GetMiniMap().Create(GetHwnd(), GetDocument(), -1, TRUE, true);
		}
	}else {
		if (GetMiniMap().GetHwnd()) {
			GetMiniMap().Close();
		}
	}
}

/*! �o�[�̔z�u�I������ */
void EditWnd::EndLayoutBars(bool bAdjust/* = true*/)
{
	int nCmdShow = pPrintPreview? SW_HIDE: SW_SHOW;
	HWND hwndToolBar = toolbar.GetRebarHwnd() ? toolbar.GetRebarHwnd(): toolbar.GetToolbarHwnd();
	if (hwndToolBar)
		::ShowWindow(hwndToolBar, nCmdShow);
	if (statusBar.GetStatusHwnd())
		::ShowWindow(statusBar.GetStatusHwnd(), nCmdShow);
	if (funcKeyWnd.GetHwnd())
		::ShowWindow(funcKeyWnd.GetHwnd(), nCmdShow);
	if (tabWnd.GetHwnd())
		::ShowWindow(tabWnd.GetHwnd(), nCmdShow);
	if (dlgFuncList.GetHwnd() && dlgFuncList.IsDocking()) {
		::ShowWindow(dlgFuncList.GetHwnd(), nCmdShow);
		// �A�E�g���C�����Ŕw��ɂ��Ă����i�S�~�`��̗}�~��j
		// ���̑΍�ȑO�́A�A�E�g���C�������h�b�L���O���Ă����ԂŁA
		// ���j���[����[�t�@���N�V�����L�[��\��]/[�X�e�[�^�X�o�[��\��]�����s���Ĕ�\���̃o�[���A�E�g���C�������ɕ\��������A
		// ���̌�A�E�B���h�E�̉������E���㉺�h���b�O���ăT�C�Y�ύX����ƃS�~������邱�Ƃ��������B
		::SetWindowPos(dlgFuncList.GetHwnd(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	}

	if (bAdjust) {
		RECT rc;
		splitterWnd.DoSplit(-1, -1);
		::GetClientRect(GetHwnd(), &rc);
		::SendMessage(GetHwnd(), WM_SIZE, nWinSizeType, MAKELONG(rc.right - rc.left, rc.bottom - rc.top));
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
void EditWnd::MessageLoop(void)
{
	MSG	msg;
	while (GetHwnd()) {
		// ���b�Z�[�W�擾
		int ret = GetMessage(&msg, NULL, 0, 0);
		if (ret == 0) break; // WM_QUIT
		if (ret == -1) break; // GetMessage���s

		// �_�C�A���O���b�Z�[�W
		     if (MyIsDialogMessage(pPrintPreview->GetPrintPreviewBarHANDLE_Safe(),	&msg)) {}	// ���Preview ����o�[
		else if (MyIsDialogMessage(dlgFind.GetHwnd(),									&msg)) {}	//�u�����v�_�C�A���O
		else if (MyIsDialogMessage(dlgFuncList.GetHwnd(),								&msg)) {}	//�u�A�E�g���C���v�_�C�A���O
		else if (MyIsDialogMessage(dlgReplace.GetHwnd(),								&msg)) {}	//�u�u���v�_�C�A���O
		else if (MyIsDialogMessage(dlgGrep.GetHwnd(),									&msg)) {}	//�uGrep�v�_�C�A���O
		else if (MyIsDialogMessage(hokanMgr.GetHwnd(),								&msg)) {}	//�u���͕⊮�v
		else if (toolbar.EatMessage(&msg)) { }													// �c�[���o�[
		// �A�N�Z�����[�^
		else {
			if (hAccel && TranslateAccelerator(msg.hwnd, hAccel, &msg)) {}
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
		return menuDrawer.OnMenuChar(hwnd, uMsg, wParam, lParam);

	case WM_SHOWWINDOW:
		if (!wParam) {
			Views_DeleteCompatibleBitmap();
		}
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);

	case WM_MENUSELECT:
		if (!statusBar.GetStatusHwnd()) {
			return 1;
		}
		uItem = (UINT) LOWORD(wParam);		// menu item or submenu index
		{
			// ���j���[�@�\�̃e�L�X�g���Z�b�g
			NativeT memWork;

			// �@�\�ɑΉ�����L�[���̎擾(����)
			NativeT** ppcAssignedKeyList;
			auto& csKeyBind = pShareData->common.keyBind;
			int nAssignedKeyNum = KeyBind::GetKeyStrList(
				G_AppInstance(),
				csKeyBind.nKeyNameArrNum,
				(KeyData*)csKeyBind.pKeyNameArr,
				&ppcAssignedKeyList,
				uItem
			);
			if (0 < nAssignedKeyNum) {
				for (int j=0; j<nAssignedKeyNum; ++j) {
					if (j > 0) {
						memWork.AppendStringLiteral(_T(" , "));
					}
					memWork.AppendNativeData(*ppcAssignedKeyList[j]);
					delete ppcAssignedKeyList[j];
				}
				delete[] ppcAssignedKeyList;
			}
			const TCHAR* pszItemStr = memWork.GetStringPtr();
			statusBar.SetStatusText(0, SBT_NOBORDERS, pszItemStr);
		}
		return 0;

	case WM_DRAWITEM:
		idCtl = (UINT) wParam;				// �R���g���[����ID
		lpdis = (DRAWITEMSTRUCT*) lParam;	// ���ڕ`����
		if (idCtl == IDW_STATUSBAR) {
			if (lpdis->itemID == 5) {
				int	nColor;
				if (pShareData->flags.bRecordingKeyMacro	// �L�[�{�[�h�}�N���̋L�^��
				 && pShareData->flags.hwndRecordingKeyMacro == GetHwnd()	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
				) {
					nColor = COLOR_BTNTEXT;
				}else {
					nColor = COLOR_3DSHADOW;
				}
				::SetTextColor(lpdis->hDC, ::GetSysColor(nColor));
				::SetBkMode(lpdis->hDC, TRANSPARENT);
				
				// �㉺�����ʒu�ɍ��
				TEXTMETRIC tm;
				::GetTextMetrics(lpdis->hDC, &tm);
				int y = (lpdis->rcItem.bottom - lpdis->rcItem.top - tm.tmHeight + 1) / 2 + lpdis->rcItem.top;
				::TextOut(lpdis->hDC, lpdis->rcItem.left, y, _T("REC"), _tcslen(_T("REC")));
				if (nColor == COLOR_BTNTEXT) {
					::TextOut(lpdis->hDC, lpdis->rcItem.left + 1, y, _T("REC"), _tcslen(_T("REC")));
				}
			}
			return 0;
		}else {
			switch (lpdis->CtlType) {
			case ODT_MENU:	// �I�[�i�[�`�惁�j���[
				// ���j���[�A�C�e���`��
				menuDrawer.DrawItem(lpdis);
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
			nItemWidth = menuDrawer.MeasureItem(lpmis->itemID, &nItemHeight);
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
		bIsActiveApp = (wParam != 0);	// ���A�v�����A�N�e�B�u���ǂ���

		// �A�N�e�B�u���Ȃ�ҏW�E�B���h�E���X�g�̐擪�Ɉړ�����
		if (bIsActiveApp) {
			AppNodeGroupHandle(0).AddEditWndList(GetHwnd());	// ���X�g�ړ�����

			// �z�C�[���X�N���[���L����Ԃ��N���A
			ClearMouseState();
		}

		// �^�C�}�[ON/OFF
		UpdateCaption();
		funcKeyWnd.Timer_ONOFF(bIsActiveApp);
		this->Timer_ONOFF(bIsActiveApp);

		return 0L;

	case WM_ENABLE:
		// �E�h���b�v�t�@�C���̎󂯓���ݒ�^����
		// Note: DragAcceptFiles��K�p�������h���b�v�ɂ��Ă� Enable/Disable �Ŏ����I�Ɏ󂯓���ݒ�^�������؂�ւ��
		if ((BOOL)wParam) {
			pDropTarget->Register_DropTarget(hWnd);
		}else {
			pDropTarget->Revoke_DropTarget();
		}
		return 0L;

	case WM_WINDOWPOSCHANGED:
		// �|�b�v�A�b�v�E�B���h�E�̕\���ؑ֎w�����|�X�g����
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
		::ShowOwnedPopups(hWnd, (BOOL)wParam);
		return 0L;

	case WM_SIZE:
//		MYTRACE(_T("WM_SIZE\n"));
		/* WM_SIZE ���� */
		if (wParam == SIZE_MINIMIZED) {
			this->UpdateCaption();
		}
		return OnSize(wParam, lParam);

	case WM_MOVE:
		// �E�B���h�E�ʒu�p��
		//	�Ō�̈ʒu�𕜌����邽�߁C�ړ�����邽�тɋ��L�������Ɉʒu��ۑ�����D
		if (WinSizeMode::Save == pShareData->common.window.eSaveWindowPos) {
			if (!::IsZoomed(GetHwnd()) && !::IsIconic(GetHwnd())) {
				// Aero Snap�ŏc�����ő剻�ŏI�����Ď���N������Ƃ��͌��̃T�C�Y�ɂ���K�v������̂ŁA
				// GetWindowRect()�ł͂Ȃ�GetWindowPlacement()�œ������[�N�G���A���W���X�N���[�����W�ɕϊ����ċL������
				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				::GetWindowPlacement(GetHwnd(), &wp);	// ���[�N�G���A���W
				RECT rcWin = wp.rcNormalPosition;
				RECT rcWork, rcMon;
				GetMonitorWorkRect(GetHwnd(), &rcWork, &rcMon);
				::OffsetRect(&rcWin, rcWork.left - rcMon.left, rcWork.top - rcMon.top);	// �X�N���[�����W�ɕϊ�
				pShareData->common.window.nWinPosX = rcWin.left;
				pShareData->common.window.nWinPosY = rcWin.top;
			}
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	case WM_SYSCOMMAND:
		// �^�u�܂Ƃߕ\���ł͕��铮��̓I�v�V�����w��ɏ]��
		//	Feb. 11, 2007 genta �����I�ׂ�悤��(MDI���Ə]������)
		if (wParam == SC_CLOSE) {
			// ���Preview���[�h�ŃE�B���h�E����鑀��̂Ƃ���Preview�����
			if (pPrintPreview) {
				PrintPreviewModeONOFF();	// ���Preview���[�h�̃I��/�I�t
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
	case WM_SYSKEYUP:	// ALT+�L�[�̃L�[���s�[�g�����̂���
	case WM_ENTERMENULOOP:
		if (GetActiveView().nAutoScrollMode) {
			GetActiveView().AutoScrollExit();
		}
		// ���b�Z�[�W�̔z��
		return Views_DispatchEvent(hwnd, uMsg, wParam, lParam);

	case WM_EXITMENULOOP:
//		MYTRACE(_T("WM_EXITMENULOOP\n"));
		if (statusBar.GetStatusHwnd()) {
			statusBar.SetStatusText(0, SBT_NOBORDERS, _T(""));
		}
		menuDrawer.EndDrawMenu();
		// ���b�Z�[�W�̔z��
		return Views_DispatchEvent(hwnd, uMsg, wParam, lParam);

	case WM_SETFOCUS:
//		MYTRACE(_T("WM_SETFOCUS\n"));

		nTimerCount = 9;

		// �r���[�Ƀt�H�[�J�X���ړ�����
		if (!pPrintPreview && pEditView) {
			::SetFocus(GetActiveView().GetHwnd());
		}
		lRes = 0;

		// ���Preview���[�h�̂Ƃ��́A�L�[����͑S��PrintPreviewBar�֓]��
		if (pPrintPreview) {
			pPrintPreview->SetFocusToPrintPreviewBar();
		}
		return lRes;

	case WM_NOTIFY:
		pnmh = (LPNMHDR) lParam;
		//	�X�e�[�^�X�o�[�̃_�u���N���b�N�Ń��[�h�ؑւ��ł���悤�ɂ���
		if (statusBar.GetStatusHwnd() && pnmh->hwndFrom == statusBar.GetStatusHwnd()) {
			if (pnmh->code == NM_DBLCLK) {
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if (mp->dwItemSpec == 6) {	//	�㏑��/�}��
					GetDocument().HandleCommand(F_CHGMOD_INS);
				}else if (mp->dwItemSpec == 5) {	//	�}�N���̋L�^�J�n�E�I��
					GetDocument().HandleCommand(F_RECKEYMACRO);
				}else if (mp->dwItemSpec == 1) {	//	���ʒu���s�ԍ��W�����v
					GetDocument().HandleCommand(F_JUMP_DIALOG);
				}else if (mp->dwItemSpec == 3) {	//	�����R�[�h���e��R�[�h
					ShowCodeBox(GetHwnd(), &GetDocument());
				}else if (mp->dwItemSpec == 4) {	//	�����R�[�h�Z�b�g�������R�[�h�Z�b�g�w��
					GetDocument().HandleCommand(F_CHG_CHARSET);
				}
			}else if (pnmh->code == NM_RCLICK) {
				LPNMMOUSE mp = (LPNMMOUSE) lParam;
				if (mp->dwItemSpec == 2) {	//	���͉��s���[�h
					enum eEolExts {
						F_CHGMOD_EOL_NEL = F_CHGMOD_EOL_CR + 1,
						F_CHGMOD_EOL_PS,
						F_CHGMOD_EOL_LS,
					};
					menuDrawer.ResetContents();
					HMENU hMenuPopUp = ::CreatePopupMenu();
					menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CRLF, 
						LS(F_CHGMOD_EOL_CRLF), _T("C")); // ���͉��s�R�[�h�w��(CRLF)
					menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LF,
						LS(F_CHGMOD_EOL_LF), _T("L")); // ���͉��s�R�[�h�w��(LF)
					menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_CR,
						LS(F_CHGMOD_EOL_CR), _T("rec")); // ���͉��s�R�[�h�w��(CR)
					// �g��EOL���L���̎������\��
					if (GetDllShareData().common.edit.bEnableExtEol) {
						menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_NEL,
							LS(STR_EDITWND_MENU_NEL), _T(""), TRUE, -2); // ���͉��s�R�[�h�w��(NEL)
						menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_LS,
							LS(STR_EDITWND_MENU_LS), _T(""), TRUE, -2); // ���͉��s�R�[�h�w��(LS)
						menuDrawer.MyAppendMenu(hMenuPopUp, MF_BYPOSITION | MF_STRING, F_CHGMOD_EOL_PS,
							LS(STR_EDITWND_MENU_PS), _T(""), TRUE, -2); // ���͉��s�R�[�h�w��(PS)
					}
					
					//	mp->pt�̓X�e�[�^�X�o�[�����̍��W�Ȃ̂ŁC�X�N���[�����W�ւ̕ϊ����K�v
					POINT po = mp->pt;
					::ClientToScreen(statusBar.GetStatusHwnd(), &po);
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

		switch (pnmh->code) {
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
		case NM_CUSTOMDRAW:
			if (pnmh->hwndFrom == toolbar.GetToolbarHwnd()) {
				//	�c�[���o�[��Owner Draw
				return toolbar.ToolBarOwnerDraw((LPNMCUSTOMDRAW)pnmh);
			}
			break;
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
		if (pShareData->flags.bRecordingKeyMacro) {					// �L�[�{�[�h�}�N���̋L�^��
			if (pShareData->flags.hwndRecordingKeyMacro == GetHwnd()) {	// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
				pShareData->flags.bRecordingKeyMacro = false;			// �L�[�{�[�h�}�N���̋L�^��
				pShareData->flags.hwndRecordingKeyMacro = NULL;		// �L�[�{�[�h�}�N�����L�^���̃E�B���h�E
			}
		}

		// �^�C�}�[���폜
		::KillTimer(GetHwnd(), IDT_TOOLBAR);

		// �h���b�v���ꂽ�t�@�C�����󂯓����̂�����
		::DragAcceptFiles(hwnd, FALSE);
		pDropTarget->Revoke_DropTarget();	// �E�{�^���h���b�v�p

		// �ҏW�E�B���h�E���X�g����̍폜
		AppNodeGroupHandle(GetHwnd()).DeleteEditWndList(GetHwnd());

		if (pShareData->handles.hwndDebug == GetHwnd()) {
			pShareData->handles.hwndDebug = NULL;
		}
		hWnd = NULL;


		// �ҏW�E�B���h�E�I�u�W�F�N�g����̃I�u�W�F�N�g�폜�v��
		::PostMessage(pShareData->handles.hwndTray, MYWM_DELETE_ME, 0, 0);

		// Windows �ɃX���b�h�̏I����v�����܂�
		::PostQuitMessage(0);

		return 0L;

	case WM_THEMECHANGED:
		// �r�W���A���X�^�C���^�N���V�b�N�X�^�C�����؂�ւ������c�[���o�[���č쐬����
		// �i�r�W���A���X�^�C��: Rebar �L��A�N���V�b�N�X�^�C��: Rebar �����j
		if (toolbar.GetToolbarHwnd()) {
			if (IsVisualStyle() == (!toolbar.GetRebarHwnd())) {
				toolbar.DestroyToolBar();
				LayoutToolBar();
				EndLayoutBars();
			}
		}
		return 0L;

	case MYWM_UIPI_CHECK:
		// �G�f�B�^�|�g���C�Ԃł�UI���������̊m�F���b�Z�[�W
		bUIPI = TRUE;	// �g���C����̕Ԏ����󂯎����
		return 0L;

	case MYWM_CLOSE:
		// �G�f�B�^�ւ̏I���v��
		if (nRet = OnClose(
				(HWND)lParam,
				(wParam & PM_CLOSE_GREPNOCONFIRM) == PM_CLOSE_GREPNOCONFIRM
			)
		) {
			// �v���O�C���FDocumentClose�C�x���g���s
			Plug::Array plugs;
			WSHIfObj::List params;
			JackManager::getInstance().GetUsablePlug(PP_DOCUMENT_CLOSE, 0, &plugs);
			for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
				(*it)->Invoke(GetActiveView(), params);
			}

			// �v���O�C���FEditorEnd�C�x���g���s
			plugs.clear();
			JackManager::getInstance().GetUsablePlug(PP_EDITOR_END, 0, &plugs);
			for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
				(*it)->Invoke(GetActiveView(), params);
			}

			// �^�u�܂Ƃߕ\���ł͕��铮��̓I�v�V�����w��ɏ]��
			if ((wParam & PM_CLOSE_EXIT) != PM_CLOSE_EXIT) {	// �S�I���v���łȂ��ꍇ
				// �^�u�܂Ƃߕ\����(����)���c���w��̏ꍇ�A�c�E�B���h�E���P�Ȃ�V�K�G�f�B�^���N�����ďI������
				if (pShareData->common.tabBar.bDispTabWnd &&
					!pShareData->common.tabBar.bDispTabWndMultiWin &&
					pShareData->common.tabBar.bTab_RetainEmptyWin
				) {
					// ���O���[�v���̎c�E�B���h�E���𒲂ׂ�
					int nGroup = AppNodeManager::getInstance().GetEditNode(GetHwnd())->GetGroup();
					if (AppNodeGroupHandle(nGroup).GetEditorWindowsNum() == 1) {
						EditNode* pEditNode = AppNodeManager::getInstance().GetEditNode(GetHwnd());
						if (pEditNode) {
							pEditNode->bClosing = true;	// �����̓^�u�\�����Ă����Ȃ��Ă���
						}
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
		pfi = (EditInfo*)&pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;

		// �ҏW�t�@�C�������i�[
		GetDocument().GetEditInfo(pfi);
		return 0L;
	case MYWM_CHANGESETTING:
		// �ݒ�ύX�̒ʒm
		switch ((e_PM_CHANGESETTING_SELECT)lParam) {
		case PM_CHANGESETTING_ALL:
			// �����I������
			SelectLang::ChangeLang(GetDllShareData().common.window.szLanguageDll);
			ShareData::getInstance().RefreshString();

			// ���C�����j���[
			LayoutMainMenu();

			// �ݒ�ύX���A�c�[���o�[���č쐬����悤�ɂ���i�o�[�̓��e�ύX�����f�j
			toolbar.DestroyToolBar();
			LayoutToolBar();

			// ��A�N�e�B�u�ȃE�B���h�E�̃c�[���o�[���X�V����
			// �A�N�e�B�u�ȃE�B���h�E�̓^�C�}�ɂ��X�V����邪�A����ȊO�̃E�B���h�E��
			// �^�C�}���~�����Ă���ݒ�ύX����ƑS���L���ƂȂ��Ă��܂����߁A������
			// �c�[���o�[���X�V����
			if (!bIsActiveApp)
				toolbar.UpdateToolbar();

			// �t�@���N�V�����L�[���č쐬����i�o�[�̓��e�A�ʒu�A�O���[�v�{�^�����̕ύX�����f�j
			funcKeyWnd.Close();
			LayoutFuncKey();

			// �^�u�o�[�̕\���^��\���؂�ւ�
			LayoutTabBar();

			// �X�e�[�^�X�o�[�̕\���^��\���؂�ւ�
			LayoutStatusBar();

			// �����X�N���[���o�[�̕\���^��\���؂�ւ�
			{
				bool b1 = !pShareData->common.window.bScrollBarHorz;
				for (int i=0; i<GetAllViewCount(); ++i) {
					bool b2 = (GetView(i).hwndHScrollBar == NULL);
					if (b1 != b2) {		// �����X�N���[���o�[���g��
						GetView(i).DestroyScrollBar();
						GetView(i).CreateScrollBar();
					}
				}
			}

			LayoutMiniMap();

			// �o�[�ύX�ŉ�ʂ�����Ȃ��悤��
			EndLayoutBars();

			// �A�N�Z�����[�^�e�[�u�����č쐬����(Wine�p)
			// �E�B���h�E���ɍ쐬�����A�N�Z�����[�^�e�[�u����j������(Wine�p)
			DeleteAccelTbl();
			// �E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
			CreateAccelTbl();
			
			if (pShareData->common.tabBar.bDispTabWnd) {
				// �^�u�\���̂܂܃O���[�v������^���Ȃ����ύX����Ă�����^�u���X�V����K�v������
				tabWnd.Refresh(false);
			}
			if (pShareData->common.tabBar.bDispTabWnd && !pShareData->common.tabBar.bDispTabWndMultiWin) {
				if (AppNodeManager::getInstance().GetEditNode(GetHwnd())->IsTopInGroup()) {
					if (!::IsWindowVisible(GetHwnd())) {
						// ::ShowWindow(GetHwnd(), SW_SHOWNA) ���Ɣ�\������\���ɐ؂�ւ��Ƃ��� Z-order �����������Ȃ邱�Ƃ�����̂� ::SetWindowPos ���g��
						::SetWindowPos(GetHwnd(), NULL, 0, 0, 0, 0,
										SWP_SHOWWINDOW | SWP_NOACTIVATE
										| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);

						// ���̃E�B���h�E�� WS_EX_TOPMOST ��Ԃ�S�E�B���h�E�ɔ��f����
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

			GetDocument().autoSaveAgent.ReloadAutoSaveParam();
			GetDocument().OnChangeSetting();	// �r���[�ɐݒ�ύX�𔽉f������
			GetDocument().docType.SetDocumentIcon();	// �����A�C�R���̍Đݒ�

			break;
		case PM_CHANGESETTING_FONT:
			GetDocument().OnChangeSetting(true);	// �t�H���g�ŕ��������ς��̂ŁA���C�A�E�g�č\�z
			break;
		case PM_CHANGESETTING_FONTSIZE:
			if ((wParam == -1 && GetLogfontCacheMode() == CharWidthCacheMode::Share)
				|| GetDocument().docType.GetDocumentType().GetIndex() == wParam
			) {
				GetDocument().OnChangeSetting( false );	// �r���[�ɐݒ�ύX�𔽉f������(���C�A�E�g���̍č쐬���Ȃ�)
			}
			break;
		case PM_CHANGESETTING_TYPE:
			typeNew = DocTypeManager().GetDocumentTypeOfPath(GetDocument().docFile.GetFilePath());
			if (GetDocument().docType.GetDocumentType().GetIndex() == wParam
				|| typeNew.GetIndex() == wParam
			) {
				GetDocument().OnChangeSetting();

				// �A�E�g���C����͉�ʏ���
				bool bAnalyzed = false;
				if (dlgFuncList.GetHwnd() && !bAnalyzed) {	// �A�E�g���C�����J���Ă���΍ĉ��
					// SHOW_NORMAL: ��͕��@���ω����Ă���΍ĉ�͂����B�����łȂ���Ε`��X�V�i�ύX���ꂽ�J���[�̓K�p�j�̂݁B
					EFunctionCode nFuncCode = dlgFuncList.GetFuncCodeRedraw(dlgFuncList.nOutlineType);
					GetActiveView().GetCommander().HandleCommand(nFuncCode, true, (LPARAM)ShowDialogType::Normal, 0, 0, 0);
				}
				if (MyGetAncestor(::GetForegroundWindow(), GA_ROOTOWNER2) == GetHwnd())
					::SetFocus(GetActiveView().GetHwnd());	// �t�H�[�J�X��߂�
			}
			break;
		case PM_CHANGESETTING_TYPE2:
			typeNew = DocTypeManager().GetDocumentTypeOfPath(GetDocument().docFile.GetFilePath());
			if (GetDocument().docType.GetDocumentType().GetIndex() == wParam
				|| typeNew.GetIndex() == wParam
			) {
				// index�̂ݍX�V
				GetDocument().docType.SetDocumentTypeIdx();
				// �^�C�v���ύX�ɂȂ����ꍇ�͓K�p����
				if (GetDocument().docType.GetDocumentType().GetIndex() != wParam) {
					::SendMessage(hWnd, MYWM_CHANGESETTING, wParam, PM_CHANGESETTING_TYPE);
				}
			}
			break;
		case PM_PrintSetting:
			{
				if (pPrintPreview) {
					pPrintPreview->OnChangeSetting();
				}
			}
			break;
		default:
			break;
		}
		return 0L;
	case MYWM_SAVEEDITSTATE:
		{
			if (pPrintPreview) {
				// �ꎞ�I�ɐݒ��߂�
				SelectCharWidthCache(CharWidthFontMode::Edit, CharWidthCacheMode::Neutral);
			}
			// �t�H���g�ύX�O�̍��W�̕ۑ�
			posSaveAry = SavePhysPosOfAllView();
			if (pPrintPreview) {
				// �ݒ��߂�
				SelectCharWidthCache(CharWidthFontMode::Print, CharWidthCacheMode::Local);
			}
		}
		return 0L; 
	case MYWM_SETACTIVEPANE:
		if ((int)wParam == -1) {
			if (lParam == 0) {
				nPane = splitterWnd.GetFirstPane();
			}else {
				nPane = splitterWnd.GetLastPane();
			}
			this->SetActivePane(nPane);
		}
		return 0L;
		
	case MYWM_SETCARETPOS:	// �J�[�\���ʒu�ύX�ʒm
		{
			//	LPARAM�ɐV���ȈӖ���ǉ�
			//	bit 0 (MASK 1): (bit 1==0�̂Ƃ�) 0/�I���N���A, 1/�I���J�n�E�ύX
			//	bit 1 (MASK 2): 0: bit 0�̐ݒ�ɏ]���D1:���݂̑I�����b�Ns��Ԃ��p��
			//	�����̎����ł� �ǂ����0�Ȃ̂ŋ��������Ɖ��߂����D
			//	�Ăяo������e_PM_SETCARETPOS_SELECTSTATE�̒l���g�����ƁD
			bool bSelect = ((lParam & 1) != 0);
			if (lParam & 2) {
				// ���݂̏�Ԃ�KEEP
				bSelect = GetActiveView().GetSelectionInfo().bSelectingLock;
			}
			
			/*
			�J�[�\���ʒu�ϊ�
			 �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			��
			 ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
			Point* ppoCaret = &(pShareData->workBuffer.logicPoint);
			Point ptCaretPos = GetDocument().layoutMgr.LogicToLayout(*ppoCaret);
			// ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤��
			// Note. ���Ƃ����s�P�ʂ̌��ʒu�Ȃ̂Ń��C�A�E�g�܂�Ԃ��̌��ʒu�𒴂��邱�Ƃ͂Ȃ��B
			//       �I���w��(bSelect==TRUE)�̏ꍇ�ɂ͂ǂ�����̂��Ó����悭�킩��Ȃ����A
			//       2007.08.22���݂ł̓A�E�g���C����̓_�C�A���O���猅�ʒu0�ŌĂяo�����
			//       �p�^�[�������Ȃ��̂Ŏ��p����ɖ��͖����B
			if (!bSelect) {
				const DocLine *pTmpDocLine = GetDocument().docLineMgr.GetLine(ppoCaret->y);
				if (pTmpDocLine) {
					if ((int)pTmpDocLine->GetLengthWithoutEOL() < ppoCaret->x) {
						ptCaretPos.x--;
					}
				}
			}
			//	�I��͈͂��l�����Ĉړ�
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
			pShareData->workBuffer.logicPoint = GetDocument().layoutMgr.LayoutToLogic(
				GetActiveView().GetCaret().GetCaretLayoutPos()
			);
		}
		return 0L;

	case MYWM_GETLINEDATA:	// �s(���s�P��)�f�[�^�̗v��
	{
		// ���L�f�[�^�F����Write������Read
		// return 0�ȏ�F�s�f�[�^����BwParam�I�t�Z�b�g���������s�f�[�^���B0��EOF��Offset�����傤�ǃo�b�t�@��������
		//       -1�ȉ��F�G���[
		int	nLineNum = (int)wParam;
		int	nLineOffset = (int)lParam;
		if (nLineNum < 0 || (int)GetDocument().docLineMgr.GetLineCount() < nLineNum) {
			return -2; // �s�ԍ��s���BLineCount == nLineNum ��EOF�s�Ƃ��ĉ��ŏ���
		}
		size_t nLineLen = 0;
		const wchar_t* pLine = GetDocument().docLineMgr.GetLine(nLineNum)->GetDocLineStrWithEOL( &nLineLen );
		if (nLineOffset < 0 || (int)nLineLen < nLineOffset) {
			return -3; // �I�t�Z�b�g�ʒu�s��
		}
		if (nLineNum == GetDocument().docLineMgr.GetLineCount()) {
			return 0; // EOF����I��
		}
 		if (!pLine) {
			return -4; // �s���ȃG���[
		}
		if (nLineLen == nLineOffset) {
 			return 0;
 		}
		pLine = GetDocument().docLineMgr.GetLine(wParam)->GetDocLineStrWithEOL( &nLineLen );
		pLine += nLineOffset;
		nLineLen -= nLineOffset;
		size_t nEnd = t_min<size_t>(nLineLen, pShareData->workBuffer.GetWorkBufferCount<EDIT_CHAR>());
		auto_memcpy( pShareData->workBuffer.GetWorkBuffer<EDIT_CHAR>(), pLine, nEnd );
		return nLineLen;
	}

	case MYWM_ADDSTRINGLEN_W:
		{
			EDIT_CHAR* pWork = pShareData->workBuffer.GetWorkBuffer<EDIT_CHAR>();
			size_t addSize = t_min((size_t)wParam, pShareData->workBuffer.GetWorkBufferCount<EDIT_CHAR>());
			GetActiveView().GetCommander().HandleCommand(F_ADDTAIL_W, true, (LPARAM)pWork, (LPARAM)addSize, 0, 0);
			GetActiveView().GetCommander().HandleCommand(F_GOFILEEND, true, 0, 0, 0, 0);
		}
		return 0L;

	// �^�u�E�B���h�E
	case MYWM_TAB_WINDOW_NOTIFY:
		tabWnd.TabWindowNotify(wParam, lParam);
		{
			RECT rc;
			::GetClientRect(GetHwnd(), &rc);
			OnSize2(nWinSizeType, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ), false);
			GetActiveView().SetIMECompFormPos();
		}
		return 0L;

	// �A�E�g���C��
	case MYWM_OUTLINE_NOTIFY:
		dlgFuncList.OnOutlineNotify(wParam, lParam);
		return 0L;

	// �o�[�̕\���E��\��
	case MYWM_BAR_CHANGE_NOTIFY:
		if (GetHwnd() != (HWND)lParam) {
			switch ((BarChangeNotifyType)wParam) {
			case BarChangeNotifyType::Toolbar:
				LayoutToolBar();
				break;
			case BarChangeNotifyType::FuncKey:
				LayoutFuncKey();
				break;
			case BarChangeNotifyType::Tab:
				LayoutTabBar();
				if (pShareData->common.tabBar.bDispTabWnd
					&& !pShareData->common.tabBar.bDispTabWndMultiWin
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
				LayoutStatusBar();
				break;
			case BarChangeNotifyType::MiniMap:
				LayoutMiniMap();
				break;
			}
			EndLayoutBars();
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
		if (!statusBar.GetStatusHwnd()) {
			PrintMenubarMessage(NULL);
		}
		return 0;

	case WM_NCACTIVATE:
		// �ҏW�E�B���h�E�ؑ֒��i�^�u�܂Ƃߎ��j�̓^�C�g���o�[�̃A�N�e�B�u�^��A�N�e�B�u��Ԃ��ł��邾���ύX���Ȃ��悤�Ɂi�P�j
		// �O�ʂɂ���̂��ҏW�E�B���h�E�Ȃ�A�N�e�B�u��Ԃ�ێ�����
		if (pShareData->flags.bEditWndChanging && IsSakuraMainWindow(::GetForegroundWindow())) {
			wParam = TRUE;	// �A�N�e�B�u
		}
		lRes = DefWindowProc(hwnd, uMsg, wParam, lParam);
		if (!statusBar.GetStatusHwnd()) {
			PrintMenubarMessage(NULL);
		}
		return lRes;

	case WM_SETTEXT:
		// �ҏW�E�B���h�E�ؑ֒��i�^�u�܂Ƃߎ��j�̓^�C�g���o�[�̃A�N�e�B�u�^��A�N�e�B�u��Ԃ��ł��邾���ύX���Ȃ��悤�Ɂi�Q�j
		// �^�C�}�[���g�p���ă^�C�g���̕ύX��x������
		if (pShareData->flags.bEditWndChanging) {
			delete[] pszLastCaption;
			pszLastCaption = new TCHAR[::_tcslen((LPCTSTR)lParam) + 1];
			::_tcscpy(pszLastCaption, (LPCTSTR)lParam);	// �ύX��̃^�C�g�����L�����Ă���
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
	int nRet = GetDocument().OnFileClose(bGrepNoConfirm);
	if (!nRet) {
		return nRet;
	}
	// �p�����[�^�Ńn���h����Ⴄ�l�ɂ����̂Ō������폜	2013/4/9 Uchi
	if (hWndActive) {
		// �A�N�e�B�u������E�B���h�E���A�N�e�B�u������
		if (IsSakuraMainWindow(hWndActive)) {
			ActivateFrameWindow(hWndActive);	// �G�f�B�^
		}else {
			::SetForegroundWindow(hWndActive);	// �^�X�N�g���C
		}
	}

	return nRet;
}


/*! WM_COMMAND���� */
void EditWnd::OnCommand(WORD wNotifyCode, WORD wID , HWND hwndCtl)
{
	// �����{�b�N�X����� WM_COMMAND �͂��ׂăR���{�{�b�N�X�ʒm
	// ##### �����{�b�N�X�����̓c�[���o�[���� WindowProc �ɏW�񂷂�ق����X�}�[�g����
	if (toolbar.GetSearchHwnd() && hwndCtl == toolbar.GetSearchHwnd()) {
		switch (wNotifyCode) {
		case CBN_SETFOCUS:
			nCurrentFocus = F_SEARCH_BOX;
			break;
		case CBN_KILLFOCUS:
			nCurrentFocus = 0;
			// �t�H�[�J�X���͂��ꂽ�Ƃ��Ɍ����L�[�ɂ��Ă��܂��B
			// �����L�[���[�h���擾
			std::wstring	strText;
			if (toolbar.GetSearchKey(strText)) {	// �L�[�����񂪂���
				// �����L�[��o�^
				if (strText.length() < _MAX_PATH) {
					SearchKeywordManager().AddToSearchKeys(strText.c_str());
				}
				GetActiveView().strCurSearchKey = strText;
				GetActiveView().bCurSearchUpdate = true;
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
		if (wID - IDM_SELWINDOW >= 0 && wID - IDM_SELWINDOW < (int)pShareData->nodes.nEditArrNum) {
			ActivateFrameWindow(pShareData->nodes.pEditArr[wID - IDM_SELWINDOW].GetHwnd());
		}
		// �ŋߎg�����t�@�C��
		else if (wID - IDM_SELMRU >= 0 && wID - IDM_SELMRU < 999) {
			// �w��t�@�C�����J����Ă��邩���ׂ�
			const MruFile mru;
			EditInfo checkEditInfo;
			mru.GetEditInfo(wID - IDM_SELMRU, &checkEditInfo);
			LoadInfo loadInfo(checkEditInfo.szPath, checkEditInfo.nCharCode, false);
			GetDocument().docFileOperation.FileLoad(&loadInfo);	//	Oct.  9, 2004 genta ���ʊ֐���
		}
		// �ŋߎg�����t�H���_
		else if (wID - IDM_SELOPENFOLDER >= 0 && wID - IDM_SELOPENFOLDER < 999) {
			// �t�H���_�擾
			const MruFolder mruFolder;
			LPCTSTR pszFolderPath = mruFolder.GetPath(wID - IDM_SELOPENFOLDER);

			// UNC�ł���ΐڑ������݂�
			NetConnect(pszFolderPath);

			//�u�t�@�C�����J���v�_�C�A���O
			LoadInfo loadInfo(_T(""), CODE_AUTODETECT, false);
			DocFileOperation& docOp = GetDocument().docFileOperation;
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
			if (wID != F_SEARCH_BOX && nCurrentFocus == F_SEARCH_BOX) {
				::SetFocus(GetActiveView().GetHwnd());
			}

			// �R�}���h�R�[�h�ɂ�鏈���U�蕪��
			//	May 19, 2006 genta ��ʃr�b�g��n��
			//	Jul. 7, 2007 genta ��ʃr�b�g��萔��
			GetDocument().HandleCommand((EFunctionCode)(wID | 0));
		}
		break;
	// �A�N�Z�����[�^����̃��b�Z�[�W
	case 1:
		{
			// �r���[�Ƀt�H�[�J�X���ړ����Ă���
			if (wID != F_SEARCH_BOX && nCurrentFocus == F_SEARCH_BOX)
				::SetFocus(GetActiveView().GetHwnd());
			auto& csKeyBind = pShareData->common.keyBind;
			EFunctionCode nFuncCode = KeyBind::GetFuncCode(
				wID,
				csKeyBind.nKeyNameArrNum,
				csKeyBind.pKeyNameArr
			);
			GetDocument().HandleCommand((EFunctionCode)(nFuncCode | FA_FROMKEYBOARD));
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
		const CommonSetting_MainMenu*	pMenu = &pShareData->common.mainMenu;
		const MainMenu*	pMainMenu;
		int		nIdxStr;
		int		nIdxEnd;
		int		nLv;
		std::vector<HMENU>	hSubMenu;
		std::wstring tmpMenuName;
		const wchar_t* pMenuName;

		nIdxStr = pMenu->nMenuTopIdx[uPos];
		nIdxEnd = (uPos < MAX_MAINMENU_TOP) ? pMenu->nMenuTopIdx[uPos + 1] : -1;
		if (nIdxEnd < 0) {
			nIdxEnd = pMenu->nMainMenuNum;
		}

		// ���j���[ ������
		menuDrawer.ResetContents();
		numMenuItems = ::GetMenuItemCount(hMenu);
		for (int i=numMenuItems-1; i>=0; --i) {
			::DeleteMenu(hMenu, i, MF_BYPOSITION);
		}

		// ���j���[�쐬
		hSubMenu.push_back(hMenu);
		nLv = 1;
		if (pMenu->mainMenuTbl[nIdxStr].type == MainMenuType::Special) {
			nLv = 0;
			--nIdxStr;
		}
		for (int i=nIdxStr+1; i<nIdxEnd; ++i) {
			pMainMenu = &pMenu->mainMenuTbl[i];
			if (pMainMenu->nLevel != nLv) {
				nLv = pMainMenu->nLevel;
				if (hSubMenu.size() < (size_t)nLv) {
					// �ی�
					break;
				}
				hMenu = hSubMenu[nLv-1];
			}
			switch (pMainMenu->type) {
			case MainMenuType::Node:
				hMenuPopUp = ::CreatePopupMenu();
				if (pMainMenu->nFunc != 0 && pMainMenu->sName[0] == L'\0') {
					// �X�g�����O�e�[�u������ǂݍ���
					tmpMenuName = LSW(pMainMenu->nFunc);
					if (MAX_MAIN_MENU_NAME_LEN < tmpMenuName.length()) {
						tmpMenuName = tmpMenuName.substr(0, MAX_MAIN_MENU_NAME_LEN);
					}
					pMenuName = tmpMenuName.c_str();
				}else {
					pMenuName = pMainMenu->sName;
				}
				menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , 
					pMenuName, pMainMenu->sKey);
				if (hSubMenu.size() > (size_t)nLv) {
					hSubMenu[nLv] = hMenuPopUp;
				}else {
					hSubMenu.push_back(hMenuPopUp);
				}
				break;
			case MainMenuType::Leaf:
				InitMenu_Function(hMenu, pMainMenu->nFunc, pMainMenu->sName, pMainMenu->sKey);
				break;
			case MainMenuType::Separator:
				menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
				break;
			case MainMenuType::Special:
				bool	bInList;		// ���X�g��1�ȏ゠��
				bInList = InitMenu_Special(hMenu, pMainMenu->nFunc);
				// ���X�g�������ꍇ�̏���
				if (!bInList) {
					// �������Ɉ͂܂�A�����X�g�Ȃ� �Ȃ�� ���̕��������X�L�b�v
					if ((i == nIdxStr + 1
						  || (pMenu->mainMenuTbl[i - 1].type == MainMenuType::Separator 
							&& pMenu->mainMenuTbl[i - 1].nLevel == pMainMenu->nLevel))
						&& i + 1 < nIdxEnd
						&& pMenu->mainMenuTbl[i + 1].type == MainMenuType::Separator 
						&& pMenu->mainMenuTbl[i + 1].nLevel == pMainMenu->nLevel) {
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

//@@@ 2002.01.14 YAZAKI ���Preview��PrintPreview�ɓƗ����������Ƃɂ��ύX
//	if (pPrintPreview)	return;	//	���Preview���[�h�Ȃ�r���B�i�����炭�r�����Ȃ��Ă������Ǝv���񂾂��ǁA�O�̂��߁j

	// �@�\�����p�\���ǂ����A�`�F�b�N��Ԃ��ǂ������ꊇ�`�F�b�N
	numMenuItems = ::GetMenuItemCount(hMenu);
	for (nPos=0; nPos<numMenuItems; ++nPos) {
		EFunctionCode	id = (EFunctionCode)::GetMenuItemID(hMenu, nPos);
		// �@�\�����p�\�����ׂ�
		//	Jan.  8, 2006 genta �@�\���L���ȏꍇ�ɂ͖����I�ɍĐݒ肵�Ȃ��悤�ɂ���D
		if (!IsFuncEnable(GetDocument(), *pShareData, id)) {
			UINT fuFlags = MF_BYCOMMAND | MF_GRAYED;
			::EnableMenuItem(hMenu, id, fuFlags);
		}

		// �@�\���`�F�b�N��Ԃ����ׂ�
		if (IsFuncChecked(GetDocument(), *pShareData, id)) {
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
		if (pShareData->common.customMenu.nCustMenuItemNumArr[j] > 0) {
			nFlag = MF_BYPOSITION | MF_STRING;
		}
		wchar_t buf[MAX_CUSTOM_MENU_NAME_LEN + 1];
		menuDrawer.MyAppendMenu(hMenu, nFlag,
			eFunc, GetDocument().funcLookup.Custmenu2Name(j, buf, _countof(buf)), pszKey);
	}
	// �}�N��
	else if (eFunc >= F_USERMACRO_0 && eFunc < F_USERMACRO_0 + MAX_CUSTMACRO) {
		MacroRec *mp = &pShareData->common.macro.macroTable[eFunc - F_USERMACRO_0];
		if (mp->IsEnabled()) {
			psName = to_wchar(mp->szName[0] ? mp->szName : mp->szFile);
			menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING,
				eFunc, psName, pszKey);
		}else {
			psName = L"-- undefined macro --";
			menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED,
				eFunc, psName, pszKey);
		}
	}
	// �v���O�C���R�}���h
	else if (eFunc >= F_PLUGCOMMAND_FIRST && eFunc < F_PLUGCOMMAND_LAST) {
		wchar_t szLabel[256];
		if (0 < JackManager::getInstance().GetCommandName( eFunc, szLabel, _countof(szLabel) )) {
			menuDrawer.MyAppendMenu(
				hMenu, MF_BYPOSITION | MF_STRING,
				eFunc, szLabel, pszKey,
				TRUE, eFunc
			);
		}else {
			// not found
			psName = L"-- undefined plugin command --";
			menuDrawer.MyAppendMenu(
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
				!pShareData->flags.bRecordingKeyMacro);
			break;
		case F_SPLIT_V:	
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				splitterWnd.GetAllSplitRows() == 1);
			break;
		case F_SPLIT_H:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				splitterWnd.GetAllSplitCols() == 1);
			break;
		case F_SPLIT_VH:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				splitterWnd.GetAllSplitRows() == 1 || splitterWnd.GetAllSplitCols() == 1);
			break;
		case F_TAB_CLOSEOTHER:
			SetMenuFuncSel(hMenu, eFunc, pszKey, pShareData->common.tabBar.bDispTabWnd);
			break;
		case F_TOPMOST:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				((DWORD)::GetWindowLongPtr(GetHwnd(), GWL_EXSTYLE) & WS_EX_TOPMOST) == 0);
			break;
		case F_BIND_WINDOW:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				(!pShareData->common.tabBar.bDispTabWnd 
				|| pShareData->common.tabBar.bDispTabWndMultiWin));
			break;
		case F_SHOWTOOLBAR:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!pShareData->common.window.bMenuIcon | !toolbar.GetToolbarHwnd());
			break;
		case F_SHOWFUNCKEY:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!pShareData->common.window.bMenuIcon | !funcKeyWnd.GetHwnd());
			break;
		case F_SHOWTAB:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!pShareData->common.window.bMenuIcon | !tabWnd.GetHwnd());
			break;
		case F_SHOWSTATUSBAR:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!pShareData->common.window.bMenuIcon | !statusBar.GetStatusHwnd());
			break;
		case F_SHOWMINIMAP:
			SetMenuFuncSel( hMenu, eFunc, pszKey, 
				!pShareData->common.window.bMenuIcon | !GetMiniMap().GetHwnd() );
			break;
		case F_TOGGLE_KEY_SEARCH:
			SetMenuFuncSel(hMenu, eFunc, pszKey, 
				!pShareData->common.window.bMenuIcon | !IsFuncChecked(GetDocument(), *pShareData, F_TOGGLE_KEY_SEARCH));
			break;
		case F_WRAPWINDOWWIDTH:
			{
				int ketas;
				wchar_t*	pszLabel;
				EditView::TOGGLE_WRAP_ACTION mode = GetActiveView().GetWrapMode(&ketas);
				if (mode == EditView::TGWRAP_NONE) {
					menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_GRAYED, F_WRAPWINDOWWIDTH , L"", pszKey);
				}else {
					wchar_t szBuf[60];
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
								GetActiveView().GetTextArea().nViewColNum
							)
						);
					}else {
						auto_sprintf_s(
							szBuf,
							LSW(STR_WRAP_WIDTH_FIXED),	//L"�܂�Ԃ�����: %d ���i�w��j",
							GetDocument().docType.GetDocumentAttribute().nMaxLineKetas
						);
					}
					menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_WRAPWINDOWWIDTH , pszLabel, pszKey);
				}
			}
			break;
		default:
			menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, eFunc, 
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
			size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
			WinListMenu(hMenu, pEditNodeArr, nRowNum, false);
			bInList = (nRowNum > 0);
			delete[] pEditNodeArr;
		}
		break;
	case F_FILE_USED_RECENTLY:		// �ŋߎg�����t�@�C��
		// MRU���X�g�̃t�@�C���̃��X�g�����j���[�ɂ���
		{
			//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
			const MruFile mru;
			mru.CreateMenu(hMenu, menuDrawer);	//	�t�@�C�����j���[
			bInList = (mru.MenuLength() > 0);
		}
		break;
	case F_FOLDER_USED_RECENTLY:	// �ŋߎg�����t�H���_
		// �ŋߎg�����t�H���_�̃��j���[���쐬
		{
			//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́AMruFolder�ɂ��ׂĈ˗�����
			const MruFolder mruFolder;
			mruFolder.CreateMenu(hMenu, menuDrawer);
			bInList = (mruFolder.MenuLength() > 0);
		}
		break;
	case F_CUSTMENU_LIST:			// �J�X�^�����j���[���X�g
		wchar_t buf[MAX_CUSTOM_MENU_NAME_LEN + 1];
		//	�E�N���b�N���j���[
		if (pShareData->common.customMenu.nCustMenuItemNumArr[0] > 0) {
			 menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING,
				 F_MENU_RBUTTON, GetDocument().funcLookup.Custmenu2Name(0, buf, _countof(buf)), L"");
			bInList = true;
		}
		// �J�X�^�����j���[
		for (int j=1; j<MAX_CUSTOM_MENU; ++j) {
			if (pShareData->common.customMenu.nCustMenuItemNumArr[j] > 0) {
				 menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING,
			 		F_CUSTMENU_BASE + j, GetDocument().funcLookup.Custmenu2Name(j, buf, _countof(buf)), L"" );
				bInList = true;
			}
		}
		break;
	case F_USERMACRO_LIST:			// �o�^�ς݃}�N�����X�g
		for (int j=0; j<MAX_CUSTMACRO; ++j) {
			MacroRec *mp = &pShareData->common.macro.macroTable[j];
			if (mp->IsEnabled()) {
				if (mp->szName[0]) {
					menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + j, mp->szName, _T(""));
				}else {
					menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_USERMACRO_0 + j, mp->szFile, _T(""));
				}
				bInList = true;
			}
		}
		break;
	case F_PLUGIN_LIST:				// �v���O�C���R�}���h���X�g
		// �v���O�C���R�}���h��񋟂���v���O�C����񋓂���
		{
			auto& jackManager = JackManager::getInstance();
			const Plugin* prevPlugin = nullptr;
			HMENU hMenuPlugin = 0;

			Plug::Array plugs = jackManager.GetPlugs(PP_COMMAND);
			for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
				const Plugin* curPlugin = &(*it)->plugin;
				if (curPlugin != prevPlugin) {
					// �v���O�C�����ς������v���O�C���|�b�v�A�b�v���j���[��o�^
					hMenuPlugin = ::CreatePopupMenu();
					menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPlugin, curPlugin->sName.c_str(), L"");
					prevPlugin = curPlugin;
				}

				// �R�}���h��o�^
				menuDrawer.MyAppendMenu(hMenuPlugin, MF_BYPOSITION | MF_STRING,
					(*it)->GetFunctionCode(), to_tchar((*it)->sLabel.c_str()), _T(""),
					TRUE, (*it)->GetFunctionCode());
			}
			bInList = (prevPlugin != nullptr);
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
void EditWnd::SetMenuFuncSel(HMENU hMenu, EFunctionCode nFunc, const wchar_t* sKey, bool flag)
{
	const wchar_t* sName = L"";
	for (size_t i=0; i<_countof(gFuncMenuName); ++i) {
		if (gFuncMenuName[i].eFunc == nFunc) {
			sName = flag ? LSW(gFuncMenuName[i].nNameId[0]) : LSW(gFuncMenuName[i].nNameId[1]);
		}
	}
	assert(auto_strlen(sName));

	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, nFunc, sName, sKey);
}


STDMETHODIMP EditWnd::DragEnter(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect)
{
	if (!pDataObject || !pdwEffect) {
		return E_INVALIDARG;
	}

	// �E�{�^���t�@�C���h���b�v�̏ꍇ������������
	if (!((dwKeyState & MK_RBUTTON) && IsDataAvailable(pDataObject, CF_HDROP))) {
		*pdwEffect = DROPEFFECT_NONE;
		return E_INVALIDARG;
	}

	// ���Preview�ł͎󂯕t���Ȃ�
	if (pPrintPreview) {
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
	UINT cFiles = ::DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	// �t�@�C�����h���b�v�����Ƃ��͕��ĊJ��
	auto& csFile = pShareData->common.file;
	if (csFile.bDropFileAndClose) {
		cFiles = 1;
	}
	// ��x�Ƀh���b�v�\�ȃt�@�C����
	if (cFiles > csFile.nDropFileNumMax) {
		cFiles = csFile.nDropFileNumMax;
	}

	// �A�N�e�B�u�ɂ���
	ActivateFrameWindow(GetHwnd());

	for (size_t i=0; i<cFiles; ++i) {
		// �t�@�C���p�X�擾�A�����B
		TCHAR szFile[_MAX_PATH + 1];
		::DragQueryFile(hDrop, i, szFile, _countof(szFile));
		SakuraEnvironment::ResolvePath(szFile);

		// �w��t�@�C�����J����Ă��邩���ׂ�
		HWND hWndOwner;
		if (ShareData::getInstance().IsPathOpened(szFile, &hWndOwner)) {
			::SendMessage(hWndOwner, MYWM_GETFILEINFO, 0, 0);
			EditInfo* pfi = (EditInfo*)&pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;
			// �A�N�e�B�u�ɂ���
			ActivateFrameWindow(hWndOwner);
			// MRU���X�g�ւ̓o�^
			MruFile mru;
			mru.Add(pfi);
		}else {
			// �ύX�t���O���I�t�ŁA�t�@�C����ǂݍ���ł��Ȃ��ꍇ
			if (GetDocument().IsAcceptLoad()) {
				// �t�@�C���ǂݍ���
				LoadInfo loadInfo(szFile, CODE_AUTODETECT, false);
				GetDocument().docFileOperation.FileLoad(&loadInfo);
			}else {
				// �t�@�C�����h���b�v�����Ƃ��͕��ĊJ��
				if (csFile.bDropFileAndClose) {
					// �t�@�C���ǂݍ���
					LoadInfo loadInfo(szFile, CODE_AUTODETECT, false);
					GetDocument().docFileOperation.FileCloseOpen(loadInfo);
				}else {
					// �ҏW�E�B���h�E�̏���`�F�b�N
					if (pShareData->nodes.nEditArrNum >= MAX_EDITWINDOWS) {	// �ő�l�C��
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

/*! WM_TIMER ���� */
LRESULT EditWnd::OnTimer(WPARAM wParam, LPARAM lParam)
{
	// �^�C�}�[ ID �ŏ�����U�蕪����
	switch (wParam) {
	case IDT_EDIT:
		OnEditTimer();
		break;
	case IDT_TOOLBAR:
		toolbar.OnToolbarTimer();
		break;
	case IDT_CAPTION:
		OnCaptionTimer();
		break;
	case IDT_SYSMENU:
		OnSysMenuTimer();
		break;
	case IDT_FIRST_IDLE:
		dlgFuncList.bEditWndReady = true;	// �G�f�B�^��ʂ̏�������
		AppNodeGroupHandle(0).PostMessageToAllEditors(MYWM_FIRST_IDLE, ::GetCurrentProcessId(), 0, NULL);	// �v���Z�X�̏���A�C�h�����O�ʒm
		::PostMessage(pShareData->handles.hwndTray, MYWM_FIRST_IDLE, (WPARAM)::GetCurrentProcessId(), (LPARAM)0);
		::KillTimer(hWnd, wParam);
		break;
	default:
		return 1L;
	}

	return 0L;
}


/*! �L���v�V�����X�V�p�^�C�}�[�̏��� */
void EditWnd::OnCaptionTimer(void)
{
	// �ҏW��ʂ̐ؑցi�^�u�܂Ƃߎ��j���I����Ă�����^�C�}�[���I�����ă^�C�g���o�[���X�V����
	// �܂��ؑ֒��Ȃ�^�C�}�[�p��
	if (!pShareData->flags.bEditWndChanging) {
		::KillTimer(GetHwnd(), IDT_CAPTION);
		::SetWindowText(GetHwnd(), pszLastCaption);
	}
}

/*! �V�X�e�����j���[�\���p�^�C�}�[�̏��� */
void EditWnd::OnSysMenuTimer(void)
{
	::KillTimer(GetHwnd(), IDT_SYSMENU);

	if (iconClicked == IconClickStatus::Clicked) {
		ReleaseCapture();

		// �V�X�e�����j���[�\��
		RECT rec;
		GetWindowRect(GetHwnd(), &rec);
		POINT pt;
		pt.x = rec.left + GetSystemMetrics(SM_CXFRAME);
		pt.y = rec.top + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
		GetMonitorWorkRect(pt, &rec);
		::PostMessage(
			GetHwnd(),
			0x0313, // �E�N���b�N�ŃV�X�e�����j���[��\������ۂɑ��M���郂�m�炵��
			0,
			MAKELPARAM((pt.x > rec.left)? pt.x: rec.left, (pt.y < rec.bottom)? pt.y: rec.bottom)
		);
	}
	iconClicked = IconClickStatus::None;
}

// ���Preview���[�h�̃I��/�I�t
void EditWnd::PrintPreviewModeONOFF(void)
{
	// Rebar ������΂�����c�[���o�[��������
	HWND hwndToolBar = toolbar.GetRebarHwnd() ? toolbar.GetRebarHwnd(): toolbar.GetToolbarHwnd();

	// ���Preview���[�h��
	if (pPrintPreview) {
		// ���Preview���[�h���������܂��B
		delete pPrintPreview;	//	�폜�B
		pPrintPreview = nullptr;	//	nullptr���ۂ��ŁA�v�����gPreview���[�h�����f���邽�߁B

		// �ʏ탂�[�h�ɖ߂�
		::ShowWindow(this->splitterWnd.GetHwnd(), SW_SHOW);
		::ShowWindow(hwndToolBar, SW_SHOW);
		::ShowWindow(statusBar.GetStatusHwnd(), SW_SHOW);
		::ShowWindow(funcKeyWnd.GetHwnd(), SW_SHOW);
		::ShowWindow(tabWnd.GetHwnd(), SW_SHOW);
		::ShowWindow(dlgFuncList.GetHwnd(), SW_SHOW);

		// ���̑��̃��[�h���X�_�C�A���O���߂�
		::ShowWindow(dlgFind.GetHwnd(), SW_SHOW);
		::ShowWindow(dlgReplace.GetHwnd(), SW_SHOW);
		::ShowWindow(dlgGrep.GetHwnd(), SW_SHOW);

		::SetFocus(GetHwnd());

		LayoutMainMenu();

		::InvalidateRect(GetHwnd(), NULL, TRUE);
	}else {
		// �ʏ탂�[�h���B��
		HMENU hMenu = ::GetMenu(GetHwnd());
		// Print Preview�ł̓��j���[���폜
		::SetMenu(GetHwnd(), NULL);
		::DestroyMenu(hMenu);
		::DrawMenuBar(GetHwnd());

		::ShowWindow(this->splitterWnd.GetHwnd(), SW_HIDE);
		::ShowWindow(hwndToolBar, SW_HIDE);
		::ShowWindow(statusBar.GetStatusHwnd(), SW_HIDE);
		::ShowWindow(funcKeyWnd.GetHwnd(), SW_HIDE);
		::ShowWindow(tabWnd.GetHwnd(), SW_HIDE);
		::ShowWindow(dlgFuncList.GetHwnd(), SW_HIDE);

		// ���̑��̃��[�h���X�_�C�A���O���B��
		::ShowWindow(dlgFind.GetHwnd(), SW_HIDE);
		::ShowWindow(dlgReplace.GetHwnd(), SW_HIDE);
		::ShowWindow(dlgGrep.GetHwnd(), SW_HIDE);

		pPrintPreview = new PrintPreview(*this);
		// ���݂̈���ݒ�
		pPrintPreview->SetPrintSetting(
			&pShareData->printSettingArr[
				GetDocument().docType.GetDocumentAttribute().nCurrentPrintSetting]
		);

		// �v�����^�̏����擾�B

		// ���݂̃f�t�H���g�v�����^�̏����擾
		BOOL bRes = pPrintPreview->GetDefaultPrinterInfo();
		if (!bRes) {
			TopInfoMessage(GetHwnd(), LS(STR_ERR_DLGEDITWND14));
			return;
		}

		// ����ݒ�̔��f
		pPrintPreview->OnChangePrintSetting();
		::InvalidateRect(GetHwnd(), NULL, TRUE);
		::UpdateWindow(GetHwnd() /* pPrintPreview->GetPrintPreviewBarHANDLE() */);

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

	int cx = LOWORD(lParam);
	int cy = HIWORD(lParam);
	auto& csWindow = pShareData->common.window;
	
	// �E�B���h�E�T�C�Y�p��
	if (wParam != SIZE_MINIMIZED) {						// �ŏ����͌p�����Ȃ�
		if (csWindow.eSaveWindowSize == WinSizeMode::Save) {		// �E�B���h�E�T�C�Y�p�������邩
			if (wParam == SIZE_MAXIMIZED) {					// �ő剻�̓T�C�Y���L�^���Ȃ�
				if (csWindow.nWinSizeType != (int)wParam) {
					csWindow.nWinSizeType = (int)wParam;
				}
			}else {
				// Aero Snap�̏c�����ő剻��ԂŏI�����Ď���N������Ƃ��͌��̃T�C�Y�ɂ���K�v������̂ŁA
				// GetWindowRect()�ł͂Ȃ�GetWindowPlacement()�œ������[�N�G���A���W���X�N���[�����W�ɕϊ����ċL������
				WINDOWPLACEMENT wp;
				wp.length = sizeof(wp);
				::GetWindowPlacement(GetHwnd(), &wp);	// ���[�N�G���A���W
				RECT rcWin = wp.rcNormalPosition;
				RECT rcWork, rcMon;
				GetMonitorWorkRect(GetHwnd(), &rcWork, &rcMon);
				::OffsetRect(&rcWin, rcWork.left - rcMon.left, rcWork.top - rcMon.top);	// �X�N���[�����W�ɕϊ�
				// �E�B���h�E�T�C�Y�Ɋւ���f�[�^���ύX���ꂽ��
				if (csWindow.nWinSizeType != (int)wParam ||
					csWindow.nWinSizeCX != rcWin.right - rcWin.left ||
					csWindow.nWinSizeCY != rcWin.bottom - rcWin.top
				) {
					csWindow.nWinSizeType = (int)wParam;
					csWindow.nWinSizeCX = rcWin.right - rcWin.left;
					csWindow.nWinSizeCY = rcWin.bottom - rcWin.top;
				}
			}
		}

		// ���ɖ߂��Ƃ��̃T�C�Y��ʂ��L��
		EditNode* p = AppNodeManager::getInstance().GetEditNode(GetHwnd());
		if (p) {
			p->showCmdRestore = ::IsZoomed(p->GetHwnd())? SW_SHOWMAXIMIZED: SW_SHOWNORMAL;
		}
	}

	nWinSizeType = (int)wParam;	// �T�C�Y�ύX�̃^�C�v

	// Rebar ������΂�����c�[���o�[��������
	HWND hwndToolBar = toolbar.GetRebarHwnd() ? toolbar.GetRebarHwnd(): toolbar.GetToolbarHwnd();
	int nToolBarHeight = 0;
	if (hwndToolBar) {
		::SendMessage(hwndToolBar, WM_SIZE, wParam, lParam);
		::GetWindowRect(hwndToolBar, &rc);
		nToolBarHeight = rc.bottom - rc.top;
	}
	int nFuncKeyWndHeight = 0;
	if (funcKeyWnd.GetHwnd()) {
		::SendMessage(funcKeyWnd.GetHwnd(), WM_SIZE, wParam, lParam);
		::GetWindowRect(funcKeyWnd.GetHwnd(), &rc);
		nFuncKeyWndHeight = rc.bottom - rc.top;
	}
	bool bMiniMapSizeBox = true;
	if (wParam == SIZE_MAXIMIZED) {
		bMiniMapSizeBox = false;
	}
	int nStatusBarHeight = 0;
	if (statusBar.GetStatusHwnd()) {
		::SendMessage(statusBar.GetStatusHwnd(), WM_SIZE, wParam, lParam);
		::GetClientRect(statusBar.GetStatusHwnd(), &rc);
		int		nStArr[8];
		// ��pszLabel[3]: �X�e�[�^�X�o�[�����R�[�h�\���̈�͑傫�߂ɂƂ��Ă���
		const TCHAR*	pszLabel[7] = { _T(""), _T("99999 �s 9999 ��"), _T("CRLF"), _T("AAAAAAAAAAAA"), _T("Unicode BOM�t"), _T("REC"), _T("�㏑") };	// Oct. 30, 2000 JEPRO �疜�s���v���	�����R�[�h�g���L���� 2008/6/21	Uchi
		int		nStArrNum = 7;
		int		nAllWidth = rc.right - rc.left;
		int		nSbxWidth = ::GetSystemMetrics(SM_CXVSCROLL) + ::GetSystemMetrics(SM_CXEDGE); // �T�C�Y�{�b�N�X�̕�
		int		nBdrWidth = ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXEDGE) * 2; // ���E�̕�
		SIZE	sz;
		// ���m�ȕ����v�Z���邽�߂ɁA�\���t�H���g���擾����hdc�ɑI��������B
		HDC hdc = ::GetDC(statusBar.GetStatusHwnd());
		HFONT hFont = (HFONT)::SendMessage(statusBar.GetStatusHwnd(), WM_GETFONT, 0, 0);
		if (hFont) {
			hFont = (HFONT)::SelectObject(hdc, hFont);
		}
		nStArr[nStArrNum - 1] = nAllWidth;
		if (wParam != SIZE_MAXIMIZED) {
			nStArr[nStArrNum - 1] -= nSbxWidth;
		}
		for (int i=nStArrNum-1; i>0; --i) {
			::GetTextExtentPoint32(hdc, pszLabel[i], _tcslen(pszLabel[i]), &sz);
			nStArr[i - 1] = nStArr[i] - (sz.cx + nBdrWidth);
		}

		//	������Ԃł͂��ׂĂ̕������u�g����v�����C���b�Z�[�W�G���A�͘g��`�悵�Ȃ��悤�ɂ��Ă���
		//	���߁C���������̘g���ςȕ��Ɏc���Ă��܂��D������ԂŘg��`�悳���Ȃ����邽�߁C
		//	�ŏ��Ɂu�g�����v��Ԃ�ݒ肵����Ńo�[�̕������s���D
		if (bUpdateStatus) {
			statusBar.SetStatusText(0, SBT_NOBORDERS, _T(""));
		}

		StatusBar_SetParts(statusBar.GetStatusHwnd(), nStArrNum, nStArr);
		if (hFont) {
			::SelectObject(hdc, hFont);
		}
		::ReleaseDC(statusBar.GetStatusHwnd(), hdc);

		::UpdateWindow(statusBar.GetStatusHwnd());	// �����`��ł���������炷
		::GetWindowRect(statusBar.GetStatusHwnd(), &rc);
		nStatusBarHeight = rc.bottom - rc.top;
		bMiniMapSizeBox = false;
	}
	RECT rcClient;
	::GetClientRect(GetHwnd(), &rcClient);

	// �^�u�E�B���h�E
	int nTabHeightBottom = 0;
	int nTabWndHeight = 0;
	if (tabWnd.GetHwnd()) {
		// �^�u���i��SizeBox/�E�B���h�E���ō������ς��\��������
		TabPosition tabPosition = pShareData->common.tabBar.eTabPosition;
		bool bHidden = false;
		if (tabPosition == TabPosition::Top) {
			// �ォ�牺�Ɉړ�����ƃS�~���\�������̂ň�x��\���ɂ���
			if (tabWnd.eTabPosition != TabPosition::None && tabWnd.eTabPosition != TabPosition::Top) {
				bHidden = true;
				::ShowWindow( tabWnd.GetHwnd(), SW_HIDE );
			}
			tabWnd.SizeBox_ONOFF( false );
			::GetWindowRect( tabWnd.GetHwnd(), &rc );
			nTabWndHeight = rc.bottom - rc.top;
			if (csWindow.nFuncKeyWnd_Place == 0) {
				::MoveWindow(tabWnd.GetHwnd(), 0, nToolBarHeight + nFuncKeyWndHeight, cx, nTabWndHeight, TRUE);
			}else {
				::MoveWindow(tabWnd.GetHwnd(), 0, nToolBarHeight, cx, nTabWndHeight, TRUE);
			}
			tabWnd.OnSize();
			::GetWindowRect( tabWnd.GetHwnd(), &rc );
			if (nTabWndHeight != rc.bottom - rc.top) {
				nTabWndHeight = rc.bottom - rc.top;
				if (csWindow.nFuncKeyWnd_Place == 0) {
					::MoveWindow( tabWnd.GetHwnd(), 0, nToolBarHeight + nFuncKeyWndHeight, cx, nTabWndHeight, TRUE );
				}else {
					::MoveWindow( tabWnd.GetHwnd(), 0, nToolBarHeight, cx, nTabWndHeight, TRUE );
				}
			}
		}else if (tabPosition == TabPosition::Bottom) {
			// �ォ�牺�Ɉړ�����ƃS�~���\�������̂ň�x��\���ɂ���
			if (tabWnd.eTabPosition != TabPosition::None && tabWnd.eTabPosition != TabPosition::Bottom) {
				bHidden = true;
				ShowWindow( tabWnd.GetHwnd(), SW_HIDE );
			}
			bool bSizeBox = true;
			if (statusBar.GetStatusHwnd()) {
				bSizeBox = false;
			}
			if (funcKeyWnd.GetHwnd()) {
				if (csWindow.nFuncKeyWnd_Place == 1 ){
					bSizeBox = false;
				}
			}
			if (wParam == SIZE_MAXIMIZED) {
				bSizeBox = false;
			}
			tabWnd.SizeBox_ONOFF( bSizeBox );
			::GetWindowRect( tabWnd.GetHwnd(), &rc );
			nTabWndHeight = rc.bottom - rc.top;
			::MoveWindow( tabWnd.GetHwnd(), 0,
				cy - nFuncKeyWndHeight - nStatusBarHeight - nTabWndHeight, cx, nTabWndHeight, TRUE );
			tabWnd.OnSize();
			::GetWindowRect( tabWnd.GetHwnd(), &rc );
			if (nTabWndHeight != rc.bottom - rc.top) {
				nTabWndHeight = rc.bottom - rc.top;
				::MoveWindow( tabWnd.GetHwnd(), 0,
					cy - nFuncKeyWndHeight - nStatusBarHeight - nTabWndHeight, cx, nTabWndHeight, TRUE );
			}
			nTabHeightBottom = rc.bottom - rc.top;
			nTabWndHeight = 0;
			bMiniMapSizeBox = false;
		}
		if (bHidden) {
			::ShowWindow( tabWnd.GetHwnd(), SW_SHOW );
		}
		tabWnd.eTabPosition = tabPosition;
	}

	if (funcKeyWnd.GetHwnd()) {
		if (csWindow.nFuncKeyWnd_Place == 0) {
			// �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
			::MoveWindow(
				funcKeyWnd.GetHwnd(),
				0,
				nToolBarHeight,
				cx,
				nFuncKeyWndHeight, TRUE);
		}else if (csWindow.nFuncKeyWnd_Place == 1) {
			// �t�@���N�V�����L�[�\���ʒu�^0:�� 1:��
			::MoveWindow(
				funcKeyWnd.GetHwnd(),
				0,
				cy - nFuncKeyWndHeight - nStatusBarHeight,
				cx,
				nFuncKeyWndHeight, TRUE
			);

			bool bSizeBox = true;
			if (statusBar.GetStatusHwnd()) {
				bSizeBox = false;
			}
			if (wParam == SIZE_MAXIMIZED) {
				bSizeBox = false;
			}
			funcKeyWnd.SizeBox_ONOFF(bSizeBox);
			bMiniMapSizeBox = false;
		}
		::UpdateWindow(funcKeyWnd.GetHwnd());	// �����`��ł���������炷
	}

	int nFuncListWidth = 0;
	int nFuncListHeight = 0;
	if (dlgFuncList.GetHwnd() && dlgFuncList.IsDocking()) {
		::SendMessage(dlgFuncList.GetHwnd(), WM_SIZE, wParam, lParam);
		::GetWindowRect(dlgFuncList.GetHwnd(), &rc);
		nFuncListWidth = rc.right - rc.left;
		nFuncListHeight = rc.bottom - rc.top;
	}

	DockSideType eDockSideFL = dlgFuncList.GetDockSide();
	int nTop = nToolBarHeight + nTabWndHeight;
	if (csWindow.nFuncKeyWnd_Place == 0) {
		nTop += nFuncKeyWndHeight;
	}
	int nHeight = cy - nToolBarHeight - nFuncKeyWndHeight - nTabWndHeight - nTabHeightBottom - nStatusBarHeight;
	if (dlgFuncList.GetHwnd() && dlgFuncList.IsDocking()) {
		::MoveWindow(
			dlgFuncList.GetHwnd(),
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
		nMiniMapWidth = GetDllShareData().common.window.nMiniMapWidth;
		::MoveWindow( pEditViewMiniMap->GetHwnd(), 
			(eDockSideFL == DockSideType::Right)? cx - nFuncListWidth - nMiniMapWidth: cx - nMiniMapWidth,
			(eDockSideFL == DockSideType::Top)? nTop + nFuncListHeight: nTop,
			nMiniMapWidth,
			(eDockSideFL == DockSideType::Top || eDockSideFL == DockSideType::Bottom)? nHeight - nFuncListHeight: nHeight,
			TRUE
		);
		GetMiniMap().SplitBoxOnOff(false, false, bMiniMapSizeBox);
	}

	::MoveWindow(
		splitterWnd.GetHwnd(),
		(eDockSideFL == DockSideType::Left)? nFuncListWidth: 0,
		(eDockSideFL == DockSideType::Top)? nTop + nFuncListHeight: nTop,
		((eDockSideFL == DockSideType::Left || eDockSideFL == DockSideType::Right)? cx - nFuncListWidth: cx) - nMiniMapWidth,
		(eDockSideFL == DockSideType::Top || eDockSideFL == DockSideType::Bottom)? nHeight - nFuncListHeight: nHeight,
		TRUE
	);

	// ���Preview���[�h��
	if (!pPrintPreview) {
		return 0L;
	}
	return pPrintPreview->OnSize(wParam, lParam);
}


// WM_PAINT �`�揈��
LRESULT EditWnd::OnPaint(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	// ���Preview���[�h��
	if (!pPrintPreview) {
		PAINTSTRUCT ps;
		::BeginPaint(hwnd, &ps);
		::EndPaint(hwnd, &ps);
		return 0L;
	}
	return pPrintPreview->OnPaint(hwnd, uMsg, wParam, lParam);
}

// ���Preview �����X�N���[���o�[���b�Z�[�W���� WM_VSCROLL
LRESULT EditWnd::OnVScroll(WPARAM wParam, LPARAM lParam)
{
	// ���Preview���[�h��
	if (!pPrintPreview) {
		return 0;
	}
	return pPrintPreview->OnVScroll(wParam, lParam);
}

// ���Preview �����X�N���[���o�[���b�Z�[�W����
LRESULT EditWnd::OnHScroll(WPARAM wParam, LPARAM lParam)
{
	// ���Preview���[�h��
	if (!pPrintPreview) {
		return 0;
	}
	return pPrintPreview->OnHScroll(wParam, lParam);
}

LRESULT EditWnd::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	// �L���v�`���[���ĉ����ꂽ���N���C�A���g�ł��������ɗ���
	if (iconClicked != IconClickStatus::None) {
		return 0;
	}
	ptDragPosOrg.x = LOWORD(lParam);	// horizontal position of cursor
	ptDragPosOrg.y = HIWORD(lParam);	// vertical position of cursor
	bDragMode = true;
	SetCapture(GetHwnd());

	return 0;
}

LRESULT EditWnd::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (iconClicked != IconClickStatus::None) {
		if (iconClicked == IconClickStatus::Down) {
			iconClicked = IconClickStatus::Clicked;
			SetTimer(GetHwnd(), IDT_SYSMENU, GetDoubleClickTime(), NULL);
		}
		return 0;
	}

	bDragMode = false;
//	MYTRACE(_T("bDragMode = FALSE (OnLButtonUp)\n"));
	ReleaseCapture();
	::InvalidateRect(GetHwnd(), NULL, TRUE);
	return 0;
}


/*!	WM_MOUSEMOVE���� */
LRESULT EditWnd::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	if (iconClicked != IconClickStatus::None) {
		// ��񉟂��ꂽ������
		if (iconClicked == IconClickStatus::Down) {
			POINT pt;
			GetCursorPos(&pt); // �X�N���[�����W
			if (SendMessage(GetHwnd(), WM_NCHITTEST, 0, pt.x | (pt.y << 16)) != HTSYSMENU) {
				ReleaseCapture();
				iconClicked = IconClickStatus::None;

				if (GetDocument().docFile.GetFilePathClass().IsValidPath()) {
					NativeW memTitle;
					NativeW memDir;
					memTitle = to_wchar(GetDocument().docFile.GetFileName());
					memDir   = to_wchar(GetDocument().docFile.GetFilePathClass().GetDirPath().c_str());

					IDataObject *DataObject;
					IMalloc *Malloc;
					IShellFolder *Desktop, *Folder;
					LPITEMIDLIST PathID, ItemID;
					SHGetMalloc(&Malloc);
					SHGetDesktopFolder(&Desktop);
					DWORD Eaten, Attribs;
					if (SUCCEEDED(Desktop->ParseDisplayName(0, NULL, memDir.GetStringPtr(), &Eaten, &PathID, &Attribs))) {
						Desktop->BindToObject(PathID, NULL, IID_IShellFolder, (void**)&Folder);
						Malloc->Free(PathID);
						if (SUCCEEDED(Folder->ParseDisplayName(0, NULL, memTitle.GetStringPtr(), &Eaten, &ItemID, &Attribs))) {
							LPCITEMIDLIST List[1];
							List[0] = ItemID;
							Folder->GetUIObjectOf(0, 1, List, IID_IDataObject, NULL, (void**)&DataObject);
							Malloc->Free(ItemID);
#define DDASTEXT
#ifdef  DDASTEXT
							// �e�L�X�g�ł���������c�֗�
							{
								FORMATETC fmt;
								fmt.cfFormat = CF_UNICODETEXT;
								fmt.ptd      = NULL;
								fmt.dwAspect = DVASPECT_CONTENT;
								fmt.lindex   = -1;
								fmt.tymed    = TYMED_HGLOBAL;

								STGMEDIUM medium;
								const wchar_t* pFilePath = to_wchar(GetDocument().docFile.GetFilePath());
								size_t Len = wcslen(pFilePath);
								medium.tymed          = TYMED_HGLOBAL;
								medium.pUnkForRelease = NULL;
								medium.hGlobal        = GlobalAlloc(GMEM_MOVEABLE, (Len + 1) * sizeof(wchar_t));
								void* p = GlobalLock(medium.hGlobal);
								CopyMemory(p, pFilePath, (Len + 1) * sizeof(wchar_t));
								GlobalUnlock(medium.hGlobal);

								DataObject->SetData(&fmt, &medium, TRUE);
							}
#endif
							// �ړ��͋֎~
							DWORD r;
							DropSource drop(TRUE);
							DoDragDrop(DataObject, &drop, DROPEFFECT_COPY | DROPEFFECT_LINK, &r);
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

//@@@ 2002.01.14 YAZAKI ���Preview��PrintPreview�ɓƗ����������Ƃɂ��ύX
	if (!pPrintPreview) {
		return 0;
	}else {
		return pPrintPreview->OnMouseMove(wParam, lParam);
	}
}


LRESULT EditWnd::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
	if (pPrintPreview) {
		return pPrintPreview->OnMouseWheel(wParam, lParam);
	}
	return Views_DispatchEvent(GetHwnd(), WM_MOUSEWHEEL, wParam, lParam);
}

/** �}�E�X�z�C�[������ */
bool EditWnd::DoMouseWheel(WPARAM wParam, LPARAM lParam)
{
	// ���Preview���[�h��
	if (!pPrintPreview) {
		// �^�u��Ȃ�E�B���h�E�؂�ւ�
		if (pShareData->common.tabBar.bChgWndByWheel && tabWnd.hwndTab) {
			POINT pt;
			pt.x = (short)LOWORD(lParam);
			pt.y = (short)HIWORD(lParam);
			int nDelta = (short)HIWORD(wParam);
			HWND hwnd = ::WindowFromPoint(pt);
			if ((hwnd == tabWnd.hwndTab || hwnd == tabWnd.GetHwnd())) {
				// ���݊J���Ă���ҏW���̃��X�g�𓾂�
				EditNode* pEditNodeArr;
				size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
				if (nRowNum > 0) {
					// �����̃E�B���h�E�𒲂ׂ�
					int i, j;
					int nGroup = 0;
					for (i=0; i<nRowNum; ++i) {
						if (GetHwnd() == pEditNodeArr[i].GetHwnd()) {
							nGroup = pEditNodeArr[i].nGroup;
							break;
						}
					}
					if (i < nRowNum) {
						if (nDelta < 0) {
							// ���̃E�B���h�E
							for (j=i+1; j<nRowNum; ++j) {
								if (nGroup == pEditNodeArr[j].nGroup)
									break;
							}
							if (j >= nRowNum) {
								for (j=0; j<i; ++j) {
									if (nGroup == pEditNodeArr[j].nGroup)
										break;
								}
							}
						}else {
							// �O�̃E�B���h�E
							for (j=i-1; j>=0; --j) {
								if (nGroup == pEditNodeArr[j].nGroup)
									break;
							}
							if (j < 0) {
								for (j=(int)nRowNum-1; j>i; --j) {
									if (nGroup == pEditNodeArr[j].nGroup)
										break;
								}
							}
						}

						// ���́ior �O�́j�E�B���h�E���A�N�e�B�u�ɂ���
						if (i != j) {
							ActivateFrameWindow(pEditNodeArr[j].GetHwnd());
						}
					}
					delete[] pEditNodeArr;
				}
				return true;	// ��������
			}
		}
		return false;	// �������Ȃ�����
	}
	return false;	// �������Ȃ�����
}

/* ����y�[�W�ݒ�
	���Preview���ɂ��A�����łȂ��Ƃ��ł��Ă΂��\��������B
*/
bool EditWnd::OnPrintPageSetting(void)
{
	// ����ݒ�iCANCEL�������Ƃ��ɔj�����邽�߂̗̈�j

	auto& docType = GetDocument().docType;
	int nCurrentPrintSetting = docType.GetDocumentAttribute().nCurrentPrintSetting;
	int nLineNumberColumns;
	if (pPrintPreview) {
		nLineNumberColumns = GetActiveView().GetTextArea().DetectWidthOfLineNumberArea_calculate(pPrintPreview->pLayoutMgr_Print); // ���Preview���͕����̌���
	}else {
		nLineNumberColumns = 3; // �t�@�C�����j���[����̐ݒ莞�͍ŏ��l
	}

	DlgPrintSetting	dlgPrintSetting;
	bool bRes = dlgPrintSetting.DoModal(
		G_AppInstance(),
//@@@ 2002.01.14 YAZAKI ���Preview��PrintPreview�ɓƗ����������Ƃɂ��ύX
		GetHwnd(),
		&nCurrentPrintSetting, // ���ݑI�����Ă������ݒ�
		pShareData->printSettingArr, // ���݂̐ݒ�̓_�C�A���O���ŕێ�����
		nLineNumberColumns // �s�ԍ��\���p�Ɍ�����n��
	) > 0;

	if (bRes) {
		bool bChangePrintSettingNo = false;
		// ���ݑI������Ă���y�[�W�ݒ�̔ԍ����ύX���ꂽ��
		if (docType.GetDocumentAttribute().nCurrentPrintSetting != nCurrentPrintSetting) {
			// �ύX�t���O(�^�C�v�ʐݒ�)
			TypeConfig* type = new TypeConfig();
			DocTypeManager().GetTypeConfig(docType.GetDocumentType(), *type);
			type->nCurrentPrintSetting = nCurrentPrintSetting;
			DocTypeManager().SetTypeConfig(docType.GetDocumentType(), *type);
			delete type;
			docType.GetDocumentAttributeWrite().nCurrentPrintSetting = nCurrentPrintSetting; // ���̐ݒ�ɂ����f
			AppNodeGroupHandle(0).SendMessageToAllEditors(
				MYWM_CHANGESETTING,
				(WPARAM)docType.GetDocumentType().GetIndex(),
				(LPARAM)PM_CHANGESETTING_TYPE,
				EditWnd::getInstance().GetHwnd()
			);
			bChangePrintSettingNo = true;
		}

//@@@ 2002.01.14 YAZAKI ���Preview��PrintPreview�ɓƗ����������Ƃɂ��ύX
		//	���Preview���̂݁B
		if (pPrintPreview) {
			// ���݂̈���ݒ�
			// 2013.08.27 ����ݒ�ԍ����ύX���ꂽ���ɑΉ��ł��Ă��Ȃ�����
			if (bChangePrintSettingNo) {
				pPrintPreview->SetPrintSetting(&pShareData->printSettingArr[docType.GetDocumentAttribute().nCurrentPrintSetting]);
			}

			// ���Preview �X�N���[���o�[������
			//pPrintPreview->InitPreviewScrollBar();

			// ����ݒ�̔��f
			// pPrintPreview->OnChangePrintSetting();

			//::InvalidateRect(GetHwnd(), NULL, TRUE);
		}
		AppNodeGroupHandle(0).SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0,
			(LPARAM)PM_PrintSetting,
			EditWnd::getInstance().GetHwnd()
		);
	}
//@@@ 2002.01.14 YAZAKI ���Preview��PrintPreview�ɓƗ����������Ƃɂ��ύX
	::UpdateWindow(GetHwnd() /* pPrintPreview->GetPrintPreviewBarHANDLE() */);
	return bRes;
}

///////////////////////////// by �S

LRESULT EditWnd::OnNcLButtonDown(WPARAM wp, LPARAM lp)
{
	LRESULT result;
	if (wp == HTSYSMENU) {
		SetCapture(GetHwnd());
		iconClicked = IconClickStatus::Down;
		result = 0;
	}else {
		result = DefWindowProc(GetHwnd(), WM_NCLBUTTONDOWN, wp, lp);
	}

	return result;
}

LRESULT EditWnd::OnNcLButtonUp(WPARAM wp, LPARAM lp)
{
	LRESULT result;
	if (iconClicked != IconClickStatus::None) {
		// �O�̂���
		ReleaseCapture();
		iconClicked = IconClickStatus::None;
		result = 0;
	}else if (wp == HTSYSMENU) {
		result = 0;
	}else {
		result = DefWindowProc(GetHwnd(), WM_NCLBUTTONUP, wp, lp);
	}

	return result;
}

LRESULT EditWnd::OnLButtonDblClk(WPARAM wp, LPARAM lp) // by �S(2)
{
	LRESULT result;
	if (iconClicked != IconClickStatus::None) {
		ReleaseCapture();
		iconClicked = IconClickStatus::DoubleClicked;

		SendMessage(GetHwnd(), WM_SYSCOMMAND, SC_CLOSE, 0);

		result = 0;
	}else {
		result = DefWindowProc(GetHwnd(), WM_LBUTTONDBLCLK, wp, lp);
	}

	return result;
}

// �h���b�v�_�E�����j���[(�J��)
int	EditWnd::CreateFileDropDownMenu(HWND hwnd)
{
	// ���j���[�\���ʒu�����߂�
	// �� TBN_DROPDOWN ���� NMTOOLBAR::iItem �� NMTOOLBAR::rcButton �ɂ̓h���b�v�_�E�����j���[(�J��)�{�^����
	//    ��������Ƃ��͂ǂ�������������P�ڂ̃{�^����񂪓���悤�Ȃ̂Ń}�E�X�ʒu����{�^���ʒu�����߂�
	POINT po;
	::GetCursorPos(&po);
	::ScreenToClient(hwnd, &po);
	int nIndex = Toolbar_Hittest(hwnd, &po);
	if (nIndex < 0) {
		return 0;
	}
	RECT rc;
	Toolbar_GetItemRect(hwnd, nIndex, &rc);
	po.x = rc.left;
	po.y = rc.bottom;
	::ClientToScreen(hwnd, &po);
	GetMonitorWorkRect(po, &rc);
	if (po.x < rc.left) {
		po.x = rc.left;
	}
	if (po.y < rc.top) {
		po.y = rc.top;
	}

	menuDrawer.ResetContents();

	// MRU���X�g�̃t�@�C���̃��X�g�����j���[�ɂ���
	const MruFile mru;
	HMENU hMenu = mru.CreateMenu(menuDrawer);
	if (mru.MenuLength() > 0) {
		menuDrawer.MyAppendMenuSep(
			hMenu,
			MF_BYPOSITION | MF_SEPARATOR,
			0,
			NULL,
			false
		);
	}

	// �ŋߎg�����t�H���_�̃��j���[���쐬
	const MruFolder mruFolder;
	HMENU hMenuPopUp = mruFolder.CreateMenu(menuDrawer);
	if (mruFolder.MenuLength() > 0) {
		// �A�N�e�B�u
		menuDrawer.MyAppendMenu(
			hMenu,
			MF_BYPOSITION | MF_STRING | MF_POPUP,
			(UINT_PTR)hMenuPopUp,
			LS(F_FOLDER_USED_RECENTLY),
			_T("")
		);
	}else {
		// ��A�N�e�B�u
		menuDrawer.MyAppendMenu(
			hMenu,
			MF_BYPOSITION | MF_STRING | MF_POPUP | MF_GRAYED,
			(UINT_PTR)hMenuPopUp,
			LS(F_FOLDER_USED_RECENTLY),
			_T("")
		);
	}

	menuDrawer.MyAppendMenuSep(hMenu, MF_BYPOSITION | MF_SEPARATOR, 0, NULL, false);

	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW, _T(""), _T("N"), false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILENEW_NEWWINDOW, _T(""), _T("medium"), false);
	menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, F_FILEOPEN, _T(""), _T("O"), false);

	int nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		,
		po.x,
		po.y,
		0,
		GetHwnd(),
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
*/
void EditWnd::InitMenubarMessageFont(void)
{
	TEXTMETRIC	tm;
	HDC			hdc;
	HFONT		hFontOld;

	// LOGFONT�̏�����
	LOGFONT lf = {0};
	lf.lfHeight			= DpiPointsToPixels(-9);	// ��DPI�Ή��i�|�C���g������Z�o�j
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
	hFontCaretPosInfo = ::CreateFontIndirect(&lf);

	hdc = ::GetDC(::GetDesktopWindow());
	hFontOld = (HFONT)::SelectObject(hdc, hFontCaretPosInfo);
	::GetTextMetrics(hdc, &tm);
	nCaretPosInfoCharWidth = tm.tmAveCharWidth;
	nCaretPosInfoCharHeight = tm.tmHeight;
	::SelectObject(hdc, hFontOld);
	::ReleaseDC(::GetDesktopWindow(), hdc);
}

/*
	@brief ���j���[�o�[�Ƀ��b�Z�[�W��\������
	
	���O�Ƀ��j���[�o�[�\���p�t�H���g������������Ă��Ȃ��Ă͂Ȃ�Ȃ��D
	�w��ł��镶�����͍ő�30�o�C�g�D����ȏ�̏ꍇ�͂����؂��ĕ\������D
*/
void EditWnd::PrintMenubarMessage(const TCHAR* msg)
{
	if (!::GetMenu(GetHwnd()))
		return;

	POINT	po, poFrame;
	RECT	rc, rcFrame;
	HFONT	hFontOld;
	int		nStrLen;

	// msg == NULL �̂Ƃ��͈ȑO�� pszMenubarMessage �ōĕ`��
	if (msg) {
		size_t len = _tcslen(msg);
		_tcsncpy(pszMenubarMessage, msg, MENUBAR_MESSAGE_MAX_LEN);
		if (len < MENUBAR_MESSAGE_MAX_LEN) {
			auto_memset(pszMenubarMessage + len, _T(' '), MENUBAR_MESSAGE_MAX_LEN - len);	//  null�I�[�͕s�v
		}
	}

	HDC hdc = ::GetWindowDC(GetHwnd());
	poFrame.x = 0;
	poFrame.y = 0;
	::ClientToScreen(GetHwnd(), &poFrame);
	::GetWindowRect(GetHwnd(), &rcFrame);
	po.x = rcFrame.right - rcFrame.left;
	po.y = poFrame.y - rcFrame.top;
	hFontOld = (HFONT)::SelectObject(hdc, hFontCaretPosInfo);
	nStrLen = MENUBAR_MESSAGE_MAX_LEN;
	rc.left = po.x - nStrLen * nCaretPosInfoCharWidth - (::GetSystemMetrics(SM_CXSIZEFRAME) + 2);
	rc.right = rc.left + nStrLen * nCaretPosInfoCharWidth + 2;
	rc.top = po.y - nCaretPosInfoCharHeight - 2;
	rc.bottom = rc.top + nCaretPosInfoCharHeight;
	::SetTextColor(hdc, ::GetSysColor(COLOR_MENUTEXT));
	//	Sep. 6, 2003 genta Windows XP(Luna)�̏ꍇ�ɂ�COLOR_MENUBAR���g��Ȃ��Ă͂Ȃ�Ȃ�
	COLORREF bkColor =
		::GetSysColor(IsWinXP_or_later() ? COLOR_MENUBAR : COLOR_MENU);
	::SetBkColor(hdc, bkColor);
	/*
	int pnCaretPosInfoDx[64];	// ������`��p�������z��
	for (i=0; i<_countof(pnCaretPosInfoDx); ++i) {
		pnCaretPosInfoDx[i] = (nCaretPosInfoCharWidth);
	}
	*/
	::ExtTextOut(hdc, rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE,&rc,pszMenubarMessage,nStrLen,NULL/*pnCaretPosInfoDx*/);
	::SelectObject(hdc, hFontOld);
	::ReleaseDC(GetHwnd(), hdc);
}

/*!
	@brief ���b�Z�[�W�̕\��
	
	�w�肳�ꂽ���b�Z�[�W���X�e�[�^�X�o�[�ɕ\������D
	�X�e�[�^�X�o�[����\���̏ꍇ�̓��j���[�o�[�̉E�[�ɕ\������D
	
	@param msg [in] �\�����郁�b�Z�[�W
*/
void EditWnd::SendStatusMessage(const TCHAR* msg)
{
	if (!statusBar.GetStatusHwnd()) {
		// ���j���[�o�[��
		PrintMenubarMessage(msg);
	}else {
		// �X�e�[�^�X�o�[��
		statusBar.SetStatusText(0, SBT_NOBORDERS, msg);
	}
}

/*! �t�@�C�����ύX�ʒm */
void EditWnd::ChangeFileNameNotify(const TCHAR* pszTabCaption, const TCHAR* _pszFilePath, bool bIsGrep)
{
	const TCHAR* pszFilePath = _pszFilePath;
	
	EditNode* p;
	int nIndex;
	
	if (!pszTabCaption) pszTabCaption = _T("");	// �K�[�h
	if (!pszFilePath) pszFilePath = _FT("");
	
	RecentEditNode	recentEditNode;
	nIndex = recentEditNode.FindItemByHwnd(GetHwnd());
	if (nIndex != -1) {
		p = recentEditNode.GetItem(nIndex);
		if (p) {
			int	size = _countof(p->szTabCaption) - 1;
			_tcsncpy(p->szTabCaption, pszTabCaption, size);
			p->szTabCaption[size] = _T('\0');

			size = _countof2(p->szFilePath) - 1;
			_tcsncpy(p->szFilePath, pszFilePath, size);
			p->szFilePath[size] = _T('\0');

			p->bIsGrep = bIsGrep;
		}
	}
	recentEditNode.Terminate();

	// �t�@�C�����ύX�ʒm���u���[�h�L���X�g����B
	int nGroup = AppNodeManager::getInstance().GetEditNode(GetHwnd())->GetGroup();
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

	// �^�u�܂Ƃߎ��� WS_EX_TOPMOST ��Ԃ�S�E�B���h�E�œ�������
	auto& csTabBar = pShareData->common.tabBar;
	if (pShareData->common.tabBar.bDispTabWnd && !pShareData->common.tabBar.bDispTabWndMultiWin) {
		hwndInsertAfter = GetHwnd();
		for (size_t i=0; i<pShareData->nodes.nEditArrNum; ++i) {
			HWND hwnd = pShareData->nodes.pEditArr[i].GetHwnd();
			if (hwnd != GetHwnd() && IsSakuraMainWindow(hwnd)) {
				if (!AppNodeManager::IsSameGroup(GetHwnd(), hwnd)) {
					continue;
				}
				::SetWindowPos(hwnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
				hwndInsertAfter = hwnd;
			}
		}
	}
}


// �^�C�}�[�̍X�V���J�n�^��~����B
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
		if (pt.x < rc.left) {
			pt.x = rc.left;
		}
		pt.y = rc.top;
	}

	// �E�B���h�E�ꗗ���j���[���|�b�v�A�b�v�\������
	if (tabWnd.GetHwnd()) {
		tabWnd.TabListMenu(pt);
	}else {
		menuDrawer.ResetContents();
		EditNode* pEditNodeArr;
		HMENU hMenu = ::CreatePopupMenu();
		size_t nRowNum = AppNodeManager::getInstance().GetOpenedWindowArr(&pEditNodeArr, true);
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

/*! @brief ���݊J���Ă���ҏW���̃��X�g�����j���[�ɂ��� */
LRESULT EditWnd::WinListMenu(HMENU hMenu, EditNode* pEditNodeArr, size_t nRowNum, bool bFull)
{
	if (nRowNum > 0) {
		TCHAR szMenu[_MAX_PATH * 2 + 3];
		FileNameManager::getInstance().TransformFileName_MakeCache();

		NONCLIENTMETRICS met;
		met.cbSize = CCSIZEOF_STRUCT(NONCLIENTMETRICS, lfMessageFont);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, met.cbSize, &met, 0);
		DCFont dcFont(met.lfMenuFont, GetHwnd());
		for (size_t i=0; i<nRowNum; ++i) {
			// �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm
			::SendMessage(pEditNodeArr[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0);
////	From Here Oct. 4, 2000 JEPRO commented out & modified	�J���Ă���t�@�C�������킩��悤�ɗ����Ƃ͈����1���琔����
			const EditInfo*	pfi = (EditInfo*)&pShareData->workBuffer.editInfo_MYWM_GETFILEINFO;
			FileNameManager::getInstance().GetMenuFullLabel_WinList(szMenu, _countof(szMenu), pfi, pEditNodeArr[i].nId, i, dcFont.GetHDC());
			menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING, IDM_SELWINDOW + pEditNodeArr[i].nIndex, szMenu, _T(""));
			if (GetHwnd() == pEditNodeArr[i].GetHwnd()) {
				::CheckMenuItem(hMenu, UINT(IDM_SELWINDOW + pEditNodeArr[i].nIndex), MF_BYCOMMAND | MF_CHECKED);
			}
		}
	}
	return 0L;
}

// �c�[���`�b�v�̃e�L�X�g���擾
void EditWnd::GetTooltipText(TCHAR* wszBuf, size_t nBufCount, int nID) const
{
	// �@�\������̎擾 -> tmp -> wszBuf
	wchar_t tmp[256];
	size_t nLen;
	GetDocument().funcLookup.Funccode2Name(nID, tmp, _countof(tmp));
	nLen = _wcstotcs(wszBuf, tmp, nBufCount);

	// �@�\�ɑΉ�����L�[���̎擾(����)
	auto& csKeyBind = pShareData->common.keyBind;
	NativeT** ppcAssignedKeyList;
	int nAssignedKeyNum = KeyBind::GetKeyStrList(
		G_AppInstance(),
		csKeyBind.nKeyNameArrNum,
		csKeyBind.pKeyNameArr,
		&ppcAssignedKeyList,
		nID
	);

	// wszBuf�֌���
	if (0 < nAssignedKeyNum) {
		for (int j=0; j<nAssignedKeyNum; ++j) {
			const TCHAR* pszKey = ppcAssignedKeyList[j]->GetStringPtr();
			size_t nKeyLen = _tcslen(pszKey);
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


/*! �^�C�}�[�̏��� */
void EditWnd::OnEditTimer(void)
{
	IncrementTimerCount(6);

	if (nTimerCount == 0 && !GetCapture()) { 
		// �t�@�C���̃^�C���X�^���v�̃`�F�b�N����
		GetDocument().autoReloadAgent.CheckFileTimeStamp();

#if 0	// �����֎~�̊Ď���p�~�i����������Ȃ�u�X�V�̊Ď��v�t���ł͂Ȃ��ʃI�v�V�����ɂ��Ăق����j
		// �t�@�C�������\�̃`�F�b�N����
		if (GetDocument()->autoReloadAgent._ToDoChecking()) {
			bool bOld = GetDocument()->docLocker.IsDocWritable();
			GetDocument()->docLocker.CheckWritable(false);
			if (bOld != GetDocument()->docLocker.IsDocWritable()) {
				this->UpdateCaption();
			}
		}
#endif
	}

	GetDocument().autoSaveAgent.CheckAutoSave();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �r���[�Ǘ�                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	EditView�̉�ʃo�b�t�@���폜
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
	if (nEditViewMaxCount < nViewCount) {
		return false;
	}
	if (GetAllViewCount() < nViewCount) {
		for (int i=GetAllViewCount(); i<nViewCount; ++i) {
			assert(!pEditViewArr[i]);
			pEditViewArr[i] = new EditView(*this);
			pEditViewArr[i]->Create(splitterWnd.GetHwnd(), GetDocument(), i, FALSE, false);
		}
		nEditViewCount = nViewCount;

		std::vector<HWND> hWndArr;
		hWndArr.reserve(nViewCount + 1);
		for (int i=0; i<nViewCount; ++i) {
			hWndArr.push_back(GetView(i).GetHwnd());
		}
		hWndArr.push_back(NULL);

		splitterWnd.SetChildWndArr(&hWndArr[0]);
	}
	return true;
}

/*
	�r���[�̍ď�����
*/
void EditWnd::InitAllViews()
{
	// �擪�փJ�[�\�����ړ�
	for (int i=0; i<GetAllViewCount(); ++i) {
		// �ړ������̏���
		auto& view = GetView(i);
		view.pHistory->Flush();

		// ���݂̑I��͈͂��I����Ԃɖ߂�
		view.GetSelectionInfo().DisableSelectArea(false);
		view.OnChangeSetting();
		view.GetCaret().MoveCursor(Point(0, 0), true);
		view.GetCaret().nCaretPosX_Prev = 0;
	}
	GetMiniMap().OnChangeSetting();
}


void EditWnd::Views_RedrawAll()
{
	// �A�N�e�B�u�ȊO���ĕ`�悵�Ă���c
	for (int v=0; v<GetAllViewCount(); ++v) {
		if (nActivePaneIndex != v) {
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
		if (nActivePaneIndex != v) {
			GetView(v).Redraw();
		}
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
	int nOldIndex = nActivePaneIndex;
	nActivePaneIndex = nIndex;
	pEditView = pEditViewArr[nActivePaneIndex];

	// �t�H�[�J�X���ړ�����
	GetView(nOldIndex).GetCaret().underLine.CaretUnderLineOFF(true);	//	2002/05/11 YAZAKI
	if (::GetActiveWindow() == GetHwnd()
		&& ::GetFocus() != GetActiveView().GetHwnd()
	) {
		// ::SetFocus()�Ńt�H�[�J�X��؂�ւ���
		::SetFocus(GetActiveView().GetHwnd());
	}else {
		// �N���Ɠ����ɃG�f�B�b�g�{�b�N�X�Ƀt�H�[�J�X�̂���_�C�A���O��\������Ɠ��Y�G�f�B�b�g�{�b�N�X��
		// �L�����b�g���\������Ȃ����(*1)���C������̂��߁A�����I�Ȑ؂�ւ�������̂̓A�N�e�B�u�y�C����
		// �؂�ւ��Ƃ������ɂ����B�� EditView::OnKillFocus()�͎��X���b�h�̃L�����b�g��j������̂�
		// (*1) -GREPDLG�I�v�V�����ɂ��GREP�_�C�A���O�\����J�t�@�C���㎩�����s�}�N���ł�InputBox�\��
		if (nActivePaneIndex != nOldIndex) {
			// �A�N�e�B�u�łȂ��Ƃ���::SetFocus()����ƃA�N�e�B�u�ɂȂ��Ă��܂�
			// �i�s���Ȃ���ɂȂ�j�̂œ����I�ɐ؂�ւ��邾���ɂ���
			GetView(nOldIndex).OnKillFocus();
			GetActiveView().OnSetFocus();
		}
	}

	GetActiveView().RedrawAll();	// �t�H�[�J�X�ړ����̍ĕ`��

	splitterWnd.SetActivePane(nIndex);

	if (dlgFind.GetHwnd()) {		//�u�����v�_�C�A���O
		// ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX
		dlgFind.ChangeView((LPARAM)&GetActiveView());
	}
	if (dlgReplace.GetHwnd()) {	//�u�u���v�_�C�A���O
		// ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX
		dlgReplace.ChangeView((LPARAM)&GetActiveView());
	}
	if (hokanMgr.GetHwnd()) {	//�u���͕⊮�v�_�C�A���O
		hokanMgr.Hide();
		// ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX
		hokanMgr.ChangeView((LPARAM)&GetActiveView());
	}
	if (dlgFuncList.GetHwnd()) {	//�u�A�E�g���C���v�_�C�A���O	
		// ���[�h���X���F���݈ʒu�\���̑ΏۂƂȂ�r���[�̕ύX
		dlgFuncList.ChangeView((LPARAM)&GetActiveView());
	}

	return;
}

/** ���ׂẴy�C���̕`��X�C�b�`��ݒ肷��

	@param bDraw [in] �`��X�C�b�`�̐ݒ�l
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

	@param pViewExclude [in] Redraw���珜�O����r���[
*/
void EditWnd::RedrawAllViews(EditView* pViewExclude)
{
	for (int i=0; i<GetAllViewCount(); ++i) {
		EditView* pView = &GetView(i);
		if (pView == pViewExclude)
			continue;
		if (i == nActivePaneIndex) {
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
		auto& selInfo = GetView(i).GetSelectionInfo();
		if (selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			selInfo.DisableSelectArea(true);
		}
	}
}


// ���ׂẴy�C���ŁA�s�ԍ��\���ɕK�v�ȕ����Đݒ肷��i�K�v�Ȃ�ĕ`�悷��j
bool EditWnd::DetectWidthOfLineNumberAreaAllPane(bool bRedraw)
{
	if (GetAllViewCount() == 1) {
		return GetActiveView().GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
	}
	// �ȉ�2,4��������

	if (GetActiveView().GetTextArea().DetectWidthOfLineNumberArea(bRedraw)) {
		// ActivePane�Ōv�Z������A�Đݒ�E�ĕ`�悪�K�v�Ɣ�������
		if (splitterWnd.GetAllSplitCols() == 2) {
			GetView(nActivePaneIndex^1).GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
		}else {
			// �\������Ă��Ȃ��̂ōĕ`�悵�Ȃ�
			GetView(nActivePaneIndex^1).GetTextArea().DetectWidthOfLineNumberArea(false);
		}
		if (splitterWnd.GetAllSplitRows() == 2) {
			GetView(nActivePaneIndex^2).GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
			if (splitterWnd.GetAllSplitCols() == 2) {
				GetView((nActivePaneIndex^1)^2).GetTextArea().DetectWidthOfLineNumberArea(bRedraw);
			}
		}else {
			GetView(nActivePaneIndex^2).GetTextArea().DetectWidthOfLineNumberArea(false);
			GetView((nActivePaneIndex^1)^2).GetTextArea().DetectWidthOfLineNumberArea(false);
		}
		return true;
	}
	return false;
}


/** �E�[�Ő܂�Ԃ�
	@param nViewColNum	[in] �E�[�Ő܂�Ԃ��y�C���̔ԍ�
	@retval �܂�Ԃ���ύX�������ǂ���
*/
bool EditWnd::WrapWindowWidth(int nPane)
{
	// �E�[�Ő܂�Ԃ�
	int nWidth = GetView(nPane).ViewColNumToWrapColNum(GetView(nPane).GetTextArea().nViewColNum);
	if (GetDocument().layoutMgr.GetMaxLineKetas() != nWidth) {
		ChangeLayoutParam(false, GetDocument().layoutMgr.GetTabSpace(), nWidth);
		ClearViewCaretPosInfo();
		return true;
	}
	return false;
}

/** �܂�Ԃ����@�֘A�̍X�V
	@retval ��ʍX�V�������ǂ���
*/
bool EditWnd::UpdateTextWrap(void)
{
	// ���̊֐��̓R�}���h���s���Ƃɏ����̍ŏI�i�K�ŗ��p����
	// �iUndo�o�^���S�r���[�X�V�̃^�C�~���O�j
	if (GetDocument().nTextWrapMethodCur == TextWrappingMethod::WindowWidth) {
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
	return false;	// ��ʍX�V���Ȃ�����
}

/*!	���C�A�E�g�p�����[�^�̕ύX

	��̓I�ɂ̓^�u���Ɛ܂�Ԃ��ʒu��ύX����D
	���݂̃h�L�������g�̃��C�A�E�g�݂̂�ύX���C���ʐݒ�͕ύX���Ȃ��D
*/
void EditWnd::ChangeLayoutParam(bool bShowProgress, size_t nTabSize, size_t nMaxLineKetas)
{
	HWND hwndProgress = NULL;
	if (bShowProgress && this) {
		hwndProgress = this->statusBar.GetProgressHwnd();
		// Status Bar���\������Ă��Ȃ��Ƃ���hwndProgressBar == NULL
	}

	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_SHOW);
	}

	// ���W�̕ۑ�
	PointEx* posSave = SavePhysPosOfAllView();

	// ���C�A�E�g�̍X�V
	GetDocument().layoutMgr.ChangeLayoutParam(nTabSize, nMaxLineKetas);
	ClearViewCaretPosInfo();

	// ���W�̕���
	// ���C�A�E�g�ύX�r���̓J�[�\���ړ��̉�ʃX�N���[���������Ȃ�
	const bool bDrawSwitchOld = SetDrawSwitchOfAllViews(false);
	RestorePhysPosOfAllView(posSave);
	SetDrawSwitchOfAllViews(bDrawSwitchOld);

	for (int i=0; i<GetAllViewCount(); ++i) {
		if (GetView(i).GetHwnd()) {
			InvalidateRect(GetView(i).GetHwnd(), NULL, TRUE);
			GetView(i).AdjustScrollBars();
		}
	}
	if (GetMiniMap().GetHwnd()) {
		InvalidateRect(GetMiniMap().GetHwnd(), NULL, TRUE);
		GetMiniMap().AdjustScrollBars();
	}
	GetActiveView().GetCaret().ShowCaretPosInfo();

	if (hwndProgress) {
		::ShowWindow(hwndProgress, SW_HIDE);
	}
}


/*!
	���C�A�E�g�̕ύX�ɐ旧���āC�S�Ă�View�̍��W�𕨗����W�ɕϊ����ĕۑ�����D

	@return �f�[�^��ۑ������z��ւ̃|�C���^

	@note �擾�����l�̓��C�A�E�g�ύX���EditWnd::RestorePhysPosOfAllView�֓n���D
	�n���Y���ƃ��������[�N�ƂȂ�D
*/
PointEx* EditWnd::SavePhysPosOfAllView()
{
	const int numOfViews = GetAllViewCount();
	const int numOfPositions = 6;
	
	PointEx* pptPosArray = new PointEx[numOfViews * numOfPositions];
	auto& layoutMgr = GetDocument().layoutMgr;
	for (int i=0; i<numOfViews; ++i) {
		auto& view = GetView(i);
		Point tmp = Point(0, view.pTextArea->GetViewTopLine());
		const Layout* layoutLine = layoutMgr.SearchLineByLayoutY(tmp.y);
		if (layoutLine) {
			int nLineCenter = layoutLine->GetLogicOffset() + (int)layoutLine->GetLengthWithoutEOL() / 2;
			pptPosArray[i * numOfPositions + 0].x = nLineCenter;
			pptPosArray[i * numOfPositions + 0].y = layoutLine->GetLogicLineNo();
		}else {
			pptPosArray[i * numOfPositions + 0].x = 0;
			pptPosArray[i * numOfPositions + 0].y = 0;
		}
		pptPosArray[i * numOfPositions + 0].ext = 0;
		auto& selInfo = view.GetSelectionInfo();
		if (selInfo.selectBgn.GetFrom().y >= 0) {
			pptPosArray[i * numOfPositions + 1] = layoutMgr.LayoutToLogicEx(
				selInfo.selectBgn.GetFrom()
			);
		}
		if (selInfo.selectBgn.GetTo().y >= 0) {
			pptPosArray[i * numOfPositions + 2] = layoutMgr.LayoutToLogicEx(
				selInfo.selectBgn.GetTo()
			);
		}
		if (selInfo.select.GetFrom().y >= 0) {
			pptPosArray[i * numOfPositions + 3] = layoutMgr.LayoutToLogicEx(
				selInfo.select.GetFrom()
			);
		}
		if (selInfo.select.GetTo().y >= 0) {
			pptPosArray[i * numOfPositions + 4] = layoutMgr.LayoutToLogicEx(
				selInfo.select.GetTo()
			);
		}
		pptPosArray[i * numOfPositions + 5] = layoutMgr.LayoutToLogicEx(
			view.GetCaret().GetCaretLayoutPos()
		);
	}
	return pptPosArray;
}


/*!	���W�̕���

	EditWnd::SavePhysPosOfAllView�ŕۑ������f�[�^�����ɍ��W�l���Čv�Z����D
*/
void EditWnd::RestorePhysPosOfAllView(PointEx* pptPosArray)
{
	const int numOfViews = GetAllViewCount();
	const int numOfPositions = 6;

	auto& layoutMgr = GetDocument().layoutMgr;
	for (int i=0; i<numOfViews; ++i) {
		Point tmp = layoutMgr.LogicToLayoutEx(
			pptPosArray[i * numOfPositions + 0]
		);
		auto& view = GetView(i);
		view.pTextArea->SetViewTopLine(tmp.y);
		auto& selInfo = view.GetSelectionInfo();
		if (selInfo.selectBgn.GetFrom().y >= 0) {
			selInfo.selectBgn.SetFrom(layoutMgr.LogicToLayoutEx(pptPosArray[i * numOfPositions + 1]));
		}
		if (selInfo.selectBgn.GetTo().y >= 0) {
			selInfo.selectBgn.SetTo(layoutMgr.LogicToLayoutEx(pptPosArray[i * numOfPositions + 2]));
		}
		if (selInfo.select.GetFrom().y >= 0) {
			selInfo.select.SetFrom(layoutMgr.LogicToLayoutEx(pptPosArray[i * numOfPositions + 3]));
		}
		if (selInfo.select.GetTo().y >= 0) {
			selInfo.select.SetTo(layoutMgr.LogicToLayoutEx(pptPosArray[i * numOfPositions + 4]));
		}
		Point ptPosXY = layoutMgr.LogicToLayoutEx(pptPosArray[i * numOfPositions + 5]);
		auto& caret = view.GetCaret();
		caret.MoveCursor(ptPosXY, false); // 2013.06.05 bScroll��true=>falase
		caret.nCaretPosX_Prev = caret.GetCaretLayoutPos().x;

		int nLeft = 0;
		auto& textArea = view.GetTextArea();
		if (textArea.nViewColNum < (int)view.GetRightEdgeForScrollBar()) {
			nLeft = view.GetRightEdgeForScrollBar() - textArea.nViewColNum;
		}
		if (nLeft < textArea.GetViewLeftCol()) {
			textArea.SetViewLeftCol(nLeft);
		}

		caret.ShowEditCaret();
	}
	GetActiveView().GetCaret().ShowCaretPosInfo();
	delete[] pptPosArray;
}

/*!
	@brief �}�E�X�̏�Ԃ��N���A����i�z�C�[���X�N���[���L����Ԃ��N���A�j

	@note �z�C�[������ɂ��y�[�W�X�N���[���E���X�N���[���Ή��̂��߂ɒǉ��B
		  �y�[�W�X�N���[���E���X�N���[������t���O��OFF����B
*/
void EditWnd::ClearMouseState(void)
{
	SetPageScrollByWheel(FALSE);		// �z�C�[������ɂ��y�[�W�X�N���[���L��
	SetHScrollByWheel(FALSE);			// �z�C�[������ɂ�鉡�X�N���[���L��
}

/*! �E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
	@note Wine�ł͕ʃv���Z�X�ō쐬�����A�N�Z�����[�^�e�[�u�����g�p���邱�Ƃ��ł��Ȃ��B
	      IsWine()�ɂ��v���Z�X���ɃA�N�Z�����[�^�e�[�u�����쐬�����悤�ɂȂ�
	      ���߁A�V���[�g�J�b�g�L�[��J�[�\���L�[������ɏ��������悤�ɂȂ�B
*/
void EditWnd::CreateAccelTbl(void)
{
	if (IsWine()) {
		auto& csKeyBind = pShareData->common.keyBind;
		hAccelWine = KeyBind::CreateAccerelator(
			csKeyBind.nKeyNameArrNum,
			csKeyBind.pKeyNameArr
		);

		if (!hAccelWine) {
			ErrorMessage(
				NULL,
				LS(STR_ERR_DLGEDITWND01)
			);
		}
	}

	hAccel = hAccelWine ? hAccelWine : pShareData->handles.hAccel;
}

/*! �E�B���h�E���ɍ쐬�����A�N�Z�����[�^�e�[�u����j������ */
void EditWnd::DeleteAccelTbl(void)
{
	hAccel = NULL;

	if (hAccelWine) {
		::DestroyAcceleratorTable(hAccelWine);
		hAccelWine = NULL;
	}
}

// �v���O�C���R�}���h���G�f�B�^�ɓo�^����
void EditWnd::RegisterPluginCommand(int idCommand)
{
	Plug* plug = JackManager::getInstance().GetCommandById(idCommand);
	RegisterPluginCommand(plug);
}

// �v���O�C���R�}���h���G�f�B�^�ɓo�^����i�ꊇ�j
void EditWnd::RegisterPluginCommand()
{
	const Plug::Array& plugs = JackManager::getInstance().GetPlugs(PP_COMMAND);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		RegisterPluginCommand(*it);
	}
}

// �v���O�C���R�}���h���G�f�B�^�ɓo�^����
void EditWnd::RegisterPluginCommand(Plug* plug)
{
	int iBitmap = MenuDrawer::TOOLBAR_ICON_PLUGCOMMAND_DEFAULT - 1;
	if (!plug->sIcon.empty()) {
		iBitmap = menuDrawer.pIcons->Add(to_tchar(plug->plugin.GetFilePath(to_tchar(plug->sIcon.c_str())).c_str()));
	}

	menuDrawer.AddToolButton(iBitmap, plug->GetFunctionCode());
}


const LOGFONT& EditWnd::GetLogfont(bool bTempSetting)
{
	if (bTempSetting && GetDocument().blfCurTemp) {
		return GetDocument().lfCur;
	}
	bool bUseTypeFont = GetDocument().docType.GetDocumentAttribute().bUseTypeFont;
	if (bUseTypeFont) {
		return GetDocument().docType.GetDocumentAttribute().lf;
	}
	return pShareData->common.view.lf;
}

int EditWnd::GetFontPointSize(bool bTempSetting)
{
	if (bTempSetting && GetDocument().blfCurTemp) {
		return GetDocument().nPointSizeCur;
	}
	bool bUseTypeFont = GetDocument().docType.GetDocumentAttribute().bUseTypeFont;
	if (bUseTypeFont) {
		return GetDocument().docType.GetDocumentAttribute().nPointSize;
	}
	return pShareData->common.view.nPointSize;
}


CharWidthCacheMode EditWnd::GetLogfontCacheMode()
{
	if (GetDocument().blfCurTemp) {
		return CharWidthCacheMode::Local;
	}
	bool bUseTypeFont = GetDocument().docType.GetDocumentAttribute().bUseTypeFont;
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
