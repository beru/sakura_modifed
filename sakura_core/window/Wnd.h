#pragma once

#include <Windows.h>
#include "_main/global.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
// ウィンドウの基本クラス
/*!
	@par Wndクラスの基本的な機能
	@li ウィンドウ作成
	@li ウィンドウメッセージ配送

	@par 普通?のウィンドウの使用方法は以下の手順
	@li RegisterWC()	ウィンドウクラス登録
	@li Create()		ウィンドウ作成
*/
class Wnd {
protected:
	friend LRESULT CALLBACK CWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	// Constructors
	Wnd(const TCHAR* pszInheritanceAppend = _T(""));
	virtual ~Wnd();
	/*
	||  Attributes & Operations
	*/

	// ウィンドウクラス登録
	ATOM RegisterWC(
		HINSTANCE	hInstance,
		HICON		hIcon,			// Handle to the class icon.
		HICON		hIconSm,		// Handle to a small icon
		HCURSOR		hCursor,		// Handle to the class cursor.
		HBRUSH		hbrBackground,	// Handle to the class background brush.
		LPCTSTR		lpszMenuName,	// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		LPCTSTR		lpszClassName	// Pointer to a null-terminated string or is an atom.
	);

	// ウィンドウ作成
	HWND Create(
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
	);

	virtual LRESULT DispatchEvent(HWND, UINT, WPARAM, LPARAM); // メッセージ配送
protected:
	// 仮想関数
	virtual LRESULT DispatchEvent_WM_APP(HWND, UINT, WPARAM, LPARAM);	// アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF)
	virtual void PreviCreateWindow(void) { return; } // ウィンドウ作成前の処理(クラス登録前) (virtual)
	virtual void AfterCreateWindow(void) { ::ShowWindow(hWnd, SW_SHOW); } // ウィンドウ作成後の処理 (virtual)

	// 仮想関数 メッセージ処理(デフォルト動作)
	#define DECLH(method) LRESULT method(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {return CallDefWndProc(hwnd, msg, wp, lp);}
	virtual DECLH(OnCreate			);	// WM_CREATE
	virtual DECLH(OnCommand			);	// WM_COMMAND
	virtual DECLH(OnPaint			);	// WM_PAINT
	virtual DECLH(OnLButtonDown		);	// WM_LBUTTONDOWN
	virtual DECLH(OnLButtonUp		);	// WM_LBUTTONUP
	virtual DECLH(OnLButtonDblClk	);	// WM_LBUTTONDBLCLK
	virtual DECLH(OnRButtonDown		);	// WM_RBUTTONDOWN
	virtual DECLH(OnRButtonUp		);	// WM_RBUTTONUP
	virtual DECLH(OnRButtonDblClk	);	// WM_RBUTTONDBLCLK
	virtual DECLH(OnMButtonDown		);	// WM_MBUTTONDOWN
	virtual DECLH(OnMButtonUp		);	// WM_MBUTTONUP
	virtual DECLH(OnMButtonDblClk	);	// WM_MBUTTONDBLCLK
	virtual DECLH(OnMouseMove		);	// WM_MOUSEMOVE
	virtual DECLH(OnMouseWheel		);	// WM_MOUSEWHEEL
	virtual DECLH(OnMouseHWheel		);	// WM_MOUSEHWHEEL
	virtual DECLH(OnTimer			);	// WM_TIMER
	virtual DECLH(OnSize			);	// WM_SIZE
	virtual DECLH(OnMove			);	// WM_MOVE
	virtual DECLH(OnClose			);	// WM_CLOSE
	virtual DECLH(OnDestroy			);	// WM_DSESTROY
	virtual DECLH(OnQueryEndSession);	// WM_QUERYENDSESSION

	virtual DECLH(OnMeasureItem		);	// WM_MEASUREITEM
	virtual DECLH(OnMenuChar		);	// WM_MENUCHAR
	virtual DECLH(OnNotify			);	// WM_NOTIFY
	virtual DECLH(OnDrawItem		);	// WM_DRAWITEM
	virtual DECLH(OnCaptureChanged	);	// WM_CAPTURECHANGED

	// デフォルトメッセージ処理
	virtual LRESULT CallDefWndProc(HWND, UINT, WPARAM, LPARAM);

public:
	// インターフェース
	HWND GetHwnd() const { return hWnd; }
	HWND GetParentHwnd() const { return hwndParent; }
	HINSTANCE GetAppInstance() const { return hInstance; }
	bool GetWindowRect(LPRECT lpRect) { return ::GetWindowRect(hWnd, lpRect) != 0; }

	// 特殊インターフェース (使用は好ましくない)
	void _SetHwnd(HWND hwnd) { hWnd = hwnd; }

	// ウィンドウ標準操作
	void DestroyWindow();

private:
	HINSTANCE	hInstance;	// アプリケーションインスタンスのハンドル
	HWND		hwndParent;	// オーナーウィンドウのハンドル
	HWND		hWnd;			// このダイアログのハンドル
#ifdef _DEBUG
	TCHAR		szClassInheritances[1024];
#endif
};

