#pragma once

void GetAppVersionInfo(HINSTANCE, int, DWORD*, DWORD*);	// リソースから製品バージョンの取得

HICON GetAppIcon(HINSTANCE hInst, int nResource, const TCHAR* szFile, bool bSmall = false);

DWORD GetDllVersion(LPCTSTR lpszDllName);	// シェルやコモンコントロール DLL のバージョン番号を取得	// 2006.06.17 ryoji

void ChangeCurrentDirectoryToExeDir();

// カレントディレクトリ移動機能付LoadLibrary
HMODULE LoadLibraryExedir(LPCTSTR pszDll);

