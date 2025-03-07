#include "StdAfx.h"
#include "os.h"
#include "util/module.h"
#include "extmodule/UxTheme.h"

/*!	Comctl32.dll のバージョン番号を取得

	@return Comctl32.dll のバージョン番号（失敗時は 0）
*/
static DWORD s_dwComctl32Version = PACKVERSION(0, 0);

DWORD GetComctl32Version()
{
	if (PACKVERSION(0, 0) == s_dwComctl32Version)
		s_dwComctl32Version = GetDllVersion(_T("Comctl32.dll"));
	return s_dwComctl32Version;
}

/*!	自分が現在ビジュアルスタイル表示状態かどうかを示す
	Win32 API の IsAppThemed() はこれとは一致しない（IsAppThemed() と IsThemeActive() との差異は不明）

	@return ビジュアルスタイル表示状態(TRUE)／クラッシック表示状態(FALSE)
*/
bool IsVisualStyle()
{
	// ロードした Comctl32.dll が Ver 6 以上で画面設定がビジュアルスタイル指定になっている場合だけ
	// ビジュアルスタイル表示になる（マニフェストで指定しないと Comctl32.dll は 6 未満になる）
	return ((GetComctl32Version() >= PACKVERSION(6, 0)) && UxTheme::getInstance().IsThemeActive());
}


/*!	指定ウィンドウでビジュアルスタイルを使わないようにする

	@param[in] hWnd ウィンドウ
*/
void PreventVisualStyle(HWND hWnd)
{
	UxTheme::getInstance().SetWindowTheme(hWnd, L"", L"");
	return;
}


/*!	コモンコントロールを初期化する */
void MyInitCommonControls()
{
	BOOL (WINAPI *pfnInitCommonControlsEx)(LPINITCOMMONCONTROLSEX);

	BOOL bInit = FALSE;
	HINSTANCE hDll = ::GetModuleHandle(_T("COMCTL32"));
	if (hDll) {
		*(FARPROC*)&pfnInitCommonControlsEx = ::GetProcAddress(hDll, "InitCommonControlsEx");
		if (pfnInitCommonControlsEx) {
			INITCOMMONCONTROLSEX icex;
			icex.dwSize = sizeof(icex);
			icex.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES;
			bInit = pfnInitCommonControlsEx(&icex);
		}
	}

	if (!bInit) {
		::InitCommonControls();
	}
}


/*!
	指定したウィンドウ／長方形領域／点／モニタに対応するモニタ作業領域を取得する

	モニタ作業領域：画面全体からシステムのタスクバーやアプリケーションのツールバーが占有する領域を除いた領域

	@param hWnd/prc/pt/hMon [in] 目的のウィンドウ／長方形領域／点／モニタ
	@param prcWork [out] モニタ作業領域
	@param prcMonitor [out] モニタ画面全体

	@retval true 対応するモニタはプライマリモニタ
	@retval false 対応するモニタは非プライマリモニタ

	@note 出力パラメータの prcWork や prcMonior に NULL を指定した場合、
	該当する領域情報は出力しない。呼び出し元は欲しいものだけを指定すればよい。
*/
bool GetMonitorWorkRect(HWND hWnd, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	HMONITOR hMon = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	return GetMonitorWorkRect(hMon, prcWork, prcMonitor);
}

bool GetMonitorWorkRect(LPCRECT prc, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	HMONITOR hMon = ::MonitorFromRect(prc, MONITOR_DEFAULTTONEAREST);
	return GetMonitorWorkRect(hMon, prcWork, prcMonitor);
}

bool GetMonitorWorkRect(POINT pt, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	HMONITOR hMon = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
	return GetMonitorWorkRect(hMon, prcWork, prcMonitor);
}

bool GetMonitorWorkRect(HMONITOR hMon, LPRECT prcWork, LPRECT prcMonitor/* = NULL*/)
{
	MONITORINFO mi = {0};
	mi.cbSize = sizeof(mi);
	::GetMonitorInfo(hMon, &mi);
	if (prcWork)
		*prcWork = mi.rcWork;		// work area rectangle of the display monitor
	if (prcMonitor)
		*prcMonitor = mi.rcMonitor;	// display monitor rectangle
	return (mi.dwFlags == MONITORINFOF_PRIMARY);
}

/*!
	@brief レジストリから文字列を読み出す．
	
	@param hive        [in]  HIVE
	@param path        [in]  レジストリキーへのパス
	@param item        [in]  レジストリアイテム名．NULLで標準のアイテム．
	@param buffer      [out] 取得文字列を格納する場所
	@param bufferCount [in]  bufferの指す領域のサイズ。文字単位。
	
	@retval true 値の取得に成功
	@retval false 値の取得に失敗
*/
bool ReadRegistry(
	HKEY hive,
	const TCHAR* path,
	const TCHAR* item,
	TCHAR* buffer,
	unsigned bufferCount
	)
{
	bool Result = false;
	
	HKEY Key;
	if (RegOpenKeyEx(hive, path, 0, KEY_READ, &Key) == ERROR_SUCCESS) {
		auto_memset(buffer, 0, bufferCount);

		DWORD dwType = REG_SZ;
		DWORD dwDataLen = (bufferCount - 1) * sizeof(TCHAR); // ※バイト単位！
		
		Result = (RegQueryValueEx(Key, item, NULL, &dwType, reinterpret_cast<LPBYTE>(buffer), &dwDataLen) == ERROR_SUCCESS);
		
		RegCloseKey(Key);
	}
	return Result;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      クリップボード                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// SetClipboardTextA,SetClipboardTextT 実装用テンプレート
//
/*! クリープボードにText形式でコピーする
	@param hwnd    [in] クリップボードのオーナー
	@param pszText [in] 設定するテキスト
	@param nLength [in] 有効なテキストの長さ。文字単位。
	
	@retval true コピー成功
	@retval false コピー失敗。場合によってはクリップボードに元の内容が残る
*/
template <class T>
bool SetClipboardTextImp(
	HWND hwnd,
	const T* pszText,
	size_t nLength
	)
{
	HGLOBAL	hgClip = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (nLength + 1) * sizeof(T));
	if (!hgClip) {
		return false;
	}
	T* pszClip = (T*)::GlobalLock(hgClip);
	if (!pszClip) {
		::GlobalFree(hgClip);
		return false;
	}
	auto_memcpy(pszClip, pszText, nLength);
	pszClip[nLength] = 0;
	::GlobalUnlock(hgClip);
	if (!::OpenClipboard(hwnd)) {
		::GlobalFree(hgClip);
		return false;
	}
	::EmptyClipboard();
	if (sizeof(T) == sizeof(char)) {
		::SetClipboardData(CF_OEMTEXT, hgClip);
	}else if (sizeof(T) == sizeof(wchar_t)) {
		::SetClipboardData(CF_UNICODETEXT, hgClip);
	}else {
		assert(0); // ※ここには来ない
	}
	::CloseClipboard();

	return true;
}

bool SetClipboardText(HWND hwnd, const char* pszText, size_t nLength)
{
	return SetClipboardTextImp<char>(hwnd, pszText, nLength);
}

bool SetClipboardText(HWND hwnd, const wchar_t* pszText, size_t nLength)
{
	return SetClipboardTextImp<wchar_t>(hwnd, pszText, nLength);
}

/*
	@note IDataObject::GetData() で tymed = TYMED_HGLOBAL を指定すること。
*/
bool IsDataAvailable(LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat)
{
	FORMATETC	fe;

	// 他のTYMEDが利用可能でも、IDataObject::GetData()で
	//  tymed = TYMED_HGLOBALを指定すれば問題ない
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = TYMED_HGLOBAL;
	return pDataObject->QueryGetData(&fe) == S_OK;
}

HGLOBAL GetGlobalData(LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat)
{
	FORMATETC fe;
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = TYMED_HGLOBAL;

	HGLOBAL hDest = NULL;
	STGMEDIUM stgMedium;
	if (pDataObject->GetData(&fe, &stgMedium) == S_OK) {
		if (!stgMedium.pUnkForRelease) {
			if (stgMedium.tymed == TYMED_HGLOBAL)
				hDest = stgMedium.hGlobal;
		}else {
			if (stgMedium.tymed == TYMED_HGLOBAL) {
				SIZE_T nSize = ::GlobalSize(stgMedium.hGlobal);
				hDest = ::GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, nSize);
				if (hDest) {
					// copy the bits
					LPVOID lpSource = ::GlobalLock(stgMedium.hGlobal);
					LPVOID lpDest = ::GlobalLock(hDest);
					memcpy_raw(lpDest, lpSource, nSize);
					::GlobalUnlock(hDest);
					::GlobalUnlock(stgMedium.hGlobal);
				}
			}
			::ReleaseStgMedium(&stgMedium);
		}
	}
	return hDest;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       システム資源                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* システムリソースを調べる
	Win16 の時は、GetFreeSystemResources という関数がありました。しかし、Win32 ではありません。
	サンクを作るだの DLL を作るだのは難しすぎます。簡単な方法を説明します。
	お使いの Windows95 の [アクセサリ]-[システムツール] にリソースメータがあるのなら、
	c:\windows\system\rsrc32.dll があるはずです。これは、リソースメータという Win32 アプリが、
	Win16 の GetFreeSystemResources 関数を呼ぶ為の DLL です。これを使いましょう。
*/
BOOL GetSystemResources(
	int* pnSystemResources,
	int* pnUserResources,
	int* pnGDIResources
	)
{
	#define GFSR_SYSTEMRESOURCES	0x0000
	#define GFSR_GDIRESOURCES		0x0001
	#define GFSR_USERRESOURCES		0x0002
	int (CALLBACK *GetFreeSystemResources)(int);

	HINSTANCE hlib = ::LoadLibraryExedir(_T("RSRC32.dll"));
	if ((INT_PTR)hlib > 32) {
		GetFreeSystemResources = (int (CALLBACK *)(int))GetProcAddress(
			hlib,
			"_MyGetFreeSystemResources32@4"
		);
		if (GetFreeSystemResources) {
			*pnSystemResources = GetFreeSystemResources(GFSR_SYSTEMRESOURCES);
			*pnUserResources = GetFreeSystemResources(GFSR_USERRESOURCES);
			*pnGDIResources = GetFreeSystemResources(GFSR_GDIRESOURCES);
			::FreeLibrary(hlib);
			return TRUE;
		}else {
			::FreeLibrary(hlib);
			return FALSE;
		}
	}else {
		return FALSE;
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        便利クラス                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// コンストラクタでカレントディレクトリを保存し、デストラクタでカレントディレクトリを復元するモノ。

CurrentDirectoryBackupPoint::CurrentDirectoryBackupPoint()
{
	size_t n = ::GetCurrentDirectory(_countof(szCurDir), szCurDir);
	if (n>0 && n<_countof(szCurDir)) {
		// ok
	}else {
		// ng
		szCurDir[0] = _T('\0');
	}
}

CurrentDirectoryBackupPoint::~CurrentDirectoryBackupPoint()
{
	if (szCurDir[0]) {
		::SetCurrentDirectory(szCurDir);
	}
}

