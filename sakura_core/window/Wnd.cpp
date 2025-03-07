// ウィンドウの基本クラス

#include "StdAfx.h"
#include "Wnd.h"
#include "util/os.h" // WM_MOUSEWHEEL


// Wndウィンドウメッセージのコールバック関数
LRESULT CALLBACK CWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Wnd* pWnd = (Wnd*)::GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (pWnd) {
		// クラスオブジェクトのポインタを使ってメッセージを配送する
		return pWnd->DispatchEvent(hwnd, uMsg, wParam, lParam);
	}else {
		// ふつうはここには来ない
		return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

// Windowsフック(CBT)
namespace CWindowCreationHook {
	int		g_nCnt  = 0;	// 参照カウンタ
	HHOOK	g_hHook = NULL;

	// フック用コールバック
	static
	LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode == HCBT_CREATEWND) {
			HWND hwnd = (HWND)wParam;
			CBT_CREATEWND* pCreateWnd = (CBT_CREATEWND*)lParam;
			Wnd* pWnd = static_cast<Wnd*>(pCreateWnd->lpcs->lpCreateParams);

			// Wnd以外のウィンドウ生成イベントは無視する
			WNDPROC wndproc = (WNDPROC)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
			if (wndproc != CWndProc) goto next;

			// ウィンドウにWndを関連付ける
			::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pWnd);

			// Wndにウィンドウを関連付ける
			pWnd->_SetHwnd(hwnd);
		}
next:
		return ::CallNextHookEx(g_hHook, nCode, wParam, lParam);
	}

	// フック開始
	void Use()
	{
		if (++g_nCnt >= 1 && !g_hHook) {
			g_hHook = ::SetWindowsHookEx(WH_CBT, CBTProc, NULL, GetCurrentThreadId());
		}
	}

	// フック終了
	void Unuse()
	{
		if (--g_nCnt <= 0 && g_hHook) {
			::UnhookWindowsHookEx(g_hHook);
			g_hHook = NULL;
		}
	}
} // namespace CWindowCreationHook


Wnd::Wnd(const TCHAR* pszInheritanceAppend)
{
	hInstance = NULL;		// アプリケーションインスタンスのハンドル
	hwndParent = NULL;		// オーナーウィンドウのハンドル
	hWnd = NULL;			// このウィンドウのハンドル
#ifdef _DEBUG
	_tcscpy(szClassInheritances, _T("Wnd"));
	_tcscat(szClassInheritances, pszInheritanceAppend);
#endif
}

Wnd::~Wnd()
{
	if (::IsWindow(hWnd)) {
		// クラスオブジェクトのポインタをNULLにして拡張ウィンドウメモリに格納しておく
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)NULL);
		::DestroyWindow(hWnd);
	}
	hWnd = NULL;
	return;
}


// ウィンドウクラス作成
ATOM Wnd::RegisterWC(
	// WNDCLASS用
	HINSTANCE	hInstance,
	HICON		hIcon,			// Handle to the class icon.
	HICON		hIconSm,		// Handle to a small icon
	HCURSOR		hCursor,		// Handle to the class cursor.
	HBRUSH		hbrBackground,	// Handle to the class background brush.
	LPCTSTR		lpszMenuName,	// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
	LPCTSTR		lpszClassName	// Pointer to a null-terminated string or is an atom.
	)
{
	hInstance = hInstance;

	// ウィンドウクラスの登録
	WNDCLASSEX wc;
	wc.cbSize = sizeof(wc);
	// サイズ変更時のちらつきを抑えるためCS_HREDRAW | CS_VREDRAW を外した
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc   = CWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 32;
	wc.hInstance     = hInstance;
	wc.hIcon         = hIcon;
	wc.hCursor       = hCursor;
	wc.hbrBackground = hbrBackground;
	wc.lpszMenuName  = lpszMenuName;
	wc.lpszClassName = lpszClassName;
	wc.hIconSm       = hIconSm;
	return ::RegisterClassEx(&wc);
}

// 作成
HWND Wnd::Create(
	// CreateWindowEx()用
	HWND		hwndParent,
	DWORD		dwExStyle,		// extended window style
	LPCTSTR		lpszClassName,	// Pointer to a null-terminated string or is an atom.
	LPCTSTR		lpWindowName,	// pointer to window name
	DWORD		dwStyle,		// window style
	int			x,				// horizontal position of window
	int			y,				// vertical position of window
	int			nWidth,			// window width
	int			nHeight,		// window height
	HMENU		hMenu			// handle to menu, or child-window identifier
)
{
	this->hwndParent = hwndParent;

	// ウィンドウ作成前の処理(クラス登録前) (virtual)
	PreviCreateWindow();

	// 初期ウィンドウサイズ
	// ウィンドウの作成

	// Windowsフックにより、ウィンドウが作成されるタイミングを横取りする
	CWindowCreationHook::Use();

	hWnd = ::CreateWindowEx(
		dwExStyle,		// extended window style
		lpszClassName,	// pointer to registered class name
		lpWindowName,	// pointer to window name
		dwStyle,		// window style
		x,				// horizontal position of window
		y,				// vertical position of window
		nWidth,			// window width
		nHeight,		// window height
		hwndParent,	// handle to parent or owner window
		hMenu,			// handle to menu, or child-window identifier
		hInstance,	// handle to application instance
		(LPVOID)this	// pointer to window-creation data
	);
	
	// Windowsフック解除
	CWindowCreationHook::Unuse();

	if (!hWnd) {
		::MessageBox(hwndParent, _T("Wnd::Create()\n\n::CreateWindowEx failed."), _T("error"), MB_OK);
		return NULL;
	}

	// ウィンドウ作成後の処理
	AfterCreateWindow();
	return hWnd;
}


// メッセージ配送
LRESULT Wnd::DispatchEvent(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	#define CALLH(message, method) case message: return method(hwnd, msg, wp, lp)
	switch (msg) {
	CALLH(WM_CREATE				, OnCreate			);
	CALLH(WM_CLOSE				, OnClose			);
	CALLH(WM_DESTROY			, OnDestroy			);
	CALLH(WM_SIZE				, OnSize			);
	CALLH(WM_MOVE				, OnMove			);
	CALLH(WM_COMMAND			, OnCommand			);
	CALLH(WM_LBUTTONDOWN		, OnLButtonDown		);
	CALLH(WM_LBUTTONUP			, OnLButtonUp		);
	CALLH(WM_LBUTTONDBLCLK		, OnLButtonDblClk	);
	CALLH(WM_RBUTTONDOWN		, OnRButtonDown		);
	CALLH(WM_RBUTTONUP			, OnRButtonUp		);
	CALLH(WM_RBUTTONDBLCLK		, OnRButtonDblClk	);
	CALLH(WM_MBUTTONDOWN		, OnMButtonDown		);
	CALLH(WM_MBUTTONUP			, OnMButtonUp		);
	CALLH(WM_MBUTTONDBLCLK		, OnMButtonDblClk	);
	CALLH(WM_MOUSEMOVE			, OnMouseMove		);
	CALLH(WM_MOUSEWHEEL			, OnMouseWheel		);
	CALLH(WM_MOUSEHWHEEL		, OnMouseHWheel		);
	CALLH(WM_PAINT				, OnPaint			);
	CALLH(WM_TIMER				, OnTimer			);
	CALLH(WM_QUERYENDSESSION	, OnQueryEndSession	);

	CALLH(WM_MEASUREITEM		, OnMeasureItem		);
	CALLH(WM_MENUCHAR			, OnMenuChar		);
	CALLH(WM_NOTIFY				, OnNotify			);
	CALLH(WM_DRAWITEM			, OnDrawItem		);
	CALLH(WM_CAPTURECHANGED		, OnCaptureChanged	);

	default:
		if (WM_APP <= msg && msg <= 0xBFFF) {
			// アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF)
			return DispatchEvent_WM_APP(hwnd, msg, wp, lp);
		}
		break;	// default
	}
	return CallDefWndProc(hwnd, msg, wp, lp);
}

// アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF)
LRESULT Wnd::DispatchEvent_WM_APP(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	return CallDefWndProc(hwnd, msg, wp, lp);
}

// デフォルトメッセージ処理
LRESULT Wnd::CallDefWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	return ::DefWindowProc(hwnd, msg, wp, lp);
}


// ウィンドウを破棄
void Wnd::DestroyWindow()
{
	if (hWnd) {
		::DestroyWindow(hWnd);
		hWnd = NULL;
	}
}


