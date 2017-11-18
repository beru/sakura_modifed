/*!	@file
	@brief UxTheme 動的ロード

	UxTheme (Windows thmeme manager) への動的アクセスクラス
*/
#include "StdAfx.h"
#include "UxTheme.h"

UxTheme::UxTheme()
	:
	bInitialized(false)
{
}

UxTheme::~UxTheme()
{
}


/*! DLL のロード

	一度しか LoadLibrary() 実行しないこと以外は DllImp::Init() と同じ
	（UxTheme 未対応 OS での LoadLibrary() 失敗の繰返しを防ぐ）

	@author ryoji
	@date 2007.04.01 ryoji 新規
*/
bool UxTheme::InitThemeDll(TCHAR* str)
{
	if (bInitialized) {
		return IsAvailable();
	}
	bInitialized = true;
	return DllImp::InitDll(str) == InitDllResultType::Success;
}

/*!
	UxTheme のファイル名を渡す
*/
LPCTSTR UxTheme::GetDllNameImp(int nIndex)
{
	return _T("UxTheme.dll");
}

bool UxTheme::InitDllImp()
{
	const ImportTable table[] = {
		{ &pfnIsThemeActive,							"IsThemeActive" },
		{ &pfnSetWindowTheme,							"SetWindowTheme" },
		{ &pfnOpenThemeData,							"OpenThemeData" },
		{ &pfnDrawThemeBackground,					"DrawThemeBackground" },
		{ &pfnDrawThemeParentBackground,				"DrawThemeParentBackground" },
		{ &pfnIsThemeBackgroundPartiallyTransparent,	"IsThemeBackgroundPartiallyTransparent" },
		{ NULL, 0 }
	};

	if (!RegisterEntries(table)) {
		return false;
	}

	return true;
}

// IsThemeActive API Wrapper
BOOL UxTheme::IsThemeActive(VOID)
{
	if (!InitThemeDll()) {
		return FALSE;
	}
	return pfnIsThemeActive();
}

// SetWindowTheme API Wrapper
HRESULT UxTheme::SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList)
{
	if (!InitThemeDll()) {
		return S_FALSE;
	}
	return pfnSetWindowTheme(hwnd, pszSubAppName, pszSubIdList);
}

// SetWindowTheme API Wrapper
HTHEME UxTheme::OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
	if (!InitThemeDll()) {
		return NULL;
	}
	return (HTHEME)pfnOpenThemeData(hwnd, pszClassList);
}

// SetWindowTheme API Wrapper
HRESULT UxTheme::DrawThemeBackground(HTHEME htheme, HDC hdc, int iPartId, int iStateId, RECT* prc, RECT* prcClip)
{
	if (!InitThemeDll()) {
		return S_FALSE;
	}
	return pfnDrawThemeBackground(htheme, hdc, iPartId, iStateId, prc, prcClip);
}

// SetWindowTheme API Wrapper
HRESULT UxTheme::DrawThemeParentBackground(HWND hwnd, HDC hdc, RECT *prc)
{
	if (!InitThemeDll()) {
		return S_FALSE;
	}
	return pfnDrawThemeParentBackground(hwnd, hdc, prc);
}

// SetWindowTheme API Wrapper
HRESULT UxTheme::IsThemeBackgroundPartiallyTransparent(HTHEME htheme, int iPartId, int iStateId)
{
	if (!InitThemeDll()) {
		return S_FALSE;
	}
	return pfnIsThemeBackgroundPartiallyTransparent(htheme, iPartId, iStateId);
}

