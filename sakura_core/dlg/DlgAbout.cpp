#include "StdAfx.h"
#include <ShellAPI.h>
#include "dlg/DlgAbout.h"
#include "uiparts/HandCursor.h"
#include "util/fileUtil.h"
#include "util/module.h"
#include "svnrev.h"
#include "sakura_rc.h"
#include "sakura.hh"

// バージョン情報ダイアログ

const DWORD p_helpids[] = {	//12900
	IDOK,					HIDOK_ABOUT,
	IDC_EDIT_ABOUT,			HIDC_ABOUT_EDIT_ABOUT,
//	IDC_STATIC_URL_UR,		12970,
//	IDC_STATIC_URL_ORG,		12971,
//	IDC_STATIC_UPDATE,		12972,
//	IDC_STATIC_VER,			12973,
//	IDC_STATIC,				-1,
	0, 0
};

#if defined(_M_IA64)
#  define TARGET_M_SUFFIX "_I64"
#elif defined(_M_AMD64)
#  define TARGET_M_SUFFIX "_A64"
#else
#  define TARGET_M_SUFFIX ""
#endif

#if defined(__BORLANDC__)
#  define COMPILER_TYPE "B"
#  define COMPILER_VER  __BORLANDC__ 
#elif defined(__GNUG__)
#  define COMPILER_TYPE "G"
#  define COMPILER_VER (__GNUC__ * 10000 + __GNUC_MINOR__  * 100 + __GNUC_PATCHLEVEL__)
#elif defined(__INTEL_COMPILER)
#  define COMPILER_TYPE "I"
#  define COMPILER_VER __INTEL_COMPILER
#elif defined(__DMC__)
#  define COMPILER_TYPE "D"
#  define COMPILER_VER __DMC__
#elif defined(_MSC_VER)
#  define COMPILER_TYPE "V"
#  define COMPILER_VER _MSC_VER
#else
#  define COMPILER_TYPE "U"
#  define COMPILER_VER 0
#endif

	#define TARGET_STRING_MODEL "W"

#ifdef _DEBUG
	#define TARGET_DEBUG_MODE "D"
#else
	#define TARGET_DEBUG_MODE "R"
#endif

#define TSTR_TARGET_MODE _T(TARGET_STRING_MODEL) _T(TARGET_DEBUG_MODE)

#ifdef _WIN32_WINDOWS
	#define MY_WIN32_WINDOWS _WIN32_WINDOWS
#else
	#define MY_WIN32_WINDOWS 0
#endif

#ifdef _WIN32_WINNT
	#define MY_WIN32_WINNT _WIN32_WINNT
#else
	#define MY_WIN32_WINNT 0
#endif

/*!
	標準以外のメッセージを捕捉する
*/
INT_PTR DlgAbout::DispatchEvent(
	HWND hWnd,
	UINT wMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	INT_PTR result = Dialog::DispatchEvent(hWnd, wMsg, wParam, lParam);
	switch (wMsg) {
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		// EDITも READONLY か DISABLEの場合 WM_CTLCOLORSTATIC になります
		if ((HWND)lParam == GetDlgItem(hWnd, IDC_EDIT_ABOUT)) {
			::SetTextColor((HDC)wParam, RGB(102, 102, 102));
		}else {
			::SetTextColor((HDC)wParam, RGB(0, 0, 0));
        }
		return (INT_PTR)GetStockObject(WHITE_BRUSH);
	}
	return result;
}

// モーダルダイアログの表示
INT_PTR DlgAbout::DoModal(HINSTANCE hInstance, HWND hwndParent)
{
	return Dialog::DoModal(hInstance, hwndParent, IDD_ABOUT, (LPARAM)NULL);
}

/*! 初期化処理 */
BOOL DlgAbout::OnInitDialog(
	HWND hwndDlg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	_SetHwnd(hwndDlg);

	TCHAR	szMsg[2048];
	TCHAR	szFile[_MAX_PATH];

	// この実行ファイルの情報
	::GetModuleFileName(NULL, szFile, _countof(szFile));
	
	// 以下の形式で出力
	// サクラエディタ   Ver. 2.0.0.0 (Rev.9999)
	//
	//      Share Ver: 96
	//      Compile Info: V 1400  WR WIN600/I601/C000/N600
	//      Last Modified: 1999/9/9 00:00:00
	//      (あればSKR_PATCH_INFOの文字列がそのまま表示)
	NativeT memMsg;
	memMsg.AppendString(LS(STR_DLGABOUT_APPNAME));
	memMsg.AppendStringLiteral(_T("   "));

	// バージョン&リビジョン情報
	DWORD dwVersionMS, dwVersionLS;
	GetAppVersionInfo(NULL, VS_VERSION_INFO, &dwVersionMS, &dwVersionLS);
#if (SVN_REV == 0)
	auto_sprintf(szMsg, _T("Ver. %d.%d.%d.%d\r\n"),
#else
	auto_sprintf(szMsg, _T("Ver. %d.%d.%d.%d (Rev.") _T(SVN_REV_STR) _T(")\r\n"),
#endif
		HIWORD(dwVersionMS),
		LOWORD(dwVersionMS),
		HIWORD(dwVersionLS),
		LOWORD(dwVersionLS)
	);
	memMsg.AppendString(szMsg);

	memMsg.AppendString(_T("\r\n"));

	// 共有メモリ情報
	auto_sprintf(szMsg,  _T("      Share Ver: %3d\r\n"),
		N_SHAREDATA_VERSION
	);
	memMsg.AppendString(szMsg);

	// コンパイル情報
	memMsg.AppendStringLiteral(_T("      Compile Info: "));
	int Compiler_ver = COMPILER_VER;
	auto_sprintf(szMsg, _T(COMPILER_TYPE) _T(TARGET_M_SUFFIX) _T("%d ")
			TSTR_TARGET_MODE _T(" WIN%03x/I%03x/C%03x/N%03x\r\n"),
		Compiler_ver,
		WINVER, _WIN32_IE, MY_WIN32_WINDOWS, MY_WIN32_WINNT
	);
	memMsg.AppendString(szMsg);

	// 更新日情報
	FileTime fileTime;
	GetLastWriteTimestamp(szFile, &fileTime);
	auto_sprintf(szMsg,  _T("      Last Modified: %d/%d/%d %02d:%02d:%02d\r\n"),
		fileTime->wYear,
		fileTime->wMonth,
		fileTime->wDay,
		fileTime->wHour,
		fileTime->wMinute,
		fileTime->wSecond
	);
	memMsg.AppendString(szMsg);

	// パッチの情報をコンパイル時に渡せるようにする
#ifdef SKR_PATCH_INFO
	memMsg.AppendStringLiteral(_T("      "));
	const TCHAR* ptszPatchInfo = to_tchar(SKR_PATCH_INFO);
	size_t patchInfoLen = auto_strlen(ptszPatchInfo);
	memMsg.AppendString(ptszPatchInfo, t_min(80, patchInfoLen));
#endif
	memMsg.AppendStringLiteral(_T("\r\n"));

	SetItemText(IDC_EDIT_VER, memMsg.GetStringPtr());

	LPCTSTR pszDesc = LS(IDS_ABOUT_DESCRIPTION);
	if (_tcslen(pszDesc) > 0) {
		_tcsncpy(szMsg, pszDesc, _countof(szMsg) - 1);
		szMsg[_countof(szMsg) - 1] = 0;
		SetItemText(IDC_EDIT_ABOUT, szMsg);
	}
	// アイコンをカスタマイズアイコンに合わせる
	HICON hIcon = GetAppIcon(hInstance, ICON_DEFAULT_APP, FN_APP_ICON, false);
	HWND hIconWnd = GetItemHwnd(IDC_STATIC_MYICON);
	
	if (hIconWnd && hIcon) {
		StCtl_SetIcon(hIconWnd, hIcon);
	}

	// URLウィンドウをサブクラス化する
	UrlUrWnd.SetSubclassWindow(GetItemHwnd(IDC_STATIC_URL_UR));

	// 基底クラスメンバ
	return Dialog::OnInitDialog(GetHwnd(), wParam, lParam);
}


BOOL DlgAbout::OnBnClicked(int wID)
{
	switch (wID) {
	case IDC_BUTTON_COPY:
		{
			HWND hwndEditVer = GetItemHwnd(IDC_EDIT_VER);
	 		EditCtl_SetSel(hwndEditVer, 0, -1); 
	 		SendMessage(hwndEditVer, WM_COPY, 0, 0);
	 		EditCtl_SetSel(hwndEditVer, -1, 0); 
 		}
		return TRUE;
	}
	return Dialog::OnBnClicked(wID);
}

BOOL DlgAbout::OnStnClicked(int wID)
{
	switch (wID) {
	case IDC_STATIC_URL_UR:
		// Web Browserの起動
		{
			TCHAR buf[512];
			GetItemText(wID, buf, _countof(buf));
			::ShellExecute(GetHwnd(), NULL, buf, NULL, NULL, SW_SHOWNORMAL);
			return TRUE;
		}
	}
	// 基底クラスメンバ
	return Dialog::OnStnClicked(wID);
}

LPVOID DlgAbout::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

BOOL UrlWnd::SetSubclassWindow(HWND hWnd)
{
	// STATICウィンドウをサブクラス化する
	// 元のSTATICは WS_TABSTOP, SS_NOTIFY スタイルのものを使用すること
	if (GetHwnd())
		return FALSE;
	if (!IsWindow(hWnd))
		return FALSE;

	// サブクラス化を実行する
	SetLastError(0);
	LONG_PTR lptr = SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
	if (lptr == 0 && GetLastError() != 0) {
		return FALSE;
	}
	pOldProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)UrlWndProc);
	if (!pOldProc) {
		return FALSE;
	}
	hWnd = hWnd;

	// 下線付きフォントに変更する
	LOGFONT lf;
	HFONT hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, (WPARAM)0, (LPARAM)0);
	GetObject(hFont, sizeof(lf), &lf);
	lf.lfUnderline = TRUE;
	hFont = CreateFontIndirect(&lf);
	if (hFont) {
		SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, (LPARAM)FALSE);
	}
	return TRUE;
}

LRESULT CALLBACK UrlWnd::UrlWndProc(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	UrlWnd* pUrlWnd = (UrlWnd*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	HDC hdc;
	POINT pt;
	RECT rc;

	switch (msg) {
	case WM_SETCURSOR:
		// カーソル形状変更
		SetHandCursor();
		return (LRESULT)0;
	case WM_LBUTTONDOWN:
		// キーボードフォーカスを自分に当てる
		SendMessage(GetParent(hWnd), WM_NEXTDLGCTL, (WPARAM)hWnd, (LPARAM)1);
		break;
	case WM_SETFOCUS:
	case WM_KILLFOCUS:
		// 再描画
		InvalidateRect(hWnd, NULL, TRUE);
		UpdateWindow(hWnd);
		break;
	case WM_GETDLGCODE:
		// デフォルトプッシュボタンのように振舞う（Enterキーの有効化）
		// 方向キーは無効化（IEのバージョン情報ダイアログと同様）
		return DLGC_DEFPUSHBUTTON | DLGC_WANTARROWS;
	case WM_MOUSEMOVE:
		// カーソルがウィンドウ内に入ったらタイマー起動
		// ウィンドウ外に出たらタイマー削除
		// 各タイミングで再描画
		BOOL bHilighted;
		pt.x = LOWORD(lParam);
		pt.y = HIWORD(lParam);
		GetClientRect(hWnd, &rc);
		bHilighted = PtInRect(&rc, pt);
		if (bHilighted != pUrlWnd->bHilighted) {
			pUrlWnd->bHilighted = bHilighted;
			InvalidateRect(hWnd, NULL, TRUE);
			if (pUrlWnd->bHilighted)
				SetTimer(hWnd, 1, 200, NULL);
			else
				KillTimer(hWnd, 1);
		}
		break;
	case WM_TIMER:
		// カーソルがウィンドウ外にある場合にも WM_MOUSEMOVE を送る
		GetCursorPos(&pt);
		ScreenToClient(hWnd, &pt);
		GetClientRect(hWnd, &rc);
		if (!PtInRect(&rc, pt))
			SendMessage(hWnd, WM_MOUSEMOVE, 0, MAKELONG(pt.x, pt.y));
		break;
	case WM_PAINT:
		// ウィンドウの描画
		PAINTSTRUCT ps;
		HFONT hFont;
		HFONT hFontOld;
		TCHAR szText[512];

		hdc = BeginPaint(hWnd, &ps);

		// 現在のクライアント矩形、テキスト、フォントを取得する
		GetClientRect(hWnd, &rc);
		GetWindowText(hWnd, szText, _countof(szText));
		hFont = (HFONT)SendMessage(hWnd, WM_GETFONT, (WPARAM)0, (LPARAM)0);

		// テキスト描画
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, pUrlWnd->bHilighted ? RGB(0x84, 0, 0): RGB(0, 0, 0xff));
		hFontOld = (HFONT)SelectObject(hdc, (HGDIOBJ)hFont);
		TextOut(hdc, 2, 0, szText, (int)_tcslen(szText));
		SelectObject(hdc, (HGDIOBJ)hFontOld);

		// フォーカス枠描画
		if (GetFocus() == hWnd)
			DrawFocusRect(hdc, &rc);

		EndPaint(hWnd, &ps);
		return (LRESULT)0;
	case WM_ERASEBKGND:
		hdc = (HDC)wParam;
		GetClientRect(hWnd, &rc);

		// 背景描画
		if (pUrlWnd->bHilighted) {
			// ハイライト時背景描画
			SetBkColor(hdc, RGB(0xff, 0xff, 0));
			::ExtTextOutW_AnyBuild(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
		}else {
			// 親にWM_CTLCOLORSTATICを送って背景ブラシを取得し、背景描画する
			HBRUSH hbr = (HBRUSH)SendMessage(GetParent(hWnd), WM_CTLCOLORSTATIC, wParam, (LPARAM)hWnd);
			HBRUSH hbrOld = (HBRUSH)SelectObject(hdc, hbr);
			FillRect(hdc, &rc, hbr);
			SelectObject(hdc, hbrOld);
		}
		return (LRESULT)1;
	case WM_DESTROY:
		// 後始末
		KillTimer(hWnd, 1);
		SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pUrlWnd->pOldProc);
		if (pUrlWnd->hFont)
			DeleteObject(pUrlWnd->hFont);
		pUrlWnd->hWnd = NULL;
		pUrlWnd->hFont = NULL;
		pUrlWnd->bHilighted = FALSE;
		pUrlWnd->pOldProc = nullptr;
		return (LRESULT)0;
	}

	return CallWindowProc(pUrlWnd->pOldProc, hWnd, msg, wParam, lParam);
}


