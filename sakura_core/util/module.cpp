#include "StdAfx.h"
#include "module.h"
#include "util/os.h"
#include "util/fileUtil.h"
#include <Shlwapi.h>

/*! 
	カレントディレクトリを実行ファイルの場所に移動
*/
void ChangeCurrentDirectoryToExeDir()
{
	TCHAR szExeDir[_MAX_PATH];
	szExeDir[0] = _T('\0');
	GetExedir(szExeDir, NULL);
	if (szExeDir[0]) {
		::SetCurrentDirectory(szExeDir);
	}else {
		// 移動できないときはSYSTEM32(9xではSYSTEM)に移動
		szExeDir[0] = _T('\0');
		int n = ::GetSystemDirectory(szExeDir, _MAX_PATH);
		if (n && n < _MAX_PATH) {
			::SetCurrentDirectory(szExeDir);
		}
	}
}

HMODULE LoadLibraryExedir(LPCTSTR pszDll)
{
	CurrentDirectoryBackupPoint dirBack;
	// DLL インジェクション対策としてEXEのフォルダに移動する
	ChangeCurrentDirectoryToExeDir();
	return ::LoadLibrary(pszDll);
}

/*!	シェルやコモンコントロール DLL のバージョン番号を取得

	@param[in] lpszDllName DLL ファイルのパス
	@return DLL のバージョン番号（失敗時は 0）
*/
DWORD GetDllVersion(LPCTSTR lpszDllName)
{
	DWORD dwVersion = 0;

	/* For security purposes, LoadLibrary should be provided with a
	   fully-qualified path to the DLL. The lpszDllName variable should be
	   tested to ensure that it is a fully qualified path before it is used. */
	HINSTANCE hinstDll = LoadLibraryExedir(lpszDllName);

	if (hinstDll) {
		DLLGETVERSIONPROC pDllGetVersion;
		pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll,
						  "DllGetVersion");

		/* Because some DLLs might not implement this function, you
		must test for it explicitly. Depending on the particular
		DLL, the lack of a DllGetVersion function can be a useful
		indicator of the version. */

		if (pDllGetVersion) {
			DLLVERSIONINFO dvi = {0};
			dvi.cbSize = sizeof(dvi);
			HRESULT hr = (*pDllGetVersion)(&dvi);
			if (SUCCEEDED(hr)) {
			   dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
			}
		}

		FreeLibrary(hinstDll);
	}
	return dwVersion;
}


/*!
	@brief アプリケーションアイコンの取得
	
	アイコンファイルが存在する場合はそこから，無い場合は
	リソースファイルから取得する
	
	@param hInst [in] Instance Handle
	@param nResource [in] デフォルトアイコン用Resource ID
	@param szFile [in] アイコンファイル名
	@param bSmall [in] true: small icon (16x16) / false: large icon (32x32)
	
	@return アイコンハンドル．失敗した場合はNULL．
*/
HICON GetAppIcon(
	HINSTANCE hInst,
	int nResource,
	const TCHAR* szFile,
	bool bSmall
	)
{
	// サイズの設定
	int size = (bSmall ? GetSystemMetrics(SM_CXSMICON) : GetSystemMetrics(SM_CXICON));

	TCHAR szPath[_MAX_PATH];

	// ファイルからの読み込みをまず試みる
	GetInidirOrExedir(szPath, szFile);

	HICON hIcon = (HICON)::LoadImage(
		NULL,
		szPath,
		IMAGE_ICON,
		size,
		size,
		LR_SHARED | LR_LOADFROMFILE
	);
	if (hIcon) {
		return hIcon;
	}

	// ファイルからの読み込みに失敗したらリソースから取得
	hIcon = (HICON)::LoadImage(
		hInst,
		MAKEINTRESOURCE(nResource),
		IMAGE_ICON,
		size,
		size,
		LR_SHARED
	);
	
	return hIcon;
}


struct VS_VERSION_INFO_HEAD {
	WORD	wLength;
	WORD	wValueLength;
	WORD	bText;
	wchar_t	szKey[16];
	VS_FIXEDFILEINFO Value;
};

/*! リソースから製品バージョンの取得 */
void GetAppVersionInfo(
	HINSTANCE	hInstance,
	int			nVersionResourceID,
	DWORD*		pdwProductVersionMS,
	DWORD*		pdwProductVersionLS
	)
{
	// リソースから製品バージョンの取得
	*pdwProductVersionMS = 0;
	*pdwProductVersionLS = 0;
	static bool bLoad = false;
	static DWORD dwVersionMS = 0;
	static DWORD dwVersionLS = 0;
	if (!hInstance && bLoad) {
		*pdwProductVersionMS = dwVersionMS;
		*pdwProductVersionLS = dwVersionLS;
		return;
	}
	HRSRC hRSRC;
	HGLOBAL hgRSRC;
	VS_VERSION_INFO_HEAD* pVVIH;
	if ((hRSRC = ::FindResource(hInstance, MAKEINTRESOURCE(nVersionResourceID), RT_VERSION))
	 && (hgRSRC = ::LoadResource(hInstance, hRSRC))
	 && (pVVIH = (VS_VERSION_INFO_HEAD*)::LockResource(hgRSRC))
	) {
		*pdwProductVersionMS = pVVIH->Value.dwProductVersionMS;
		*pdwProductVersionLS = pVVIH->Value.dwProductVersionLS;
		if (!hInstance) {
			dwVersionMS = pVVIH->Value.dwProductVersionMS;
			dwVersionLS = pVVIH->Value.dwProductVersionLS;
			bLoad = true;
		}
	}
	return;

}

