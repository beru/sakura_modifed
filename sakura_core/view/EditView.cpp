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
		pEdit = (EditView*) g_pEditView;
		return pEdit->DispatchEvent(hwnd, uMsg, wParam, lParam);
	default:
		pEdit = (EditView*) ::GetWindowLongPtr(hwnd, 0);
		if (pEdit) {
			if (uMsg == WM_COMMAND) {
				::SendMessage(::GetParent(pEdit->hwndParent), WM_COMMAND, wParam,  lParam);
			}else {
				return pEdit->DispatchEvent(hwnd, uMsg, wParam, lParam);
			}
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
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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
	bActivateByMouse(false),
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


BOOL EditView::Create(
	HWND		hwndParent,	// 親
	EditDoc&	editDoc,	// 参照するドキュメント
	int			nMyIndex,	// ビューのインデックス
	BOOL		bShow,		// 作成時に表示するかどうか
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
	pRegexKeyword = nullptr;

	SetDrawSwitch(true);
	pDropTarget = new DropTarget(this);
	_SetDragMode(FALSE);					// 選択テキストのドラッグ中か
	bCurSrchKeyMark = false;				// 検索文字列
	strCurSearchKey.clear();
	curSearchOption.Reset();				// 検索／置換 オプション
	bCurSearchUpdate = false;
	nCurSearchKeySequence = -1;

	nMyIndex = 0;

	//	メニューバーへのメッセージ表示機能はEditWndへ移管

	// 共有データ構造体のアドレスを返す
	bCommandRunning = false;	// コマンドの実行中
	bDoing_UndoRedo = false;	// Undo, Redoの実行中か
	pcsbwVSplitBox = nullptr;	// 垂直分割ボックス
	pcsbwHSplitBox = nullptr;	// 水平分割ボックス
	hwndVScrollBar = NULL;
	nVScrollRate = 1;			// 垂直スクロールバーの縮尺
	hwndHScrollBar = NULL;
	hwndSizeBox = NULL;

	ptSrchStartPos_PHY.Set(-1, -1);	// 検索/置換開始時のカーソル位置  (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	bSearch = false;					// 検索/置換開始位置を登録するか
	
	ptBracketPairPos_PHY.Set(-1, -1); // 対括弧の位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	ptBracketCaretPos_PHY.Set(-1, -1);

	bDrawBracketPairFlag = false;
	GetSelectionInfo().bDrawSelectArea = false;	// 選択範囲を描画したか

	crBack = -1;				// テキストの背景色
	crBack2 = -1;
	
	szComposition[0] = _T('\0');

	auto& textArea = GetTextArea();
	// ルーラー表示
	textArea.SetAreaTop(textArea.GetAreaTop() + GetDllShareData().common.window.nRulerHeight);	// ルーラー高さ
	GetRuler().SetRedrawFlag();	// ルーラー全体を描き直す時=true
	hdcCompatDC = NULL;			// 再描画用コンパチブルＤＣ
	hbmpCompatBMP = NULL;		// 再描画用メモリＢＭＰ
	hbmpCompatBMPOld = NULL;	// 再描画用メモリＢＭＰ(OLD)
	nCompatBMPWidth = -1;
	nCompatBMPHeight = -1;
	
	nOldUnderLineY = -1;
	nOldCursorLineX = -1;
	nOldCursorVLineWidth = 1;
	nOldUnderLineYHeightReal = 0;

	curRegexp.InitDll(GetDllShareData().common.search.szRegexpLib);

	dwTipTimer = ::GetTickCount();	// 辞書Tip起動タイマー
	bInMenuLoop = false;				// メニュー モーダル ループに入っています
//	MYTRACE(_T("EditView::EditView()おわり\n"));
	bHokan = false;

	pHistory->SetMax(30);

	// OSによって再変換の方式を変える
	if (!OsSupportReconvert()) {
		// 95 or NTならば
		uMSIMEReconvertMsg = ::RegisterWindowMessage(RWM_RECONVERT);
		uATOKReconvertMsg = ::RegisterWindowMessage(MSGNAME_ATOK_RECONVERT) ;
		uWM_MSIME_RECONVERTREQUEST = ::RegisterWindowMessage(_T("MSIMEReconvertRequest"));
		
		hAtokModule = LoadLibraryExedir(_T("ATOK10WC.DLL"));
		AT_ImmSetReconvertString = NULL;
		if (hAtokModule) {
			AT_ImmSetReconvertString = (BOOL (WINAPI *)(HIMC, int, PRECONVERTSTRING, DWORD )) GetProcAddress(hAtokModule,"AT_ImmSetReconvertString");
		}
	}else { 
		// それ以外のOSのときはOS標準を使用する
		uMSIMEReconvertMsg = 0;
		uATOKReconvertMsg = 0 ;
		hAtokModule = 0;
	}
	
	nISearchMode = 0;
	pMigemo = nullptr;

	dwTripleClickCheck = 0;		// トリプルクリックチェック用時刻初期化

	mouseDownPos.Set(-INT_MAX, -INT_MAX);

	bMiniMapMouseDown = false;

	//↑今までコンストラクタでやってたこと
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//↓今までCreateでやってたこと

	WNDCLASS wc;
	this->hwndParent = hwndParent;
	pEditDoc = &editDoc;
	pTypeData = &editDoc.docType.GetDocumentAttribute();
	this->nMyIndex = nMyIndex;

	pRegexKeyword = new RegexKeyword(GetDllShareData().common.search.szRegexpLib);
	pRegexKeyword->RegexKeySetTypes(pTypeData);

	textArea.SetTopYohaku(GetDllShareData().common.window.nRulerBottomSpace); 	// ルーラーとテキストの隙間
	textArea.SetAreaTop(textArea.GetTopYohaku());								// 表示域の上端座標
	// ルーラー表示
	if (pTypeData->colorInfoArr[COLORIDX_RULER].bDisp) {
		textArea.SetAreaTop(textArea.GetAreaTop() + GetDllShareData().common.window.nRulerHeight);	// ルーラー高さ
	}

	// ウィンドウクラスの登録
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

	// 辞書Tip表示ウィンドウ作成
	tipWnd.Create(G_AppInstance(), GetHwnd()/*GetDllShareData().sHandles.hwndTray*/);

	// 再描画用コンパチブルＤＣ
	UseCompatibleDC(GetDllShareData().common.window.bUseCompatibleBMP);

	// 垂直分割ボックス
	pcsbwVSplitBox = new SplitBoxWnd;
	pcsbwVSplitBox->Create(G_AppInstance(), GetHwnd(), TRUE);
	// 水平分割ボックス
	pcsbwHSplitBox = new SplitBoxWnd;
	pcsbwHSplitBox->Create(G_AppInstance(), GetHwnd(), FALSE);

	// スクロールバー作成
	CreateScrollBar();

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

	// 再描画用コンパチブルＤＣ
	//	hbmpCompatBMPもここで削除される．
	UseCompatibleDC(FALSE);

	delete pDropTarget;
	pDropTarget = nullptr;

	delete pHistory;
	pHistory = nullptr;

	delete pRegexKeyword;
	pRegexKeyword = nullptr;
	
	// 再変換
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
//                         イベント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// TCHAR→wchar_t変換。
inline wchar_t tchar_to_wchar(TCHAR tc)
{
	return tc;
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
		if (editWnd.DoMouseWheel(wParam, lParam)) {
			return 0L;
		}
		return OnMOUSEWHEEL(wParam, lParam);

	case WM_MOUSEHWHEEL:
		return OnMOUSEHWHEEL(wParam, lParam);

	case WM_CREATE:
		::SetWindowLongPtr(hwnd, 0, (LONG_PTR) this);
		return 0L;

	case WM_SHOWWINDOW:
		// ウィンドウ非表示の再に互換BMPを廃棄してメモリーを節約する
		if (hwnd == GetHwnd() && (BOOL)wParam == FALSE) {
			DeleteCompatibleBitmap();
		}
		return 0L;

	case WM_SIZE:
		OnSize(LOWORD(lParam), HIWORD(lParam));
		return 0L;

	case WM_SETFOCUS:
		OnSetFocus();
		// 親ウィンドウのタイトルを更新
		editWnd.UpdateCaption();
		return 0L;
	case WM_KILLFOCUS:
		OnKillFocus();
		// ホイールスクロール有無状態をクリア
		editWnd.ClearMouseState();
		return 0L;
	case WM_CHAR:
		// コントロールコード入力禁止
		if (WCODE::IsControlCode((wchar_t)wParam)) {
			ErrorBeep();
		}else {
			GetCommander().HandleCommand(F_WCHAR, true, (wchar_t)wParam, 0, 0, 0);
		}
		return 0L;

	case WM_IME_NOTIFY:
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
			dwSize += sizeof(wchar_t);

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
			if (bHideMouse && 0 <= nMousePouse) {
				nMousePouse = -1;
				::SetCursor(NULL);
			}
			GetCommander().HandleCommand(F_INSTEXT_W, true, (LPARAM)lptstr, wcslen(lptstr), TRUE, 0);
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
		if (!IsInsMode()) { /* 上書きモードか？ */
			GetCommander().HandleCommand(F_IME_CHAR, true, wParam, 0, 0, 0);
		}
		return 0L;

	case WM_PASTE:
		return GetCommander().HandleCommand(F_PASTE, true, 0, 0, 0, 0);

	case WM_COPY:
		return GetCommander().HandleCommand(F_COPY, true, 0, 0, 0, 0);

	case WM_KEYUP:
		// キーリピート状態
		GetCommander().bPrevCommand = 0;
		return 0L;

	// ALT+xでALTを押したままだとキーリピートがOFFにならない対策
	case WM_SYSKEYUP:
		GetCommander().bPrevCommand = 0;
		// 念のため呼ぶ
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);

	case WM_LBUTTONDBLCLK:
		if (bMiniMap) {
			return 0L;
		}
		// 非アクティブウィンドウのダブルクリック時はここでカーソルを移動する
		if (bActivateByMouse) {
			// アクティブなペインを設定
			editWnd.SetActivePane(nMyIndex);
			// カーソルをクリック位置へ移動する
			OnLBUTTONDOWN(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));	
			bActivateByMouse = false;		// マウスによるアクティベートを示すフラグをOFF
		}
		//		MYTRACE(_T(" WM_LBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD(lParam), HIWORD(lParam));
		OnLBUTTONDBLCLK(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		return 0L;

	case WM_MBUTTONDOWN:
		OnMBUTTONDOWN(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));

		return 0L;

	case WM_MBUTTONUP:
		OnMBUTTONUP(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		return 0L;

	case WM_LBUTTONDOWN:
		bActivateByMouse = false;		// マウスによるアクティベートを示すフラグをOFF
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
//			// アクティブなペインを設定
//			editWnd.SetActivePane(nMyIndex);
//		}
//		return 0L;
	case WM_RBUTTONUP:
//		MYTRACE(_T(" WM_RBUTTONUP wParam=%08xh, x=%d y=%d\n"), wParam, LOWORD(lParam), HIWORD(lParam));
		OnRBUTTONUP(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));
		return 0L;

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
		{
			int Scroll = OnVScroll(
				(int) LOWORD(wParam), ((int) HIWORD(wParam)) * nVScrollRate);

			//	シフトキーが押されていないときだけ同期スクロール
			if (!GetKeyState_Shift()) {
				SyncScrollV(Scroll);
			}
		}

		return 0L;

	case WM_HSCROLL:
//		MYTRACE(_T("	WM_HSCROLL nPos=%d\n"), GetScrollPos(hwndHScrollBar, SB_CTL));
		{
			int Scroll = OnHScroll(
				(int) LOWORD(wParam), ((int) HIWORD(wParam)));

			//	シフトキーが押されていないときだけ同期スクロール
			if (!GetKeyState_Shift()) {
				SyncScrollH(Scroll);
			}
		}

		return 0L;

	case WM_ENTERMENULOOP:
		bInMenuLoop = true;	// メニュー モーダル ループに入っています

		// 辞書Tipが起動されている
		if (dwTipTimer == 0) {
			// 辞書Tipを消す
			tipWnd.Hide();
			dwTipTimer = ::GetTickCount();	// 辞書Tip起動タイマー
		}
		if (bHokan) {
			editWnd.hokanMgr.Hide();
			bHokan = false;
		}
		return 0L;

	case WM_EXITMENULOOP:
		bInMenuLoop = false;	// メニュー モーダル ループに入っています
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

		// タイマー終了
		::KillTimer(GetHwnd(), IDT_ROLLMOUSE);


//		MYTRACE(_T("	WM_DESTROY\n"));
		/*
		||子ウィンドウの破棄
		*/
		if (hwndVScrollBar) {
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
		SAFE_DELETE(pcsbwVSplitBox);	// 垂直分割ボックス
		SAFE_DELETE(pcsbwHSplitBox);	// 水平分割ボックス

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

	case MYWM_IME_REQUEST:  // 再変換
		
		switch (wParam) {
		case IMR_RECONVERTSTRING:
			return SetReconvertStruct((PRECONVERTSTRING)lParam, UNICODE_BOOL);
			
		case IMR_CONFIRMRECONVERTSTRING:
			return SetSelectionFromReonvert((PRECONVERTSTRING)lParam, UNICODE_BOOL);
			
		// MS-IME 2002 だと「カーソル位置の前後の内容を参照して変換を行う」の機能
		case IMR_DOCUMENTFEED:
			return SetReconvertStruct((PRECONVERTSTRING)lParam, UNICODE_BOOL, true);
			
		//default:
		}
		// 0LではなくTSFが何かするかもしれないのでDefにまかせる
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	
	case MYWM_DROPFILES:	// 独自のドロップファイル通知
		OnMyDropFiles((HDROP)wParam);
		return 0L;

	// マウスクリックにてアクティベートされた時はカーソル位置を移動しない
	case WM_MOUSEACTIVATE:
		LRESULT nRes;
		nRes = ::DefWindowProc(hwnd, uMsg, wParam, lParam);	// 親に先に処理させる
		if (nRes == MA_NOACTIVATE || nRes == MA_NOACTIVATEANDEAT) {
			return nRes;
		}

		// マウスクリックによりバックグラウンドウィンドウがアクティベートされた
		if (1
			&& GetDllShareData().common.general.bNoCaretMoveByActivation
			&& !editWnd.IsActiveApp()
		) {
			bActivateByMouse = true;		// マウスによるアクティベート
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
				if (!bMiniMap) {
					editWnd.SetActivePane(nMyIndex);
				}
			}else if (0
				|| (pcsbwVSplitBox && hwndCursorPos == pcsbwVSplitBox->GetHwnd())
				|| (pcsbwHSplitBox && hwndCursorPos == pcsbwHSplitBox->GetHwnd())
			) {
				// 分割ボックス上にマウスがあるときはアクティブペインを切り替えない
				// （併せて MYWM_SETACTIVEPANE のポストにより分割線のゴミが残っていた問題も修正）
			}else {
				// スクロールバー上にマウスがあるかもしれないので MYWM_SETACTIVEPANE をポストする
				// SetActivePane() にはスクロールバーのスクロール範囲調整処理が含まれているが、
				// このタイミング（WM_MOUSEACTIVATE）でスクロール範囲を変更するのはまずい。
				// 例えば Win XP/Vista だとスクロール範囲が小さくなってスクロールバーが有効から
				// 無効に切り替わるとそれ以後スクロールバーが機能しなくなる。
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
		// wParam RedoUndoフラグは無視する
		if (lParam) {
			GetCommander().HandleCommand(F_INSTEXT_W, true, lParam, wcslen((wchar_t*)lParam), TRUE, 0);
		}
		return 0L; // not use.
	}

	default:
		if (0
			|| (uMSIMEReconvertMsg && (uMsg == uMSIMEReconvertMsg)) 
			|| (uATOKReconvertMsg && (uMsg == uATOKReconvertMsg))
		) {
			switch (wParam) {
			case IMR_RECONVERTSTRING:
				return SetReconvertStruct((PRECONVERTSTRING)lParam, true);
				
			case IMR_CONFIRMRECONVERTSTRING:
				return SetSelectionFromReonvert((PRECONVERTSTRING)lParam, true);
			}
			return 0L;
		}

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
		// ウィンドウ無効時にも互換BMPを破棄する
		DeleteCompatibleBitmap();
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
	if (pcsbwVSplitBox) {
		nVSplitHeight = 7;
		::MoveWindow(pcsbwVSplitBox->GetHwnd(), cx - nCxVScroll , 0, nCxVScroll, nVSplitHeight, TRUE);
	}
	// 水平分割ボックス
	if (pcsbwHSplitBox) {
		nHSplitWidth = 7;
		::MoveWindow(pcsbwHSplitBox->GetHwnd(), 0, cy - nCyHScroll, nHSplitWidth, nCyHScroll, TRUE);
	}
	// 垂直スクロールバー
	if (hwndVScrollBar) {
		::MoveWindow(hwndVScrollBar, cx - nCxVScroll , 0 + nVSplitHeight, nCxVScroll, cy - nCyVScroll - nVSplitHeight, TRUE);
	}
	// 水平スクロールバー
	if (hwndHScrollBar) {
		::MoveWindow(hwndHScrollBar, 0 + nHSplitWidth, cy - nCyHScroll, cx - nCxVScroll - nHSplitWidth, nCyHScroll, TRUE);
	}

	// サイズボックス
	if (hwndSizeBox) {
		::MoveWindow(hwndSizeBox, cx - nCxVScroll, cy - nCyHScroll, nCxHScroll, nCyVScroll, TRUE);
	}
	auto& textArea = GetTextArea();
	int nAreaWidthOld  = textArea.GetAreaWidth();
	int nAreaHeightOld = textArea.GetAreaHeight();

	// エリア情報更新
	textArea.TextArea_OnSize(
		Size(cx, cy),
		nCxVScroll,
		hwndHScrollBar ? nCyHScroll : 0
	);

	// 再描画用メモリＢＭＰ
	if (hdcCompatDC) {
		CreateOrUpdateCompatibleBitmap(cx, cy);
 	}

	// サイズ変更時の折り返し位置再計算
	bool wrapChanged = false;
	if (pEditDoc->nTextWrapMethodCur == TextWrappingMethod::WindowWidth) {
		if (nMyIndex == 0) {	// 左上隅のビューのサイズ変更時のみ処理する
			// 右端で折り返すモードなら右端で折り返す
			wrapChanged = editWnd.WrapWindowWidth(0);
		}
	}

	if (!wrapChanged) {	// 折り返し位置が変更されていない
		AdjustScrollBars();				// スクロールバーの状態を更新する
	}

	// キャレットの表示(右・下に隠れていた場合)
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

	// 親ウィンドウのタイトルを更新
	editWnd.UpdateCaption(); // [Q] 本当に必要？

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


// 入力フォーカスを受け取ったときの処理
void EditView::OnSetFocus(void)
{
	if (bMiniMap) {
		return;
	}
	// EOFのみのレイアウト行は、0桁目のみ有効.EOFより下の行のある場合は、EOF位置にする
	{
		Point ptPos = GetCaret().GetCaretLayoutPos();
		if (GetCaret().GetAdjustCursorPos(&ptPos)) {
			GetCaret().MoveCursor(ptPos, false);
			GetCaret().nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;
		}
	}

	GetCaret().ShowEditCaret();

	SetIMECompFormFont();

	// ルーラのカーソルをグレーから黒に変更する
	HDC hdc = ::GetDC(GetHwnd());
	GetRuler().DispRuler(hdc);
	::ReleaseDC(GetHwnd(), hdc);

	// 対括弧の強調表示(描画)
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


// 入力フォーカスを失ったときの処理
void EditView::OnKillFocus(void)
{
	if (bMiniMap) {
		return;
	}
	DrawBracketPair(false);
	bDrawBracketPairFlag = false;

	GetCaret().DestroyCaret();

	// ルーラー描画
	// ルーラのカーソルを黒からグレーに変更する
	HDC	hdc = ::GetDC(GetHwnd());
	GetRuler().DispRuler(hdc);
	::ReleaseDC(GetHwnd(), hdc);

	// 辞書Tipが起動されている
	if (dwTipTimer == 0) {
		// 辞書Tipを消す
		tipWnd.Hide();
		dwTipTimer = ::GetTickCount();	// 辞書Tip起動タイマー
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

	// IMEのフォントも変更
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
	GetCaret().nCaretPosX_Prevの更新も併せて行う．
*/
void EditView::MoveCursorSelecting(
	Point	ptWk_CaretPos,		// [in] 移動先レイアウト位置
	bool	bSelect,			// true: 選択する  false: 選択解除
	int		nCaretMarginRate	// 縦スクロール開始位置を決める値
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
		/*	現在のカーソル位置によって選択範囲を変更．*/
		GetSelectionInfo().ChangeSelectAreaByCurrentCursor(ptWk_CaretPos);
	}
	GetCaret().MoveCursor(ptWk_CaretPos, true, nCaretMarginRate);
	GetCaret().nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           解析                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	指定カーソル位置にURLが有る場合のその範囲を調べる
*/
bool EditView::IsCurrentPositionURL(
	const Point& ptCaretPos,	// [in]  カーソル位置
	Range* pUrlRange,			// [out] URL範囲。ロジック単位。
	std::wstring* pwstrURL		// [out] URL文字列受け取り先。NULLを指定した場合はURL文字列を受け取らない。
	)
{
	MY_RUNNINGTIMER(runningTimer, "EditView::IsCurrentPositionURL");

	// URLを強調表示するかどうかチェックする
	bool bDispUrl = TypeSupport(*this, COLORIDX_URL).IsDisp();
	bool bUseRegexKeyword = false;
	if (pTypeData->bUseRegexKeyword) {
		const wchar_t* pKeyword = pTypeData->regexKeywordList;
		for (int i=0; i<MAX_REGEX_KEYWORD; ++i) {
			if (*pKeyword == L'\0')
				break;
			if (pTypeData->regexKeywordArr[i].nColorIndex == COLORIDX_URL) {
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

	// 正規表現キーワード（URL色指定）行検索開始処理
	if (bUseRegexKeyword) {
		pRegexKeyword->RegexKeyLineStart();
	}

	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	Point ptXY = pEditDoc->layoutMgr.LayoutToLogic(ptCaretPos);
	size_t nLineLen;
	const wchar_t* pLine = pEditDoc->docLineMgr.GetLine(ptXY.y)->GetDocLineStrWithEOL(&nLineLen);

	int nMatchColor;
	size_t nUrlLen = 0;
	int i = t_max(0, (int)ptXY.x - (int)_MAX_PATH);
	//nLineLen = __min(nLineLen, ptXY.x + _MAX_PATH);
	while (i <= ptXY.x && i < (int)nLineLen) {
		bool bMatch = (bUseRegexKeyword
					&& pRegexKeyword->RegexIsKeyword(StringRef(pLine, nLineLen), i, &nUrlLen, &nMatchColor)
					&& nMatchColor == COLORIDX_URL);
		if (!bMatch) {
			bMatch = (bDispUrl
						&& (i == 0 || !IS_KEYWORD_CHAR(pLine[i - 1]))
						&& IsURL(&pLine[i], (nLineLen - i), &nUrlLen));	// 指定アドレスがURLの先頭ならばTRUEとその長さを返す
		}
		if (bMatch) {
			if (i <= ptXY.x && ptXY.x < i + (int)nUrlLen) {
				// URLを返す場合
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
//                         イベント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

VOID EditView::OnTimer(
	HWND hwnd,			// handle of window for timer messages
	UINT uMsg,			// WM_TIMER message
	UINT_PTR idEvent,	// timer identifier
	DWORD dwTime		// current system time
	)
{
	if (GetDllShareData().common.edit.bUseOLE_DragDrop) {	// OLEによるドラッグ & ドロップを使う
		if (IsDragSource()) {
			return;
		}
	}
	POINT po;
	RECT rc;
	// 範囲選択中でない場合
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
			if (KeywordHelpSearchDict(LID_SKH_ONTIMER, &po, &rc)) {
				// 辞書Tipを表示
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

	Point sPos;

	int	nLineNum;
	int	nDelLen;
	size_t nLineLen;
	size_t nLineLen2;
	WaitCursor waitCursor(GetHwnd());

	Point ptFromLogic = pEditDoc->layoutMgr.LayoutToLogic(GetSelectionInfo().select.GetFrom());

	// 矩形範囲選択中か
	if (GetSelectionInfo().IsBoxSelecting()) {

		// 2点を対角とする矩形を求める
		Rect rcSelLayout;
		TwoPointToRect(
			&rcSelLayout,
			GetSelectionInfo().select.GetFrom(),	// 範囲選択開始
			GetSelectionInfo().select.GetTo()		// 範囲選択終了
		);

		// 現在の選択範囲を非選択状態に戻す
		GetSelectionInfo().DisableSelectArea(false);

		size_t nIdxFrom = 0;
		size_t nIdxTo = 0;
		for (nLineNum=rcSelLayout.bottom; nLineNum>=rcSelLayout.top-1; --nLineNum) {
			const Layout* pLayout;
			size_t nDelPosNext = nIdxFrom;
			size_t nDelLenNext = nIdxTo - nIdxFrom;
			const wchar_t* pLine = pEditDoc->layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
			if (pLine) {
				// 指定された桁に対応する行のデータ内の位置を調べる
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

				// 指定位置の指定長データ削除
				DeleteData2(
					sPos,
					nDelLen,
					&memBuf
				);
				
				{
					// 機能種別によるバッファの変換
					ConvertMediator::ConvMemory(&memBuf, nFuncCode, pEditDoc->layoutMgr.GetTabSpace(), sPos.x);

					// 現在位置にデータを挿入
					Point ptLayoutNew;	// 挿入された部分の次の位置
					InsertData_CEditView(
						sPos,
						memBuf.GetStringPtr(),
						memBuf.GetStringLength(),
						&ptLayoutNew,
						false
					);

					// カーソルを移動
					GetCaret().MoveCursor(ptLayoutNew, false);
					GetCaret().nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;
				}
			}
		}
		// 挿入データの先頭位置へカーソルを移動
		GetCaret().MoveCursor(rcSelLayout.UpperLeft(), true);
		GetCaret().nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;

		if (!bDoing_UndoRedo) {	// Undo, Redoの実行中か
			// 操作の追加
			commander.GetOpeBlk()->AppendOpe(
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
		ConvertMediator::ConvMemory(&memBuf, nFuncCode, pEditDoc->layoutMgr.GetTabSpace(), GetSelectionInfo().select.GetFrom().x);

		// データ置換 削除&挿入にも使える
		ReplaceData_CEditView(
			GetSelectionInfo().select,
			memBuf.GetStringPtr(),		// 挿入するデータ
			memBuf.GetStringLength(),	// 挿入するデータの長さ
			false,
			bDoing_UndoRedo ? nullptr : commander.GetOpeBlk()
		);

		// 選択エリアの復元
		Point ptFrom = pEditDoc->layoutMgr.LogicToLayout(ptFromLogic);
		GetSelectionInfo().SetSelectArea(Range(ptFrom, GetCaret().GetCaretLayoutPos()));
		GetCaret().MoveCursor(GetSelectionInfo().select.GetTo(), true);
		GetCaret().nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;

		if (!bDoing_UndoRedo) {	// Undo, Redoの実行中か
			// 操作の追加
			commander.GetOpeBlk()->AppendOpe(
				new MoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
	}
	RedrawAll();	// 対象が矩形だった場合も最後に再描画する
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         メニュー                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ポップアップメニュー(右クリック)
int	EditView::CreatePopUpMenu_R(void)
{
	MenuDrawer& menuDrawer = editWnd.GetMenuDrawer();
	menuDrawer.ResetContents();

	// 右クリックメニューの定義はカスタムメニュー配列の0番目
	int nMenuIdx = CUSTMENU_INDEX_FOR_RBUTTONUP;

	// Note: ViewCommander::Command_CustMenu と大体同じ
	HMENU hMenu = ::CreatePopupMenu();

	if (!GetSelectionInfo().IsMouseSelecting()) {
		POINT po;
		RECT rc;
		if (KeywordHelpSearchDict(LID_SKH_POPUPMENU_R, &po, &rc)) {
			menuDrawer.MyAppendMenu(hMenu, 0, IDM_COPYDICINFO, LS(STR_MENU_KEYWORDINFO), _T("K"));
			menuDrawer.MyAppendMenu(hMenu, 0, IDM_JUMPDICT, LS(STR_MENU_OPENKEYWORDDIC), _T("L"));
			menuDrawer.MyAppendMenuSep(hMenu, MF_SEPARATOR, F_0, _T(""));
		}
	}
	return CreatePopUpMenuSub(hMenu, nMenuIdx, nullptr);
}

/*! ポップアップメニューの作成(Sub)
	hMenuは作成済み
*/
int	EditView::CreatePopUpMenuSub(HMENU hMenu, int nMenuIdx, int* pParentMenus)
{
	wchar_t szLabel[300];

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
				wchar_t buf[MAX_CUSTOM_MENU_NAME_LEN + 1];
				LPCWSTR p = GetDocument().funcLookup.Custmenu2Name(nCustIdx, buf, _countof(buf));
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
			funcLookup.Funccode2Name(code, szLabel, 256);
			// キー
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
		// 後は親に処理してもらう
		return -1;
	}

	int cMenuItems = ::GetMenuItemCount(hMenu);
	for (int nPos=0; nPos<cMenuItems; ++nPos) {
		EFunctionCode id = (EFunctionCode)::GetMenuItemID(hMenu, nPos);
		UINT fuFlags;
		// 機能が利用可能か調べる
		// 機能が有効な場合には明示的に再設定しないようにする．
		if (!IsFuncEnable(GetDocument(), GetDllShareData(), id)) {
			fuFlags = MF_BYCOMMAND | MF_GRAYED;
			::EnableMenuItem(hMenu, id, fuFlags);
		}

		// 機能がチェック状態か調べる
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
//                         設定反映                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 設定変更を反映させる
void EditView::OnChangeSetting()
{
	if (!GetHwnd()) {
		return;
	}
	auto& csWindow = GetDllShareData().common.window;
	auto& textArea = GetTextArea();
	textArea.SetTopYohaku(csWindow.nRulerBottomSpace); 	// ルーラーとテキストの隙間
	textArea.SetAreaTop(textArea.GetTopYohaku());									// 表示域の上端座標

	// 文書種別更新
	pTypeData = &pEditDoc->docType.GetDocumentAttribute();

	// ルーラー表示
	if (pTypeData->colorInfoArr[COLORIDX_RULER].bDisp && !bMiniMap) {
		textArea.SetAreaTop(textArea.GetAreaTop() + csWindow.nRulerHeight);	// ルーラー高さ
	}
	textArea.SetLeftYohaku(csWindow.nLineNumRightSpace);

	// フォントの変更
	SetFont();

	//	画面キャッシュ用CompatibleDCを用意する
	UseCompatibleDC(csWindow.bUseCompatibleBMP);

	// ウィンドウサイズの変更処理
	RECT rc;
	::GetClientRect(GetHwnd(), &rc);
	OnSize(rc.right, rc.bottom);

	// フォントが変わった
	tipWnd.ChangeFont(&(GetDllShareData().common.helper.lf));
	
	// 再描画
	if (!editWnd.pPrintPreview) {
		::InvalidateRect(GetHwnd(), NULL, TRUE);
	}
	TypeSupport textType(*this, COLORIDX_TEXT);
	crBack = textType.GetBackColor();
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
		if (!pcsbwVSplitBox) {	// 垂直分割ボックス
			pcsbwVSplitBox = new SplitBoxWnd;
			pcsbwVSplitBox->Create(G_AppInstance(), GetHwnd(), TRUE);
		}
	}else {
		SAFE_DELETE(pcsbwVSplitBox);	// 垂直分割ボックス
	}
	if (bHorz) {
		if (!pcsbwHSplitBox) {	// 水平分割ボックス
			pcsbwHSplitBox = new SplitBoxWnd;
			pcsbwHSplitBox->Create(G_AppInstance(), GetHwnd(), FALSE);
		}
	}else {
		SAFE_DELETE(pcsbwHSplitBox);	// 水平分割ボックス
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
	正常時はTRUE,範囲未選択の場合は false を返す
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

	// 範囲選択がされていない
	if (!GetSelectionInfo().IsTextSelected()) {
		return false;
	}
	if (bWithLineNumber) {	// 行番号を付与する
		// 行番号表示に必要な桁数を計算
		// 桁はレイアウト単位である必要がある
		nLineNumCols = GetTextArea().DetectWidthOfLineNumberArea_calculate(&pEditDoc->layoutMgr, true);
		nLineNumCols += 1;
		pszLineNum = new wchar_t[nLineNumCols + 1];
	}

	if (GetSelectionInfo().IsBoxSelecting()) {	// 矩形範囲選択中
		Rect rcSel;
		// 2点を対角とする矩形を求める
		TwoPointToRect(
			&rcSel,
			GetSelectionInfo().select.GetFrom(),	// 範囲選択開始
			GetSelectionInfo().select.GetTo()		// 範囲選択終了
		);
		memBuf->SetString(L"");

		// サイズ分だけ要領をとっておく。
		// 結構大まかに見ています。
		int i = rcSel.bottom - rcSel.top + 1;

		// 最初に行数分の改行量を計算してしまう。
		size_t nBufSize = wcslen(WCODE::CRLF) * i;

		// 実際の文字量。
		const wchar_t* pLine = pEditDoc->layoutMgr.GetLineStr(rcSel.top, &nLineLen, &pLayout);
		for (; i!=0 && pLayout; --i, pLayout=pLayout->GetNextLayout()) {
			pLine = pLayout->GetPtr() + pLayout->GetLogicOffset();
			nLineLen = pLayout->GetLengthWithEOL();
			if (pLine) {
				// 指定された桁に対応する行のデータ内の位置を調べる
				nIdxFrom	= LineColumnToIndex(pLayout, rcSel.left );
				nIdxTo		= LineColumnToIndex(pLayout, rcSel.right);
				ASSERT_GE(nIdxTo, nIdxFrom);
				nBufSize += nIdxTo - nIdxFrom;
			}
			if (bLineOnly) {	// 複数行選択の場合は先頭の行のみ
				break;
			}
		}

		// 大まかに見た容量を元にサイズをあらかじめ確保しておく。
		memBuf->AllocStringBuffer(nBufSize);

		bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
		nRowNum = 0;
		for (nLineNum=rcSel.top; (int)nLineNum<=rcSel.bottom; ++nLineNum) {
			const wchar_t* pLine = pEditDoc->layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
			if (pLine) {
				// 指定された桁に対応する行のデータ内の位置を調べる
				nIdxFrom	= LineColumnToIndex(pLayout, rcSel.left );
				nIdxTo		= LineColumnToIndex(pLayout, rcSel.right);
				// pLineがNULLのとき(矩形エリアの端がEOFのみの行を含むとき)は以下を処理しない
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
			if (bLineOnly) {	// 複数行選択の場合は先頭の行のみ
				break;
			}
		}
	}else {
		memBuf->SetString(L"");

		//  これから貼り付けに使う領域の大まかなサイズを取得する。
		//  大まかというレベルですので、サイズ計算の誤差が（容量を多く見積もる方に）結構出ると思いますが、
		// まぁ、速さ優先ということで勘弁してください。
		//  無駄な容量確保が出ていますので、もう少し精度を上げたいところですが・・・。
		//  とはいえ、逆に小さく見積もることになってしまうと、かなり速度をとられる要因になってしまうので
		// 困ってしまうところですが・・・。
		pEditDoc->layoutMgr.GetLineStr(GetSelectionInfo().select.GetFrom().y, &nLineLen, &pLayout);
		size_t nBufSize = 0;

		int i = (GetSelectionInfo().select.GetTo().y - GetSelectionInfo().select.GetFrom().y);

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
		nBufSize *= i;

		// 実際の各行の長さ。
		for (; i!=0 && pLayout; --i, pLayout=pLayout->GetNextLayout()) {
			nBufSize += pLayout->GetLengthWithoutEOL() + appendEol.GetLen();
			if (bLineOnly) {	// 複数行選択の場合は先頭の行のみ
				break;
			}
		}

		// 調べた長さ分だけバッファを取っておく。
		memBuf->AllocStringBuffer(nBufSize);

		for (nLineNum=GetSelectionInfo().select.GetFrom().y; (int)nLineNum<=GetSelectionInfo().select.GetTo().y; ++nLineNum) {
			const wchar_t* pLine = pEditDoc->layoutMgr.GetLineStr(nLineNum, &nLineLen, &pLayout);
			if (!pLine) {
				break;
			}
			if (nLineNum == GetSelectionInfo().select.GetFrom().y) {
				// 指定された桁に対応する行のデータ内の位置を調べる
				nIdxFrom = LineColumnToIndex(pLayout, GetSelectionInfo().select.GetFrom().x);
			}else {
				nIdxFrom = 0;
			}
			if (nLineNum == GetSelectionInfo().select.GetTo().y) {
				// 指定された桁に対応する行のデータ内の位置を調べる
				nIdxTo = LineColumnToIndex(pLayout, GetSelectionInfo().select.GetTo().x);
			}else {
				nIdxTo = nLineLen;
			}
			ASSERT_GE(nIdxTo, nIdxFrom);
			if (nIdxTo - nIdxFrom == 0) {
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
						memBuf->AppendString((neweol == EolType::Unknown) ?
							pEditDoc->docEditor.GetNewLineCode().GetValue2() :	//	コード保存
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
		delete[] pszLineNum;
	}
	return true;
}

/* 選択範囲内の１行の選択
	@param bCursorPos 選択開始行の代わりにカーソル位置の行を取得
	通常選択ならロジック行、矩形なら選択範囲内のレイアウト行１行を選択
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
		// 矩形範囲選択(レイアウト処理)
		const Layout* pLayout;
		Rect rcSel;

		// 2点を対角とする矩形を求める
		TwoPointToRect(
			&rcSel,
			select.GetFrom(),	// 範囲選択開始
			select.GetTo()	// 範囲選択終了
		);
		const wchar_t* pLine = layoutMgr.GetLineStr(rcSel.top, &nLineLen, &pLayout);
		if (pLine && pLayout) {
			nLineLen = pLayout->GetLengthWithoutEOL();
			// 指定された桁に対応する行のデータ内の位置を調べる
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
		// 線形選択(ロジック行処理)
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

/* 指定カーソル位置が選択エリア内にあるか
	【戻り値】
	-1	選択エリアより前方 or 無選択
	0	選択エリア内
	1	選択エリアより後方
*/
int EditView::IsCurrentPositionSelected(
	Point	ptCaretPos		// カーソル位置
	)
{
	auto& selInfo = GetSelectionInfo();
	if (!selInfo.IsTextSelected()) {	// テキストが選択されているか
		return -1;
	}
	Rect	rcSel;
	Point	po;
	auto& select = selInfo.select;

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
int EditView::IsCurrentPositionSelectedTEST(
	const Point& ptCaretPos,	// カーソル位置
	const Range& select		//
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
	const wchar_t*	pszQuote,		// 先頭に付ける引用符
	bool			bWithLineNumber	// 行番号を付与する
	)
{
	NativeW	memBuf;

	if (!GetSelectionInfo().IsTextSelected()) {	// テキストが選択されているか
		return;
	}
	{	// 選択範囲内の全行を選択状態にする
		Range sSelect(GetSelectionInfo().select);
		const Layout* pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(sSelect.GetFrom().y);
		if (!pLayout) return;
		sSelect.SetFromX(pLayout->GetIndent());
		pLayout = GetDocument().layoutMgr.SearchLineByLayoutY(sSelect.GetTo().y);
		if (pLayout && (GetSelectionInfo().IsBoxSelecting() || sSelect.GetTo().x > (int)pLayout->GetIndent())) {
			// 選択範囲を次行頭まで拡大する
			sSelect.SetToY(sSelect.GetTo().y + 1);
			pLayout = pLayout->GetNextLayout();
		}
		sSelect.SetToX(pLayout? pLayout->GetIndent(): 0);
		GetCaret().GetAdjustCursorPos(&sSelect.GetTo());	// EOF行を超えていたら座標修正

		GetSelectionInfo().DisableSelectArea(false);
		GetSelectionInfo().SetSelectArea(sSelect);

		GetCaret().MoveCursor(GetSelectionInfo().select.GetTo(), false);
		GetCaret().ShowEditCaret();
	}
	// 再描画
	//	::UpdateWindow();
	Call_OnPaint((int)PaintAreaType::LineNumber | (int)PaintAreaType::Body, false);
	// 選択範囲をクリップボードにコピー
	// 選択範囲のデータを取得
	// 正常時はTRUE,範囲未選択の場合は終了する
	if (!GetSelectedData(
			&memBuf,
			false,
			pszQuote, // 引用符
			bWithLineNumber, // 行番号を付与する
			GetDllShareData().common.edit.bAddCRLFWhenCopy // 折り返し位置に改行記号を入れる
		)
	) {
		ErrorBeep();
		return;
	}
	// クリップボードにデータを設定
	MySetClipboardData(memBuf.GetStringPtr(), memBuf.GetStringLength(), false);
}


/*! クリップボードからデータを取得 */
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

/* クリップボードにデータを設定 */
bool EditView::MySetClipboardData(
	const char* pszText,
	size_t nTextLen,
	bool bColumnSelect,
	bool bLineSelect /*= false*/
	)
{
	// wchar_tに変換
	std::vector<wchar_t> buf;
	mbstowcs_vector(pszText, nTextLen, &buf);
	return MySetClipboardData(&buf[0], buf.size()-1, bColumnSelect, bLineSelect);
}

bool EditView::MySetClipboardData(
	const wchar_t* pszText,
	size_t nTextLen,
	bool bColumnSelect,
	bool bLineSelect /*= false*/
	)
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
/*! カーソルの縦線の座標で作画範囲か */
inline bool EditView::IsDrawCursorVLinePos(int posX)
{
	auto& textArea = GetTextArea();
	return posX >= textArea.GetAreaLeft() - 2
		&& posX >  textArea.GetAreaLeft() - GetDllShareData().common.window.nLineNumRightSpace // 隙間(+1)がないときは線を引かない判定
		&& posX <= textArea.GetAreaRight();
}

// カーソル行アンダーラインのON
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
	
	int bCursorLineBgDraw = false;
	auto& textArea = GetTextArea();
	
	// カーソル行の描画
	if (1
		&& bDraw
		&& bCursorLineBg
		&& GetDrawSwitch()
		&& GetCaret().GetCaretLayoutPos().y >= textArea.GetViewTopLine()
		&& !bDoing_UndoRedo	// Undo, Redoの実行中か
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

			// 描画
			HDC hdc = this->GetDC();
			OnPaint(hdc, &ps, FALSE);
			this->ReleaseDC(hdc);

			GetCaret().underLine.UnLock();
		}
	}
	
	int nCursorVLineX = -1;
	if (bCursorVLine) {
		// カーソル位置縦線。-1してキャレットの左に来るように。
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
		// カーソル位置縦線の描画
		// アンダーラインと縦線の交点で、下線が上になるように先に縦線を引く。
		HDC hdc = ::GetDC(GetHwnd());
		{
			Graphics gr(hdc);
			gr.SetPen(pTypeData->colorInfoArr[COLORIDX_CURSORVLINE].colorAttr.cTEXT);
			::MoveToEx(gr, nOldCursorLineX, textArea.GetAreaTop(), NULL);
			::LineTo(  gr, nOldCursorLineX, textArea.GetAreaBottom());
			int nBoldX = nOldCursorLineX - 1;
			// 「太字」のときは2dotの線にする。その際カーソルに掛からないように左側を太くする
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
		}	// ReleaseDC の前に gr デストラクト
		::ReleaseDC(GetHwnd(), hdc);
	}
	
	int nUnderLineY = -1;
	if (bUnderLine) {
		nUnderLineY = textArea.GetAreaTop() + (GetCaret().GetCaretLayoutPos().y - textArea.GetViewTopLine())
			 * GetTextMetrics().GetHankakuDy() + GetTextMetrics().GetHankakuHeight();
	}

	if (1
		&& bDraw
		&& GetDrawSwitch()
		&& nUnderLineY >= textArea.GetAreaTop()
		&& !bDoing_UndoRedo	// Undo, Redoの実行中か
		&& !GetSelectionInfo().IsTextSelecting()
		&& !DisalbeUnderLine
	) {
		if (!bCursorLineBgDraw || nOldUnderLineY == -1) {
			nOldUnderLineY = GetCaret().GetCaretLayoutPos().y;
			nOldUnderLineYBg = nOldUnderLineY;
		}
		nOldUnderLineYMargin = GetTextMetrics().GetHankakuHeight();
		nOldUnderLineYHeightReal = 1;
//		MYTRACE(_T("★カーソル行アンダーラインの描画\n"));
		// ★カーソル行アンダーラインの描画
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
		}	// ReleaseDC の前に gr デストラクト
		::ReleaseDC(GetHwnd(), hdc);
	}
}

// カーソル行アンダーラインのOFF
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
			&& !bDoing_UndoRedo	// Undo, Redoの実行中か
			&& !caret.underLine.GetUnderLineDoNotOFF()	// アンダーラインを消去するか
		) {
			// -- -- カーソル行アンダーラインの消去（無理やり） -- -- //
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

			//	不本意ながら選択情報をバックアップ。
//			Range sSelectBackup = GetSelectionInfo().select;
//			GetSelectionInfo().select.Clear(-1);

			if (ps.rcPaint.bottom - ps.rcPaint.top) {
				// 描画
				HDC hdc = this->GetDC();
				// 可能なら互換BMPからコピーして再作画
				OnPaint(hdc, &ps, (ps.rcPaint.bottom - ps.rcPaint.top) == 1);
				this->ReleaseDC(hdc);
			}
			nOldUnderLineYHeight = 0;

			//	選択情報を復元
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

	// 互換BMPによる画面バッファ
	// カーソル位置縦線
	if (nOldCursorLineX != -1) {
		if (1
			&& bDraw
			&& GetDrawSwitch()
			&& IsDrawCursorVLinePos(nOldCursorLineX)
			&& !bDoing_UndoRedo
			&& !caret.underLine.GetVertLineDoNotOFF()	// カーソル位置縦線を消去するか
			&& !DisalbeUnderLine
		) {
			PAINTSTRUCT ps;
			ps.rcPaint.left = nOldCursorLineX - (nOldCursorVLineWidth - 1);
			ps.rcPaint.right = nOldCursorLineX + 1;
			ps.rcPaint.top = textArea.GetAreaTop();
			ps.rcPaint.bottom = textArea.GetAreaBottom();
			HDC hdc = ::GetDC(GetHwnd());
			caret.underLine.Lock();
			//	不本意ながら選択情報をバックアップ。
			Range sSelectBackup = this->GetSelectionInfo().select;
			this->GetSelectionInfo().select.Clear(-1);
			// 可能なら互換BMPからコピーして再作画
			OnPaint(hdc, &ps, TRUE);
			//	選択情報を復元
			this->GetSelectionInfo().select = sSelectBackup;
			caret.underLine.UnLock();
			ReleaseDC(hdc);
		}
		nOldCursorLineX = -1;
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         状態表示                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	検索／置換／ブックマーク検索時の状態をステータスバーに表示する
*/
void EditView::SendStatusMessage(const TCHAR* msg)
{
	editWnd.SendStatusMessage(msg);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        編集モード                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	挿入モード取得 */
bool EditView::IsInsMode(void) const
{
	return pEditDoc->docEditor.IsInsMode();
}

void EditView::SetInsMode(bool mode)
{
	pEditDoc->docEditor.SetInsMode(mode);
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
	pHistory->Flush();

	// 現在の選択範囲を非選択状態に戻す
	GetSelectionInfo().DisableSelectArea(false);

	OnChangeSetting();
	GetCaret().MoveCursor(Point(0, 0), true);
	GetCaret().nCaretPosX_Prev = 0;
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// 改行コードの設定内からここに移動
	editWnd.GetActiveView().GetCaret().ShowCaretPosInfo();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          その他                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// 現在のカーソル行位置を履歴に登録する
void EditView::AddCurrentLineToHistory(void)
{
	Point ptPos = pEditDoc->layoutMgr.LayoutToLogic(GetCaret().GetCaretLayoutPos());

	MarkMgr::Mark m(ptPos);
	pHistory->Add(m);
}


// 補完ウィンドウ用のキーワードヘルプ表示
bool EditView::ShowKeywordHelp(
	POINT po,
	LPCWSTR pszHelp,
	LPRECT prcHokanWin
	)
{
	NativeW	memCurText;
	RECT	rcTipWin,
			rcDesktop;

	if (pTypeData->bUseKeywordHelp) { // キーワードヘルプを使用する
		if (!bInMenuLoop			// メニュー モーダル ループに入っていない
			&& dwTipTimer != 0	// 辞書Tipを表示していない
		) {
			memCurText.SetString(pszHelp);

			// 既に検索済みか
			if (NativeW::IsEqual(memCurText, tipWnd.key)) {
				// 該当するキーがなかった
				if (!tipWnd.KeyWasHit) {
					return false;
				}
			}else {
				tipWnd.key = memCurText;
				// 検索実行
				if (!KeySearchCore(&tipWnd.key))
					return false;
			}
			dwTipTimer = 0;	// 辞書Tipを表示している

			// 辞書Tipのサイズを取得
			tipWnd.GetWindowSize(&rcTipWin);

			// マルチモニタ対応
			::GetMonitorWorkRect(tipWnd.GetHwnd(), &rcDesktop);

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

			// 辞書Tipを表示
			tipWnd.Show(po.x, po.y , NULL , &rcTipWin);
			return true;
		}
	}
	return false;
}

/*!
	@brief 指定位置または指定範囲がテキストの存在しないエリアかチェックする

	@param[in] ptFrom  指定位置または指定範囲開始
	@param[in] ptTo    指定範囲終了
	@param[in] bSelect    範囲指定
	@param[in] bBoxSelect 矩形選択
	
	@retval true  指定位置または指定範囲内にテキストが存在しない
			false 指定位置または指定範囲内にテキストが存在する
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
		size_t nLineLen;

		result = true;
		for (int nLineNum=nLineFrom; nLineNum<=nLineTo; ++nLineNum) {
			if ((pLayout = pEditDoc->layoutMgr.SearchLineByLayoutY(nLineNum))) {
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

// アンドゥバッファの処理
void EditView::SetUndoBuffer(bool bPaintLineNumber)
{
	OpeBlk* pOpe = commander.GetOpeBlk();
	if (pOpe && pOpe->Release() == 0) {
		if (0 < pOpe->GetNum()) {	// 操作の数を返す
			// 操作の追加
			GetDocument().docEditor.opeBuf.AppendOpeBlk(pOpe);

			if (!editWnd.UpdateTextWrap())	{	// 折り返し方法関連の更新
				if (0 < pOpe->GetNum() - GetDocument().docEditor.nOpeBlkRedawCount) {
					editWnd.RedrawAllViews(this);	//	他のペインの表示を更新
				}
			}
		}else {
			delete pOpe;
		}
		commander.SetOpeBlk(nullptr);
	}
}

