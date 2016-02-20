/*!	@file
	@brief 文書ウィンドウの管理

	@author Norio Nakatani
	@date	1998/03/13 作成
	@date   2005/09/02 D.S.Koba GetSizeOfCharで書き換え
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, genta, JEPRO, MIK
	Copyright (C) 2001, genta, GAE, MIK, hor, asa-o, Stonee, Misaka, novice, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, Moca, minfu, KK, novice, ai, Azumaiya, genta
	Copyright (C) 2003, MIK, ai, ryoji, Moca, wmlhq, genta
	Copyright (C) 2004, genta, Moca, novice, naoh, isearch, fotomo
	Copyright (C) 2005, genta, MIK, novice, aroka, D.S.Koba, かろと, Moca
	Copyright (C) 2006, Moca, aroka, ryoji, fon, genta
	Copyright (C) 2007, ryoji, じゅうじ, maru
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

EditView*	g_m_pEditView;
LRESULT CALLBACK EditViewWndProc(HWND, UINT, WPARAM, LPARAM);
VOID CALLBACK EditViewTimerProc(HWND, UINT, UINT_PTR, DWORD);

#define IDT_ROLLMOUSE	1

/*
|| ウィンドウプロシージャ
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
		pEdit = (EditView*) g_m_pEditView;
		return pEdit->DispatchEvent(hwnd, uMsg, wParam, lParam);
	default:
		pEdit = (EditView*) ::GetWindowLongPtr(hwnd, 0);
		if (pEdit) {
			//	May 16, 2000 genta
			//	From Here
			if (uMsg == WM_COMMAND) {
				::SendMessage(::GetParent(pEdit->m_hwndParent), WM_COMMAND, wParam,  lParam);
			}else {
				return pEdit->DispatchEvent(hwnd, uMsg, wParam, lParam);
			}
			//	To Here
		}
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}


/*
||  タイマーメッセージのコールバック関数
||
||	現在は、マウスによる領域選択時のスクロール処理のためにタイマーを使用しています。
*/
VOID CALLBACK EditViewTimerProc(
	HWND hwnd,		// handle of window for timer messages
	UINT uMsg,		// WM_TIMER message
	UINT_PTR idEvent,	// timer identifier
	DWORD dwTime 	// current system time
)
{
	EditView* pEditView = (EditView*)::GetWindowLongPtr(hwnd, 0);
	if (pEditView) {
		pEditView->OnTimer(hwnd, uMsg, idEvent, dwTime);
	}
	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、Processにひとつあるのみ。
EditView::EditView(EditWnd* pEditWnd)
	:
	ViewCalc(this),				// warning C4355: 'this' : ベース メンバー初期化子リストで使用されました。
	m_pEditWnd(pEditWnd),
	m_pTextArea(NULL),
	m_pCaret(NULL),
	m_pRuler(NULL),
	m_viewSelect(this),			// warning C4355: 'this' : ベース メンバー初期化子リストで使用されました。
	m_parser(this),				// warning C4355: 'this' : ベース メンバー初期化子リストで使用されました。
	m_textDrawer(this),			// warning C4355: 'this' : ベース メンバー初期化子リストで使用されました。
	m_commander(this),				// warning C4355: 'this' : ベース メンバー初期化子リストで使用されました。
	m_hwndVScrollBar(NULL),
	m_hwndHScrollBar(NULL),
	m_pDropTarget(NULL),
	m_bActivateByMouse(FALSE),	// 2007.10.02 nasukoji
	m_nWheelDelta(0),
	m_eWheelScroll(F_0),
	m_nMousePouse(0),
	m_nAutoScrollMode(0),
	m_AT_ImmSetReconvertString(NULL),
	m_pHistory(NULL),
	m_pRegexKeyword(NULL),
	m_hAtokModule(NULL)
{
}


// 2007.10.23 kobake コンストラクタ内の処理をすべてCreateに移しました。(初期化処理が不必要に分散していたため)
BOOL EditView::Create(
	HWND		hwndParent,	//!< 親
	EditDoc*	pEditDoc,	//!< 参照するドキュメント
	int			nMyIndex,	//!< ビューのインデックス
	BOOL		bShow,		//!< 作成時に表示するかどうか
	bool		bMiniMap
)
{
	m_bMiniMap = bMiniMap;
	m_pTextArea = new TextArea(this);
	m_pCaret = new Caret(this, pEditDoc);
	m_pRuler = new Ruler(this, pEditDoc);
	if (m_bMiniMap) {
		m_pViewFont = m_pEditWnd->m_pViewFontMiniMap;
	}else {
		m_pViewFont = m_pEditWnd->m_pViewFont;
	}

	m_pHistory = new AutoMarkMgr;
	m_pRegexKeyword = NULL;				// 2007.04.08 ryoji

	SetDrawSwitch(true);
	m_pDropTarget = new DropTarget(this);
	_SetDragMode(FALSE);					// 選択テキストのドラッグ中か
	m_bCurSrchKeyMark = false;				// 検索文字列
	//	Jun. 27, 2001 genta
	m_strCurSearchKey.clear();
	m_curSearchOption.Reset();				// 検索／置換 オプション
	m_bCurSearchUpdate = false;
	m_nCurSearchKeySequence = -1;

	m_nMyIndex = 0;

	//	Dec. 4, 2002 genta
	//	メニューバーへのメッセージ表示機能はEditWndへ移管

	// 共有データ構造体のアドレスを返す
	m_bCommandRunning = FALSE;	// コマンドの実行中
	m_bDoing_UndoRedo = FALSE;	// アンドゥ・リドゥの実行中か
	m_pcsbwVSplitBox = NULL;	// 垂直分割ボックス
	m_pcsbwHSplitBox = NULL;	// 水平分割ボックス
	m_hwndVScrollBar = NULL;
	m_nVScrollRate = 1;			// 垂直スクロールバーの縮尺
	m_hwndHScrollBar = NULL;
	m_hwndSizeBox = NULL;

	m_ptSrchStartPos_PHY.Set(LogicInt(-1), LogicInt(-1));	// 検索/置換開始時のカーソル位置  (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	m_bSearch = FALSE;					// 検索/置換開始位置を登録するか											// 02/06/26 ai
	
	m_ptBracketPairPos_PHY.Set(LogicInt(-1), LogicInt(-1)); // 対括弧の位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	m_ptBracketCaretPos_PHY.Set(LogicInt(-1), LogicInt(-1));

	m_bDrawBracketPairFlag = FALSE;	// 03/02/18 ai
	GetSelectionInfo().m_bDrawSelectArea = false;	// 選択範囲を描画したか		// 02/12/13 ai

	m_crBack = -1;				/* テキストの背景色 */			// 2006.12.16 ryoji
	m_crBack2 = -1;
	
	m_szComposition[0] = _T('\0');

	auto& textArea = GetTextArea();
	// ルーラー表示
	textArea.SetAreaTop(textArea.GetAreaTop() + GetDllShareData().m_common.window.nRulerHeight);	// ルーラー高さ
	GetRuler().SetRedrawFlag();	// ルーラー全体を描き直す時=true   2002.02.25 Add By KK
	m_hdcCompatDC = NULL;		// 再描画用コンパチブルＤＣ
	m_hbmpCompatBMP = NULL;		// 再描画用メモリＢＭＰ
	m_hbmpCompatBMPOld = NULL;	// 再描画用メモリＢＭＰ(OLD)
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	m_nCompatBMPWidth = -1;
	m_nCompatBMPHeight = -1;
	// To Here 2007.09.09 Moca
	
	m_nOldUnderLineY = -1;
	m_nOldCursorLineX = -1;
	m_nOldCursorVLineWidth = 1;
	m_nOldUnderLineYHeightReal = 0;

	//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
	//	2007.08.12 genta 初期化にShareDataの値が必要になった
	m_curRegexp.InitDll(GetDllShareData().m_common.search.szRegexpLib);

	// 2004.02.08 m_hFont_ZENは未使用により削除
	m_dwTipTimer = ::GetTickCount();	// 辞書Tip起動タイマー
	m_bInMenuLoop = FALSE;				// メニュー モーダル ループに入っています
//	MYTRACE(_T("EditView::EditView()おわり\n"));
	m_bHokan = FALSE;

	//	Aug. 31, 2000 genta
	m_pHistory->SetMax(30);

	// from here  2002.04.09 minfu OSによって再変換の方式を変える
	//	YAZAKI COsVersionInfoのカプセル化は守りましょ。
	if (!OsSupportReconvert()) {
		// 95 or NTならば
		m_uMSIMEReconvertMsg = ::RegisterWindowMessage(RWM_RECONVERT);
		m_uATOKReconvertMsg = ::RegisterWindowMessage(MSGNAME_ATOK_RECONVERT) ;
		m_uWM_MSIME_RECONVERTREQUEST = ::RegisterWindowMessage(_T("MSIMEReconvertRequest"));
		
		m_hAtokModule = LoadLibraryExedir(_T("ATOK10WC.DLL"));
		m_AT_ImmSetReconvertString = NULL;
		if (m_hAtokModule) {
			m_AT_ImmSetReconvertString =(BOOL (WINAPI *)(HIMC, int, PRECONVERTSTRING, DWORD )) GetProcAddress(m_hAtokModule,"AT_ImmSetReconvertString");
		}
	}else { 
		// それ以外のOSのときはOS標準を使用する
		m_uMSIMEReconvertMsg = 0;
		m_uATOKReconvertMsg = 0 ;
		m_hAtokModule = 0;	//@@@ 2002.04.14 MIK
	}
	// to here  2002.04.10 minfu
	
	// 2004.10.23 isearch
	m_nISearchMode = 0;
	m_pMigemo = NULL;

	// 2007.10.02 nasukoji
	m_dwTripleClickCheck = 0;		// トリプルクリックチェック用時刻初期化

	// 2010.07.15 Moca
	m_mouseDownPos.Set(-INT_MAX, -INT_MAX);

	m_bMiniMapMouseDown = false;

	//↑今までコンストラクタでやってたこと
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//↓今までCreateでやってたこと

	WNDCLASS wc;
	m_hwndParent = hwndParent;
	m_pEditDoc = pEditDoc;
	m_pTypeData = &m_pEditDoc->m_docType.GetDocumentAttribute();
	m_nMyIndex = nMyIndex;

	//	2007.08.18 genta 初期化にShareDataの値が必要になった
	m_pRegexKeyword = new RegexKeyword(GetDllShareData().m_common.search.szRegexpLib);	//@@@ 2001.11.17 add MIK
	m_pRegexKeyword->RegexKeySetTypes(m_pTypeData);	//@@@ 2001.11.17 add MIK

	textArea.SetTopYohaku(GetDllShareData().m_common.window.nRulerBottomSpace); 	// ルーラーとテキストの隙間
	textArea.SetAreaTop(textArea.GetTopYohaku());								// 表示域の上端座標
	// ルーラー表示
	if (m_pTypeData->colorInfoArr[COLORIDX_RULER].bDisp) {
		textArea.SetAreaTop(textArea.GetAreaTop() + GetDllShareData().m_common.window.nRulerHeight);	// ルーラー高さ
	}

	// ウィンドウクラスの登録
	//	Apr. 27, 2000 genta
	//	サイズ変更時のちらつきを抑えるためCS_HREDRAW | CS_VREDRAW を外した
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

	// エディタウィンドウの作成
	g_m_pEditView = this;
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

	m_pDropTarget->Register_DropTarget(GetHwnd());

	// 辞書Tip表示ウィンドウ作成
	m_tipWnd.Create(G_AppInstance(), GetHwnd()/*GetDllShareData().m_sHandles.m_hwndTray*/);

	// 再描画用コンパチブルＤＣ
	// 2007.09.09 Moca 互換BMPによる画面バッファ
	// 2007.09.30 genta 関数化
	UseCompatibleDC(GetDllShareData().m_common.window.bUseCompatibleBMP);

	// 垂直分割ボックス
	m_pcsbwVSplitBox = new SplitBoxWnd;
	m_pcsbwVSplitBox->Create(G_AppInstance(), GetHwnd(), TRUE);
	// 水平分割ボックス
	m_pcsbwHSplitBox = new SplitBoxWnd;
	m_pcsbwHSplitBox->Create(G_AppInstance(), GetHwnd(), FALSE);

	// スクロールバー作成
	CreateScrollBar();		// 2006.12.19 ryoji

	SetFont();

	if (bShow) {
		ShowWindow(GetHwnd(), SW_SHOW);
	}

	// キーボードの現在のリピート間隔を取得
	int nKeyBoardSpeed;
	SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &nKeyBoardSpeed, 0);

	// タイマー起動
	if (::SetTimer(GetHwnd(), IDT_ROLLMOUSE, nKeyBoardSpeed, EditViewTimerProc) == 0) {
		WarningMessage(GetHwnd(), LS(STR_VIEW_TIMER));
	}

	m_bHideMouse = false;
	RegKey reg;
	BYTE bUserPref[8] = {0};
	reg.Open(HKEY_CURRENT_USER, _T("Control Panel\\Desktop"));
	reg.GetValueBINARY(_T("UserPreferencesMask"), bUserPref, sizeof(bUserPref));
	if ((bUserPref[2] & 0x01) == 1) {
		m_bHideMouse = true;
	}

	TypeSupport textType(this, COLORIDX_TEXT);
	m_crBack = textType.GetBackColor();

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

	// 再描画用コンパチブルＤＣ
	//	2007.09.30 genta 関数化
	//	m_hbmpCompatBMPもここで削除される．
	UseCompatibleDC(FALSE);

	delete m_pDropTarget;
	m_pDropTarget = NULL;

	delete m_pHistory;
	m_pHistory = NULL;

	delete m_pRegexKeyword;	//@@@ 2001.11.17 add MIK
	m_pRegexKeyword = NULL;
	
	// 再変換 2002.04.10 minfu
	if (m_hAtokModule)
		FreeLibrary(m_hAtokModule);

	delete m_pTextArea;
	m_pTextArea = NULL;
	delete m_pCaret;
	m_pCaret = NULL;
	delete m_pRuler;
	m_pRuler = NULL;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         イベント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// TCHAR→WCHAR変換。
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
|| メッセージディスパッチャ
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
		if (m_pEditWnd->DoMouseWheel(wParam, lParam)) {
			return 0L;
		}
		return OnMOUSEWHEEL(wParam, lParam);

	case WM_MOUSEHWHEEL:
		return OnMOUSEHWHEEL(wParam, lParam);

	case WM_CREATE:
		::SetWindowLongPtr(hwnd, 0, (LONG_PTR) this);
		return 0L;

		// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	case WM_SHOWWINDOW:
		// ウィンドウ非表示の再に互換BMPを廃棄してメモリーを節約する
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

		// 親ウィンドウのタイトルを更新
		m_pEditWnd->UpdateCaption();

		return 0L;
	case WM_KILLFOCUS:
		OnKillFocus();

		// 2009.01.17 nasukoji	ホイールスクロール有無状態をクリア
		m_pEditWnd->ClearMouseState();

		return 0L;
	case WM_CHAR:
#ifdef _UNICODE
		// コントロールコード入力禁止
		if (WCODE::IsControlCode((wchar_t)wParam)) {
			ErrorBeep();
		}else {
			GetCommander().HandleCommand(F_WCHAR, true, WCHAR(wParam), 0, 0, 0);
		}
#else
		// SJIS固有
		{
			static BYTE preChar = 0;
			if (preChar == 0 && ! _IS_SJIS_1((unsigned char)wParam)) {
				// ASCII , 半角カタカナ
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

			m_szComposition[0] = _T('\0');

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

			// テキストを貼り付け
			if (m_bHideMouse && 0 <= m_nMousePouse) {
				m_nMousePouse = -1;
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
		m_szComposition[0] = _T('\0');
		return DefWindowProc(hwnd, uMsg, wParam, lParam);

	case WM_IME_CHAR:
		if (!IsInsMode() /* Oct. 2, 2005 genta */) { /* 上書きモードか？ */
			GetCommander().HandleCommand(F_IME_CHAR, true, wParam, 0, 0, 0);
		}
		return 0L;

	// From Here 2008.03.24 Moca ATOK等の要求にこたえる
	case WM_PASTE:
		return GetCommander().HandleCommand(F_PASTE, true, 0, 0, 0, 0);

	case WM_COPY:
		return GetCommander().HandleCommand(F_COPY, true, 0, 0, 0, 0);
	// To Here 2008.03.24 Moca

	case WM_KEYUP:
		// キーリピート状態
		GetCommander().m_bPrevCommand = 0;
		return 0L;

	// 2004.04.27 Moca From Here ALT+xでALTを押したままだとキーリピートがOFFにならない対策
	case WM_SYSKEYUP:
		GetCommander().m_bPrevCommand = 0;
		// 念のため呼ぶ
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	// 2004.04.27 To Here

	case WM_LBUTTONDBLCLK:
		if (m_bMiniMap) {
			return 0L;
		}
		// 2007.10.02 nasukoji	非アクティブウィンドウのダブルクリック時はここでカーソルを移動する
		// 2007.10.12 genta フォーカス移動のため，OnLBUTTONDBLCLKより移動
		if (m_bActivateByMouse) {
			// アクティブなペインを設定
			m_pEditWnd->SetActivePane(m_nMyIndex);
			// カーソルをクリック位置へ移動する
			OnLBUTTONDOWN(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));	
			// 2007.10.02 nasukoji
			m_bActivateByMouse = FALSE;		// マウスによるアクティベートを示すフラグをOFF
		}
		//		MYTRACE(_T(" WM_LBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD(lParam), HIWORD(lParam));
		OnLBUTTONDBLCLK(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		return 0L;

// novice 2004/10/11 マウス中ボタン対応
	case WM_MBUTTONDOWN:
		OnMBUTTONDOWN(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));

		return 0L;

	case WM_MBUTTONUP:
		// 2009.01.17 nasukoji	ボタンUPでコマンドを起動するように変更
		OnMBUTTONUP(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		return 0L;

	case WM_LBUTTONDOWN:
		// 2007.10.02 nasukoji
		m_bActivateByMouse = FALSE;		// マウスによるアクティベートを示すフラグをOFF
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
//		if (m_nMyIndex != m_pEditWnd->GetActivePane()) {
//			// アクティブなペインを設定
//			m_pEditWnd->SetActivePane(m_nMyIndex);
//		}
//		return 0L;
	case WM_RBUTTONUP:
//		MYTRACE(_T(" WM_RBUTTONUP wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD(lParam), HIWORD(lParam));
		OnRBUTTONUP(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		return 0L;

// novice 2004/10/10 マウスサイドボタン対応
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
		// 2009.01.17 nasukoji	ボタンUPでコマンドを起動するように変更
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
//		MYTRACE(_T("	WM_VSCROLL nPos=%d\n"), GetScrollPos(m_hwndVScrollBar, SB_CTL));
		//	Sep. 11, 2004 genta 同期スクロールの関数化
		{
			LayoutInt Scroll = OnVScroll(
				(int) LOWORD(wParam), ((int) HIWORD(wParam)) * m_nVScrollRate);

			//	シフトキーが押されていないときだけ同期スクロール
			if (!GetKeyState_Shift()) {
				SyncScrollV(Scroll);
			}
		}

		return 0L;

	case WM_HSCROLL:
//		MYTRACE(_T("	WM_HSCROLL nPos=%d\n"), GetScrollPos(m_hwndHScrollBar, SB_CTL));
		//	Sep. 11, 2004 genta 同期スクロールの関数化
		{
			LayoutInt Scroll = OnHScroll(
				(int) LOWORD(wParam), ((int) HIWORD(wParam)));

			//	シフトキーが押されていないときだけ同期スクロール
			if (!GetKeyState_Shift()) {
				SyncScrollH(Scroll);
			}
		}

		return 0L;

	case WM_ENTERMENULOOP:
		m_bInMenuLoop = TRUE;	// メニュー モーダル ループに入っています

		// 辞書Tipが起動されている
		if (m_dwTipTimer == 0) {
			// 辞書Tipを消す
			m_tipWnd.Hide();
			m_dwTipTimer = ::GetTickCount();	// 辞書Tip起動タイマー
		}
		if (m_bHokan) {
			m_pEditWnd->m_hokanMgr.Hide();
			m_bHokan = FALSE;
		}
		return 0L;

	case WM_EXITMENULOOP:
		m_bInMenuLoop = FALSE;	// メニュー モーダル ループに入っています
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
		m_pDropTarget->Revoke_DropTarget();

		// タイマー終了
		::KillTimer(GetHwnd(), IDT_ROLLMOUSE);


//		MYTRACE(_T("	WM_DESTROY\n"));
		/*
		||子ウィンドウの破棄
		*/
		if (m_hwndVScrollBar) {	// Aug. 20, 2005 Aroka
			::DestroyWindow(m_hwndVScrollBar);
			m_hwndVScrollBar = NULL;
		}
		if (m_hwndHScrollBar) {
			::DestroyWindow(m_hwndHScrollBar);
			m_hwndHScrollBar = NULL;
		}
		if (m_hwndSizeBox) {
			::DestroyWindow(m_hwndSizeBox);
			m_hwndSizeBox = NULL;
		}
		SAFE_DELETE(m_pcsbwVSplitBox);	// 垂直分割ボックス
		SAFE_DELETE(m_pcsbwHSplitBox);	// 水平分割ボックス

		SetHwnd(NULL);
		return 0L;

	case MYWM_DOSPLIT:
//		nPosX = (int)wParam;
//		nPosY = (int)lParam;
//		MYTRACE(_T("MYWM_DOSPLIT nPosX=%d nPosY=%d\n"), nPosX, nPosY);
		::SendMessage(m_hwndParent, MYWM_DOSPLIT, wParam, lParam);
		return 0L;

	case MYWM_SETACTIVEPANE:
		m_pEditWnd->SetActivePane(m_nMyIndex);
		::PostMessage(m_hwndParent, MYWM_SETACTIVEPANE, (WPARAM)m_nMyIndex, 0);
		return 0L;

	case MYWM_IME_REQUEST:  // 再変換  by minfu 2002.03.27 // 20020331 aroka
		
		// 2002.04.09 switch case に変更  minfu 
		switch (wParam) {
		case IMR_RECONVERTSTRING:
			return SetReconvertStruct((PRECONVERTSTRING)lParam, UNICODE_BOOL);
			
		case IMR_CONFIRMRECONVERTSTRING:
			return SetSelectionFromReonvert((PRECONVERTSTRING)lParam, UNICODE_BOOL);
			
		// 2010.03.16 MS-IME 2002 だと「カーソル位置の前後の内容を参照して変換を行う」の機能
		case IMR_DOCUMENTFEED:
			return SetReconvertStruct((PRECONVERTSTRING)lParam, UNICODE_BOOL, true);
			
		//default:
		}
		// 2010.03.16 0LではなくTSFが何かするかもしれないのでDefにまかせる
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	
	case MYWM_DROPFILES:	// 独自のドロップファイル通知	// 2008.06.20 ryoji
		OnMyDropFiles((HDROP)wParam);
		return 0L;

	// 2007.10.02 nasukoji	マウスクリックにてアクティベートされた時はカーソル位置を移動しない
	case WM_MOUSEACTIVATE:
		LRESULT nRes;
		nRes = ::DefWindowProc(hwnd, uMsg, wParam, lParam);	// 親に先に処理させる
		if (nRes == MA_NOACTIVATE || nRes == MA_NOACTIVATEANDEAT) {
			return nRes;
		}

		// マウスクリックによりバックグラウンドウィンドウがアクティベートされた
		//	2007.10.08 genta オプション追加
		if (1
			&& GetDllShareData().m_common.general.bNoCaretMoveByActivation
			&& !m_pEditWnd->IsActiveApp()
		) {
			m_bActivateByMouse = TRUE;		// マウスによるアクティベート
			return MA_ACTIVATEANDEAT;		// アクティベート後イベントを破棄
		}

		// アクティブなペインを設定
		if (::GetFocus() != GetHwnd()) {
			POINT ptCursor;
			::GetCursorPos(&ptCursor);
			HWND hwndCursorPos = ::WindowFromPoint(ptCursor);
			if (hwndCursorPos == GetHwnd()) {
				// ビュー上にマウスがあるので SetActivePane() を直接呼び出す
				// （個別のマウスメッセージが届く前にアクティブペインを設定しておく）
				if (!m_bMiniMap) {
					m_pEditWnd->SetActivePane(m_nMyIndex);
				}
			}else if (0
				|| (m_pcsbwVSplitBox && hwndCursorPos == m_pcsbwVSplitBox->GetHwnd())
				|| (m_pcsbwHSplitBox && hwndCursorPos == m_pcsbwHSplitBox->GetHwnd())
			) {
				// 2010.01.19 ryoji
				// 分割ボックス上にマウスがあるときはアクティブペインを切り替えない
				// （併せて MYWM_SETACTIVEPANE のポストにより分割線のゴミが残っていた問題も修正）
			}else {
				// 2008.05.24 ryoji
				// スクロールバー上にマウスがあるかもしれないので MYWM_SETACTIVEPANE をポストする
				// SetActivePane() にはスクロールバーのスクロール範囲調整処理が含まれているが、
				// このタイミング（WM_MOUSEACTIVATE）でスクロール範囲を変更するのはまずい。
				// 例えば Win XP/Vista だとスクロール範囲が小さくなってスクロールバーが有効から
				// 無効に切り替わるとそれ以後スクロールバーが機能しなくなる。
				if (!m_bMiniMap) {
					::PostMessage(GetHwnd(), MYWM_SETACTIVEPANE, (WPARAM)m_nMyIndex, 0);
				}
			}
		}

		return nRes;

	case EM_GETLIMITTEXT:
		return INT_MAX;
	case EM_REPLACESEL:
	{
		// wParam RedoUndoフラグは無視する
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
// << 20020331 aroka 再変換対応 for 95/NT
		if (0
			|| (m_uMSIMEReconvertMsg && (uMsg == m_uMSIMEReconvertMsg)) 
			|| (m_uATOKReconvertMsg && (uMsg == m_uATOKReconvertMsg))
		) {
		// 2002.04.08 switch case に変更 minfu 
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
//                    ウィンドウイベント                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void EditView::OnMove(int x, int y, int nWidth, int nHeight)
{
	MoveWindow(GetHwnd(), x, y, nWidth, nHeight, TRUE);
	return;
}


// ウィンドウサイズの変更処理
void EditView::OnSize(int cx, int cy)
{
	if (!GetHwnd() 
		|| (cx == 0 && cy == 0)
	) {
		// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
		// ウィンドウ無効時にも互換BMPを破棄する
		DeleteCompatibleBitmap();
		// To Here 2007.09.09 Moca
		return;
	}

	int	nVSplitHeight = 0;	// 垂直分割ボックスの高さ
	int	nHSplitWidth  = 0;	// 水平分割ボックスの幅

	// スクロールバーのサイズ基準値を取得
	int nCxHScroll = ::GetSystemMetrics(SM_CXHSCROLL);
	int nCyHScroll = ::GetSystemMetrics(SM_CYHSCROLL);
	int nCxVScroll = ::GetSystemMetrics(SM_CXVSCROLL);
	int nCyVScroll = ::GetSystemMetrics(SM_CYVSCROLL);

	// 垂直分割ボックス
	if (m_pcsbwVSplitBox) {
		nVSplitHeight = 7;
		::MoveWindow(m_pcsbwVSplitBox->GetHwnd(), cx - nCxVScroll , 0, nCxVScroll, nVSplitHeight, TRUE);
	}
	// 水平分割ボックス
	if (m_pcsbwHSplitBox) {
		nHSplitWidth = 7;
		::MoveWindow(m_pcsbwHSplitBox->GetHwnd(), 0, cy - nCyHScroll, nHSplitWidth, nCyHScroll, TRUE);
	}
	// 垂直スクロールバー
	if (m_hwndVScrollBar) {
		::MoveWindow(m_hwndVScrollBar, cx - nCxVScroll , 0 + nVSplitHeight, nCxVScroll, cy - nCyVScroll - nVSplitHeight, TRUE);
	}
	// 水平スクロールバー
	if (m_hwndHScrollBar) {
		::MoveWindow(m_hwndHScrollBar, 0 + nHSplitWidth, cy - nCyHScroll, cx - nCxVScroll - nHSplitWidth, nCyHScroll, TRUE);
	}

	// サイズボックス
	if (m_hwndSizeBox) {
		::MoveWindow(m_hwndSizeBox, cx - nCxVScroll, cy - nCyHScroll, nCxHScroll, nCyVScroll, TRUE);
	}
	int nAreaWidthOld  = GetTextArea().GetAreaWidth();
	int nAreaHeightOld = GetTextArea().GetAreaHeight();

	// エリア情報更新
	GetTextArea().TextArea_OnSize(
		Size(cx, cy),
		nCxVScroll,
		m_hwndHScrollBar ? nCyHScroll : 0
	);

	// 再描画用メモリＢＭＰ
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	if (m_hdcCompatDC) {
		CreateOrUpdateCompatibleBitmap(cx, cy);
 	}
	// To Here 2007.09.09 Moca

	// 2008.06.06 nasukoji	サイズ変更時の折り返し位置再計算
	bool wrapChanged = false;
	if (m_pEditDoc->m_nTextWrapMethodCur == TextWrappingMethod::WindowWidth) {
		if (m_nMyIndex == 0) {	// 左上隅のビューのサイズ変更時のみ処理する
			// 右端で折り返すモードなら右端で折り返す	// 2008.06.08 ryoji
			wrapChanged = m_pEditWnd->WrapWindowWidth(0);
		}
	}

	if (!wrapChanged) {	// 折り返し位置が変更されていない
		AdjustScrollBars();				// スクロールバーの状態を更新する
	}

	// キャレットの表示(右・下に隠れていた場合)
	GetCaret().ShowEditCaret();

	if (IsBkBitmap()) {
		EBackgroundImagePos imgPos = m_pTypeData->backImgPos;
		if (imgPos != BGIMAGE_TOP_LEFT) {
			bool bUpdateWidth = false;
			bool bUpdateHeight = false;
			switch (imgPos) {
			case BGIMAGE_TOP_RIGHT:
			case BGIMAGE_BOTTOM_RIGHT:
			case BGIMAGE_CENTER_RIGHT:
			case BGIMAGE_TOP_CENTER:
			case BGIMAGE_BOTTOM_CENTER:
			case BGIMAGE_CENTER:
				bUpdateWidth = true;
				break;
			}
			switch (imgPos) {
			case BGIMAGE_BOTTOM_CENTER:
			case BGIMAGE_BOTTOM_LEFT:
			case BGIMAGE_BOTTOM_RIGHT:
			case BGIMAGE_CENTER:
			case BGIMAGE_CENTER_LEFT:
			case BGIMAGE_CENTER_RIGHT:
				bUpdateHeight = true;
				break;
			}
			if (bUpdateWidth  && nAreaWidthOld  != GetTextArea().GetAreaWidth() ||
			    bUpdateHeight && nAreaHeightOld != GetTextArea().GetAreaHeight()
			) {
				InvalidateRect(NULL, FALSE);
			}
		}
	}

	// 親ウィンドウのタイトルを更新
	m_pEditWnd->UpdateCaption(); // [Q] genta 本当に必要？

	if (m_pEditWnd->GetMiniMap().GetHwnd()) {
		EditView& miniMap = m_pEditWnd->GetMiniMap();
		if (miniMap.m_nPageViewTop != GetTextArea().GetViewTopLine()
			|| miniMap.m_nPageViewBottom != GetTextArea().GetBottomLine()
		) {
			MiniMapRedraw(true);
		}
	}
	return;
}


// 入力フォーカスを受け取ったときの処理
void EditView::OnSetFocus(void)
{
	if (m_bMiniMap) {
		return;
	}
	// 2004.04.02 Moca EOFのみのレイアウト行は、0桁目のみ有効.EOFより下の行のある場合は、EOF位置にする
	{
		LayoutPoint ptPos = GetCaret().GetCaretLayoutPos();
		if (GetCaret().GetAdjustCursorPos(&ptPos)) {
			GetCaret().MoveCursor(ptPos, false);
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
	}

	GetCaret().ShowEditCaret();

	SetIMECompFormFont();

	// ルーラのカーソルをグレーから黒に変更する
	HDC hdc = ::GetDC(GetHwnd());
	GetRuler().DispRuler(hdc);
	::ReleaseDC(GetHwnd(), hdc);

	// 03/02/18 対括弧の強調表示(描画) ai
	m_bDrawBracketPairFlag = TRUE;
	DrawBracketPair(true);

	m_pEditWnd->m_toolbar.AcceptSharedSearchKey();

	if (m_pEditWnd->GetMiniMap().GetHwnd()) {
		EditView& miniMap = m_pEditWnd->GetMiniMap();
		if (miniMap.m_nPageViewTop != GetTextArea().GetViewTopLine()
			|| miniMap.m_nPageViewBottom != GetTextArea().GetBottomLine()
		) {
			MiniMapRedraw(true);
		}
	}
}


// 入力フォーカスを失ったときの処理
void EditView::OnKillFocus(void)
{
	if (m_bMiniMap) {
		return;
	}
	// 03/02/18 対括弧の強調表示(消去) ai
	DrawBracketPair(false);
	m_bDrawBracketPairFlag = FALSE;

	GetCaret().DestroyCaret();

	// ルーラー描画
	// ルーラのカーソルを黒からグレーに変更する
	HDC	hdc = ::GetDC(GetHwnd());
	GetRuler().DispRuler(hdc);
	::ReleaseDC(GetHwnd(), hdc);

	// 辞書Tipが起動されている
	if (m_dwTipTimer == 0) {
		// 辞書Tipを消す
		m_tipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	// 辞書Tip起動タイマー
	}

	if (m_bHokan) {
		m_pEditWnd->m_hokanMgr.Hide();
		m_bHokan = FALSE;
	}
	if (m_nAutoScrollMode) {
		AutoScrollExit();
	}

	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           設定                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// フォントの変更
void EditView::SetFont(void)
{
	// メトリクス更新
	GetTextMetrics().Update(GetFontset().GetFontHan());

	// エリア情報を更新
	HDC hdc = ::GetDC(GetHwnd());
	GetTextArea().UpdateAreaMetrics(hdc);
	::ReleaseDC(GetHwnd(), hdc);

	// 行番号表示に必要な幅を設定
	GetTextArea().DetectWidthOfLineNumberArea(false);

	// ぜんぶ再描画
	::InvalidateRect(GetHwnd(), NULL, TRUE);

	//	Oct. 11, 2002 genta IMEのフォントも変更
	SetIMECompFormFont();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        キャレット                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief 選択を考慮した行桁指定によるカーソル移動

	選択状態チェック→カーソル移動→選択領域更新という処理が
	あちこちのコマンドにあるのでまとめることにした．
	また，戻り値はほとんど使われていないのでvoidにした．

	選択状態を考慮してカーソルを移動する．
	非選択が指定された場合には既存選択範囲を解除して移動する．
	選択が指定された場合には選択範囲の開始・変更を併せて行う．
	インタラクティブ操作を前提とするため，必要に応じたスクロールを行う．
	カーソル移動後は上下移動でもカラム位置を保つよう，
	GetCaret().m_nCaretPosX_Prevの更新も併せて行う．

	@date 2006.07.09 genta 新規作成
*/
void EditView::MoveCursorSelecting(
	LayoutPoint	ptWk_CaretPos,		//!< [in] 移動先レイアウト位置
	bool			bSelect,			//!< true: 選択する  false: 選択解除
	int				nCaretMarginRate	//!< 縦スクロール開始位置を決める値
)
{
	if (bSelect) {
		if (!GetSelectionInfo().IsTextSelected()) {	// テキストが選択されているか
			// 現在のカーソル位置から選択を開始する
			GetSelectionInfo().BeginSelectArea();
		}
	}else {
		if (GetSelectionInfo().IsTextSelected()) {	// テキストが選択されているか
			// 現在の選択範囲を非選択状態に戻す
			GetSelectionInfo().DisableSelectArea(true);
		}
	}
	GetCaret().GetAdjustCursorPos(&ptWk_CaretPos);
	if (bSelect) {
		/*	現在のカーソル位置によって選択範囲を変更．
		
			2004.04.02 Moca 
			キャレット位置が不正だった場合にMoveCursorの移動結果が
			引数で与えた座標とは異なることがあるため，
			nPosX, nPosYの代わりに実際の移動結果を使うように．
		*/
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptWk_CaretPos);
	}
	GetCaret().MoveCursor(ptWk_CaretPos, true, nCaretMarginRate);	// 2007.08.22 ryoji nCaretMarginRateが使われていなかった
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           解析                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	指定カーソル位置にURLが有る場合のその範囲を調べる

	2007.01.18 kobake URL文字列の受け取りをwstringで行うように変更
	2007.05.27 ryoji URL色指定の正規表現キーワードにマッチする文字列もURLとみなす
	                 URLの強調表示OFFのチェックはこの関数内で行うように変更
*/
bool EditView::IsCurrentPositionURL(
	const LayoutPoint&	ptCaretPos,		//!< [in]  カーソル位置
	LogicRange*			pUrlRange,		//!< [out] URL範囲。ロジック単位。
	std::wstring*		pwstrURL		//!< [out] URL文字列受け取り先。NULLを指定した場合はURL文字列を受け取らない。
	)
{
	MY_RUNNINGTIMER(runningTimer, "EditView::IsCurrentPositionURL");

	// URLを強調表示するかどうかチェックする	// 2009.05.27 ryoji
	bool bDispUrl = TypeSupport(this, COLORIDX_URL).IsDisp();
	bool bUseRegexKeyword = false;
	if (m_pTypeData->bUseRegexKeyword) {
		const wchar_t* pKeyword = m_pTypeData->regexKeywordList;
		for (int i=0; i<MAX_REGEX_KEYWORD; ++i) {
			if (*pKeyword == L'\0')
				break;
			if (m_pTypeData->regexKeywordArr[i].m_nColorIndex == COLORIDX_URL) {
				bUseRegexKeyword = true;	// URL色指定の正規表現キーワードがある
				break;
			}
			for (; *pKeyword!='\0'; ++pKeyword) {}
			++pKeyword;
		}
	}
	if (!bDispUrl && !bUseRegexKeyword) {
		return false;	// URL強調表示しないのでURLではない
	}

	// 正規表現キーワード（URL色指定）行検索開始処理	// 2009.05.27 ryoji
	if (bUseRegexKeyword) {
		m_pRegexKeyword->RegexKeyLineStart();
	}

	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	LogicPoint ptXY;
	m_pEditDoc->m_layoutMgr.LayoutToLogic(
		ptCaretPos,
		&ptXY
	);
	LogicInt nLineLen;
	const wchar_t* pLine = m_pEditDoc->m_docLineMgr.GetLine(ptXY.GetY2())->GetDocLineStrWithEOL(&nLineLen); // 2007.10.09 kobake レイアウト・ロジック混在バグ修正

	int nMatchColor;
	int nUrlLen = 0;
	LogicInt i = LogicInt(t_max(LogicInt(0), ptXY.GetX2() - _MAX_PATH));	// 2009.05.22 ryoji 200->_MAX_PATH
	//nLineLen = LogicInt(__min(nLineLen, ptXY.GetX2() + _MAX_PATH));
	while (i <= ptXY.GetX2() && i < nLineLen) {
		bool bMatch = (bUseRegexKeyword
					&& m_pRegexKeyword->RegexIsKeyword(StringRef(pLine, nLineLen), i, &nUrlLen, &nMatchColor)
					&& nMatchColor == COLORIDX_URL);
		if (!bMatch) {
			bMatch = (bDispUrl
						&& (i == 0 || !IS_KEYWORD_CHAR(pLine[i - 1]))	// 2009.05.22 ryoji CColor_Url::BeginColor()と同条件に
						&& IsURL(&pLine[i], (Int)(nLineLen - i), &nUrlLen));	// 指定アドレスがURLの先頭ならばTRUEとその長さを返す
		}
		if (bMatch) {
			if (i <= ptXY.GetX2() && ptXY.GetX2() < i + LogicInt(nUrlLen)) {
				// URLを返す場合
				if (pwstrURL) {
					pwstrURL->assign(&pLine[i], nUrlLen);
				}
				pUrlRange->SetLine(ptXY.GetY2());
				pUrlRange->SetXs(i, i + LogicInt(nUrlLen));
				return true;
			}else {
				i += LogicInt(nUrlLen);
				continue;
			}
		}
		++i;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         イベント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

VOID EditView::OnTimer(
	HWND hwnd,		// handle of window for timer messages
	UINT uMsg,		// WM_TIMER message
	UINT_PTR idEvent,	// timer identifier
	DWORD dwTime 	// current system time
	)
{
	if (GetDllShareData().m_common.edit.bUseOLE_DragDrop) {	// OLEによるドラッグ & ドロップを使う
		if (IsDragSource()) {
			return;
		}
	}
	POINT po;
	RECT rc;
	// 範囲選択中でない場合
	if (!GetSelectionInfo().IsMouseSelecting()) {
		if (m_bMiniMap) {
			bool bHide;
			if (MiniMapCursorLineTip(&po, &rc, &bHide)) {
				m_tipWnd.m_bAlignLeft = true;
				m_tipWnd.Show( po.x, po.y + m_pEditWnd->GetActiveView().GetTextMetrics().GetHankakuHeight(), NULL );
			}else {
				if (bHide && m_dwTipTimer == 0) {
					m_tipWnd.Hide();
				}
			}
		}else {
			if (KeywordHelpSearchDict(LID_SKH_ONTIMER, &po, &rc)) {	// 2006.04.10 fon
				// 辞書Tipを表示
				m_tipWnd.Show( po.x, po.y + GetTextMetrics().GetHankakuHeight(), NULL );
			}
		}
	}else {
		::GetCursorPos(&po);
		::GetWindowRect(GetHwnd(), &rc);
		if (!PtInRect(&rc, po)) {
			OnMOUSEMOVE(0, GetSelectionInfo().m_ptMouseRollPosOld.x, GetSelectionInfo().m_ptMouseRollPosOld.y);
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           変換                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 選択エリアのテキストを指定方法で変換
void EditView::ConvSelectedArea(EFunctionCode nFuncCode)
{
	// テキストが選択されているか
	if (!GetSelectionInfo().IsTextSelected()) {
		return;
	}

	NativeW memBuf;

	LayoutPoint sPos;

	LogicInt	nIdxFrom;
	LogicInt	nIdxTo;
	LayoutInt	nLineNum;
	LogicInt	nDelLen;
	LogicInt	nDelPosNext;
	LogicInt	nDelLenNext;
	LogicInt	nLineLen;
	LogicInt	nLineLen2;
	WaitCursor waitCursor(GetHwnd());

	LogicPoint ptFromLogic;	// 2009.07.18 ryoji Logicで記憶するように変更
	m_pEditDoc->m_layoutMgr.LayoutToLogic(
		GetSelectionInfo().m_select.GetFrom(),
		&ptFromLogic
	);

	// 矩形範囲選択中か
	if (GetSelectionInfo().IsBoxSelecting()) {

		// 2点を対角とする矩形を求める
		LayoutRect	rcSelLayout;
		TwoPointToRect(
			&rcSelLayout,
			GetSelectionInfo().m_select.GetFrom(),	// 範囲選択開始
			GetSelectionInfo().m_select.GetTo()		// 範囲選択終了
		);

		// 現在の選択範囲を非選択状態に戻す
		GetSelectionInfo().DisableSelectArea(false);	// 2009.07.18 ryoji true -> false 各行にアンダーラインが残る問題の修正

		nIdxFrom = LogicInt(0);
		nIdxTo = LogicInt(0);
		for (nLineNum=rcSelLayout.bottom; nLineNum>=rcSelLayout.top-1; --nLineNum) {
			const Layout* pLayout;
			nDelPosNext = nIdxFrom;
			nDelLenNext	= nIdxTo - nIdxFrom;
			const wchar_t* pLine = m_pEditDoc->m_layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
			if (pLine) {
				// 指定された桁に対応する行のデータ内の位置を調べる
				nIdxFrom	= LineColumnToIndex(pLayout, rcSelLayout.left);
				nIdxTo		= LineColumnToIndex(pLayout, rcSelLayout.right);

				bool bExtEol = GetDllShareData().m_common.edit.bEnableExtEol;
				for (LogicInt i=nIdxFrom; i<=nIdxTo; ++i) {
					if (WCODE::IsLineDelimiter(pLine[i], bExtEol)) {
						nIdxTo = i;
						break;
					}
				}
			}else {
				nIdxFrom	= LogicInt(0);
				nIdxTo		= LogicInt(0);
			}
			LogicInt	nDelPos = nDelPosNext;
			nDelLen	= nDelLenNext;
			if (nLineNum < rcSelLayout.bottom && 0 < nDelLen) {
				m_pEditDoc->m_layoutMgr.GetLineStr(nLineNum + LayoutInt(1), &nLineLen2, &pLayout);
				sPos.Set(
					LineIndexToColumn(pLayout, nDelPos),
					nLineNum + 1
				);

				// 2007.10.18 COpe整理
				// 指定位置の指定長データ削除
				DeleteData2(
					sPos,
					nDelLen,
					&memBuf
				);
				
				{
					// 機能種別によるバッファの変換
					ConvertMediator::ConvMemory(&memBuf, nFuncCode, (Int)m_pEditDoc->m_layoutMgr.GetTabSpace(), (Int)sPos.GetX2());

					// 現在位置にデータを挿入
					LayoutPoint ptLayoutNew;	// 挿入された部分の次の位置
					InsertData_CEditView(
						sPos,
						memBuf.GetStringPtr(),
						memBuf.GetStringLength(),
						&ptLayoutNew,
						false	// 2009.07.18 ryoji true -> false 各行にアンダーラインが残る問題の修正
					);

					// カーソルを移動
					GetCaret().MoveCursor(ptLayoutNew, false);
					GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
				}
			}
		}
		// 挿入データの先頭位置へカーソルを移動
		GetCaret().MoveCursor(rcSelLayout.UpperLeft(), true);
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		if (!m_bDoing_UndoRedo) {	// アンドゥ・リドゥの実行中か
			// 操作の追加
			m_commander.GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
	}else {
		// 選択範囲のデータを取得
		// 正常時はTRUE,範囲未選択の場合はFALSEを返す
		GetSelectedDataSimple(memBuf);

		// 機能種別によるバッファの変換
		ConvertMediator::ConvMemory(&memBuf, nFuncCode, (Int)m_pEditDoc->m_layoutMgr.GetTabSpace(), (Int)GetSelectionInfo().m_select.GetFrom().GetX2());

		// データ置換 削除&挿入にも使える
		ReplaceData_CEditView(
			GetSelectionInfo().m_select,
			memBuf.GetStringPtr(),		// 挿入するデータ	 // 2002/2/10 aroka CMemory変更
			memBuf.GetStringLength(),	// 挿入するデータの長さ // 2002/2/10 aroka CMemory変更
			false,
			m_bDoing_UndoRedo ? NULL : m_commander.GetOpeBlk()
		);

		// From Here 2001.12.03 hor
		//	選択エリアの復元
		LayoutPoint ptFrom;	// 2009.07.18 ryoji LogicからLayoutに戻す
		m_pEditDoc->m_layoutMgr.LogicToLayout(
			ptFromLogic,
			&ptFrom
		);
		GetSelectionInfo().SetSelectArea(LayoutRange(ptFrom, GetCaret().GetCaretLayoutPos()));	// 2009.07.25 ryoji
		GetCaret().MoveCursor(GetSelectionInfo().m_select.GetTo(), true);
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		if (!m_bDoing_UndoRedo) {	// アンドゥ・リドゥの実行中か
			// 操作の追加
			m_commander.GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
		// To Here 2001.12.03 hor
	}
	RedrawAll();	// 2009.07.18 ryoji 対象が矩形だった場合も最後に再描画する
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         メニュー                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ポップアップメニュー(右クリック)
int	EditView::CreatePopUpMenu_R(void)
{
	MenuDrawer& menuDrawer = m_pEditWnd->GetMenuDrawer();
	menuDrawer.ResetContents();

	// 右クリックメニューの定義はカスタムメニュー配列の0番目
	int nMenuIdx = CUSTMENU_INDEX_FOR_RBUTTONUP;	// マジックナンバー排除	//@@@ 2003.06.13 MIK

	// Note: CViewCommander::Command_CUSTMENU と大体同じ
	HMENU hMenu = ::CreatePopupMenu();

	// 2010.07.24 Moca オーナードロー対応のために前に移動してMenuDrawer経由で追加する
	if (!GetSelectionInfo().IsMouseSelecting()) {
		POINT po;
		RECT rc;
		if (KeywordHelpSearchDict(LID_SKH_POPUPMENU_R, &po, &rc)) {	// 2006.04.10 fon
			menuDrawer.MyAppendMenu(hMenu, 0, IDM_COPYDICINFO, LS(STR_MENU_KEYWORDINFO), _T("K"));	// 2006.04.10 fon ToolTip内容を直接表示するのをやめた
			menuDrawer.MyAppendMenu(hMenu, 0, IDM_JUMPDICT, LS(STR_MENU_OPENKEYWORDDIC), _T("L"));	// 2006.04.10 fon
			menuDrawer.MyAppendMenuSep(hMenu, MF_SEPARATOR, F_0, _T(""));
		}
	}
	return CreatePopUpMenuSub(hMenu, nMenuIdx, NULL);
}

/*! ポップアップメニューの作成(Sub)
	hMenuは作成済み
	@date 2013.06.15 新規作成 ポップアップメニューとメインメニューの表示方法を統合
*/
int	EditView::CreatePopUpMenuSub(HMENU hMenu, int nMenuIdx, int* pParentMenus)
{
	WCHAR szLabel[300];

	MenuDrawer& menuDrawer = m_pEditWnd->GetMenuDrawer();
	FuncLookup& funcLookup = m_pEditDoc->m_funcLookup;

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
	auto& csCustomMenu = GetDllShareData().m_common.customMenu;
	for (int i=0; i<csCustomMenu.nCustMenuItemNumArr[nMenuIdx]; ++i) {
		EFunctionCode code = csCustomMenu.nCustMenuItemFuncArr[nMenuIdx][i];
		bool bAppend = false;
		if (code == F_0) {
			// 2010.07.24 メニュー配列に入れる
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
				LPCWSTR p = GetDocument()->m_funcLookup.Custmenu2Name(nCustIdx, buf, _countof(buf));
				wchar_t keys[2];
				keys[0] = csCustomMenu.nCustMenuItemKeyArr[nMenuIdx][i];
				keys[1] = 0;
				HMENU hMenuPopUp = ::CreatePopupMenu();
				menuDrawer.MyAppendMenu(hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hMenuPopUp , p, keys);
				CreatePopUpMenuSub(hMenuPopUp, nCustIdx, pNextParam);
				bAppend = true;
			}else {
				// ループしているときは、従来同様別で表示
			}
		}
		if (!bAppend) {
			//	Oct. 3, 2001 genta
			funcLookup.Funccode2Name(code, szLabel, 256);
			// キー
			if (F_SPECIAL_FIRST <= code && code <= F_SPECIAL_LAST) {
				m_pEditWnd->InitMenu_Special(hMenu, code);
			}else {
				wchar_t keys[2];
				keys[0] = csCustomMenu.nCustMenuItemKeyArr[nMenuIdx][i];
				keys[1] = 0;
				m_pEditWnd->InitMenu_Function(hMenu, code, szLabel, keys);
			}
		}
	}

	pNextParam[nParamIndex] = 0;
	if (pParentMenus) {
		// 後は親に処理してもらう
		return -1;
	}

	int cMenuItems = ::GetMenuItemCount(hMenu);
	for (int nPos=0; nPos<cMenuItems; ++nPos) {
		EFunctionCode id = (EFunctionCode)::GetMenuItemID(hMenu, nPos);
		UINT fuFlags;
		// 機能が利用可能か調べる
		//	Jan.  8, 2006 genta 機能が有効な場合には明示的に再設定しないようにする．
		if (!IsFuncEnable(GetDocument(), &GetDllShareData(), id)) {
			fuFlags = MF_BYCOMMAND | MF_GRAYED;
			::EnableMenuItem(hMenu, id, fuFlags);
		}

		// 機能がチェック状態か調べる
		if (IsFuncChecked(GetDocument(), &GetDllShareData(), id)) {
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
		::GetParent(m_hwndParent)/*GetHwnd()*/,
		NULL
	);
	::DestroyMenu(hMenu);
	return nId;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         設定反映                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 設定変更を反映させる
void EditView::OnChangeSetting()
{
	if (!GetHwnd()) {
		return;
	}
	auto& csWindow = GetDllShareData().m_common.window;
	GetTextArea().SetTopYohaku(csWindow.nRulerBottomSpace); 	// ルーラーとテキストの隙間
	GetTextArea().SetAreaTop(GetTextArea().GetTopYohaku());									// 表示域の上端座標

	// 文書種別更新
	m_pTypeData = &m_pEditDoc->m_docType.GetDocumentAttribute();

	// ルーラー表示
	if (m_pTypeData->colorInfoArr[COLORIDX_RULER].bDisp && !m_bMiniMap) {
		GetTextArea().SetAreaTop(GetTextArea().GetAreaTop() + csWindow.nRulerHeight);	// ルーラー高さ
	}
	GetTextArea().SetLeftYohaku(csWindow.nLineNumRightSpace);

	// フォントの変更
	SetFont();

	// フォントが変わっているかもしれないので、カーソル移動
	// スクロールバーが移動するので呼び出し元でやる
	// GetCaret().MoveCursor(GetCaret().GetCaretLayoutPos(), true);

	// スクロールバーの状態を更新する
	// AdjustScrollBars();

	//	2007.09.30 genta 画面キャッシュ用CompatibleDCを用意する
	UseCompatibleDC(csWindow.bUseCompatibleBMP);

	// ウィンドウサイズの変更処理
	RECT rc;
	::GetClientRect(GetHwnd(), &rc);
	OnSize(rc.right, rc.bottom);

	// フォントが変わった
	m_tipWnd.ChangeFont(&(GetDllShareData().m_common.helper.lf));
	
	// 再描画
	if (!m_pEditWnd->m_pPrintPreview) {
		::InvalidateRect(GetHwnd(), NULL, TRUE);
	}
	TypeSupport textType(this, COLORIDX_TEXT);
	m_crBack = textType.GetBackColor();
}


// 自分の表示状態を他のビューにコピー
void EditView::CopyViewStatus(EditView* pView) const
{
	if (!pView) {
		return;
	}
	if (pView == this) {
		return;
	}

	// 入力状態
	GetCaret().CopyCaretStatus(&pView->GetCaret());

	// 選択状態
	GetSelectionInfo().CopySelectStatus(&pView->GetSelectionInfo());

	// 画面情報
	GetTextArea().CopyTextAreaStatus(&pView->GetTextArea());

	// 表示方法
	GetTextMetrics().CopyTextMetricsStatus(&pView->GetTextMetrics());
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       分割ボックス                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 縦・横の分割ボックス・サイズボックスのＯＮ／ＯＦＦ
void EditView::SplitBoxOnOff(bool bVert, bool bHorz, bool bSizeBox)
{
	RECT	rc;
	if (bVert) {
		if (!m_pcsbwVSplitBox) {	// 垂直分割ボックス
			m_pcsbwVSplitBox = new SplitBoxWnd;
			m_pcsbwVSplitBox->Create(G_AppInstance(), GetHwnd(), TRUE);
		}
	}else {
		SAFE_DELETE(m_pcsbwVSplitBox);	// 垂直分割ボックス
	}
	if (bHorz) {
		if (!m_pcsbwHSplitBox) {	// 水平分割ボックス
			m_pcsbwHSplitBox = new SplitBoxWnd;
			m_pcsbwHSplitBox->Create(G_AppInstance(), GetHwnd(), FALSE);
		}
	}else {
		SAFE_DELETE(m_pcsbwHSplitBox);	// 水平分割ボックス
	}

	if (bSizeBox) {
		if (m_hwndSizeBox) {
			::DestroyWindow(m_hwndSizeBox);
			m_hwndSizeBox = NULL;
		}
		m_hwndSizeBox = ::CreateWindowEx(
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
		if (m_hwndSizeBox) {
			::DestroyWindow(m_hwndSizeBox);
			m_hwndSizeBox = NULL;
		}
		m_hwndSizeBox = ::CreateWindowEx(
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
	::ShowWindow(m_hwndSizeBox, SW_SHOW);

	::GetClientRect(GetHwnd(), &rc);
	OnSize(rc.right, rc.bottom);
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       テキスト選択                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 選択範囲のデータを取得
	正常時はTRUE,範囲未選択の場合はFALSEを返す
*/
bool EditView::GetSelectedDataSimple(NativeW& memBuf)
{
	return GetSelectedData(&memBuf, false, NULL, false, false, EolType::Unknown);
}

/* 選択範囲のデータを取得
	正常時はTRUE,範囲未選択の場合はFALSEを返す
*/
bool EditView::GetSelectedData(
	NativeW*		memBuf,
	bool			bLineOnly,
	const wchar_t*	pszQuote,			// 先頭に付ける引用符
	bool			bWithLineNumber,	// 行番号を付与する
	bool			bAddCRLFWhenCopy,	// 折り返し位置で改行記号を入れる
	EolType			neweol				// コピー後の改行コード EolType::Noneはコード保存
	)
{
	LogicInt		nLineLen;
	LayoutInt		nLineNum;
	LogicInt		nIdxFrom;
	LogicInt		nIdxTo;
	int				nRowNum;
	int				nLineNumCols = 0;
	wchar_t*		pszLineNum = NULL;
	const wchar_t*	pszSpaces = L"                    ";
	const Layout*	pLayout;
	Eol				appendEol(neweol);

	// 範囲選択がされていない
	if (!GetSelectionInfo().IsTextSelected()) {
		return false;
	}
	if (bWithLineNumber) {	// 行番号を付与する
		// 行番号表示に必要な桁数を計算
		// 2014.11.30 桁はレイアウト単位である必要がある
		nLineNumCols = GetTextArea().DetectWidthOfLineNumberArea_calculate(&m_pEditDoc->m_layoutMgr, true);
		nLineNumCols += 1;
		pszLineNum = new wchar_t[nLineNumCols + 1];
	}

	if (GetSelectionInfo().IsBoxSelecting()) {	// 矩形範囲選択中
		LayoutRect rcSel;
		// 2点を対角とする矩形を求める
		TwoPointToRect(
			&rcSel,
			GetSelectionInfo().m_select.GetFrom(),	// 範囲選択開始
			GetSelectionInfo().m_select.GetTo()		// 範囲選択終了
		);
		memBuf->SetString(L"");

		//<< 2002/04/18 Azumaiya
		// サイズ分だけ要領をとっておく。
		// 結構大まかに見ています。
		LayoutInt i = rcSel.bottom - rcSel.top + 1; // 2013.05.06 「+1」

		// 最初に行数分の改行量を計算してしまう。
		int nBufSize = wcslen(WCODE::CRLF) * (Int)i;

		// 実際の文字量。
		const wchar_t* pLine = m_pEditDoc->m_layoutMgr.GetLineStr(rcSel.top, &nLineLen, &pLayout);
		for (; i!=LayoutInt(0) && pLayout; --i, pLayout=pLayout->GetNextLayout()) {
			pLine = pLayout->GetPtr() + pLayout->GetLogicOffset();
			nLineLen = LogicInt(pLayout->GetLengthWithEOL());
			if (pLine) {
				// 指定された桁に対応する行のデータ内の位置を調べる
				nIdxFrom	= LineColumnToIndex(pLayout, rcSel.left );
				nIdxTo		= LineColumnToIndex(pLayout, rcSel.right);

				nBufSize += nIdxTo - nIdxFrom;
			}
			if (bLineOnly) {	// 複数行選択の場合は先頭の行のみ
				break;
			}
		}

		// 大まかに見た容量を元にサイズをあらかじめ確保しておく。
		memBuf->AllocStringBuffer(nBufSize);
		//>> 2002/04/18 Azumaiya

		bool bExtEol = GetDllShareData().m_common.edit.bEnableExtEol;
		nRowNum = 0;
		for (nLineNum=rcSel.top; nLineNum<=rcSel.bottom; ++nLineNum) {
			const wchar_t* pLine = m_pEditDoc->m_layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
			if (pLine) {
				// 指定された桁に対応する行のデータ内の位置を調べる
				nIdxFrom	= LineColumnToIndex(pLayout, rcSel.left );
				nIdxTo		= LineColumnToIndex(pLayout, rcSel.right);
				// 2002.02.08 hor
				// pLineがNULLのとき(矩形エリアの端がEOFのみの行を含むとき)は以下を処理しない
				if (nIdxTo - nIdxFrom > 0) {
					if (WCODE::IsLineDelimiter(pLine[nIdxTo - 1], bExtEol)) {
						memBuf->AppendString(&pLine[nIdxFrom], nIdxTo - nIdxFrom - 1);
					}else {
						memBuf->AppendString(&pLine[nIdxFrom], nIdxTo - nIdxFrom);
					}
				}
			}
			++nRowNum;
			memBuf->AppendString(WCODE::CRLF);
			if (bLineOnly) {	// 複数行選択の場合は先頭の行のみ
				break;
			}
		}
	}else {
		memBuf->SetString(L"");

		//<< 2002/04/18 Azumaiya
		//  これから貼り付けに使う領域の大まかなサイズを取得する。
		//  大まかというレベルですので、サイズ計算の誤差が（容量を多く見積もる方に）結構出ると思いますが、
		// まぁ、速さ優先ということで勘弁してください。
		//  無駄な容量確保が出ていますので、もう少し精度を上げたいところですが・・・。
		//  とはいえ、逆に小さく見積もることになってしまうと、かなり速度をとられる要因になってしまうので
		// 困ってしまうところですが・・・。
		m_pEditDoc->m_layoutMgr.GetLineStr(GetSelectionInfo().m_select.GetFrom().GetY2(), &nLineLen, &pLayout);
		int nBufSize = 0;

		int i = (Int)(GetSelectionInfo().m_select.GetTo().y - GetSelectionInfo().m_select.GetFrom().y);

		// 先頭に引用符を付けるとき。
		if (pszQuote) {
			nBufSize += wcslen(pszQuote);
		}

		// 行番号を付ける。
		if (bWithLineNumber) {
			nBufSize += nLineNumCols;
		}

		// 改行コードについて。
		if (neweol == EolType::Unknown) {
			nBufSize += wcslen(WCODE::CRLF);
		}else {
			nBufSize += appendEol.GetLen();
		}

		// すべての行について同様の操作をするので、行数倍する。
		nBufSize *= (Int)i;

		// 実際の各行の長さ。
		for (; i!=0 && pLayout; --i, pLayout=pLayout->GetNextLayout()) {
			nBufSize += pLayout->GetLengthWithoutEOL() + appendEol.GetLen();
			if (bLineOnly) {	// 複数行選択の場合は先頭の行のみ
				break;
			}
		}

		// 調べた長さ分だけバッファを取っておく。
		memBuf->AllocStringBuffer(nBufSize);
		//>> 2002/04/18 Azumaiya

		for (nLineNum=GetSelectionInfo().m_select.GetFrom().GetY2(); nLineNum<=GetSelectionInfo().m_select.GetTo().y; ++nLineNum) {
			const wchar_t* pLine = m_pEditDoc->m_layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
			if (!pLine) {
				break;
			}
			if (nLineNum == GetSelectionInfo().m_select.GetFrom().y) {
				// 指定された桁に対応する行のデータ内の位置を調べる
				nIdxFrom = LineColumnToIndex(pLayout, GetSelectionInfo().m_select.GetFrom().GetX2());
			}else {
				nIdxFrom = LogicInt(0);
			}
			if (nLineNum == GetSelectionInfo().m_select.GetTo().y) {
				// 指定された桁に対応する行のデータ内の位置を調べる
				nIdxTo = LineColumnToIndex(pLayout, GetSelectionInfo().m_select.GetTo().GetX2());
			}else {
				nIdxTo = nLineLen;
			}
			if (nIdxTo - nIdxFrom == LogicInt(0)) {
				continue;
			}

			if (pszQuote && pszQuote[0] != L'\0') {	// 先頭に付ける引用符
				memBuf->AppendString(pszQuote);
			}
			if (bWithLineNumber) {	// 行番号を付与する
				auto_sprintf(pszLineNum, L" %d:" , nLineNum + 1);
				memBuf->AppendString(pszSpaces, nLineNumCols - wcslen(pszLineNum));
				memBuf->AppendString(pszLineNum);
			}

			if (EolType::None != pLayout->GetLayoutEol()) {
				if (nIdxTo >= nLineLen) {
					memBuf->AppendString(&pLine[nIdxFrom], nLineLen - 1 - nIdxFrom);
					//	Jul. 25, 2000 genta
					memBuf->AppendString((neweol == EolType::Unknown) ?
						(pLayout->GetLayoutEol()).GetValue2() :	//	コード保存
						appendEol.GetValue2());			//	新規改行コード
				}else {
					memBuf->AppendString(&pLine[nIdxFrom], nIdxTo - nIdxFrom);
				}
			}else {
				memBuf->AppendString(&pLine[nIdxFrom], nIdxTo - nIdxFrom);
				if (nIdxTo >= nLineLen) {
					if (bAddCRLFWhenCopy ||  // 折り返し行に改行を付けてコピー
						pszQuote || // 先頭に付ける引用符
						bWithLineNumber 	// 行番号を付与する
					) {
						//	Jul. 25, 2000 genta
						memBuf->AppendString((neweol == EolType::Unknown) ?
							m_pEditDoc->m_docEditor.GetNewLineCode().GetValue2() :	//	コード保存
							appendEol.GetValue2());		//	新規改行コード
					}
				}
			}
			if (bLineOnly) {	// 複数行選択の場合は先頭の行のみ
				break;
			}
		}
	}
	if (bWithLineNumber) {	// 行番号を付与する
		delete [] pszLineNum;
	}
	return true;
}

/* 選択範囲内の１行の選択
	@param bCursorPos 選択開始行の代わりにカーソル位置の行を取得
	通常選択ならロジック行、矩形なら選択範囲内のレイアウト行１行を選択
	2010.09.04 Moca 新規作成
*/
bool EditView::GetSelectedDataOne(NativeW& memBuf, int nMaxLen)
{
	LogicInt		nLineLen;
	LogicInt		nIdxFrom;
	LogicInt		nIdxTo;
	LogicInt		nSelectLen;
	auto& layoutMgr = m_pEditDoc->m_layoutMgr;
	auto& selInfo = GetSelectionInfo();

	if (!selInfo.IsTextSelected()) {
		return false;
	}

	auto& select = selInfo.m_select;

	memBuf.SetString(L"");
	if (selInfo.IsBoxSelecting()) {
		// 矩形範囲選択(レイアウト処理)
		const Layout* pLayout;
		LayoutRect rcSel;

		// 2点を対角とする矩形を求める
		TwoPointToRect(
			&rcSel,
			select.GetFrom(),	// 範囲選択開始
			select.GetTo()	// 範囲選択終了
		);
		const wchar_t* pLine = layoutMgr.GetLineStr(rcSel.top, &nLineLen, &pLayout);
		if (pLine && pLayout) {
			nLineLen = pLayout->GetLengthWithoutEOL();
			if (pLine) {
				// 指定された桁に対応する行のデータ内の位置を調べる
				nIdxFrom	= LineColumnToIndex(pLayout, rcSel.left );
				nIdxTo		= LineColumnToIndex(pLayout, rcSel.right);
			}
			nSelectLen = nIdxTo - nIdxFrom;
			if (0 < nSelectLen) {
				memBuf.AppendString(&pLine[nIdxFrom], t_min<int>(nMaxLen, t_min<int>(nSelectLen, nLineLen - nIdxFrom)));
			}
		}
	}else {
		// 線形選択(ロジック行処理)
		LogicPoint ptFrom;
		LogicPoint ptTo;
		layoutMgr.LayoutToLogic(select.GetFrom(), &ptFrom);
		layoutMgr.LayoutToLogic(select.GetTo(),   &ptTo);
		LogicInt targetY = ptFrom.y;

		const DocLine* pDocLine = m_pEditDoc->m_docLineMgr.GetLine(targetY);
		if (pDocLine) {
			const wchar_t* pLine = pDocLine->GetPtr();
			nLineLen = pDocLine->GetLengthWithoutEOL();
			nIdxFrom = ptFrom.x;
			if (targetY == ptTo.y) {
				nIdxTo = ptTo.x;
			}else {
				nIdxTo = nLineLen;
			}
			nSelectLen = nIdxTo - nIdxFrom;
			if (0 < nSelectLen) {
				memBuf.AppendString(&pLine[nIdxFrom], t_min<int>(nMaxLen, t_min<int>(nSelectLen, nLineLen - nIdxFrom)));
			}
		}
	}
	return 0 < memBuf.GetStringLength();
}

/* 指定カーソル位置が選択エリア内にあるか
	【戻り値】
	-1	選択エリアより前方 or 無選択
	0	選択エリア内
	1	選択エリアより後方
*/
int EditView::IsCurrentPositionSelected(
	LayoutPoint	ptCaretPos		// カーソル位置
	)
{
	auto& selInfo = GetSelectionInfo();
	if (!selInfo.IsTextSelected()) {	// テキストが選択されているか
		return -1;
	}
	LayoutRect	rcSel;
	LayoutPoint	po;
	auto& select = selInfo.m_select;

	// 矩形範囲選択中か
	if (selInfo.IsBoxSelecting()) {
		// 2点を対角とする矩形を求める
		TwoPointToRect(
			&rcSel,
			select.GetFrom(),	// 範囲選択開始
			select.GetTo()		// 範囲選択終了
		);
		++rcSel.bottom;
		po = ptCaretPos;
		if (IsDragSource()) {
			if (GetKeyState_Control()) { // Ctrlキーが押されていたか
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
				if (GetKeyState_Control()) {	// Ctrlキーが押されていたか
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
				if (GetKeyState_Control()) {	// Ctrlキーが押されていたか
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

/* 指定カーソル位置が選択エリア内にあるか (テスト)
	【戻り値】
	-1	選択エリアより前方 or 無選択
	0	選択エリア内
	1	選択エリアより後方
*/
// 2007.09.01 kobake 整理
int EditView::IsCurrentPositionSelectedTEST(
	const LayoutPoint& ptCaretPos,      // カーソル位置
	const LayoutRange& select //
	) const
{
	if (!GetSelectionInfo().IsTextSelected()) {	// テキストが選択されているか
		return -1;
	}

	if (PointCompare(ptCaretPos, select.GetFrom()) < 0) return -1;
	if (PointCompare(ptCaretPos, select.GetTo()) >= 0) return 1;

	return 0;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      クリップボード                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// 選択範囲内の全行をクリップボードにコピーする
void EditView::CopySelectedAllLines(
	const wchar_t*	pszQuote,		//!< 先頭に付ける引用符
	bool			bWithLineNumber	//!< 行番号を付与する
	)
{
	NativeW	memBuf;

	if (!GetSelectionInfo().IsTextSelected()) {	// テキストが選択されているか
		return;
	}
	{	// 選択範囲内の全行を選択状態にする
		LayoutRange sSelect(GetSelectionInfo().m_select);
		const Layout* pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(sSelect.GetFrom().y);
		if (!pLayout) return;
		sSelect.SetFromX(pLayout->GetIndent());
		pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY(sSelect.GetTo().y);
		if (pLayout && (GetSelectionInfo().IsBoxSelecting() || sSelect.GetTo().x > pLayout->GetIndent())) {
			// 選択範囲を次行頭まで拡大する
			sSelect.SetToY(sSelect.GetTo().y + 1);
			pLayout = pLayout->GetNextLayout();
		}
		sSelect.SetToX(pLayout? pLayout->GetIndent(): LayoutInt(0));
		GetCaret().GetAdjustCursorPos(sSelect.GetToPointer());	// EOF行を超えていたら座標修正

		GetSelectionInfo().DisableSelectArea(false); // 2011.06.03 true →false
		GetSelectionInfo().SetSelectArea(sSelect);

		GetCaret().MoveCursor(GetSelectionInfo().m_select.GetTo(), false);
		GetCaret().ShowEditCaret();
	}
	// 再描画
	//	::UpdateWindow();
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	Call_OnPaint((int)PaintAreaType::LineNumber | (int)PaintAreaType::Body, false);
	// To Here 2007.09.09 Moca
	// 選択範囲をクリップボードにコピー
	// 選択範囲のデータを取得
	// 正常時はTRUE,範囲未選択の場合は終了する
	if (!GetSelectedData(
			&memBuf,
			false,
			pszQuote, // 引用符
			bWithLineNumber, // 行番号を付与する
			GetDllShareData().m_common.edit.bAddCRLFWhenCopy // 折り返し位置に改行記号を入れる
		)
	) {
		ErrorBeep();
		return;
	}
	// クリップボードにデータを設定
	MySetClipboardData(memBuf.GetStringPtr(), memBuf.GetStringLength(), false);
}


/*! クリップボードからデータを取得
	@date 2005.05.29 novice UNICODE TEXT 対応処理を追加
	@date 2007.10.04 ryoji MSDEVLineSelect対応処理を追加
	@date 2008.09.10 bosagami パス貼り付け対応
*/
bool EditView::MyGetClipboardData(NativeW& memBuf, bool* pbColumnSelect, bool* pbLineSelect /*= NULL*/)
{
	if (pbColumnSelect)
		*pbColumnSelect = false;

	if (pbLineSelect)
		*pbLineSelect = false;

	if (!Clipboard::HasValidData())
		return false;
	
	Clipboard clipboard(GetHwnd());
	if (!clipboard)
		return false;

	Eol eol = m_pEditDoc->m_docEditor.GetNewLineCode();
	if (!clipboard.GetText(&memBuf, pbColumnSelect, pbLineSelect, eol)) {
		return false;
	}

	return true;
}

/* クリップボードにデータを設定
	@date 2004.02.17 Moca エラーチェックするように
 */
bool EditView::MySetClipboardData(const ACHAR* pszText, int nTextLen, bool bColumnSelect, bool bLineSelect /*= false*/)
{
	// WCHARに変換
	std::vector<wchar_t> buf;
	mbstowcs_vector(pszText, nTextLen, &buf);
	return MySetClipboardData(&buf[0], buf.size()-1, bColumnSelect, bLineSelect);
}

bool EditView::MySetClipboardData(const WCHAR* pszText, int nTextLen, bool bColumnSelect, bool bLineSelect /*= false*/)
{
	// Windowsクリップボードにコピー
	Clipboard clipboard(GetHwnd());
	if (!clipboard) {
		return false;
	}
	clipboard.Empty();
	return clipboard.SetText(pszText, nTextLen, bColumnSelect, bLineSelect);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      アンダーライン                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*! カーソルの縦線の座標で作画範囲か
	@2010.08.21 関数に分離。太線対応
*/
inline bool EditView::IsDrawCursorVLinePos(int posX)
{
	return posX >= GetTextArea().GetAreaLeft() - 2	// 2010.08.10 ryoji テキストと行番号の隙間が半角文字幅より大きいと隙間位置にあるカーソルの縦線が描画される問題修正
		&& posX >  GetTextArea().GetAreaLeft() - GetDllShareData().m_common.window.nLineNumRightSpace // 隙間(+1)がないときは線を引かない判定
		&& posX <= GetTextArea().GetAreaRight();
}

// カーソル行アンダーラインのON
void EditView::CaretUnderLineON(bool bDraw, bool bDrawPaint, bool DisalbeUnderLine)
{
	bool bUnderLine = m_pTypeData->colorInfoArr[COLORIDX_UNDERLINE].bDisp;
	bool bCursorVLine = m_pTypeData->colorInfoArr[COLORIDX_CURSORVLINE].bDisp;
	bool bCursorLineBg = m_pTypeData->colorInfoArr[COLORIDX_CARETLINEBG].bDisp;
	if (!bUnderLine && !bCursorVLine && !bCursorLineBg) {
		return;
	}
	
//	2010.08.24 下線・縦線が残るバグの修正
//	ON(作画)→ON(未作画)のパターンなどで、画面上に線が残ったままになるので座標をクリアしてはいけない
//	m_nOldCursorLineX = -1;
//	m_nOldUnderLineY  = -1;
	// 2011.12.06 Moca IsTextSelected → IsTextSelecting に変更。ロック中も下線を表示しない
	int bCursorLineBgDraw = false;
	
	// カーソル行の描画
	if (1
		&& bDraw
		&& bCursorLineBg
		&& GetDrawSwitch()
		&& GetCaret().GetCaretLayoutPos().GetY2() >= GetTextArea().GetViewTopLine()
		&& !m_bDoing_UndoRedo	// アンドゥ・リドゥの実行中か
	) {
		bCursorLineBgDraw = true;

		m_nOldUnderLineY = GetCaret().GetCaretLayoutPos().GetY2();
		m_nOldUnderLineYBg = m_nOldUnderLineY;
		m_nOldUnderLineYHeight = GetTextMetrics().GetHankakuDy();
		if (bDrawPaint) {
			GetCaret().m_underLine.Lock();
			PAINTSTRUCT ps;
			ps.rcPaint.left = 0;
			ps.rcPaint.right = GetTextArea().GetAreaRight();
			ps.rcPaint.top = GetTextArea().GenerateYPx(m_nOldUnderLineY);
			ps.rcPaint.bottom = ps.rcPaint.top + m_nOldUnderLineYHeight;

			// 描画
			HDC hdc = this->GetDC();
			OnPaint(hdc, &ps, FALSE);
			this->ReleaseDC(hdc);

			GetCaret().m_underLine.UnLock();
		}
	}
	
	int nCursorVLineX = -1;
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	if (bCursorVLine) {
		// カーソル位置縦線。-1してキャレットの左に来るように。
		nCursorVLineX = GetTextArea().GetAreaLeft() + (Int)(GetCaret().GetCaretLayoutPos().GetX2() - GetTextArea().GetViewLeftCol())
			* (m_pTypeData->nColumnSpace + GetTextMetrics().GetHankakuWidth()) - 1;
	}

	if (1
		&& bDraw
		&& GetDrawSwitch()
		&& IsDrawCursorVLinePos(nCursorVLineX)
		&& !m_bDoing_UndoRedo
		&& !GetSelectionInfo().IsTextSelecting()
		&& !DisalbeUnderLine
	) {
		m_nOldCursorLineX = nCursorVLineX;
		// カーソル位置縦線の描画
		// アンダーラインと縦線の交点で、下線が上になるように先に縦線を引く。
		HDC hdc = ::GetDC(GetHwnd());
		{
			Graphics gr(hdc);
			gr.SetPen(m_pTypeData->colorInfoArr[COLORIDX_CURSORVLINE].colorAttr.cTEXT);
			::MoveToEx(gr, m_nOldCursorLineX, GetTextArea().GetAreaTop(), NULL);
			::LineTo(  gr, m_nOldCursorLineX, GetTextArea().GetAreaBottom());
			int nBoldX = m_nOldCursorLineX - 1;
			// 「太字」のときは2dotの線にする。その際カーソルに掛からないように左側を太くする
			if (1
				&& m_pTypeData->colorInfoArr[COLORIDX_CURSORVLINE].fontAttr.bBoldFont
				&& IsDrawCursorVLinePos(nBoldX)
			) {
				::MoveToEx(gr, nBoldX, GetTextArea().GetAreaTop(), NULL);
				::LineTo(  gr, nBoldX, GetTextArea().GetAreaBottom());
				m_nOldCursorVLineWidth = 2;
			}else {
				m_nOldCursorVLineWidth = 1;
			}
		}	// ReleaseDC の前に gr デストラクト
		::ReleaseDC(GetHwnd(), hdc);
	}
	
	int nUnderLineY = -1;
	if (bUnderLine) {
		nUnderLineY = GetTextArea().GetAreaTop() + (Int)(GetCaret().GetCaretLayoutPos().GetY2() - GetTextArea().GetViewTopLine())
			 * GetTextMetrics().GetHankakuDy() + GetTextMetrics().GetHankakuHeight();
	}
	// To Here 2007.09.09 Moca

	if (1
		&& bDraw
		&& GetDrawSwitch()
		&& nUnderLineY >= GetTextArea().GetAreaTop()
		&& !m_bDoing_UndoRedo	// アンドゥ・リドゥの実行中か
		&& !GetSelectionInfo().IsTextSelecting()
		&& !DisalbeUnderLine
	) {
		if (!bCursorLineBgDraw || m_nOldUnderLineY == -1) {
			m_nOldUnderLineY = GetCaret().GetCaretLayoutPos().GetY2();
			m_nOldUnderLineYBg = m_nOldUnderLineY;
		}
		m_nOldUnderLineYMargin = GetTextMetrics().GetHankakuHeight();
		m_nOldUnderLineYHeightReal = 1;
//		MYTRACE(_T("★カーソル行アンダーラインの描画\n"));
		// ★カーソル行アンダーラインの描画
		HDC		hdc = ::GetDC(GetHwnd());
		{
			Graphics gr(hdc);
			gr.SetPen(m_pTypeData->colorInfoArr[COLORIDX_UNDERLINE].colorAttr.cTEXT);
			::MoveToEx(
				gr,
				GetTextArea().GetAreaLeft(),
				nUnderLineY,
				NULL
			);
			::LineTo(
				gr,
				GetTextArea().GetAreaRight(),
				nUnderLineY
			);
		}	// ReleaseDC の前に gr デストラクト
		::ReleaseDC(GetHwnd(), hdc);
	}
}

// カーソル行アンダーラインのOFF
void EditView::CaretUnderLineOFF(bool bDraw, bool bDrawPaint, bool bResetFlag, bool DisalbeUnderLine)
{
	if (1
		&& !m_pTypeData->colorInfoArr[COLORIDX_UNDERLINE].bDisp
		&& !m_pTypeData->colorInfoArr[COLORIDX_CURSORVLINE].bDisp
		&& !m_pTypeData->colorInfoArr[COLORIDX_CARETLINEBG].bDisp
	) {
		return;
	}
	if (m_nOldUnderLineY != -1) {
		if (1
			&& bDraw
			&& GetDrawSwitch()
			&& m_nOldUnderLineY >= GetTextArea().GetViewTopLine()
			&& !m_bDoing_UndoRedo	// アンドゥ・リドゥの実行中か
			&& !GetCaret().m_underLine.GetUnderLineDoNotOFF()	// アンダーラインを消去するか
		) {
			// -- -- カーソル行アンダーラインの消去（無理やり） -- -- //
			int nUnderLineY; // client px
			LayoutYInt nY = m_nOldUnderLineY - GetTextArea().GetViewTopLine();
			if (nY < 0) {
				nUnderLineY = -1;
			}else if (GetTextArea().m_nViewRowNum < nY) {
				nUnderLineY = GetTextArea().GetAreaBottom() + 1;
			}else {
				nUnderLineY = GetTextArea().GetAreaTop() + (Int)(nY) * GetTextMetrics().GetHankakuDy();
			}
			
			GetCaret().m_underLine.Lock();

			PAINTSTRUCT ps;
			ps.rcPaint.left = 0;
			ps.rcPaint.right = GetTextArea().GetAreaRight();
			int height;
			if (bDrawPaint && m_nOldUnderLineYHeight != 0) {
				ps.rcPaint.top = nUnderLineY;
				height = t_max(m_nOldUnderLineYHeight, m_nOldUnderLineYMargin + m_nOldUnderLineYHeightReal);
			}else {
				ps.rcPaint.top = nUnderLineY + m_nOldUnderLineYMargin;
				height = m_nOldUnderLineYHeightReal;
			}
			ps.rcPaint.bottom = ps.rcPaint.top + height;

			//	不本意ながら選択情報をバックアップ。
//			LayoutRange sSelectBackup = GetSelectionInfo().m_select;
//			GetSelectionInfo().m_select.Clear(-1);

			if (ps.rcPaint.bottom - ps.rcPaint.top) {
				// 描画
				HDC hdc = this->GetDC();
				// 可能なら互換BMPからコピーして再作画
				OnPaint(hdc, &ps, (ps.rcPaint.bottom - ps.rcPaint.top) == 1);
				this->ReleaseDC(hdc);
			}
			m_nOldUnderLineYHeight = 0;


			//	選択情報を復元
			GetCaret().m_underLine.UnLock();
			
			if (bDrawPaint) {
				m_nOldUnderLineYBg = -1;
			}
		}
		if (bResetFlag) {
			m_nOldUnderLineY = -1;
		}
		m_nOldUnderLineYHeightReal = 0;
	}

	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	// カーソル位置縦線
	if (m_nOldCursorLineX != -1) {
		if (1
			&& bDraw
			&& GetDrawSwitch()
			&& IsDrawCursorVLinePos(m_nOldCursorLineX)
			&& !m_bDoing_UndoRedo
			&& !GetCaret().m_underLine.GetVertLineDoNotOFF()	// カーソル位置縦線を消去するか
			&& !DisalbeUnderLine
		) {
			PAINTSTRUCT ps;
			ps.rcPaint.left = m_nOldCursorLineX - (m_nOldCursorVLineWidth - 1);
			ps.rcPaint.right = m_nOldCursorLineX + 1;
			ps.rcPaint.top = GetTextArea().GetAreaTop();
			ps.rcPaint.bottom = GetTextArea().GetAreaBottom();
			HDC hdc = ::GetDC(GetHwnd());
			GetCaret().m_underLine.Lock();
			//	不本意ながら選択情報をバックアップ。
			LayoutRange sSelectBackup = this->GetSelectionInfo().m_select;
			this->GetSelectionInfo().m_select.Clear(-1);
			// 可能なら互換BMPからコピーして再作画
			OnPaint(hdc, &ps, TRUE);
			//	選択情報を復元
			this->GetSelectionInfo().m_select = sSelectBackup;
			GetCaret().m_underLine.UnLock();
			ReleaseDC(hdc);
		}
		m_nOldCursorLineX = -1;
	}
	// To Here 2007.09.09 Moca
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         状態表示                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	検索／置換／ブックマーク検索時の状態をステータスバーに表示する

	@date 2002.01.26 hor 新規作成
	@date 2002.12.04 genta 実体をEditWndへ移動
*/
void EditView::SendStatusMessage(const TCHAR* msg)
{
	m_pEditWnd->SendStatusMessage(msg);
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        編集モード                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	挿入モード取得

	@date 2005.10.02 genta 管理方法変更のため関数化
*/
bool EditView::IsInsMode(void) const
{
	return m_pEditDoc->m_docEditor.IsInsMode();
}

void EditView::SetInsMode(bool mode)
{
	m_pEditDoc->m_docEditor.SetInsMode(mode);
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         イベント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void EditView::OnAfterLoad(const LoadInfo& loadInfo)
{
	if (!GetHwnd()) {
		// MiniMap 非表示
		return;
	}
	// -- -- ※ InitAllViewでやってたこと -- -- //
	m_pHistory->Flush();

	// 現在の選択範囲を非選択状態に戻す
	GetSelectionInfo().DisableSelectArea(false);

	OnChangeSetting();
	GetCaret().MoveCursor(LayoutPoint(0, 0), true);
	GetCaret().m_nCaretPosX_Prev = LayoutInt(0);
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//	2004.05.13 Moca 改行コードの設定内からここに移動
	m_pEditWnd->GetActiveView().GetCaret().ShowCaretPosInfo();
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          その他                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



//!	現在のカーソル行位置を履歴に登録する
void EditView::AddCurrentLineToHistory(void)
{
	LogicPoint ptPos;

	m_pEditDoc->m_layoutMgr.LayoutToLogic(GetCaret().GetCaretLayoutPos(), &ptPos);

	MarkMgr::Mark m(ptPos);
	m_pHistory->Add(m);
}


//	2001/06/18 Start by asa-o: 補完ウィンドウ用のキーワードヘルプ表示
bool  EditView::ShowKeywordHelp(POINT po, LPCWSTR pszHelp, LPRECT prcHokanWin)
{
	NativeW	memCurText;
	RECT		rcTipWin,
				rcDesktop;

	if (m_pTypeData->bUseKeywordHelp) { // キーワードヘルプを使用する
		if (!m_bInMenuLoop			// メニュー モーダル ループに入っていない
			&& m_dwTipTimer != 0	// 辞書Tipを表示していない
		) {
			memCurText.SetString(pszHelp);

			// 既に検索済みか
			if (NativeW::IsEqual(memCurText, m_tipWnd.m_key)) {
				// 該当するキーがなかった
				if (!m_tipWnd.m_KeyWasHit) {
					return false;
				}
			}else {
				m_tipWnd.m_key = memCurText;
				// 検索実行
				if (!KeySearchCore(&m_tipWnd.m_key))	// 2006.04.10 fon
					return FALSE;
			}
			m_dwTipTimer = 0;	// 辞書Tipを表示している

		// 2001/06/19 Start by asa-o: 辞書Tipの表示位置調整
			// 辞書Tipのサイズを取得
			m_tipWnd.GetWindowSize(&rcTipWin);

			//	May 01, 2004 genta マルチモニタ対応
			::GetMonitorWorkRect(m_tipWnd.GetHwnd(), &rcDesktop);

			// 右に入る
			if (prcHokanWin->right + rcTipWin.right < rcDesktop.right) {
				// そのまま
			// 左に入る
			}else if (rcDesktop.left < prcHokanWin->left - rcTipWin.right) {
				// 左に表示
				po.x = prcHokanWin->left - (rcTipWin.right + 8);
			// どちらもスペースが無いとき広いほうに表示
			}else if (rcDesktop.right - prcHokanWin->right > prcHokanWin->left) {
				// 右に表示 そのまま
			}else {
				// 左に表示
				po.x = prcHokanWin->left - (rcTipWin.right + 8);
			}
		// 2001/06/19 End

			// 辞書Tipを表示
			m_tipWnd.Show(po.x, po.y , NULL , &rcTipWin);
			return true;
		}
	}
	return false;
}
//	2001/06/18 End

/*!
	@brief 指定位置または指定範囲がテキストの存在しないエリアかチェックする

	@param[in] ptFrom  指定位置または指定範囲開始
	@param[in] ptTo    指定範囲終了
	@param[in] bSelect    範囲指定
	@param[in] bBoxSelect 矩形選択
	
	@retval true  指定位置または指定範囲内にテキストが存在しない
			false 指定位置または指定範囲内にテキストが存在する

	@date 2008.08.03 nasukoji	新規作成
*/
bool EditView::IsEmptyArea(LayoutPoint ptFrom, LayoutPoint ptTo, bool bSelect, bool bBoxSelect) const
{
	bool result;

	LayoutInt nColumnFrom = ptFrom.GetX2();
	LayoutInt nLineFrom = ptFrom.GetY2();
	LayoutInt nColumnTo = ptTo.GetX2();
	LayoutInt nLineTo = ptTo.GetY2();

	if (bSelect && !bBoxSelect && nLineFrom != nLineTo) {	// 複数行の範囲指定
		// 複数行通常選択した場合、必ずテキストを含む
		result = false;
	}else {
		if (bSelect) {
			// 範囲の調整
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
		LayoutInt nLineLen;

		result = true;
		for (LayoutInt nLineNum=nLineFrom; nLineNum<=nLineTo; ++nLineNum) {
			if ((pLayout = m_pEditDoc->m_layoutMgr.SearchLineByLayoutY(nLineNum))) {
				// 指定位置に対応する行のデータ内の位置
				LineColumnToIndex2(pLayout, nColumnFrom, &nLineLen);
				if (nLineLen == 0) {	// 折り返しや改行コードより右の場合には nLineLen に行全体の表示桁数が入る
					result = false;		// 指定位置または指定範囲内にテキストがある
					break;
				}
			}
		}
	}

	return result;
}

//! アンドゥバッファの処理
void EditView::SetUndoBuffer(bool bPaintLineNumber)
{
	OpeBlk* pOpe = m_commander.GetOpeBlk();
	if (pOpe && pOpe->Release() == 0) {
		if (0 < pOpe->GetNum()) {	// 操作の数を返す
			// 操作の追加
			GetDocument()->m_docEditor.m_opeBuf.AppendOpeBlk(pOpe);

			// 2013.05.01 Moca 正確に変更行を表示するようになったので不要
			//  if (bPaintLineNumber
			//   &&	m_pEditDoc->m_docEditor.m_opeBuf.GetCurrentPointer() == 1)	// 全Undo状態からの変更か？	// 2009.03.26 ryoji
			//  	Call_OnPaint(PaintAreaType::LineNumber, false);	// 自ペインの行番号（変更行）表示を更新 ← 変更行のみの表示更新で済ませている場合があるため

			if (!m_pEditWnd->UpdateTextWrap())	{	// 折り返し方法関連の更新	// 2008.06.10 ryoji
				if (0 < pOpe->GetNum() - GetDocument()->m_docEditor.m_nOpeBlkRedawCount) {
					m_pEditWnd->RedrawAllViews(this);	//	他のペインの表示を更新
				}
			}
		}else {
			delete pOpe;
		}
		m_commander.SetOpeBlk(NULL);
	}
}

