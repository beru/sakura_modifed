#include "StdAfx.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "env/SakuraEnvironment.h"
#include <limits.h>
#include "window.h"

int DPI::nDpiX = 96;
int DPI::nDpiY = 96;
bool DPI::bInitialized = false;

/**	指定したウィンドウの祖先のハンドルを取得する

	GetAncestor() APIがWin95で使えないのでそのかわり

	WS_POPUPスタイルを持たないウィンドウ（ex.CDlgFuncListダイアログ）だと、
	GA_ROOTOWNERでは編集ウィンドウまで遡れないみたい。GetAncestor() APIでも同様。
	本関数固有に用意したGA_ROOTOWNER2では遡ることができる。
*/
HWND MyGetAncestor(HWND hWnd, UINT gaFlags)
{
	HWND hwndAncestor;
	HWND hwndDesktop = ::GetDesktopWindow();
	HWND hwndWk;

	if (hWnd == hwndDesktop)
		return NULL;

	switch (gaFlags) {
	case GA_PARENT:	// 親ウィンドウを返す（オーナーは返さない）
		hwndAncestor = ((DWORD)::GetWindowLongPtr(hWnd, GWL_STYLE) & WS_CHILD)? ::GetParent(hWnd): hwndDesktop;
		break;

	case GA_ROOT:	// 親子関係を遡って直近上位のトップレベルウィンドウを返す
		hwndAncestor = hWnd;
		while ((DWORD)::GetWindowLongPtr(hwndAncestor, GWL_STYLE) & WS_CHILD)
			hwndAncestor = ::GetParent(hwndAncestor);
		break;

	case GA_ROOTOWNER:	// 親子関係と所有関係をGetParent()で遡って所有されていないトップレベルウィンドウを返す
		hwndWk = hWnd;
		do {
			hwndAncestor = hwndWk;
			hwndWk = ::GetParent(hwndAncestor);
		}while (hwndWk);
		break;

	case GA_ROOTOWNER2:	// 所有関係をGetWindow()で遡って所有されていないトップレベルウィンドウを返す
		hwndWk = hWnd;
		do {
			hwndAncestor = hwndWk;
			hwndWk = ::GetParent(hwndAncestor);
			if (!hwndWk)
				hwndWk = ::GetWindow(hwndAncestor, GW_OWNER);
		}while (hwndWk);
		break;

	default:
		hwndAncestor = NULL;
		break;
	}

	return hwndAncestor;
}


/*!
	処理中のユーザー操作を可能にする
	ブロッキングフック(?)（メッセージ配送
*/
BOOL BlockingHook(HWND hwndDlgCancel)
{
	MSG msg;
	BOOL ret;
	// メッセージをあるだけ処理するように
	while ((ret = (BOOL)::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) != 0) {
		if (msg.message == WM_QUIT) {
			return FALSE;
		}
		if (hwndDlgCancel && IsDialogMessage(hwndDlgCancel, &msg)) {
		}else {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
	return TRUE; // ret;
}


/** フレームウィンドウをアクティブにする
	対象がdisableのときは最近のポップアップをフォアグラウンド化する．
		（モーダルダイアログやメッセージボックスを表示しているようなとき）
*/
void ActivateFrameWindow(HWND hwnd)
{
	// 編集ウィンドウでタブまとめ表示の場合は表示位置を復元する
	DllSharedData* pShareData = &GetDllShareData();
	if (pShareData->common.tabBar.bDispTabWnd && !pShareData->common.tabBar.bDispTabWndMultiWin) {
		if (IsSakuraMainWindow(hwnd)) {
			if (pShareData->flags.bEditWndChanging) {
				return;	// 切替の最中(busy)は要求を無視する
			}
			pShareData->flags.bEditWndChanging = true;	// 編集ウィンドウ切替中ON

			// 対象ウィンドウのスレッドに位置合わせを依頼する
			DWORD_PTR dwResult;
			::SendMessageTimeout(
				hwnd,
				MYWM_TAB_WINDOW_NOTIFY,
				(WPARAM)TabWndNotifyType::Adjust,
				(LPARAM)NULL,
				SMTO_ABORTIFHUNG | SMTO_BLOCK,
				10000,
				&dwResult
			);
		}
	}

	// 対象がdisableのときは最近のポップアップをフォアグラウンド化する
	HWND hwndActivate = ::IsWindowEnabled(hwnd)? hwnd: ::GetLastActivePopup(hwnd);
	if (::IsIconic(hwnd)) {
		::ShowWindow(hwnd, SW_RESTORE);
	}else if (::IsZoomed(hwnd)) {
		::ShowWindow(hwnd, SW_MAXIMIZE);
	}else {
		::ShowWindow(hwnd, SW_SHOW);
	}
	::SetForegroundWindow(hwndActivate);
	::BringWindowToTop(hwndActivate);

	if (pShareData) {
		pShareData->flags.bEditWndChanging = false;	// 編集ウィンドウ切替中OFF
	}

	return;
}


TextWidthCalc::TextWidthCalc(HWND hParent, int nID)
{
	assert_warning(hParent);

	hwnd = ::GetDlgItem(hParent, nID);
	hDC = ::GetDC(hwnd);
	assert(hDC);
	hFont = (HFONT)::SendMessage(hwnd, WM_GETFONT, 0, 0);
	hFontOld = (HFONT)::SelectObject(hDC, hFont);
	nCx = 0;
	nExt = 0;
	bHDCComp = false;
	bFromDC = false;
}

TextWidthCalc::TextWidthCalc(HWND hwndThis)
{
	assert_warning(hwndThis);

	hwnd = hwndThis;
	hDC = ::GetDC(hwnd);
	assert(hDC);
	hFont = (HFONT)::SendMessage(hwnd, WM_GETFONT, 0, 0);
	hFontOld = (HFONT)::SelectObject(hDC, hFont);
	nCx = 0;
	nExt = 0;
	bHDCComp = false;
	bFromDC = false;
}

TextWidthCalc::TextWidthCalc(HFONT font)
{
	hwnd = 0;
	HDC hDCTemp = ::GetDC( NULL ); // Desktop
	hDC = ::CreateCompatibleDC( hDCTemp );
	::ReleaseDC( NULL, hDCTemp );
	assert(hDC);
	hFont = font;
	hFontOld = (HFONT)::SelectObject(hDC, hFont);
	nCx = 0;
	nExt = 0;
	bHDCComp = true;
	bFromDC = false;
}

TextWidthCalc::TextWidthCalc(HDC hdc)
{
	hwnd = 0;
	hDC = hdc;
	assert(hDC);
	nCx = 0;
	nExt = 0;
	bHDCComp = true;
	bFromDC = true;
}

TextWidthCalc::~TextWidthCalc()
{
	if (hDC && !bFromDC) {
		::SelectObject(hDC, hFontOld);
		if (bHDCComp) {
			::DeleteDC(hDC);
		}else {
			::ReleaseDC(hwnd, hDC);
		}
		hwnd = 0;
		hDC = 0;
	}
}

bool TextWidthCalc::SetWidthIfMax(int width)
{
	return SetWidthIfMax(0, INT_MIN);
}

bool TextWidthCalc::SetWidthIfMax(int width, int extCx)
{
	if (extCx == INT_MIN) {
		extCx = nExt;
	}
	if (nCx < width + extCx) {
		nCx = width + extCx;
		return true;
	}
	return false;
}

bool TextWidthCalc::SetTextWidthIfMax(LPCTSTR pszText)
{
	return SetTextWidthIfMax(pszText, INT_MIN);
}

bool TextWidthCalc::SetTextWidthIfMax(LPCTSTR pszText, int extCx)
{
	SIZE size;
	if (::GetTextExtentPoint32(hDC, pszText, (int)_tcslen(pszText), &size)) {
		return SetWidthIfMax(size.cx, extCx);
	}
	return false;
}

int TextWidthCalc::GetTextWidth(LPCTSTR pszText) const
{
	SIZE size;
	if (::GetTextExtentPoint32(hDC, pszText, (int)_tcslen(pszText), &size)) {
		return size.cx;
	}
	return 0;
}

int TextWidthCalc::GetTextHeight() const
{
	TEXTMETRIC tm;
	::GetTextMetrics(hDC, &tm);
	return tm.tmHeight;
}

FontAutoDeleter::FontAutoDeleter()
	: hFontOld(NULL)
	, hFont(NULL)
	, hwnd(NULL)
{}

FontAutoDeleter::~FontAutoDeleter()
{
	if (hFont) {
		DeleteObject(hFont);
		hFont = NULL;
	}
}

void FontAutoDeleter::SetFont(HFONT hfontOld, HFONT hfont, HWND hwnd)
{
	if (hFont) {
		::DeleteObject(hFont);
	}
	if (hFont != hfontOld) {
		hFontOld = hfontOld;
	}
	hFont = hfont;
	hwnd = hwnd;
}

/*! ウィンドウのリリース(WM_DESTROY用)
*/
void FontAutoDeleter::ReleaseOnDestroy()
{
	if (hFont) {
		::DeleteObject(hFont);
		hFont = NULL;
	}
	hFontOld = NULL;
}

/*! ウィンドウ生存中のリリース
*/
#if 0
void FontAutoDeleter::Release()
{
	if (hwnd && hFont) {
		::SendMessage(hwnd, WM_SETFONT, (WPARAM)m_hFontOld, FALSE);
		::DeleteObject(hFont);
		hFont = NULL;
		hwnd = NULL;
	}
}
#endif

