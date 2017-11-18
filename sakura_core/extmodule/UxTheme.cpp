/*!	@file
	@brief UxTheme ���I���[�h

	UxTheme (Windows thmeme manager) �ւ̓��I�A�N�Z�X�N���X
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


/*! DLL �̃��[�h

	��x���� LoadLibrary() ���s���Ȃ����ƈȊO�� DllImp::Init() �Ɠ���
	�iUxTheme ���Ή� OS �ł� LoadLibrary() ���s�̌J�Ԃ���h���j

	@author ryoji
	@date 2007.04.01 ryoji �V�K
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
	UxTheme �̃t�@�C������n��
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

