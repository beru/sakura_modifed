/*!	@file
	@brief UxTheme ���I���[�h

	UxTheme (Windows thmeme manager) �ւ̓��I�A�N�Z�X�N���X

	@author ryoji
	@date Apr. 1, 2007
*/
/*
	Copyright (C) 2007, ryoji

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "UxTheme.h"

UxTheme::UxTheme()
	:
	m_bInitialized(false)
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
	if (m_bInitialized) {
		return IsAvailable();
	}
	m_bInitialized = true;
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
		{ &m_pfnIsThemeActive,							"IsThemeActive" },
		{ &m_pfnSetWindowTheme,							"SetWindowTheme" },
		{ &m_pfnOpenThemeData,							"OpenThemeData" },
		{ &m_pfnDrawThemeBackground,					"DrawThemeBackground" },
		{ &m_pfnDrawThemeParentBackground,				"DrawThemeParentBackground" },
		{ &m_pfnIsThemeBackgroundPartiallyTransparent,	"IsThemeBackgroundPartiallyTransparent" },
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
	return m_pfnIsThemeActive();
}

// SetWindowTheme API Wrapper
HRESULT UxTheme::SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList)
{
	if (!InitThemeDll()) {
		return S_FALSE;
	}
	return m_pfnSetWindowTheme(hwnd, pszSubAppName, pszSubIdList);
}

// SetWindowTheme API Wrapper
HTHEME UxTheme::OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
	if (!InitThemeDll()) {
		return NULL;
	}
	return (HTHEME)m_pfnOpenThemeData(hwnd, pszClassList);
}

// SetWindowTheme API Wrapper
HRESULT UxTheme::DrawThemeBackground(HTHEME htheme, HDC hdc, int iPartId, int iStateId, RECT* prc, RECT* prcClip)
{
	if (!InitThemeDll()) {
		return S_FALSE;
	}
	return m_pfnDrawThemeBackground(htheme, hdc, iPartId, iStateId, prc, prcClip);
}

// SetWindowTheme API Wrapper
HRESULT UxTheme::DrawThemeParentBackground(HWND hwnd, HDC hdc, RECT *prc)
{
	if (!InitThemeDll()) {
		return S_FALSE;
	}
	return m_pfnDrawThemeParentBackground(hwnd, hdc, prc);
}

// SetWindowTheme API Wrapper
HRESULT UxTheme::IsThemeBackgroundPartiallyTransparent(HTHEME htheme, int iPartId, int iStateId)
{
	if (!InitThemeDll()) {
		return S_FALSE;
	}
	return m_pfnIsThemeBackgroundPartiallyTransparent(htheme, iPartId, iStateId);
}

