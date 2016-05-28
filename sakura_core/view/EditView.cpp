/*!	@file
	@brief �����E�B���h�E�̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
	@date   2005/09/02 D.S.Koba GetSizeOfChar�ŏ�������
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, genta, JEPRO, MIK
	Copyright (C) 2001, genta, GAE, MIK, hor, asa-o, Stonee, Misaka, novice, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, Moca, minfu, KK, novice, ai, Azumaiya, genta
	Copyright (C) 2003, MIK, ai, ryoji, Moca, wmlhq, genta
	Copyright (C) 2004, genta, Moca, novice, naoh, isearch, fotomo
	Copyright (C) 2005, genta, MIK, novice, aroka, D.S.Koba, �����, Moca
	Copyright (C) 2006, Moca, aroka, ryoji, fon, genta
	Copyright (C) 2007, ryoji, ���イ��, maru
	Copyright (C) 2009, nasukoji, ryoji
	Copyright (C) 2010, ryoji

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
#include <limits.h>
#include "view/EditView.h"
#include "view/ViewFont.h"
#include "view/Ruler.h"
#include "uiparts/WaitCursor.h"
#include "window/EditWnd.h"
#include "window/SplitBoxWnd.h"///
#include "OpeBlk.h"///
#include "cmd/ViewCommander_inline.h"
#include "_os/DropTarget.h"///
#include "_os/Clipboard.h"
#include "_os/OsVersionInfo.h"
#include "MarkMgr.h"///
#include "types/TypeSupport.h"
#include "convert/Convert.h"
#include "util/RegKey.h"
#include "util/string_ex2.h"
#include "util/os.h" //WM_MOUSEWHEEL,IMR_RECONVERTSTRING,WM_XBUTTON*,IMR_CONFIRMRECONVERTSTRING
#include "util/module.h"
#include "debug/RunningTimer.h"

#ifndef IMR_DOCUMENTFEED
#define IMR_DOCUMENTFEED 0x0007
#endif

EditView*	g_pEditView;
LRESULT CALLBACK EditViewWndProc(HWND, UINT, WPARAM, LPARAM);
VOID CALLBACK EditViewTimerProc(HWND, UINT, UINT_PTR, DWORD);

#define IDT_ROLLMOUSE	1

/*
|| �E�B���h�E�v���V�[�W��
||
*/

LRESULT CALLBACK EditViewWndProc(
	HWND		hwnd,	// handle of window
	UINT		uMsg,	// message identifier
	WPARAM		wParam,	// first message parameter
	LPARAM		lParam 	// second message parameter
	)
{
//	DEBUG_TRACE(_T("EditViewWndProc(0x%08X): %ls\n"), hwnd, GetWindowsMessageName(uMsg));

	EditView* pEdit;
	switch (uMsg) {
	case WM_CREATE:
		pEdit = (EditView*) g_pEditView;
		return pEdit->DispatchEvent(hwnd, uMsg, wParam, lParam);
	default:
		pEdit = (EditView*) ::GetWindowLongPtr(hwnd, 0);
		if (pEdit) {
			//	May 16, 2000 genta
			//	From Here
			if (uMsg == WM_COMMAND) {
				::SendMessage(::GetParent(pEdit->hwndParent), WM_COMMAND, wParam,  lParam);
			}else {
				return pEdit->DispatchEvent(hwnd, uMsg, wParam, lParam);
			}
			//	To Here
		}
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}


/*
||  �^�C�}�[���b�Z�[�W�̃R�[���o�b�N�֐�
||
||	���݂́A�}�E�X�ɂ��̈�I�����̃X�N���[�������̂��߂Ƀ^�C�}�[���g�p���Ă��܂��B
*/
VOID CALLBACK EditViewTimerProc(
	HWND hwnd,			// handle of window for timer messages
	UINT uMsg,			// WM_TIMER message
	UINT_PTR idEvent,	// timer identifier
	DWORD dwTime		// current system time
	)
{
	EditView* pEditView = (EditView*)::GetWindowLongPtr(hwnd, 0);
	if (pEditView) {
		pEditView->OnTimer(hwnd, uMsg, idEvent, dwTime);
	}
	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �����Ɣj��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́AProcess�ɂЂƂ���̂݁B
EditView::EditView(EditWnd& editWnd)
	:
	ViewCalc(*this),
	editWnd(editWnd),
	pTextArea(nullptr),
	pCaret(nullptr),
	pRuler(nullptr),
	viewSelect(*this),
	parser(*this),
	textDrawer(*this),
	commander(*this),
	hwndVScrollBar(NULL),
	hwndHScrollBar(NULL),
	pDropTarget(nullptr),
	bActivateByMouse(false),	// 2007.10.02 nasukoji
	nWheelDelta(0),
	eWheelScroll(F_0),
	nMousePouse(0),
	nAutoScrollMode(0),
	AT_ImmSetReconvertString(NULL),
	pHistory(nullptr),
	pRegexKeyword(nullptr),
	hAtokModule(NULL)
{
}


// 2007.10.23 kobake �R���X�g���N�^���̏��������ׂ�Create�Ɉڂ��܂����B(�������������s�K�v�ɕ��U���Ă�������)
BOOL EditView::Create(
	HWND		hwndParent,	// �e
	EditDoc&	editDoc,	// �Q�Ƃ���h�L�������g
	int			nMyIndex,	// �r���[�̃C���f�b�N�X
	BOOL		bShow,		// �쐬���ɕ\�����邩�ǂ���
	bool		bMiniMap
	)
{
	this->bMiniMap = bMiniMap;
	pTextArea = new TextArea(*this);
	pCaret = new Caret(*this, editDoc);
	pRuler = new Ruler(*this, editDoc);
	if (bMiniMap) {
		pViewFont = editWnd.pViewFontMiniMap;
	}else {
		pViewFont = editWnd.pViewFont;
	}

	pHistory = new AutoMarkMgr;
	pRegexKeyword = nullptr;				// 2007.04.08 ryoji

	SetDrawSwitch(true);
	pDropTarget = new DropTarget(this);
	_SetDragMode(FALSE);					// �I���e�L�X�g�̃h���b�O����
	bCurSrchKeyMark = false;				// ����������
	//	Jun. 27, 2001 genta
	strCurSearchKey.clear();
	curSearchOption.Reset();				// �����^�u�� �I�v�V����
	bCurSearchUpdate = false;
	nCurSearchKeySequence = -1;

	nMyIndex = 0;

	//	Dec. 4, 2002 genta
	//	���j���[�o�[�ւ̃��b�Z�[�W�\���@�\��EditWnd�ֈڊ�

	// ���L�f�[�^�\���̂̃A�h���X��Ԃ�
	bCommandRunning = false;	// �R�}���h�̎��s��
	bDoing_UndoRedo = false;	// Undo, Redo�̎��s����
	pcsbwVSplitBox = nullptr;	// ���������{�b�N�X
	pcsbwHSplitBox = nullptr;	// ���������{�b�N�X
	hwndVScrollBar = NULL;
	nVScrollRate = 1;			// �����X�N���[���o�[�̏k��
	hwndHScrollBar = NULL;
	hwndSizeBox = NULL;

	ptSrchStartPos_PHY.Set(-1, -1);	// ����/�u���J�n���̃J�[�\���ʒu  (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	bSearch = false;					// ����/�u���J�n�ʒu��o�^���邩											// 02/06/26 ai
	
	ptBracketPairPos_PHY.Set(-1, -1); // �Ί��ʂ̈ʒu (���s�P�ʍs�擪����̃o�C�g��(0�J�n), ���s�P�ʍs�̍s�ԍ�(0�J�n))
	ptBracketCaretPos_PHY.Set(-1, -1);

	bDrawBracketPairFlag = false;	// 03/02/18 ai
	GetSelectionInfo().bDrawSelectArea = false;	// �I��͈͂�`�悵����		// 02/12/13 ai

	crBack = -1;				// �e�L�X�g�̔w�i�F			// 2006.12.16 ryoji
	crBack2 = -1;
	
	szComposition[0] = _T('\0');

	auto& textArea = GetTextArea();
	// ���[���[�\��
	textArea.SetAreaTop(textArea.GetAreaTop() + GetDllShareData().common.window.nRulerHeight);	// ���[���[����
	GetRuler().SetRedrawFlag();	// ���[���[�S�̂�`��������=true   2002.02.25 Add By KK
	hdcCompatDC = NULL;			// �ĕ`��p�R���p�`�u���c�b
	hbmpCompatBMP = NULL;		// �ĕ`��p�������a�l�o
	hbmpCompatBMPOld = NULL;	// �ĕ`��p�������a�l�o(OLD)
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	nCompatBMPWidth = -1;
	nCompatBMPHeight = -1;
	// To Here 2007.09.09 Moca
	
	nOldUnderLineY = -1;
	nOldCursorLineX = -1;
	nOldCursorVLineWidth = 1;
	nOldUnderLineYHeightReal = 0;

	//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
	//	2007.08.12 genta ��������ShareData�̒l���K�v�ɂȂ���
	curRegexp.InitDll(GetDllShareData().common.search.szRegexpLib);

	// 2004.02.08 hFont_ZEN�͖��g�p�ɂ��폜
	dwTipTimer = ::GetTickCount();	// ����Tip�N���^�C�}�[
	bInMenuLoop = false;				// ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂�
//	MYTRACE(_T("EditView::EditView()�����\n"));
	bHokan = false;

	//	Aug. 31, 2000 genta
	pHistory->SetMax(30);

	// from here  2002.04.09 minfu OS�ɂ���čĕϊ��̕�����ς���
	//	YAZAKI COsVersionInfo�̃J�v�Z�����͎��܂���B
	if (!OsSupportReconvert()) {
		// 95 or NT�Ȃ��
		uMSIMEReconvertMsg = ::RegisterWindowMessage(RWM_RECONVERT);
		uATOKReconvertMsg = ::RegisterWindowMessage(MSGNAME_ATOK_RECONVERT) ;
		uWM_MSIME_RECONVERTREQUEST = ::RegisterWindowMessage(_T("MSIMEReconvertRequest"));
		
		hAtokModule = LoadLibraryExedir(_T("ATOK10WC.DLL"));
		AT_ImmSetReconvertString = NULL;
		if (hAtokModule) {
			AT_ImmSetReconvertString = (BOOL (WINAPI *)(HIMC, int, PRECONVERTSTRING, DWORD )) GetProcAddress(hAtokModule,"AT_ImmSetReconvertString");
		}
	}else { 
		// ����ȊO��OS�̂Ƃ���OS�W�����g�p����
		uMSIMEReconvertMsg = 0;
		uATOKReconvertMsg = 0 ;
		hAtokModule = 0;	//@@@ 2002.04.14 MIK
	}
	// to here  2002.04.10 minfu
	
	// 2004.10.23 isearch
	nISearchMode = 0;
	pMigemo = nullptr;

	// 2007.10.02 nasukoji
	dwTripleClickCheck = 0;		// �g���v���N���b�N�`�F�b�N�p����������

	// 2010.07.15 Moca
	mouseDownPos.Set(-INT_MAX, -INT_MAX);

	bMiniMapMouseDown = false;

	//�����܂ŃR���X�g���N�^�ł���Ă�����
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//�����܂�Create�ł���Ă�����

	WNDCLASS wc;
	this->hwndParent = hwndParent;
	pEditDoc = &editDoc;
	pTypeData = &editDoc.docType.GetDocumentAttribute();
	this->nMyIndex = nMyIndex;

	//	2007.08.18 genta ��������ShareData�̒l���K�v�ɂȂ���
	pRegexKeyword = new RegexKeyword(GetDllShareData().common.search.szRegexpLib);	//@@@ 2001.11.17 add MIK
	pRegexKeyword->RegexKeySetTypes(pTypeData);	//@@@ 2001.11.17 add MIK

	textArea.SetTopYohaku(GetDllShareData().common.window.nRulerBottomSpace); 	// ���[���[�ƃe�L�X�g�̌���
	textArea.SetAreaTop(textArea.GetTopYohaku());								// �\����̏�[���W
	// ���[���[�\��
	if (pTypeData->colorInfoArr[COLORIDX_RULER].bDisp) {
		textArea.SetAreaTop(textArea.GetAreaTop() + GetDllShareData().common.window.nRulerHeight);	// ���[���[����
	}

	// �E�B���h�E�N���X�̓o�^
	//	Apr. 27, 2000 genta
	//	�T�C�Y�ύX���̂������}���邽��CS_HREDRAW | CS_VREDRAW ���O����
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= EditViewWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= sizeof(LONG_PTR);
	wc.hInstance		= G_AppInstance();
	wc.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor			= NULL/*LoadCursor(NULL, IDC_IBEAM)*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_WINDOW + 1)*/;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= GSTR_VIEWNAME;
	if (::RegisterClass(&wc) == 0) {
	}

	// �G�f�B�^�E�B���h�E�̍쐬
	g_pEditView = this;
	SetHwnd(
		::CreateWindowEx(
			WS_EX_STATICEDGE,		// extended window style
			GSTR_VIEWNAME,			// pointer to registered class name
			GSTR_VIEWNAME,			// pointer to window name
			0						// window style
			| WS_VISIBLE
			| WS_CHILD
			| WS_CLIPCHILDREN
			,
			CW_USEDEFAULT,			// horizontal position of window
			0,						// vertical position of window
			CW_USEDEFAULT,			// window width
			0,						// window height
			hwndParent,				// handle to parent or owner window
			NULL,					// handle to menu or child-window identifier
			G_AppInstance(),		// handle to application instance
			(LPVOID)this			// pointer to window-creation data
		)
	);
	if (!GetHwnd()) {
		return FALSE;
	}

	pDropTarget->Register_DropTarget(GetHwnd());

	// ����Tip�\���E�B���h�E�쐬
	tipWnd.Create(G_AppInstance(), GetHwnd()/*GetDllShareData().sHandles.hwndTray*/);

	// �ĕ`��p�R���p�`�u���c�b
	// 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	// 2007.09.30 genta �֐���
	UseCompatibleDC(GetDllShareData().common.window.bUseCompatibleBMP);

	// ���������{�b�N�X
	pcsbwVSplitBox = new SplitBoxWnd;
	pcsbwVSplitBox->Create(G_AppInstance(), GetHwnd(), TRUE);
	// ���������{�b�N�X
	pcsbwHSplitBox = new SplitBoxWnd;
	pcsbwHSplitBox->Create(G_AppInstance(), GetHwnd(), FALSE);

	// �X�N���[���o�[�쐬
	CreateScrollBar();		// 2006.12.19 ryoji

	SetFont();

	if (bShow) {
		ShowWindow(GetHwnd(), SW_SHOW);
	}

	// �L�[�{�[�h�̌��݂̃��s�[�g�Ԋu���擾
	int nKeyBoardSpeed;
	SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &nKeyBoardSpeed, 0);

	// �^�C�}�[�N��
	if (::SetTimer(GetHwnd(), IDT_ROLLMOUSE, nKeyBoardSpeed, EditViewTimerProc) == 0) {
		WarningMessage(GetHwnd(), LS(STR_VIEW_TIMER));
	}

	bHideMouse = false;
	RegKey reg;
	BYTE bUserPref[8] = {0};
	reg.Open(HKEY_CURRENT_USER, _T("Control Panel\\Desktop"));
	reg.GetValueBINARY(_T("UserPreferencesMask"), bUserPref, sizeof(bUserPref));
	if ((bUserPref[2] & 0x01) == 1) {
		bHideMouse = true;
	}

	TypeSupport textType(*this, COLORIDX_TEXT);
	crBack = textType.GetBackColor();

	return TRUE;
}


EditView::~EditView()
{
	Close();
}

void EditView::Close()
{
	if (GetHwnd()) {
		::DestroyWindow(GetHwnd());
	}

	// �ĕ`��p�R���p�`�u���c�b
	//	2007.09.30 genta �֐���
	//	hbmpCompatBMP�������ō폜�����D
	UseCompatibleDC(FALSE);

	delete pDropTarget;
	pDropTarget = nullptr;

	delete pHistory;
	pHistory = nullptr;

	delete pRegexKeyword;	//@@@ 2001.11.17 add MIK
	pRegexKeyword = nullptr;
	
	// �ĕϊ� 2002.04.10 minfu
	if (hAtokModule)
		FreeLibrary(hAtokModule);

	delete pTextArea;
	pTextArea = nullptr;
	delete pCaret;
	pCaret = nullptr;
	delete pRuler;
	pRuler = nullptr;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �C�x���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// TCHAR��WCHAR�ϊ��B
inline wchar_t tchar_to_wchar(TCHAR tc)
{
#ifdef _UNICODE
	return tc;
#else
	WCHAR wc = 0;
	mbtowc(&wc, &tc, sizeof(tc));
	return wc;
#endif
}

/*
|| ���b�Z�[�W�f�B�X�p�b�`��
*/
LRESULT EditView::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
	)
{
	HDC hdc;
//	int nPosX;
//	int nPosY;
	switch (uMsg) {
	case WM_MOUSEWHEEL:
		if (editWnd.DoMouseWheel(wParam, lParam)) {
			return 0L;
		}
		return OnMOUSEWHEEL(wParam, lParam);

	case WM_MOUSEHWHEEL:
		return OnMOUSEHWHEEL(wParam, lParam);

	case WM_CREATE:
		::SetWindowLongPtr(hwnd, 0, (LONG_PTR) this);
		return 0L;

		// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	case WM_SHOWWINDOW:
		// �E�B���h�E��\���̍ĂɌ݊�BMP��p�����ă������[��ߖ񂷂�
		if (hwnd == GetHwnd() && (BOOL)wParam == FALSE) {
			DeleteCompatibleBitmap();
		}
		return 0L;
	// To Here 2007.09.09 Moca

	case WM_SIZE:
		OnSize(LOWORD(lParam), HIWORD(lParam));
		return 0L;

	case WM_SETFOCUS:
		OnSetFocus();
		// �e�E�B���h�E�̃^�C�g�����X�V
		editWnd.UpdateCaption();
		return 0L;
	case WM_KILLFOCUS:
		OnKillFocus();
		// 2009.01.17 nasukoji	�z�C�[���X�N���[���L����Ԃ��N���A
		editWnd.ClearMouseState();
		return 0L;
	case WM_CHAR:
#ifdef _UNICODE
		// �R���g���[���R�[�h���͋֎~
		if (WCODE::IsControlCode((wchar_t)wParam)) {
			ErrorBeep();
		}else {
			GetCommander().HandleCommand(F_WCHAR, true, WCHAR(wParam), 0, 0, 0);
		}
#else
		// SJIS�ŗL
		{
			static BYTE preChar = 0;
			if (preChar == 0 && ! _IS_SJIS_1((unsigned char)wParam)) {
				// ASCII , ���p�J�^�J�i
				if (ACODE::IsControlCode((char)wParam)) {
					ErrorBeep();
				}else {
					GetCommander().HandleCommand(F_WCHAR, true, tchar_to_wchar((ACHAR)wParam), 0, 0, 0);
				}
			}else {
				if (preChar) {
					WORD wordData = MAKEWORD((BYTE)wParam, preChar);
					GetCommander().HandleCommand(F_IME_CHAR, true, wordData, 0, 0, 0);
					preChar = 0;
				}else {
					preChar = (BYTE)wParam;
				}
			}
		}
#endif
		return 0L;

	case WM_IME_NOTIFY:	// Nov. 26, 2006 genta
		if (wParam == IMN_SETCONVERSIONMODE || wParam == IMN_SETOPENSTATUS) {
			GetCaret().ShowEditCaret();
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	case WM_IME_COMPOSITION:
		if (IsInsMode() && (lParam & GCS_RESULTSTR)) {
			HIMC hIMC;
			DWORD dwSize;
			HGLOBAL hstr;
			hIMC = ImmGetContext(hwnd);

			szComposition[0] = _T('\0');

			if (!hIMC) {
				return 0;
//				MyError(ERROR_NULLCONTEXT);
			}

			// Get the size of the result string.
			dwSize = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);

			// increase buffer size for NULL terminator,
			//	maybe it is in Unicode
			dwSize += sizeof(WCHAR);

			hstr = GlobalAlloc(GHND, dwSize);
			if (!hstr) {
				return 0;
//				 MyError(ERROR_GLOBALALLOC);
			}

			LPTSTR lptstr = (LPTSTR)GlobalLock(hstr);
			if (!lptstr) {
				return 0;
//				 MyError(ERROR_GLOBALLOCK);
			}

			// Get the result strings that is generated by IME into lptstr.
			ImmGetCompositionString(hIMC, GCS_RESULTSTR, lptstr, dwSize);

			// �e�L�X�g��\��t��
			if (bHideMouse && 0 <= nMousePouse) {
				nMousePouse = -1;
				::SetCursor(NULL);
			}
#ifdef _UNICODE
			GetCommander().HandleCommand(F_INSTEXT_W, true, (LPARAM)lptstr, wcslen(lptstr), TRUE, 0);
#else
			std::wstring wstr = to_wchar(lptstr);
			GetCommander().HandleCommand(F_INSTEXT_W, true, (LPARAM)wstr.c_str(), wstr.length(), TRUE, 0);
#endif
			ImmReleaseContext(hwnd, hIMC);

			// add this string into text buffer of application

			GlobalUnlock(hstr);
			GlobalFree(hstr);
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	case WM_IME_ENDCOMPOSITION:
		szComposition[0] = _T('\0');
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	case WM_IME_CHAR:
		if (!IsInsMode() /* Oct. 2, 2005 genta */) { /* �㏑�����[�h���H */
			GetCommander().HandleCommand(F_IME_CHAR, true, wParam, 0, 0, 0);
		}
		return 0L;

	// From Here 2008.03.24 Moca ATOK���̗v���ɂ�������
	case WM_PASTE:
		return GetCommander().HandleCommand(F_PASTE, true, 0, 0, 0, 0);

	case WM_COPY:
		return GetCommander().HandleCommand(F_COPY, true, 0, 0, 0, 0);
	// To Here 2008.03.24 Moca

	case WM_KEYUP:
		// �L�[���s�[�g���
		GetCommander().bPrevCommand = 0;
		return 0L;

	// 2004.04.27 Moca From Here ALT+x��ALT���������܂܂��ƃL�[���s�[�g��OFF�ɂȂ�Ȃ��΍�
	case WM_SYSKEYUP:
		GetCommander().bPrevCommand = 0;
		// �O�̂��ߌĂ�
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	// 2004.04.27 To Here

	case WM_LBUTTONDBLCLK:
		if (bMiniMap) {
			return 0L;
		}
		// 2007.10.02 nasukoji	��A�N�e�B�u�E�B���h�E�̃_�u���N���b�N���͂����ŃJ�[�\�����ړ�����
		// 2007.10.12 genta �t�H�[�J�X�ړ��̂��߁COnLBUTTONDBLCLK���ړ�
		if (bActivateByMouse) {
			// �A�N�e�B�u�ȃy�C����ݒ�
			editWnd.SetActivePane(nMyIndex);
			// �J�[�\�����N���b�N�ʒu�ֈړ�����
			OnLBUTTONDOWN(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));	
			// 2007.10.02 nasukoji
			bActivateByMouse = false;		// �}�E�X�ɂ��A�N�e�B�x�[�g�������t���O��OFF
		}
		//		MYTRACE(_T(" WM_LBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD(lParam), HIWORD(lParam));
		OnLBUTTONDBLCLK(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		return 0L;

// novice 2004/10/11 �}�E�X���{�^���Ή�
	case WM_MBUTTONDOWN:
		OnMBUTTONDOWN(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));

		return 0L;

	case WM_MBUTTONUP:
		// 2009.01.17 nasukoji	�{�^��UP�ŃR�}���h���N������悤�ɕύX
		OnMBUTTONUP(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		return 0L;

	case WM_LBUTTONDOWN:
		// 2007.10.02 nasukoji
		bActivateByMouse = false;		// �}�E�X�ɂ��A�N�e�B�x�[�g�������t���O��OFF
//		MYTRACE(_T(" WM_LBUTTONDOWN wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD(lParam), HIWORD(lParam));
		OnLBUTTONDOWN(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		return 0L;

	case WM_LBUTTONUP:

//		MYTRACE(_T(" WM_LBUTTONUP wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD(lParam), HIWORD(lParam));
		OnLBUTTONUP(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		return 0L;
	case WM_MOUSEMOVE:
		OnMOUSEMOVE(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		return 0L;

	case WM_RBUTTONDBLCLK:
//		MYTRACE(_T(" WM_RBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD(lParam), HIWORD(lParam));
		return 0L;
//	case WM_RBUTTONDOWN:
//		MYTRACE(_T(" WM_RBUTTONDOWN wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD(lParam), HIWORD(lParam));
//		OnRBUTTONDOWN(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
//		if (nMyIndex != editWnd.GetActivePane()) {
//			// �A�N�e�B�u�ȃy�C����ݒ�
//			editWnd.SetActivePane(nMyIndex);
//		}
//		return 0L;
	case WM_RBUTTONUP:
//		MYTRACE(_T(" WM_RBUTTONUP wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD(lParam), HIWORD(lParam));
		OnRBUTTONUP(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		return 0L;

// novice 2004/10/10 �}�E�X�T�C�h�{�^���Ή�
	case WM_XBUTTONDOWN:
		switch (HIWORD(wParam)) {
		case XBUTTON1:
			OnXLBUTTONDOWN(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
			break;
		case XBUTTON2:
			OnXRBUTTONDOWN(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
			break;
		}

		return TRUE;

	case WM_XBUTTONUP:
		// 2009.01.17 nasukoji	�{�^��UP�ŃR�}���h���N������悤�ɕύX
		switch (HIWORD(wParam)) {
		case XBUTTON1:
			OnXLBUTTONUP(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
			break;
		case XBUTTON2:
			OnXRBUTTONUP(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
			break;
		}

		return TRUE;

	case WM_VSCROLL:
//		MYTRACE(_T("	WM_VSCROLL nPos=%d\n"), GetScrollPos(hwndVScrollBar, SB_CTL));
		//	Sep. 11, 2004 genta �����X�N���[���̊֐���
		{
			int Scroll = OnVScroll(
				(int) LOWORD(wParam), ((int) HIWORD(wParam)) * nVScrollRate);

			//	�V�t�g�L�[��������Ă��Ȃ��Ƃ����������X�N���[��
			if (!GetKeyState_Shift()) {
				SyncScrollV(Scroll);
			}
		}

		return 0L;

	case WM_HSCROLL:
//		MYTRACE(_T("	WM_HSCROLL nPos=%d\n"), GetScrollPos(hwndHScrollBar, SB_CTL));
		//	Sep. 11, 2004 genta �����X�N���[���̊֐���
		{
			int Scroll = OnHScroll(
				(int) LOWORD(wParam), ((int) HIWORD(wParam)));

			//	�V�t�g�L�[��������Ă��Ȃ��Ƃ����������X�N���[��
			if (!GetKeyState_Shift()) {
				SyncScrollH(Scroll);
			}
		}

		return 0L;

	case WM_ENTERMENULOOP:
		bInMenuLoop = true;	// ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂�

		// ����Tip���N������Ă���
		if (dwTipTimer == 0) {
			// ����Tip������
			tipWnd.Hide();
			dwTipTimer = ::GetTickCount();	// ����Tip�N���^�C�}�[
		}
		if (bHokan) {
			editWnd.hokanMgr.Hide();
			bHokan = false;
		}
		return 0L;

	case WM_EXITMENULOOP:
		bInMenuLoop = false;	// ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂�
		return 0L;


	case WM_PAINT:
		{
			PAINTSTRUCT	ps;
			hdc = ::BeginPaint(hwnd, &ps);
			OnPaint(hdc, &ps, FALSE);
			::EndPaint(hwnd, &ps);
		}
		return 0L;

	case WM_CLOSE:
//		MYTRACE(_T("	WM_CLOSE\n"));
		::DestroyWindow(hwnd);
		return 0L;
	case WM_DESTROY:
		pDropTarget->Revoke_DropTarget();

		// �^�C�}�[�I��
		::KillTimer(GetHwnd(), IDT_ROLLMOUSE);


//		MYTRACE(_T("	WM_DESTROY\n"));
		/*
		||�q�E�B���h�E�̔j��
		*/
		if (hwndVScrollBar) {	// Aug. 20, 2005 Aroka
			::DestroyWindow(hwndVScrollBar);
			hwndVScrollBar = NULL;
		}
		if (hwndHScrollBar) {
			::DestroyWindow(hwndHScrollBar);
			hwndHScrollBar = NULL;
		}
		if (hwndSizeBox) {
			::DestroyWindow(hwndSizeBox);
			hwndSizeBox = NULL;
		}
		SAFE_DELETE(pcsbwVSplitBox);	// ���������{�b�N�X
		SAFE_DELETE(pcsbwHSplitBox);	// ���������{�b�N�X

		SetHwnd(NULL);
		return 0L;

	case MYWM_DOSPLIT:
//		nPosX = (int)wParam;
//		nPosY = (int)lParam;
//		MYTRACE(_T("MYWM_DOSPLIT nPosX=%d nPosY=%d\n"), nPosX, nPosY);
		::SendMessage(hwndParent, MYWM_DOSPLIT, wParam, lParam);
		return 0L;

	case MYWM_SETACTIVEPANE:
		editWnd.SetActivePane(nMyIndex);
		::PostMessage(hwndParent, MYWM_SETACTIVEPANE, (WPARAM)nMyIndex, 0);
		return 0L;

	case MYWM_IME_REQUEST:  // �ĕϊ�  by minfu 2002.03.27 // 20020331 aroka
		
		// 2002.04.09 switch case �ɕύX  minfu 
		switch (wParam) {
		case IMR_RECONVERTSTRING:
			return SetReconvertStruct((PRECONVERTSTRING)lParam, UNICODE_BOOL);
			
		case IMR_CONFIRMRECONVERTSTRING:
			return SetSelectionFromReonvert((PRECONVERTSTRING)lParam, UNICODE_BOOL);
			
		// 2010.03.16 MS-IME 2002 ���Ɓu�J�[�\���ʒu�̑O��̓��e���Q�Ƃ��ĕϊ����s���v�̋@�\
		case IMR_DOCUMENTFEED:
			return SetReconvertStruct((PRECONVERTSTRING)lParam, UNICODE_BOOL, true);
			
		//default:
		}
		// 2010.03.16 0L�ł͂Ȃ�TSF���������邩������Ȃ��̂�Def�ɂ܂�����
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	
	case MYWM_DROPFILES:	// �Ǝ��̃h���b�v�t�@�C���ʒm	// 2008.06.20 ryoji
		OnMyDropFiles((HDROP)wParam);
		return 0L;

	// 2007.10.02 nasukoji	�}�E�X�N���b�N�ɂăA�N�e�B�x�[�g���ꂽ���̓J�[�\���ʒu���ړ����Ȃ�
	case WM_MOUSEACTIVATE:
		LRESULT nRes;
		nRes = ::DefWindowProc(hwnd, uMsg, wParam, lParam);	// �e�ɐ�ɏ���������
		if (nRes == MA_NOACTIVATE || nRes == MA_NOACTIVATEANDEAT) {
			return nRes;
		}

		// �}�E�X�N���b�N�ɂ��o�b�N�O���E���h�E�B���h�E���A�N�e�B�x�[�g���ꂽ
		//	2007.10.08 genta �I�v�V�����ǉ�
		if (1
			&& GetDllShareData().common.general.bNoCaretMoveByActivation
			&& !editWnd.IsActiveApp()
		) {
			bActivateByMouse = true;		// �}�E�X�ɂ��A�N�e�B�x�[�g
			return MA_ACTIVATEANDEAT;		// �A�N�e�B�x�[�g��C�x���g��j��
		}

		// �A�N�e�B�u�ȃy�C����ݒ�
		if (::GetFocus() != GetHwnd()) {
			POINT ptCursor;
			::GetCursorPos(&ptCursor);
			HWND hwndCursorPos = ::WindowFromPoint(ptCursor);
			if (hwndCursorPos == GetHwnd()) {
				// �r���[��Ƀ}�E�X������̂� SetActivePane() �𒼐ڌĂяo��
				// �i�ʂ̃}�E�X���b�Z�[�W���͂��O�ɃA�N�e�B�u�y�C����ݒ肵�Ă����j
				if (!bMiniMap) {
					editWnd.SetActivePane(nMyIndex);
				}
			}else if (0
				|| (pcsbwVSplitBox && hwndCursorPos == pcsbwVSplitBox->GetHwnd())
				|| (pcsbwHSplitBox && hwndCursorPos == pcsbwHSplitBox->GetHwnd())
			) {
				// 2010.01.19 ryoji
				// �����{�b�N�X��Ƀ}�E�X������Ƃ��̓A�N�e�B�u�y�C����؂�ւ��Ȃ�
				// �i������ MYWM_SETACTIVEPANE �̃|�X�g�ɂ�蕪�����̃S�~���c���Ă��������C���j
			}else {
				// 2008.05.24 ryoji
				// �X�N���[���o�[��Ƀ}�E�X�����邩������Ȃ��̂� MYWM_SETACTIVEPANE ���|�X�g����
				// SetActivePane() �ɂ̓X�N���[���o�[�̃X�N���[���͈͒����������܂܂�Ă��邪�A
				// ���̃^�C�~���O�iWM_MOUSEACTIVATE�j�ŃX�N���[���͈͂�ύX����̂͂܂����B
				// �Ⴆ�� Win XP/Vista ���ƃX�N���[���͈͂��������Ȃ��ăX�N���[���o�[���L������
				// �����ɐ؂�ւ��Ƃ���Ȍ�X�N���[���o�[���@�\���Ȃ��Ȃ�B
				if (!bMiniMap) {
					::PostMessage(GetHwnd(), MYWM_SETACTIVEPANE, (WPARAM)nMyIndex, 0);
				}
			}
		}

		return nRes;

	case EM_GETLIMITTEXT:
		return INT_MAX;
	case EM_REPLACESEL:
	{
		// wParam RedoUndo�t���O�͖�������
		if (lParam) {
#ifdef _UNICODE
			GetCommander().HandleCommand(F_INSTEXT_W, true, lParam, wcslen((wchar_t*)lParam), TRUE, 0);
#else
			std::wstring text = to_wchar((LPCTSTR)lParam);
			GetCommander().HandleCommand(F_INSTEXT_W, true, (LPARAM)text.c_str(), text.length(), TRUE, 0);
#endif
		}
		return 0L; // not use.
	}

	default:
// << 20020331 aroka �ĕϊ��Ή� for 95/NT
		if (0
			|| (uMSIMEReconvertMsg && (uMsg == uMSIMEReconvertMsg)) 
			|| (uATOKReconvertMsg && (uMsg == uATOKReconvertMsg))
		) {
		// 2002.04.08 switch case �ɕύX minfu 
			switch (wParam) {
			case IMR_RECONVERTSTRING:
				return SetReconvertStruct((PRECONVERTSTRING)lParam, true);
				
			case IMR_CONFIRMRECONVERTSTRING:
				return SetSelectionFromReonvert((PRECONVERTSTRING)lParam, true);
			}
			return 0L;
		}
// >> by aroka

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    �E�B���h�E�C�x���g                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void EditView::OnMove(int x, int y, int nWidth, int nHeight)
{
	MoveWindow(GetHwnd(), x, y, nWidth, nHeight, TRUE);
	return;
}


// �E�B���h�E�T�C�Y�̕ύX����
void EditView::OnSize(int cx, int cy)
{
	if (!GetHwnd() 
		|| (cx == 0 && cy == 0)
	) {
		// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
		// �E�B���h�E�������ɂ��݊�BMP��j������
		DeleteCompatibleBitmap();
		// To Here 2007.09.09 Moca
		return;
	}

	int	nVSplitHeight = 0;	// ���������{�b�N�X�̍���
	int	nHSplitWidth  = 0;	// ���������{�b�N�X�̕�

	// �X�N���[���o�[�̃T�C�Y��l���擾
	int nCxHScroll = ::GetSystemMetrics(SM_CXHSCROLL);
	int nCyHScroll = ::GetSystemMetrics(SM_CYHSCROLL);
	int nCxVScroll = ::GetSystemMetrics(SM_CXVSCROLL);
	int nCyVScroll = ::GetSystemMetrics(SM_CYVSCROLL);

	// ���������{�b�N�X
	if (pcsbwVSplitBox) {
		nVSplitHeight = 7;
		::MoveWindow(pcsbwVSplitBox->GetHwnd(), cx - nCxVScroll , 0, nCxVScroll, nVSplitHeight, TRUE);
	}
	// ���������{�b�N�X
	if (pcsbwHSplitBox) {
		nHSplitWidth = 7;
		::MoveWindow(pcsbwHSplitBox->GetHwnd(), 0, cy - nCyHScroll, nHSplitWidth, nCyHScroll, TRUE);
	}
	// �����X�N���[���o�[
	if (hwndVScrollBar) {
		::MoveWindow(hwndVScrollBar, cx - nCxVScroll , 0 + nVSplitHeight, nCxVScroll, cy - nCyVScroll - nVSplitHeight, TRUE);
	}
	// �����X�N���[���o�[
	if (hwndHScrollBar) {
		::MoveWindow(hwndHScrollBar, 0 + nHSplitWidth, cy - nCyHScroll, cx - nCxVScroll - nHSplitWidth, nCyHScroll, TRUE);
	}

	// �T�C�Y�{�b�N�X
	if (hwndSizeBox) {
		::MoveWindow(hwndSizeBox, cx - nCxVScroll, cy - nCyHScroll, nCxHScroll, nCyVScroll, TRUE);
	}
	auto& textArea = GetTextArea();
	int nAreaWidthOld  = textArea.GetAreaWidth();
	int nAreaHeightOld = textArea.GetAreaHeight();

	// �G���A���X�V
	textArea.TextArea_OnSize(
		Size(cx, cy),
		nCxVScroll,
		hwndHScrollBar ? nCyHScroll : 0
	);

	// �ĕ`��p�������a�l�o
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	if (hdcCompatDC) {
		CreateOrUpdateCompatibleBitmap(cx, cy);
 	}
	// To Here 2007.09.09 Moca

	// 2008.06.06 nasukoji	�T�C�Y�ύX���̐܂�Ԃ��ʒu�Čv�Z
	bool wrapChanged = false;
	if (pEditDoc->nTextWrapMethodCur == TextWrappingMethod::WindowWidth) {
		if (nMyIndex == 0) {	// ������̃r���[�̃T�C�Y�ύX���̂ݏ�������
			// �E�[�Ő܂�Ԃ����[�h�Ȃ�E�[�Ő܂�Ԃ�	// 2008.06.08 ryoji
			wrapChanged = editWnd.WrapWindowWidth(0);
		}
	}

	if (!wrapChanged) {	// �܂�Ԃ��ʒu���ύX����Ă��Ȃ�
		AdjustScrollBars();				// �X�N���[���o�[�̏�Ԃ��X�V����
	}

	// �L�����b�g�̕\��(�E�E���ɉB��Ă����ꍇ)
	GetCaret().ShowEditCaret();

	if (IsBkBitmap()) {
		BackgroundImagePosType imgPos = pTypeData->backImgPos;
		if (imgPos != BackgroundImagePosType::TopLeft) {
			bool bUpdateWidth = false;
			bool bUpdateHeight = false;
			switch (imgPos) {
			case BackgroundImagePosType::TopRight:
			case BackgroundImagePosType::BottomRight:
			case BackgroundImagePosType::CenterRight:
			case BackgroundImagePosType::TopCenter:
			case BackgroundImagePosType::BottomCenter:
			case BackgroundImagePosType::Center:
				bUpdateWidth = true;
				break;
			}
			switch (imgPos) {
			case BackgroundImagePosType::BottomCenter:
			case BackgroundImagePosType::BottomLeft:
			case BackgroundImagePosType::BottomRight:
			case BackgroundImagePosType::Center:
			case BackgroundImagePosType::CenterLeft:
			case BackgroundImagePosType::CenterRight:
				bUpdateHeight = true;
				break;
			}
			if (bUpdateWidth  && nAreaWidthOld  != textArea.GetAreaWidth() ||
			    bUpdateHeight && nAreaHeightOld != textArea.GetAreaHeight()
			) {
				InvalidateRect(NULL, FALSE);
			}
		}
	}

	// �e�E�B���h�E�̃^�C�g�����X�V
	editWnd.UpdateCaption(); // [Q] genta �{���ɕK�v�H

	if (editWnd.GetMiniMap().GetHwnd()) {
		EditView& miniMap = editWnd.GetMiniMap();
		if (miniMap.nPageViewTop != textArea.GetViewTopLine()
			|| miniMap.nPageViewBottom != textArea.GetBottomLine()
		) {
			MiniMapRedraw(true);
		}
	}
	return;
}


// ���̓t�H�[�J�X���󂯎�����Ƃ��̏���
void EditView::OnSetFocus(void)
{
	if (bMiniMap) {
		return;
	}
	// 2004.04.02 Moca EOF�݂̂̃��C�A�E�g�s�́A0���ڂ̂ݗL��.EOF��艺�̍s�̂���ꍇ�́AEOF�ʒu�ɂ���
	{
		Point ptPos = GetCaret().GetCaretLayoutPos();
		if (GetCaret().GetAdjustCursorPos(&ptPos)) {
			GetCaret().MoveCursor(ptPos, false);
			GetCaret().nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;
		}
	}

	GetCaret().ShowEditCaret();

	SetIMECompFormFont();

	// ���[���̃J�[�\�����O���[���獕�ɕύX����
	HDC hdc = ::GetDC(GetHwnd());
	GetRuler().DispRuler(hdc);
	::ReleaseDC(GetHwnd(), hdc);

	// 03/02/18 �Ί��ʂ̋����\��(�`��) ai
	bDrawBracketPairFlag = true;
	DrawBracketPair(true);

	editWnd.toolbar.AcceptSharedSearchKey();

	if (editWnd.GetMiniMap().GetHwnd()) {
		EditView& miniMap = editWnd.GetMiniMap();
		if (miniMap.nPageViewTop != GetTextArea().GetViewTopLine()
			|| miniMap.nPageViewBottom != GetTextArea().GetBottomLine()
		) {
			MiniMapRedraw(true);
		}
	}
}


// ���̓t�H�[�J�X���������Ƃ��̏���
void EditView::OnKillFocus(void)
{
	if (bMiniMap) {
		return;
	}
	// 03/02/18 �Ί��ʂ̋����\��(����) ai
	DrawBracketPair(false);
	bDrawBracketPairFlag = false;

	GetCaret().DestroyCaret();

	// ���[���[�`��
	// ���[���̃J�[�\����������O���[�ɕύX����
	HDC	hdc = ::GetDC(GetHwnd());
	GetRuler().DispRuler(hdc);
	::ReleaseDC(GetHwnd(), hdc);

	// ����Tip���N������Ă���
	if (dwTipTimer == 0) {
		// ����Tip������
		tipWnd.Hide();
		dwTipTimer = ::GetTickCount();	// ����Tip�N���^�C�}�[
	}

	if (bHokan) {
		editWnd.hokanMgr.Hide();
		bHokan = false;
	}
	if (nAutoScrollMode) {
		AutoScrollExit();
	}

	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �ݒ�                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �t�H���g�̕ύX
void EditView::SetFont(void)
{
	// ���g���N�X�X�V
	GetTextMetrics().Update(GetFontset().GetFontHan());

	// �G���A�����X�V
	HDC hdc = ::GetDC(GetHwnd());
	GetTextArea().UpdateAreaMetrics(hdc);
	::ReleaseDC(GetHwnd(), hdc);

	// �s�ԍ��\���ɕK�v�ȕ���ݒ�
	GetTextArea().DetectWidthOfLineNumberArea(false);

	// ����ԍĕ`��
	::InvalidateRect(GetHwnd(), NULL, TRUE);

	//	Oct. 11, 2002 genta IME�̃t�H���g���ύX
	SetIMECompFormFont();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �L�����b�g                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief �I�����l�������s���w��ɂ��J�[�\���ړ�

	�I����ԃ`�F�b�N���J�[�\���ړ����I��̈�X�V�Ƃ���������
	���������̃R�}���h�ɂ���̂ł܂Ƃ߂邱�Ƃɂ����D
	�܂��C�߂�l�͂قƂ�ǎg���Ă��Ȃ��̂�void�ɂ����D

	�I����Ԃ��l�����ăJ�[�\�����ړ�����D
	��I�����w�肳�ꂽ�ꍇ�ɂ͊����I��͈͂��������Ĉړ�����D
	�I�����w�肳�ꂽ�ꍇ�ɂ͑I��͈͂̊J�n�E�ύX�𕹂��čs���D
	�C���^���N�e�B�u�����O��Ƃ��邽�߁C�K�v�ɉ������X�N���[�����s���D
	�J�[�\���ړ���͏㉺�ړ��ł��J�����ʒu��ۂ悤�C
	GetCaret().nCaretPosX_Prev�̍X�V�������čs���D

	@date 2006.07.09 genta �V�K�쐬
*/
void EditView::MoveCursorSelecting(
	Point	ptWk_CaretPos,		// [in] �ړ��惌�C�A�E�g�ʒu
	bool	bSelect,			// true: �I������  false: �I������
	int		nCaretMarginRate	// �c�X�N���[���J�n�ʒu�����߂�l
	)
{
	if (bSelect) {
		if (!GetSelectionInfo().IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
			// ���݂̃J�[�\���ʒu����I�����J�n����
			GetSelectionInfo().BeginSelectArea();
		}
	}else {
		if (GetSelectionInfo().IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			GetSelectionInfo().DisableSelectArea(true);
		}
	}
	GetCaret().GetAdjustCursorPos(&ptWk_CaretPos);
	if (bSelect) {
		/*	���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX�D
		
			2004.04.02 Moca 
			�L�����b�g�ʒu���s���������ꍇ��MoveCursor�̈ړ����ʂ�
			�����ŗ^�������W�Ƃ͈قȂ邱�Ƃ����邽�߁C
			nPosX, nPosY�̑���Ɏ��ۂ̈ړ����ʂ��g���悤�ɁD
		*/
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptWk_CaretPos);
	}
	GetCaret().MoveCursor(ptWk_CaretPos, true, nCaretMarginRate);	// 2007.08.22 ryoji nCaretMarginRate���g���Ă��Ȃ�����
	GetCaret().nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	�w��J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ�

	2007.01.18 kobake URL������̎󂯎���wstring�ōs���悤�ɕύX
	2007.05.27 ryoji URL�F�w��̐��K�\���L�[���[�h�Ƀ}�b�`���镶�����URL�Ƃ݂Ȃ�
	                 URL�̋����\��OFF�̃`�F�b�N�͂��̊֐����ōs���悤�ɕύX
*/
bool EditView::IsCurrentPositionURL(
	const Point& ptCaretPos,	// [in]  �J�[�\���ʒu
	Range* pUrlRange,			// [out] URL�͈́B���W�b�N�P�ʁB
	std::wstring* pwstrURL		// [out] URL������󂯎���BNULL���w�肵���ꍇ��URL��������󂯎��Ȃ��B
	)
{
	MY_RUNNINGTIMER(runningTimer, "EditView::IsCurrentPositionURL");

	// URL�������\�����邩�ǂ����`�F�b�N����	// 2009.05.27 ryoji
	bool bDispUrl = TypeSupport(*this, COLORIDX_URL).IsDisp();
	bool bUseRegexKeyword = false;
	if (pTypeData->bUseRegexKeyword) {
		const wchar_t* pKeyword = pTypeData->regexKeywordList;
		for (int i=0; i<MAX_REGEX_KEYWORD; ++i) {
			if (*pKeyword == L'\0')
				break;
			if (pTypeData->regexKeywordArr[i].nColorIndex == COLORIDX_URL) {
				bUseRegexKeyword = true;	// URL�F�w��̐��K�\���L�[���[�h������
				break;
			}
			for (; *pKeyword!='\0'; ++pKeyword) {}
			++pKeyword;
		}
	}
	if (!bDispUrl && !bUseRegexKeyword) {
		return false;	// URL�����\�����Ȃ��̂�URL�ł͂Ȃ�
	}

	// ���K�\���L�[���[�h�iURL�F�w��j�s�����J�n����	// 2009.05.27 ryoji
	if (bUseRegexKeyword) {
		pRegexKeyword->RegexKeyLineStart();
	}

	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	Point ptXY = pEditDoc->layoutMgr.LayoutToLogic(ptCaretPos);
	size_t nLineLen;
	const wchar_t* pLine = pEditDoc->docLineMgr.GetLine(ptXY.y)->GetDocLineStrWithEOL(&nLineLen); // 2007.10.09 kobake ���C�A�E�g�E���W�b�N���݃o�O�C��

	int nMatchColor;
	size_t nUrlLen = 0;
	int i = t_max(0, (int)ptXY.x - (int)_MAX_PATH);	// 2009.05.22 ryoji 200->_MAX_PATH
	//nLineLen = __min(nLineLen, ptXY.x + _MAX_PATH);
	while (i <= ptXY.x && i < (int)nLineLen) {
		bool bMatch = (bUseRegexKeyword
					&& pRegexKeyword->RegexIsKeyword(StringRef(pLine, nLineLen), i, &nUrlLen, &nMatchColor)
					&& nMatchColor == COLORIDX_URL);
		if (!bMatch) {
			bMatch = (bDispUrl
						&& (i == 0 || !IS_KEYWORD_CHAR(pLine[i - 1]))	// 2009.05.22 ryoji CColor_Url::BeginColor()�Ɠ�������
						&& IsURL(&pLine[i], (nLineLen - i), &nUrlLen));	// �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ�
		}
		if (bMatch) {
			if (i <= ptXY.x && ptXY.x < i + (int)nUrlLen) {
				// URL��Ԃ��ꍇ
				if (pwstrURL) {
					pwstrURL->assign(&pLine[i], nUrlLen);
				}
				pUrlRange->SetLine(ptXY.y);
				pUrlRange->SetXs(i, i + nUrlLen);
				return true;
			}else {
				i += nUrlLen;
				continue;
			}
		}
		++i;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �C�x���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

VOID EditView::OnTimer(
	HWND hwnd,			// handle of window for timer messages
	UINT uMsg,			// WM_TIMER message
	UINT_PTR idEvent,	// timer identifier
	DWORD dwTime		// current system time
	)
{
	if (GetDllShareData().common.edit.bUseOLE_DragDrop) {	// OLE�ɂ��h���b�O & �h���b�v���g��
		if (IsDragSource()) {
			return;
		}
	}
	POINT po;
	RECT rc;
	// �͈͑I�𒆂łȂ��ꍇ
	if (!GetSelectionInfo().IsMouseSelecting()) {
		if (bMiniMap) {
			bool bHide;
			if (MiniMapCursorLineTip(&po, &rc, &bHide)) {
				tipWnd.bAlignLeft = true;
				tipWnd.Show( po.x, po.y + editWnd.GetActiveView().GetTextMetrics().GetHankakuHeight(), NULL );
			}else {
				if (bHide && dwTipTimer == 0) {
					tipWnd.Hide();
				}
			}
		}else {
			if (KeywordHelpSearchDict(LID_SKH_ONTIMER, &po, &rc)) {	// 2006.04.10 fon
				// ����Tip��\��
				tipWnd.Show( po.x, po.y + GetTextMetrics().GetHankakuHeight(), NULL );
			}
		}
	}else {
		::GetCursorPos(&po);
		::GetWindowRect(GetHwnd(), &rc);
		if (!PtInRect(&rc, po)) {
			OnMOUSEMOVE(0, GetSelectionInfo().ptMouseRollPosOld.x, GetSelectionInfo().ptMouseRollPosOld.y);
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �ϊ�                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �I���G���A�̃e�L�X�g���w����@�ŕϊ�
void EditView::ConvSelectedArea(EFunctionCode nFuncCode)
{
	// �e�L�X�g���I������Ă��邩
	if (!GetSelectionInfo().IsTextSelected()) {
		return;
	}

	NativeW memBuf;

	Point sPos;

	int	nLineNum;
	int	nDelLen;
	size_t nLineLen;
	size_t nLineLen2;
	WaitCursor waitCursor(GetHwnd());

	// 2009.07.18 ryoji Logic�ŋL������悤�ɕύX
	Point ptFromLogic = pEditDoc->layoutMgr.LayoutToLogic(GetSelectionInfo().select.GetFrom());

	// ��`�͈͑I�𒆂�
	if (GetSelectionInfo().IsBoxSelecting()) {

		// 2�_��Ίp�Ƃ����`�����߂�
		Rect rcSelLayout;
		TwoPointToRect(
			&rcSelLayout,
			GetSelectionInfo().select.GetFrom(),	// �͈͑I���J�n
			GetSelectionInfo().select.GetTo()		// �͈͑I���I��
		);

		// ���݂̑I��͈͂��I����Ԃɖ߂�
		GetSelectionInfo().DisableSelectArea(false);	// 2009.07.18 ryoji true -> false �e�s�ɃA���_�[���C�����c����̏C��

		size_t nIdxFrom = 0;
		size_t nIdxTo = 0;
		for (nLineNum=rcSelLayout.bottom; nLineNum>=rcSelLayout.top-1; --nLineNum) {
			const Layout* pLayout;
			size_t nDelPosNext = nIdxFrom;
			size_t nDelLenNext = nIdxTo - nIdxFrom;
			const wchar_t* pLine = pEditDoc->layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
			if (pLine) {
				// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
				nIdxFrom	= LineColumnToIndex(pLayout, rcSelLayout.left);
				nIdxTo		= LineColumnToIndex(pLayout, rcSelLayout.right);

				bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
				for (size_t i=nIdxFrom; i<=nIdxTo; ++i) {
					if (WCODE::IsLineDelimiter(pLine[i], bExtEol)) {
						nIdxTo = i;
						break;
					}
				}
			}else {
				nIdxFrom = 0;
				nIdxTo = 0;
			}
			size_t nDelPos = nDelPosNext;
			nDelLen	= nDelLenNext;
			if (nLineNum < rcSelLayout.bottom && 0 < nDelLen) {
				pEditDoc->layoutMgr.GetLineStr(nLineNum + 1, &nLineLen2, &pLayout);
				sPos.Set(
					LineIndexToColumn(pLayout, nDelPos),
					nLineNum + 1
				);

				// 2007.10.18 COpe����
				// �w��ʒu�̎w�蒷�f�[�^�폜
				DeleteData2(
					sPos,
					nDelLen,
					&memBuf
				);
				
				{
					// �@�\��ʂɂ��o�b�t�@�̕ϊ�
					ConvertMediator::ConvMemory(&memBuf, nFuncCode, pEditDoc->layoutMgr.GetTabSpace(), sPos.x);

					// ���݈ʒu�Ƀf�[�^��}��
					Point ptLayoutNew;	// �}�����ꂽ�����̎��̈ʒu
					InsertData_CEditView(
						sPos,
						memBuf.GetStringPtr(),
						memBuf.GetStringLength(),
						&ptLayoutNew,
						false	// 2009.07.18 ryoji true -> false �e�s�ɃA���_�[���C�����c����̏C��
					);

					// �J�[�\�����ړ�
					GetCaret().MoveCursor(ptLayoutNew, false);
					GetCaret().nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;
				}
			}
		}
		// �}���f�[�^�̐擪�ʒu�փJ�[�\�����ړ�
		GetCaret().MoveCursor(rcSelLayout.UpperLeft(), true);
		GetCaret().nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;

		if (!bDoing_UndoRedo) {	// Undo, Redo�̎��s����
			// ����̒ǉ�
			commander.GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
				)
			);
		}
	}else {
		// �I��͈͂̃f�[�^���擾
		// ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ�
		GetSelectedDataSimple(memBuf);

		// �@�\��ʂɂ��o�b�t�@�̕ϊ�
		ConvertMediator::ConvMemory(&memBuf, nFuncCode, pEditDoc->layoutMgr.GetTabSpace(), GetSelectionInfo().select.GetFrom().x);

		// �f�[�^�u�� �폜&�}���ɂ��g����
		ReplaceData_CEditView(
			GetSelectionInfo().select,
			memBuf.GetStringPtr(),		// �}������f�[�^	 // 2002/2/10 aroka CMemory�ύX
			memBuf.GetStringLength(),	// �}������f�[�^�̒��� // 2002/2/10 aroka CMemory�ύX
			false,
			bDoing_UndoRedo ? nullptr : commander.GetOpeBlk()
		);

		// From Here 2001.12.03 hor
		// �I���G���A�̕���
		// 2009.07.18 ryoji Logic����Layout�ɖ߂�
		Point ptFrom = pEditDoc->layoutMgr.LogicToLayout(ptFromLogic);
		GetSelectionInfo().SetSelectArea(Range(ptFrom, GetCaret().GetCaretLayoutPos()));	// 2009.07.25 ryoji
		GetCaret().MoveCursor(GetSelectionInfo().select.GetTo(), true);
		GetCaret().nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;

		if (!bDoing_UndoRedo) {	// Undo, Redo�̎��s����
			// ����̒ǉ�
			commander.GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// ����O��̃L�����b�g�ʒu
				)
			);
		}
		// To Here 2001.12.03 hor
	}
	RedrawAll();	// 2009.07.18 ryoji �Ώۂ���`�������ꍇ���Ō�ɍĕ`�悷��
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ���j���[                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �|�b�v�A�b�v���j���[(�E�N���b�N)
int	EditView::CreatePopUpMenu_R(void)
{
	MenuDrawer& menuDrawer = editWnd.GetMenuDrawer();
	menuDrawer.ResetContents();

	// �E�N���b�N���j���[�̒�`�̓J�X�^�����j���[�z���0�Ԗ�
	int nMenuIdx = CUSTMENU_INDEX_FOR_RBUTTONUP;	// �}�W�b�N�i���o�[�r��	//@@@ 2003.06.13 MIK

	// Note: ViewCommander::Command_CustMenu �Ƒ�̓���
	HMENU hMenu = ::CreatePopupMenu();

	// 2010.07.24 Moca �I�[�i�[�h���[�Ή��̂��߂ɑO�Ɉړ�����MenuDrawer�o�R�Œǉ�����
	if (!GetSelectionInfo().IsMouseSelecting()) {
		POINT po;
		RECT rc;
		if (KeywordHelpSearchDict(LID_SKH_POPUPMENU_R, &po, &rc)) {	// 2006.04.10 fon
			menuDrawer.MyAppendMenu(hMenu, 0, IDM_COPYDICINFO, LS(STR_MENU_KEYWORDINFO), _T("K"));	// 2006.04.10 fon ToolTip���e�𒼐ڕ\������̂���߂�
			menuDrawer.MyAppendMenu(hMenu, 0, IDM_JUMPDICT, LS(STR_MENU_OPENKEYWORDDIC), _T("L"));	// 2006.04.10 fon
			menuDrawer.MyAppendMenuSep(hMenu, MF_SEPARATOR, F_0, _T(""));
		}
	}
	return CreatePopUpMenuSub(hMenu, nMenuIdx, nullptr);
}

/*! �|�b�v�A�b�v���j���[�̍쐬(Sub)
	hMenu�͍쐬�ς�
	@date 2013.06.15 �V�K�쐬 �|�b�v�A�b�v���j���[�ƃ��C�����j���[�̕\�����@�𓝍�
*/
int	EditView::CreatePopUpMenuSub(HMENU hMenu, int nMenuIdx, int* pParentMenus)
{
	WCHAR szLabel[300];

	MenuDrawer& menuDrawer = editWnd.GetMenuDrawer();
	FuncLookup& funcLookup = pEditDoc->funcLookup;

	int nParamIndex = 0;
	int nParentMenu[MAX_CUSTOM_MENU + 1];
	int* pNextParam = nParentMenu;
	{
		if (pParentMenus) {
			int k;
			for (k=0; pParentMenus[k]!=0; ++k) {
			}
			nParamIndex = k;
			pNextParam = pParentMenus;
		}else {
			memset_raw(nParentMenu, 0, sizeof(nParentMenu));
		}
		EFunctionCode nThisCode = F_0;
		if (nMenuIdx == CUSTMENU_INDEX_FOR_RBUTTONUP) {
			nThisCode = F_MENU_RBUTTON;
		}else {
			nThisCode = EFunctionCode(nMenuIdx + F_CUSTMENU_1 - 1);
		}
		pNextParam[nParamIndex] = nThisCode;
	}
	auto& csCustomMenu = GetDllShareData().common.customMenu;
	for (int i=0; i<csCustomMenu.nCustMenuItemNumArr[nMenuIdx]; ++i) {
		EFunctionCode code = csCustomMenu.nCustMenuItemFuncArr[nMenuIdx][i];
		bool bAppend = false;
		if (code == F_0) {
			// 2010.07.24 ���j���[�z��ɓ����
			menuDrawer.MyAppendMenuSep(hMenu, MF_SEPARATOR, F_0, _T(""));
			bAppend = true;
		}else if (code == F_MENU_RBUTTON || (F_CUSTMENU_1 <= code && code <= F_CUSTMENU_LAST)) {
			int nCustIdx = 0;
			if (code == F_MENU_RBUTTON) {
				nCustIdx = CUSTMENU_INDEX_FOR_RBUTTONUP;
			}else {
				nCustIdx = code - F_CUSTMENU_1 + 1;
			}
			bool bMenuLoop = !csCustomMenu.bCustMenuPopupArr[nCustIdx];
			if (!bMenuLoop) {
				for (int k=0; pNextParam[k]!=0; ++k) {
					if (pNextParam[k] == code) {
						bMenuLoop = true;
						break;
					}
				}
			}
			if (!bMenuLoop) {
				WCHAR buf[MAX_CUSTOM_MENU_NAME_LEN + 1];
				LPCWSTR p = GetDocument().funcLookup.Custmenu2Name(nCustIdx, buf, _countof(buf));
				wchar_t keys[2];
				keys[0] = csCustomMenu.nCustMenuItemKeyArr[nMenuIdx][i];
				keys[1] = 0;
				HMENU hMenuPopUp = ::CreatePopupMenu();
				menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , p, keys);
				CreatePopUpMenuSub(hMenuPopUp, nCustIdx, pNextParam);
				bAppend = true;
			}else {
				// ���[�v���Ă���Ƃ��́A�]�����l�ʂŕ\��
			}
		}
		if (!bAppend) {
			//	Oct. 3, 2001 genta
			funcLookup.Funccode2Name(code, szLabel, 256);
			// �L�[
			if (F_SPECIAL_FIRST <= code && code <= F_SPECIAL_LAST) {
				editWnd.InitMenu_Special(hMenu, code);
			}else {
				wchar_t keys[2];
				keys[0] = csCustomMenu.nCustMenuItemKeyArr[nMenuIdx][i];
				keys[1] = 0;
				editWnd.InitMenu_Function(hMenu, code, szLabel, keys);
			}
		}
	}

	pNextParam[nParamIndex] = 0;
	if (pParentMenus) {
		// ��͐e�ɏ������Ă��炤
		return -1;
	}

	int cMenuItems = ::GetMenuItemCount(hMenu);
	for (int nPos=0; nPos<cMenuItems; ++nPos) {
		EFunctionCode id = (EFunctionCode)::GetMenuItemID(hMenu, nPos);
		UINT fuFlags;
		// �@�\�����p�\�����ׂ�
		//	Jan.  8, 2006 genta �@�\���L���ȏꍇ�ɂ͖����I�ɍĐݒ肵�Ȃ��悤�ɂ���D
		if (!IsFuncEnable(GetDocument(), GetDllShareData(), id)) {
			fuFlags = MF_BYCOMMAND | MF_GRAYED;
			::EnableMenuItem(hMenu, id, fuFlags);
		}

		// �@�\���`�F�b�N��Ԃ����ׂ�
		if (IsFuncChecked(GetDocument(), GetDllShareData(), id)) {
			fuFlags = MF_BYCOMMAND | MF_CHECKED;
			::CheckMenuItem(hMenu, id, fuFlags);
		}
	}

	POINT po;
	po.x = 0;
	po.y = 0;
	::GetCursorPos(&po);
	po.y -= 4;
	int nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		::GetParent(hwndParent)/*GetHwnd()*/,
		NULL
	);
	::DestroyMenu(hMenu);
	return nId;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �ݒ蔽�f                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �ݒ�ύX�𔽉f������
void EditView::OnChangeSetting()
{
	if (!GetHwnd()) {
		return;
	}
	auto& csWindow = GetDllShareData().common.window;
	auto& textArea = GetTextArea();
	textArea.SetTopYohaku(csWindow.nRulerBottomSpace); 	// ���[���[�ƃe�L�X�g�̌���
	textArea.SetAreaTop(textArea.GetTopYohaku());									// �\����̏�[���W

	// ������ʍX�V
	pTypeData = &pEditDoc->docType.GetDocumentAttribute();

	// ���[���[�\��
	if (pTypeData->colorInfoArr[COLORIDX_RULER].bDisp && !bMiniMap) {
		textArea.SetAreaTop(textArea.GetAreaTop() + csWindow.nRulerHeight);	// ���[���[����
	}
	textArea.SetLeftYohaku(csWindow.nLineNumRightSpace);

	// �t�H���g�̕ύX
	SetFont();

	// �t�H���g���ς���Ă��邩������Ȃ��̂ŁA�J�[�\���ړ�
	// �X�N���[���o�[���ړ�����̂ŌĂяo�����ł��
	// GetCaret().MoveCursor(GetCaret().GetCaretLayoutPos(), true);

	// �X�N���[���o�[�̏�Ԃ��X�V����
	// AdjustScrollBars();

	//	2007.09.30 genta ��ʃL���b�V���pCompatibleDC��p�ӂ���
	UseCompatibleDC(csWindow.bUseCompatibleBMP);

	// �E�B���h�E�T�C�Y�̕ύX����
	RECT rc;
	::GetClientRect(GetHwnd(), &rc);
	OnSize(rc.right, rc.bottom);

	// �t�H���g���ς����
	tipWnd.ChangeFont(&(GetDllShareData().common.helper.lf));
	
	// �ĕ`��
	if (!editWnd.pPrintPreview) {
		::InvalidateRect(GetHwnd(), NULL, TRUE);
	}
	TypeSupport textType(*this, COLORIDX_TEXT);
	crBack = textType.GetBackColor();
}


// �����̕\����Ԃ𑼂̃r���[�ɃR�s�[
void EditView::CopyViewStatus(EditView* pView) const
{
	if (!pView) {
		return;
	}
	if (pView == this) {
		return;
	}

	// ���͏��
	GetCaret().CopyCaretStatus(&pView->GetCaret());

	// �I�����
	GetSelectionInfo().CopySelectStatus(&pView->GetSelectionInfo());

	// ��ʏ��
	GetTextArea().CopyTextAreaStatus(&pView->GetTextArea());

	// �\�����@
	GetTextMetrics().CopyTextMetricsStatus(&pView->GetTextMetrics());
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �����{�b�N�X                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �c�E���̕����{�b�N�X�E�T�C�Y�{�b�N�X�̂n�m�^�n�e�e
void EditView::SplitBoxOnOff(bool bVert, bool bHorz, bool bSizeBox)
{
	RECT	rc;
	if (bVert) {
		if (!pcsbwVSplitBox) {	// ���������{�b�N�X
			pcsbwVSplitBox = new SplitBoxWnd;
			pcsbwVSplitBox->Create(G_AppInstance(), GetHwnd(), TRUE);
		}
	}else {
		SAFE_DELETE(pcsbwVSplitBox);	// ���������{�b�N�X
	}
	if (bHorz) {
		if (!pcsbwHSplitBox) {	// ���������{�b�N�X
			pcsbwHSplitBox = new SplitBoxWnd;
			pcsbwHSplitBox->Create(G_AppInstance(), GetHwnd(), FALSE);
		}
	}else {
		SAFE_DELETE(pcsbwHSplitBox);	// ���������{�b�N�X
	}

	if (bSizeBox) {
		if (hwndSizeBox) {
			::DestroyWindow(hwndSizeBox);
			hwndSizeBox = NULL;
		}
		hwndSizeBox = ::CreateWindowEx(
			0L,													// no extended styles
			_T("SCROLLBAR"),									// scroll bar control class
			NULL,												// text for window title bar
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, // scroll bar styles
			0,													// horizontal position
			0,													// vertical position
			200,												// width of the scroll bar
			CW_USEDEFAULT,										// default height
			GetHwnd(),											// handle of main window
			(HMENU) NULL,										// no menu for a scroll bar
			G_AppInstance(),									// instance owning this window
			(LPVOID) NULL										// pointer not needed
		);
	}else {
		if (hwndSizeBox) {
			::DestroyWindow(hwndSizeBox);
			hwndSizeBox = NULL;
		}
		hwndSizeBox = ::CreateWindowEx(
			0L,														// no extended styles
			_T("STATIC"),											// scroll bar control class
			NULL,													// text for window title bar
			WS_VISIBLE | WS_CHILD /*| SBS_SIZEBOX | SBS_SIZEGRIP*/, // scroll bar styles
			0,														// horizontal position
			0,														// vertical position
			200,													// width of the scroll bar
			CW_USEDEFAULT,											// default height
			GetHwnd(),												// handle of main window
			(HMENU) NULL,											// no menu for a scroll bar
			G_AppInstance(),										// instance owning this window
			(LPVOID) NULL											// pointer not needed
		);
	}
	::ShowWindow(hwndSizeBox, SW_SHOW);

	::GetClientRect(GetHwnd(), &rc);
	OnSize(rc.right, rc.bottom);
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �e�L�X�g�I��                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* �I��͈͂̃f�[�^���擾
	���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ�
*/
bool EditView::GetSelectedDataSimple(NativeW& memBuf)
{
	return GetSelectedData(&memBuf, false, NULL, false, false, EolType::Unknown);
}

/* �I��͈͂̃f�[�^���擾
	���펞��TRUE,�͈͖��I���̏ꍇ�� false ��Ԃ�
*/
bool EditView::GetSelectedData(
	NativeW*		memBuf,
	bool			bLineOnly,
	const wchar_t*	pszQuote,			// �擪�ɕt������p��
	bool			bWithLineNumber,	// �s�ԍ���t�^����
	bool			bAddCRLFWhenCopy,	// �܂�Ԃ��ʒu�ŉ��s�L��������
	EolType			neweol				// �R�s�[��̉��s�R�[�h EolType::None�̓R�[�h�ۑ�
	)
{
	size_t			nLineLen;
	size_t			nLineNum;
	size_t			nIdxFrom;
	size_t			nIdxTo;
	size_t			nRowNum;
	size_t			nLineNumCols = 0;
	wchar_t*		pszLineNum = NULL;
	const wchar_t*	pszSpaces = L"                    ";
	const Layout*	pLayout;
	Eol				appendEol(neweol);

	// �͈͑I��������Ă��Ȃ�
	if (!GetSelectionInfo().IsTextSelected()) {
		return false;
	}
	if (bWithLineNumber) {	// �s�ԍ���t�^����
		// �s�ԍ��\���ɕK�v�Ȍ������v�Z
		// 2014.11.30 ���̓��C�A�E�g�P�ʂł���K�v������
		nLineNumCols = GetTextArea().DetectWidthOfLineNumberArea_calculate(&pEditDoc->layoutMgr, true);
		nLineNumCols += 1;
		pszLineNum = new wchar_t[nLineNumCols + 1];
	}

	if (GetSelectionInfo().IsBoxSelecting()) {	// ��`�͈͑I��
		Rect rcSel;
		// 2�_��Ίp�Ƃ����`�����߂�
		TwoPointToRect(
			&rcSel,
			GetSelectionInfo().select.GetFrom(),	// �͈͑I���J�n
			GetSelectionInfo().select.GetTo()		// �͈͑I���I��
		);
		memBuf->SetString(L"");

		//<< 2002/04/18 Azumaiya
		// �T�C�Y�������v�̂��Ƃ��Ă����B
		// ���\��܂��Ɍ��Ă��܂��B
		int i = rcSel.bottom - rcSel.top + 1; // 2013.05.06 �u+1�v

		// �ŏ��ɍs�����̉��s�ʂ��v�Z���Ă��܂��B
		size_t nBufSize = wcslen(WCODE::CRLF) * i;

		// ���ۂ̕����ʁB
		const wchar_t* pLine = pEditDoc->layoutMgr.GetLineStr(rcSel.top, &nLineLen, &pLayout);
		for (; i!=0 && pLayout; --i, pLayout=pLayout->GetNextLayout()) {
			pLine = pLayout->GetPtr() + pLayout->GetLogicOffset();
			nLineLen = pLayout->GetLengthWithEOL();
			if (pLine) {
				// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
				nIdxFrom	= LineColumnToIndex(pLayout, rcSel.left );
				nIdxTo		= LineColumnToIndex(pLayout, rcSel.right);
				ASSERT_GE(nIdxTo, nIdxFrom);
				nBufSize += nIdxTo - nIdxFrom;
			}
			if (bLineOnly) {	// �����s�I���̏ꍇ�͐擪�̍s�̂�
				break;
			}
		}

		// ��܂��Ɍ����e�ʂ����ɃT�C�Y�����炩���ߊm�ۂ��Ă����B
		memBuf->AllocStringBuffer(nBufSize);
		//>> 2002/04/18 Azumaiya

		bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
		nRowNum = 0;
		for (nLineNum=rcSel.top; (int)nLineNum<=rcSel.bottom; ++nLineNum) {
			const wchar_t* pLine = pEditDoc->layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
			if (pLine) {
				// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
				nIdxFrom	= LineColumnToIndex(pLayout, rcSel.left );
				nIdxTo		= LineColumnToIndex(pLayout, rcSel.right);
				// 2002.02.08 hor
				// pLine��NULL�̂Ƃ�(��`�G���A�̒[��EOF�݂̂̍s���܂ނƂ�)�͈ȉ����������Ȃ�
				ASSERT_GE(nIdxTo, nIdxFrom);
				if (nIdxTo - nIdxFrom > 0) {
					if (WCODE::IsLineDelimiter(pLine[nIdxTo - 1], bExtEol)) {
						memBuf->AppendString(&pLine[nIdxFrom], nIdxTo - nIdxFrom - 1);
					}else {
						memBuf->AppendString(&pLine[nIdxFrom], nIdxTo - nIdxFrom);
					}
				}
			}
			++nRowNum;
			memBuf->AppendStringLiteral(WCODE::CRLF);
			if (bLineOnly) {	// �����s�I���̏ꍇ�͐擪�̍s�̂�
				break;
			}
		}
	}else {
		memBuf->SetString(L"");

		//<< 2002/04/18 Azumaiya
		//  ���ꂩ��\��t���Ɏg���̈�̑�܂��ȃT�C�Y���擾����B
		//  ��܂��Ƃ������x���ł��̂ŁA�T�C�Y�v�Z�̌덷���i�e�ʂ𑽂����ς�����Ɂj���\�o��Ǝv���܂����A
		// �܂��A�����D��Ƃ������ƂŊ��ق��Ă��������B
		//  ���ʂȗe�ʊm�ۂ��o�Ă��܂��̂ŁA�����������x���グ�����Ƃ���ł����E�E�E�B
		//  �Ƃ͂����A�t�ɏ��������ς��邱�ƂɂȂ��Ă��܂��ƁA���Ȃ葬�x���Ƃ���v���ɂȂ��Ă��܂��̂�
		// �����Ă��܂��Ƃ���ł����E�E�E�B
		pEditDoc->layoutMgr.GetLineStr(GetSelectionInfo().select.GetFrom().y, &nLineLen, &pLayout);
		int nBufSize = 0;

		int i = (GetSelectionInfo().select.GetTo().y - GetSelectionInfo().select.GetFrom().y);

		// �擪�Ɉ��p����t����Ƃ��B
		if (pszQuote) {
			nBufSize += wcslen(pszQuote);
		}

		// �s�ԍ���t����B
		if (bWithLineNumber) {
			nBufSize += nLineNumCols;
		}

		// ���s�R�[�h�ɂ��āB
		if (neweol == EolType::Unknown) {
			nBufSize += wcslen(WCODE::CRLF);
		}else {
			nBufSize += appendEol.GetLen();
		}

		// ���ׂĂ̍s�ɂ��ē��l�̑��������̂ŁA�s���{����B
		nBufSize *= i;

		// ���ۂ̊e�s�̒����B
		for (; i!=0 && pLayout; --i, pLayout=pLayout->GetNextLayout()) {
			nBufSize += pLayout->GetLengthWithoutEOL() + appendEol.GetLen();
			if (bLineOnly) {	// �����s�I���̏ꍇ�͐擪�̍s�̂�
				break;
			}
		}

		// ���ׂ������������o�b�t�@������Ă����B
		memBuf->AllocStringBuffer(nBufSize);
		//>> 2002/04/18 Azumaiya

		for (nLineNum=GetSelectionInfo().select.GetFrom().y; (int)nLineNum<=GetSelectionInfo().select.GetTo().y; ++nLineNum) {
			const wchar_t* pLine = pEditDoc->layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
			if (!pLine) {
				break;
			}
			if (nLineNum == GetSelectionInfo().select.GetFrom().y) {
				// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
				nIdxFrom = LineColumnToIndex(pLayout, GetSelectionInfo().select.GetFrom().x);
			}else {
				nIdxFrom = 0;
			}
			if (nLineNum == GetSelectionInfo().select.GetTo().y) {
				// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
				nIdxTo = LineColumnToIndex(pLayout, GetSelectionInfo().select.GetTo().x);
			}else {
				nIdxTo = nLineLen;
			}
			ASSERT_GE(nIdxTo, nIdxFrom);
			if (nIdxTo - nIdxFrom == 0) {
				continue;
			}

			if (pszQuote && pszQuote[0] != L'\0') {	// �擪�ɕt������p��
				memBuf->AppendString(pszQuote);
			}
			if (bWithLineNumber) {	// �s�ԍ���t�^����
				auto_sprintf(pszLineNum, L" %d:" , nLineNum + 1);
				memBuf->AppendString(pszSpaces, nLineNumCols - wcslen(pszLineNum));
				memBuf->AppendString(pszLineNum);
			}

			if (EolType::None != pLayout->GetLayoutEol()) {
				if (nIdxTo >= nLineLen) {
					memBuf->AppendString(&pLine[nIdxFrom], nLineLen - 1 - nIdxFrom);
					//	Jul. 25, 2000 genta
					memBuf->AppendString((neweol == EolType::Unknown) ?
						(pLayout->GetLayoutEol()).GetValue2() :	//	�R�[�h�ۑ�
						appendEol.GetValue2());			//	�V�K���s�R�[�h
				}else {
					memBuf->AppendString(&pLine[nIdxFrom], nIdxTo - nIdxFrom);
				}
			}else {
				memBuf->AppendString(&pLine[nIdxFrom], nIdxTo - nIdxFrom);
				if (nIdxTo >= nLineLen) {
					if (bAddCRLFWhenCopy ||  // �܂�Ԃ��s�ɉ��s��t���ăR�s�[
						pszQuote || // �擪�ɕt������p��
						bWithLineNumber 	// �s�ԍ���t�^����
					) {
						//	Jul. 25, 2000 genta
						memBuf->AppendString((neweol == EolType::Unknown) ?
							pEditDoc->docEditor.GetNewLineCode().GetValue2() :	//	�R�[�h�ۑ�
							appendEol.GetValue2());		//	�V�K���s�R�[�h
					}
				}
			}
			if (bLineOnly) {	// �����s�I���̏ꍇ�͐擪�̍s�̂�
				break;
			}
		}
	}
	if (bWithLineNumber) {	// �s�ԍ���t�^����
		delete[] pszLineNum;
	}
	return true;
}

/* �I��͈͓��̂P�s�̑I��
	@param bCursorPos �I���J�n�s�̑���ɃJ�[�\���ʒu�̍s���擾
	�ʏ�I���Ȃ烍�W�b�N�s�A��`�Ȃ�I��͈͓��̃��C�A�E�g�s�P�s��I��
	2010.09.04 Moca �V�K�쐬
*/
bool EditView::GetSelectedDataOne(NativeW& memBuf, size_t nMaxLen)
{
	size_t	nLineLen;
	size_t	nIdxFrom;
	size_t	nIdxTo;
	size_t	nSelectLen;
	auto& layoutMgr = pEditDoc->layoutMgr;
	auto& selInfo = GetSelectionInfo();

	if (!selInfo.IsTextSelected()) {
		return false;
	}

	auto& select = selInfo.select;

	memBuf.SetString(L"");
	if (selInfo.IsBoxSelecting()) {
		// ��`�͈͑I��(���C�A�E�g����)
		const Layout* pLayout;
		Rect rcSel;

		// 2�_��Ίp�Ƃ����`�����߂�
		TwoPointToRect(
			&rcSel,
			select.GetFrom(),	// �͈͑I���J�n
			select.GetTo()	// �͈͑I���I��
		);
		const wchar_t* pLine = layoutMgr.GetLineStr(rcSel.top, &nLineLen, &pLayout);
		if (pLine && pLayout) {
			nLineLen = pLayout->GetLengthWithoutEOL();
			// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
			nIdxFrom = LineColumnToIndex(pLayout, rcSel.left );
			nIdxTo = LineColumnToIndex(pLayout, rcSel.right);
			ASSERT_GE(nIdxTo, nIdxFrom);
			ASSERT_GE(nLineLen, nIdxFrom);
			nSelectLen = nIdxTo - nIdxFrom;
			if (0 < nSelectLen) {
				memBuf.AppendString(&pLine[nIdxFrom], t_min(nMaxLen, t_min(nSelectLen, nLineLen - nIdxFrom)));
			}
		}
	}else {
		// ���`�I��(���W�b�N�s����)
		Point ptFrom = layoutMgr.LayoutToLogic(select.GetFrom());
		Point ptTo = layoutMgr.LayoutToLogic(select.GetTo());
		int targetY = ptFrom.y;

		const DocLine* pDocLine = pEditDoc->docLineMgr.GetLine(targetY);
		if (pDocLine) {
			const wchar_t* pLine = pDocLine->GetPtr();
			nLineLen = pDocLine->GetLengthWithoutEOL();
			nIdxFrom = ptFrom.x;
			if (targetY == ptTo.y) {
				nIdxTo = ptTo.x;
			}else {
				nIdxTo = nLineLen;
			}
			ASSERT_GE(nIdxTo, nIdxFrom);
			ASSERT_GE(nLineLen, nIdxFrom);
			nSelectLen = nIdxTo - nIdxFrom;
			if (0 < nSelectLen) {
				memBuf.AppendString(&pLine[nIdxFrom], t_min(nMaxLen, t_min(nSelectLen, nLineLen - nIdxFrom)));
			}
		}
	}
	return 0 < memBuf.GetStringLength();
}

/* �w��J�[�\���ʒu���I���G���A���ɂ��邩
	�y�߂�l�z
	-1	�I���G���A���O�� or ���I��
	0	�I���G���A��
	1	�I���G���A�����
*/
int EditView::IsCurrentPositionSelected(
	Point	ptCaretPos		// �J�[�\���ʒu
	)
{
	auto& selInfo = GetSelectionInfo();
	if (!selInfo.IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		return -1;
	}
	Rect	rcSel;
	Point	po;
	auto& select = selInfo.select;

	// ��`�͈͑I�𒆂�
	if (selInfo.IsBoxSelecting()) {
		// 2�_��Ίp�Ƃ����`�����߂�
		TwoPointToRect(
			&rcSel,
			select.GetFrom(),	// �͈͑I���J�n
			select.GetTo()		// �͈͑I���I��
		);
		++rcSel.bottom;
		po = ptCaretPos;
		if (IsDragSource()) {
			if (GetKeyState_Control()) { // Ctrl�L�[��������Ă�����
				++rcSel.left;
			}else {
				++rcSel.right;
			}
		}
		if (rcSel.PtInRect(po)) {
			return 0;
		}
		if (rcSel.top > ptCaretPos.y) {
			return -1;
		}
		if (rcSel.bottom < ptCaretPos.y) {
			return 1;
		}
		if (rcSel.left > ptCaretPos.x) {
			return -1;
		}
		if (rcSel.right < ptCaretPos.x) {
			return 1;
		}
	}else {
		if (select.GetFrom().y > ptCaretPos.y) {
			return -1;
		}
		if (select.GetTo().y < ptCaretPos.y) {
			return 1;
		}
		if (select.GetFrom().y == ptCaretPos.y) {
			if (IsDragSource()) {
				if (GetKeyState_Control()) {	// Ctrl�L�[��������Ă�����
					if (select.GetFrom().x >= ptCaretPos.x) {
						return -1;
					}
				}else {
					if (select.GetFrom().x > ptCaretPos.x) {
						return -1;
					}
				}
			}else if (select.GetFrom().x > ptCaretPos.x) {
				return -1;
			}
		}
		if (select.GetTo().y == ptCaretPos.y) {
			if (IsDragSource()) {
				if (GetKeyState_Control()) {	// Ctrl�L�[��������Ă�����
					if (select.GetTo().x <= ptCaretPos.x) {
						return 1;
					}
				}else {
					if (select.GetTo().x < ptCaretPos.x) {
						return 1;
					}
				}
			}else if (select.GetTo().x <= ptCaretPos.x) {
				return 1;
			}
		}
		return 0;
	}
	return -1;
}

/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 (�e�X�g)
	�y�߂�l�z
	-1	�I���G���A���O�� or ���I��
	0	�I���G���A��
	1	�I���G���A�����
*/
// 2007.09.01 kobake ����
int EditView::IsCurrentPositionSelectedTEST(
	const Point& ptCaretPos,	// �J�[�\���ʒu
	const Range& select		//
	) const
{
	if (!GetSelectionInfo().IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		return -1;
	}

	if (PointCompare(ptCaretPos, select.GetFrom()) < 0) return -1;
	if (PointCompare(ptCaretPos, select.GetTo()) >= 0) return 1;

	return 0;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �N���b�v�{�[�h                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[����
void EditView::CopySelectedAllLines(
	const wchar_t*	pszQuote,		// �擪�ɕt������p��
	bool			bWithLineNumber	// �s�ԍ���t�^����
	)
{
	NativeW	memBuf;

	if (!GetSelectionInfo().IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		return;
	}
	{	// �I��͈͓��̑S�s��I����Ԃɂ���
		Range sSelect(GetSelectionInfo().select);
		const Layout* pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(sSelect.GetFrom().y);
		if (!pLayout) return;
		sSelect.SetFromX(pLayout->GetIndent());
		pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(sSelect.GetTo().y);
		if (pLayout && (GetSelectionInfo().IsBoxSelecting() || sSelect.GetTo().x > (int)pLayout->GetIndent())) {
			// �I��͈͂����s���܂Ŋg�傷��
			sSelect.SetToY(sSelect.GetTo().y + 1);
			pLayout = pLayout->GetNextLayout();
		}
		sSelect.SetToX(pLayout? pLayout->GetIndent(): 0);
		GetCaret().GetAdjustCursorPos(&sSelect.GetTo());	// EOF�s�𒴂��Ă�������W�C��

		GetSelectionInfo().DisableSelectArea(false); // 2011.06.03 true ��false
		GetSelectionInfo().SetSelectArea(sSelect);

		GetCaret().MoveCursor(GetSelectionInfo().select.GetTo(), false);
		GetCaret().ShowEditCaret();
	}
	// �ĕ`��
	//	::UpdateWindow();
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	Call_OnPaint((int)PaintAreaType::LineNumber | (int)PaintAreaType::Body, false);
	// To Here 2007.09.09 Moca
	// �I��͈͂��N���b�v�{�[�h�ɃR�s�[
	// �I��͈͂̃f�[�^���擾
	// ���펞��TRUE,�͈͖��I���̏ꍇ�͏I������
	if (!GetSelectedData(
			&memBuf,
			false,
			pszQuote, // ���p��
			bWithLineNumber, // �s�ԍ���t�^����
			GetDllShareData().common.edit.bAddCRLFWhenCopy // �܂�Ԃ��ʒu�ɉ��s�L��������
		)
	) {
		ErrorBeep();
		return;
	}
	// �N���b�v�{�[�h�Ƀf�[�^��ݒ�
	MySetClipboardData(memBuf.GetStringPtr(), memBuf.GetStringLength(), false);
}


/*! �N���b�v�{�[�h����f�[�^���擾
	@date 2005.05.29 novice UNICODE TEXT �Ή�������ǉ�
	@date 2007.10.04 ryoji MSDEVLineSelect�Ή�������ǉ�
	@date 2008.09.10 bosagami �p�X�\��t���Ή�
*/
bool EditView::MyGetClipboardData(
	NativeW& memBuf,
	bool* pbColumnSelect,
	bool* pbLineSelect /*= nullptr*/
	)
{
	if (pbColumnSelect) {
		*pbColumnSelect = false;
	}

	if (pbLineSelect) {
		*pbLineSelect = false;
	}

	if (!Clipboard::HasValidData()) {
		return false;
	}
	
	Clipboard clipboard(GetHwnd());
	if (!clipboard) {
		return false;
	}

	Eol eol = pEditDoc->docEditor.GetNewLineCode();
	if (!clipboard.GetText(&memBuf, pbColumnSelect, pbLineSelect, eol)) {
		return false;
	}

	return true;
}

/* �N���b�v�{�[�h�Ƀf�[�^��ݒ�
	@date 2004.02.17 Moca �G���[�`�F�b�N����悤��
 */
bool EditView::MySetClipboardData(
	const ACHAR* pszText,
	size_t nTextLen,
	bool bColumnSelect,
	bool bLineSelect /*= false*/
	)
{
	// WCHAR�ɕϊ�
	std::vector<wchar_t> buf;
	mbstowcs_vector(pszText, nTextLen, &buf);
	return MySetClipboardData(&buf[0], buf.size()-1, bColumnSelect, bLineSelect);
}

bool EditView::MySetClipboardData(
	const WCHAR* pszText,
	size_t nTextLen,
	bool bColumnSelect,
	bool bLineSelect /*= false*/
	)
{
	// Windows�N���b�v�{�[�h�ɃR�s�[
	Clipboard clipboard(GetHwnd());
	if (!clipboard) {
		return false;
	}
	clipboard.Empty();
	return clipboard.SetText(pszText, nTextLen, bColumnSelect, bLineSelect);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �A���_�[���C��                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*! �J�[�\���̏c���̍��W�ō��͈͂�
	@2010.08.21 �֐��ɕ����B�����Ή�
*/
inline bool EditView::IsDrawCursorVLinePos(int posX)
{
	auto& textArea = GetTextArea();
	return posX >= textArea.GetAreaLeft() - 2	// 2010.08.10 ryoji �e�L�X�g�ƍs�ԍ��̌��Ԃ����p���������傫���ƌ��Ԉʒu�ɂ���J�[�\���̏c�����`�悳�����C��
		&& posX >  textArea.GetAreaLeft() - GetDllShareData().common.window.nLineNumRightSpace // ����(+1)���Ȃ��Ƃ��͐��������Ȃ�����
		&& posX <= textArea.GetAreaRight();
}

// �J�[�\���s�A���_�[���C����ON
void EditView::CaretUnderLineON(
	bool bDraw,
	bool bDrawPaint,
	bool DisalbeUnderLine
	)
{
	bool bUnderLine = pTypeData->colorInfoArr[COLORIDX_UNDERLINE].bDisp;
	bool bCursorVLine = pTypeData->colorInfoArr[COLORIDX_CURSORVLINE].bDisp;
	bool bCursorLineBg = pTypeData->colorInfoArr[COLORIDX_CARETLINEBG].bDisp;
	if (!bUnderLine && !bCursorVLine && !bCursorLineBg) {
		return;
	}
	
//	2010.08.24 �����E�c�����c��o�O�̏C��
//	ON(���)��ON(�����)�̃p�^�[���ȂǂŁA��ʏ�ɐ����c�����܂܂ɂȂ�̂ō��W���N���A���Ă͂����Ȃ�
//	nOldCursorLineX = -1;
//	nOldUnderLineY  = -1;
	// 2011.12.06 Moca IsTextSelected �� IsTextSelecting �ɕύX�B���b�N����������\�����Ȃ�
	int bCursorLineBgDraw = false;
	auto& textArea = GetTextArea();
	
	// �J�[�\���s�̕`��
	if (1
		&& bDraw
		&& bCursorLineBg
		&& GetDrawSwitch()
		&& GetCaret().GetCaretLayoutPos().y >= textArea.GetViewTopLine()
		&& !bDoing_UndoRedo	// Undo, Redo�̎��s����
	) {
		bCursorLineBgDraw = true;

		nOldUnderLineY = GetCaret().GetCaretLayoutPos().y;
		nOldUnderLineYBg = nOldUnderLineY;
		nOldUnderLineYHeight = GetTextMetrics().GetHankakuDy();
		if (bDrawPaint) {
			GetCaret().underLine.Lock();
			PAINTSTRUCT ps;
			ps.rcPaint.left = 0;
			ps.rcPaint.right = textArea.GetAreaRight();
			ps.rcPaint.top = textArea.GenerateYPx(nOldUnderLineY);
			ps.rcPaint.bottom = ps.rcPaint.top + nOldUnderLineYHeight;

			// �`��
			HDC hdc = this->GetDC();
			OnPaint(hdc, &ps, FALSE);
			this->ReleaseDC(hdc);

			GetCaret().underLine.UnLock();
		}
	}
	
	int nCursorVLineX = -1;
	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	if (bCursorVLine) {
		// �J�[�\���ʒu�c���B-1���ăL�����b�g�̍��ɗ���悤�ɁB
		nCursorVLineX = textArea.GetAreaLeft() + (GetCaret().GetCaretLayoutPos().x - textArea.GetViewLeftCol())
			* (pTypeData->nColumnSpace + GetTextMetrics().GetHankakuWidth()) - 1;
	}

	if (1
		&& bDraw
		&& GetDrawSwitch()
		&& IsDrawCursorVLinePos(nCursorVLineX)
		&& !bDoing_UndoRedo
		&& !GetSelectionInfo().IsTextSelecting()
		&& !DisalbeUnderLine
	) {
		nOldCursorLineX = nCursorVLineX;
		// �J�[�\���ʒu�c���̕`��
		// �A���_�[���C���Əc���̌�_�ŁA��������ɂȂ�悤�ɐ�ɏc���������B
		HDC hdc = ::GetDC(GetHwnd());
		{
			Graphics gr(hdc);
			gr.SetPen(pTypeData->colorInfoArr[COLORIDX_CURSORVLINE].colorAttr.cTEXT);
			::MoveToEx(gr, nOldCursorLineX, textArea.GetAreaTop(), NULL);
			::LineTo(  gr, nOldCursorLineX, textArea.GetAreaBottom());
			int nBoldX = nOldCursorLineX - 1;
			// �u�����v�̂Ƃ���2dot�̐��ɂ���B���̍ۃJ�[�\���Ɋ|����Ȃ��悤�ɍ����𑾂�����
			if (1
				&& pTypeData->colorInfoArr[COLORIDX_CURSORVLINE].fontAttr.bBoldFont
				&& IsDrawCursorVLinePos(nBoldX)
			) {
				::MoveToEx(gr, nBoldX, textArea.GetAreaTop(), NULL);
				::LineTo(  gr, nBoldX, textArea.GetAreaBottom());
				nOldCursorVLineWidth = 2;
			}else {
				nOldCursorVLineWidth = 1;
			}
		}	// ReleaseDC �̑O�� gr �f�X�g���N�g
		::ReleaseDC(GetHwnd(), hdc);
	}
	
	int nUnderLineY = -1;
	if (bUnderLine) {
		nUnderLineY = textArea.GetAreaTop() + (GetCaret().GetCaretLayoutPos().y - textArea.GetViewTopLine())
			 * GetTextMetrics().GetHankakuDy() + GetTextMetrics().GetHankakuHeight();
	}
	// To Here 2007.09.09 Moca

	if (1
		&& bDraw
		&& GetDrawSwitch()
		&& nUnderLineY >= textArea.GetAreaTop()
		&& !bDoing_UndoRedo	// Undo, Redo�̎��s����
		&& !GetSelectionInfo().IsTextSelecting()
		&& !DisalbeUnderLine
	) {
		if (!bCursorLineBgDraw || nOldUnderLineY == -1) {
			nOldUnderLineY = GetCaret().GetCaretLayoutPos().y;
			nOldUnderLineYBg = nOldUnderLineY;
		}
		nOldUnderLineYMargin = GetTextMetrics().GetHankakuHeight();
		nOldUnderLineYHeightReal = 1;
//		MYTRACE(_T("���J�[�\���s�A���_�[���C���̕`��\n"));
		// ���J�[�\���s�A���_�[���C���̕`��
		HDC		hdc = ::GetDC(GetHwnd());
		{
			Graphics gr(hdc);
			gr.SetPen(pTypeData->colorInfoArr[COLORIDX_UNDERLINE].colorAttr.cTEXT);
			::MoveToEx(
				gr,
				textArea.GetAreaLeft(),
				nUnderLineY,
				NULL
			);
			::LineTo(
				gr,
				textArea.GetAreaRight(),
				nUnderLineY
			);
		}	// ReleaseDC �̑O�� gr �f�X�g���N�g
		::ReleaseDC(GetHwnd(), hdc);
	}
}

// �J�[�\���s�A���_�[���C����OFF
void EditView::CaretUnderLineOFF(
	bool bDraw,
	bool bDrawPaint,
	bool bResetFlag,
	bool DisalbeUnderLine
	)
{
	if (1
		&& !pTypeData->colorInfoArr[COLORIDX_UNDERLINE].bDisp
		&& !pTypeData->colorInfoArr[COLORIDX_CURSORVLINE].bDisp
		&& !pTypeData->colorInfoArr[COLORIDX_CARETLINEBG].bDisp
	) {
		return;
	}
	auto& textArea = GetTextArea();
	auto& caret = GetCaret();
	if (nOldUnderLineY != -1) {
		if (1
			&& bDraw
			&& GetDrawSwitch()
			&& nOldUnderLineY >= textArea.GetViewTopLine()
			&& !bDoing_UndoRedo	// Undo, Redo�̎��s����
			&& !caret.underLine.GetUnderLineDoNotOFF()	// �A���_�[���C�����������邩
		) {
			// -- -- �J�[�\���s�A���_�[���C���̏����i�������j -- -- //
			int nUnderLineY; // client px
			int nY = nOldUnderLineY - textArea.GetViewTopLine();
			if (nY < 0) {
				nUnderLineY = -1;
			}else if (textArea.nViewRowNum < nY) {
				nUnderLineY = textArea.GetAreaBottom() + 1;
			}else {
				nUnderLineY = textArea.GetAreaTop() + nY * GetTextMetrics().GetHankakuDy();
			}
			
			caret.underLine.Lock();

			PAINTSTRUCT ps;
			ps.rcPaint.left = 0;
			ps.rcPaint.right = textArea.GetAreaRight();
			int height;
			if (bDrawPaint && nOldUnderLineYHeight != 0) {
				ps.rcPaint.top = nUnderLineY;
				height = t_max(nOldUnderLineYHeight, nOldUnderLineYMargin + nOldUnderLineYHeightReal);
			}else {
				ps.rcPaint.top = nUnderLineY + nOldUnderLineYMargin;
				height = nOldUnderLineYHeightReal;
			}
			ps.rcPaint.bottom = ps.rcPaint.top + height;

			//	�s�{�ӂȂ���I�������o�b�N�A�b�v�B
//			Range sSelectBackup = GetSelectionInfo().select;
//			GetSelectionInfo().select.Clear(-1);

			if (ps.rcPaint.bottom - ps.rcPaint.top) {
				// �`��
				HDC hdc = this->GetDC();
				// �\�Ȃ�݊�BMP����R�s�[���čč��
				OnPaint(hdc, &ps, (ps.rcPaint.bottom - ps.rcPaint.top) == 1);
				this->ReleaseDC(hdc);
			}
			nOldUnderLineYHeight = 0;

			//	�I�����𕜌�
			caret.underLine.UnLock();
			
			if (bDrawPaint) {
				nOldUnderLineYBg = -1;
			}
		}
		if (bResetFlag) {
			nOldUnderLineY = -1;
		}
		nOldUnderLineYHeightReal = 0;
	}

	// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
	// �J�[�\���ʒu�c��
	if (nOldCursorLineX != -1) {
		if (1
			&& bDraw
			&& GetDrawSwitch()
			&& IsDrawCursorVLinePos(nOldCursorLineX)
			&& !bDoing_UndoRedo
			&& !caret.underLine.GetVertLineDoNotOFF()	// �J�[�\���ʒu�c�����������邩
			&& !DisalbeUnderLine
		) {
			PAINTSTRUCT ps;
			ps.rcPaint.left = nOldCursorLineX - (nOldCursorVLineWidth - 1);
			ps.rcPaint.right = nOldCursorLineX + 1;
			ps.rcPaint.top = textArea.GetAreaTop();
			ps.rcPaint.bottom = textArea.GetAreaBottom();
			HDC hdc = ::GetDC(GetHwnd());
			caret.underLine.Lock();
			//	�s�{�ӂȂ���I�������o�b�N�A�b�v�B
			Range sSelectBackup = this->GetSelectionInfo().select;
			this->GetSelectionInfo().select.Clear(-1);
			// �\�Ȃ�݊�BMP����R�s�[���čč��
			OnPaint(hdc, &ps, TRUE);
			//	�I�����𕜌�
			this->GetSelectionInfo().select = sSelectBackup;
			caret.underLine.UnLock();
			ReleaseDC(hdc);
		}
		nOldCursorLineX = -1;
	}
	// To Here 2007.09.09 Moca
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ��ԕ\��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	�����^�u���^�u�b�N�}�[�N�������̏�Ԃ��X�e�[�^�X�o�[�ɕ\������

	@date 2002.01.26 hor �V�K�쐬
	@date 2002.12.04 genta ���̂�EditWnd�ֈړ�
*/
void EditView::SendStatusMessage(const TCHAR* msg)
{
	editWnd.SendStatusMessage(msg);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �ҏW���[�h                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	�}�����[�h�擾

	@date 2005.10.02 genta �Ǘ����@�ύX�̂��ߊ֐���
*/
bool EditView::IsInsMode(void) const
{
	return pEditDoc->docEditor.IsInsMode();
}

void EditView::SetInsMode(bool mode)
{
	pEditDoc->docEditor.SetInsMode(mode);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �C�x���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void EditView::OnAfterLoad(const LoadInfo& loadInfo)
{
	if (!GetHwnd()) {
		// MiniMap ��\��
		return;
	}
	// -- -- �� InitAllView�ł���Ă����� -- -- //
	pHistory->Flush();

	// ���݂̑I��͈͂��I����Ԃɖ߂�
	GetSelectionInfo().DisableSelectArea(false);

	OnChangeSetting();
	GetCaret().MoveCursor(Point(0, 0), true);
	GetCaret().nCaretPosX_Prev = 0;
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//	2004.05.13 Moca ���s�R�[�h�̐ݒ�����炱���Ɉړ�
	editWnd.GetActiveView().GetCaret().ShowCaretPosInfo();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���̑�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// ���݂̃J�[�\���s�ʒu�𗚗��ɓo�^����
void EditView::AddCurrentLineToHistory(void)
{
	Point ptPos = pEditDoc->layoutMgr.LayoutToLogic(GetCaret().GetCaretLayoutPos());

	MarkMgr::Mark m(ptPos);
	pHistory->Add(m);
}


//	2001/06/18 Start by asa-o: �⊮�E�B���h�E�p�̃L�[���[�h�w���v�\��
bool EditView::ShowKeywordHelp(
	POINT po,
	LPCWSTR pszHelp,
	LPRECT prcHokanWin
	)
{
	NativeW	memCurText;
	RECT	rcTipWin,
			rcDesktop;

	if (pTypeData->bUseKeywordHelp) { // �L�[���[�h�w���v���g�p����
		if (!bInMenuLoop			// ���j���[ ���[�_�� ���[�v�ɓ����Ă��Ȃ�
			&& dwTipTimer != 0	// ����Tip��\�����Ă��Ȃ�
		) {
			memCurText.SetString(pszHelp);

			// ���Ɍ����ς݂�
			if (NativeW::IsEqual(memCurText, tipWnd.key)) {
				// �Y������L�[���Ȃ�����
				if (!tipWnd.KeyWasHit) {
					return false;
				}
			}else {
				tipWnd.key = memCurText;
				// �������s
				if (!KeySearchCore(&tipWnd.key))	// 2006.04.10 fon
					return false;
			}
			dwTipTimer = 0;	// ����Tip��\�����Ă���

		// 2001/06/19 Start by asa-o: ����Tip�̕\���ʒu����
			// ����Tip�̃T�C�Y���擾
			tipWnd.GetWindowSize(&rcTipWin);

			//	May 01, 2004 genta �}���`���j�^�Ή�
			::GetMonitorWorkRect(tipWnd.GetHwnd(), &rcDesktop);

			// �E�ɓ���
			if (prcHokanWin->right + rcTipWin.right < rcDesktop.right) {
				// ���̂܂�
			// ���ɓ���
			}else if (rcDesktop.left < prcHokanWin->left - rcTipWin.right) {
				// ���ɕ\��
				po.x = prcHokanWin->left - (rcTipWin.right + 8);
			// �ǂ�����X�y�[�X�������Ƃ��L���ق��ɕ\��
			}else if (rcDesktop.right - prcHokanWin->right > prcHokanWin->left) {
				// �E�ɕ\�� ���̂܂�
			}else {
				// ���ɕ\��
				po.x = prcHokanWin->left - (rcTipWin.right + 8);
			}
		// 2001/06/19 End

			// ����Tip��\��
			tipWnd.Show(po.x, po.y , NULL , &rcTipWin);
			return true;
		}
	}
	return false;
}
//	2001/06/18 End

/*!
	@brief �w��ʒu�܂��͎w��͈͂��e�L�X�g�̑��݂��Ȃ��G���A���`�F�b�N����

	@param[in] ptFrom  �w��ʒu�܂��͎w��͈͊J�n
	@param[in] ptTo    �w��͈͏I��
	@param[in] bSelect    �͈͎w��
	@param[in] bBoxSelect ��`�I��
	
	@retval true  �w��ʒu�܂��͎w��͈͓��Ƀe�L�X�g�����݂��Ȃ�
			false �w��ʒu�܂��͎w��͈͓��Ƀe�L�X�g�����݂���

	@date 2008.08.03 nasukoji	�V�K�쐬
*/
bool EditView::IsEmptyArea(
	Point ptFrom,
	Point ptTo,
	bool bSelect,
	bool bBoxSelect
	) const
{
	bool result;

	int nColumnFrom = ptFrom.x;
	int nLineFrom = ptFrom.y;
	int nColumnTo = ptTo.x;
	int nLineTo = ptTo.y;

	if (bSelect && !bBoxSelect && nLineFrom != nLineTo) {	// �����s�͈͎̔w��
		// �����s�ʏ�I�������ꍇ�A�K���e�L�X�g���܂�
		result = false;
	}else {
		if (bSelect) {
			// �͈͂̒���
			if (nLineFrom > nLineTo) {
				std::swap(nLineFrom, nLineTo);
			}

			if (nColumnFrom > nColumnTo) {
				std::swap(nColumnFrom, nColumnTo);
			}
		}else {
			nLineTo = nLineFrom;
		}

		const Layout*	pLayout;
		size_t nLineLen;

		result = true;
		for (int nLineNum=nLineFrom; nLineNum<=nLineTo; ++nLineNum) {
			if ((pLayout = pEditDoc->layoutMgr.SearchLineByLayoutY(nLineNum))) {
				// �w��ʒu�ɑΉ�����s�̃f�[�^���̈ʒu
				LineColumnToIndex2(pLayout, nColumnFrom, &nLineLen);
				if (nLineLen == 0) {	// �܂�Ԃ�����s�R�[�h���E�̏ꍇ�ɂ� nLineLen �ɍs�S�̂̕\������������
					result = false;		// �w��ʒu�܂��͎w��͈͓��Ƀe�L�X�g������
					break;
				}
			}
		}
	}

	return result;
}

// �A���h�D�o�b�t�@�̏���
void EditView::SetUndoBuffer(bool bPaintLineNumber)
{
	OpeBlk* pOpe = commander.GetOpeBlk();
	if (pOpe && pOpe->Release() == 0) {
		if (0 < pOpe->GetNum()) {	// ����̐���Ԃ�
			// ����̒ǉ�
			GetDocument().docEditor.opeBuf.AppendOpeBlk(pOpe);

			// 2013.05.01 Moca ���m�ɕύX�s��\������悤�ɂȂ����̂ŕs�v
			//  if (bPaintLineNumber
			//   &&	pEditDoc->docEditor.opeBuf.GetCurrentPointer() == 1)	// �SUndo��Ԃ���̕ύX���H	// 2009.03.26 ryoji
			//  	Call_OnPaint(PaintAreaType::LineNumber, false);	// ���y�C���̍s�ԍ��i�ύX�s�j�\�����X�V �� �ύX�s�݂̂̕\���X�V�ōς܂��Ă���ꍇ�����邽��

			if (!editWnd.UpdateTextWrap())	{	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
				if (0 < pOpe->GetNum() - GetDocument().docEditor.nOpeBlkRedawCount) {
					editWnd.RedrawAllViews(this);	//	���̃y�C���̕\�����X�V
				}
			}
		}else {
			delete pOpe;
		}
		commander.SetOpeBlk(nullptr);
	}
}

