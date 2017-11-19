#pragma once

#include <ObjIdl.h> // LPDATAOBJECT

// システム資源
BOOL GetSystemResources(int*, int*, int*);	// システムリソースを調べる
BOOL CheckSystemResources(const TCHAR*);		// システムリソースのチェック

// クリップボード
bool SetClipboardText(HWND hwnd, const char* pszText, size_t nLength);	// クリープボードにText形式でコピーする。ANSI版。nLengthは文字単位。
bool SetClipboardText(HWND hwnd, const wchar_t* pszText, size_t nLength);	// クリープボードにText形式でコピーする。UNICODE版。nLengthは文字単位。
bool IsDataAvailable(LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat);
HGLOBAL GetGlobalData(LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat);

bool ReadRegistry(HKEY Hive, const TCHAR* Path, const TCHAR* Item, TCHAR* Buffer, unsigned BufferCount);

//	マルチモニタ対応のデスクトップ領域取得
bool GetMonitorWorkRect(HWND     hWnd, LPRECT prcWork, LPRECT prcMonitor = NULL);
bool GetMonitorWorkRect(LPCRECT  prc,  LPRECT prcWork, LPRECT prcMonitor = NULL);
bool GetMonitorWorkRect(POINT    pt,   LPRECT prcWork, LPRECT prcMonitor = NULL);
bool GetMonitorWorkRect(HMONITOR hMon, LPRECT prcWork, LPRECT prcMonitor = NULL);

#define PACKVERSION(major, minor) MAKELONG(minor, major)
DWORD GetComctl32Version();					// Comctl32.dll のバージョン番号を取得
bool IsVisualStyle();						// 自分が現在ビジュアルスタイル表示状態かどうかを示す
void PreventVisualStyle(HWND hWnd);		// 指定ウィンドウでビジュアルスタイルを使わないようにする
void MyInitCommonControls();				// コモンコントロールを初期化する

// カレントディレクトリユーティリティ。
// コンストラクタでカレントディレクトリを保存し、デストラクタでカレントディレクトリを復元するモノ。
class CurrentDirectoryBackupPoint {
public:
	CurrentDirectoryBackupPoint();
	~CurrentDirectoryBackupPoint();
private:
	TCHAR szCurDir[_MAX_PATH];
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      メッセージ定数                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- マウス -- -- //

#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL	0x020A
#endif
// マウスサイドボタン対応
#ifndef WM_XBUTTONDOWN
	#define WM_XBUTTONDOWN   0x020B
	#define WM_XBUTTONUP     0x020C
	#define WM_XBUTTONDBLCLK 0x020D
#endif
#ifndef XBUTTON1
	#define XBUTTON1 0x0001
	#define XBUTTON2 0x0002
#endif

// -- -- テーマ -- -- //

#ifndef	WM_THEMECHANGED
#define WM_THEMECHANGED		0x031A
#endif

// -- -- IME (imm.h) -- -- //

#ifndef IMR_RECONVERTSTRING
#define IMR_RECONVERTSTRING             0x0004
#endif // IMR_RECONVERTSTRING

#ifndef IMR_CONFIRMRECONVERTSTRING
#define IMR_CONFIRMRECONVERTSTRING             0x0005
#endif // IMR_CONFIRMRECONVERTSTRING
