/*
	Copyright (C) 2013-2014, Plugins developers

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
#include <Windows.h>
#include <Shlobj.h>
#include <Ole2.h>
#include <commdlg.h>
#include <string>
#include <time.h>
#include "CommonTools.h"

///////////////////////////////////////////////////////////////////////////////
int CALLBACK SHBrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	switch(uMsg){
	case BFFM_INITIALIZED:
		::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)lpData);
		break;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
BOOL GetOpenFolderNameDialog(HWND hWnd, LPCWSTR lpszTitle, LPCWSTR lpszInitFolder, WideString& strFolder)
{
	BOOL bResult = FALSE;

	::OleInitialize(NULL);

	wchar_t* lpszBuffer = new wchar_t[MAX_PATH_LENGTH];
	if(lpszBuffer != NULL){
		::ZeroMemory(lpszBuffer, MAX_PATH_LENGTH * sizeof(wchar_t));
#ifdef __MINGW32__
		wcscpy(lpszBuffer, strFolder.c_str());
#else
		wcscpy_s(lpszBuffer, MAX_PATH_LENGTH, strFolder.c_str());
#endif
		BROWSEINFO bi;
		::ZeroMemory(&bi, sizeof(bi));
		bi.hwndOwner      = hWnd;
		bi.pidlRoot       = NULL;
		bi.pszDisplayName = lpszBuffer;
		bi.lpszTitle      = lpszTitle;
		bi.ulFlags        = BIF_RETURNONLYFSDIRS;
		bi.lpfn           = SHBrowseCallbackProc;
		bi.lParam         = (LPARAM)lpszInitFolder;
		bi.iImage         = 0;
		LPITEMIDLIST lpItemIDList = ::SHBrowseForFolder(&bi);
		if(lpItemIDList != NULL){
			bResult = ::SHGetPathFromIDList(lpItemIDList, lpszBuffer);
			strFolder = lpszBuffer;
			::CoTaskMemFree(lpItemIDList);
		}
		delete[] lpszBuffer;
	}

	::OleUninitialize();

	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
BOOL GetOpenFileNameDialog(HINSTANCE hInstance, HWND hWnd, LPCWSTR lpszTitle, LPCWSTR lpszInitFolder, LPCWSTR lpszFilter, WideString& strFileName)
{
	BOOL bResult = FALSE;
	wchar_t* lpszBuffer = new wchar_t[MAX_PATH_LENGTH];
	if(lpszBuffer != NULL){
		::ZeroMemory(lpszBuffer, MAX_PATH_LENGTH * sizeof(wchar_t));
#ifdef __MINGW32__
		wcscpy(lpszBuffer, strFileName.c_str());
#else
		wcscpy_s(lpszBuffer, MAX_PATH_LENGTH, strFileName.c_str());
#endif
		OPENFILENAME ofn;
		::ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize     = sizeof(OPENFILENAME);
		ofn.hwndOwner       = hWnd;
		ofn.hInstance       = hInstance;
		ofn.lpstrFilter     = lpszFilter;
		ofn.nFilterIndex    = 0;
		ofn.lpstrFile       = lpszBuffer;
		ofn.nMaxFile        = MAX_PATH_LENGTH;
		ofn.lpstrInitialDir = lpszInitFolder;
		ofn.lpstrTitle      = lpszTitle;
		ofn.Flags           = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		ofn.lpstrDefExt     = L"";
		ofn.lCustData       = (LPARAM)NULL;
		bResult = ::GetOpenFileName(&ofn);
		if(bResult){
			strFileName = lpszBuffer;
		}
		delete[] lpszBuffer;
	}
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
BOOL GetSaveFileNameDialog(HINSTANCE hInstance, HWND hWnd, LPCWSTR lpszTitle, LPCWSTR lpszInitFolder, LPCWSTR lpszFilter, WideString& strFileName)
{
	BOOL bResult = FALSE;
	wchar_t* lpszBuffer = new wchar_t[MAX_PATH_LENGTH];
	if(lpszBuffer != NULL){
		::ZeroMemory(lpszBuffer, MAX_PATH_LENGTH * sizeof(wchar_t));
#ifdef __MINGW32__
		wcscpy(lpszBuffer, strFileName.c_str());
#else
		wcscpy_s(lpszBuffer, MAX_PATH_LENGTH, strFileName.c_str());
#endif
		OPENFILENAME ofn;
		::ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize     = sizeof(OPENFILENAME);
		ofn.hwndOwner       = hWnd;
		ofn.hInstance       = hInstance;
		ofn.lpstrFilter     = lpszFilter;
		ofn.nFilterIndex    = 0;
		ofn.lpstrFile       = lpszBuffer;
		ofn.nMaxFile        = MAX_PATH_LENGTH;
		ofn.lpstrInitialDir = lpszInitFolder;
		ofn.lpstrTitle      = lpszTitle;
		ofn.Flags           = OFN_CREATEPROMPT | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
		ofn.lpstrDefExt     = L"";
		ofn.lCustData       = (LPARAM)NULL;
		bResult = ::GetSaveFileName(&ofn);
		if(bResult){
			strFileName = lpszBuffer;
		}
		delete[] lpszBuffer;
	}
	return bResult;
}

///////////////////////////////////////////////////////////////////////////////
WideString GetParentFolder(WideString& strPath)
{
	WideString strResult = strPath;
	if(strPath.length() > 3){
		if(strPath.substr(strPath.length() - 1, 1) == L"\\"){
			strPath = strPath.substr(0, strPath.length() - 1);
		}
		const wchar_t* p = wcsrchr(strPath.c_str(), L'\\');
		if(p != NULL){
			int n = p - strPath.c_str();
			if(n < 3) n = 3;
			strResult = strPath.substr(0, n);
		}
	}
	return strResult;
}

///////////////////////////////////////////////////////////////////////////////
WideString GetTempFileName(LPCWSTR lpszPathName, LPCWSTR lpszPrefix, bool bFullPath)
{
	WideString strResult = L"";
	wchar_t* lpszBuffer = new wchar_t[MAX_PATH_LENGTH];
	::ZeroMemory(lpszBuffer, MAX_PATH_LENGTH * sizeof(wchar_t));
	if(::GetTempFileName(lpszPathName, lpszPrefix, 0, lpszBuffer) == 0){
		wsprintf(lpszBuffer, L"%s\\%s\\%08x", lpszPathName, lpszPrefix, (DWORD)time(NULL));
	}
	wchar_t* p = lpszBuffer;
	if (!bFullPath) {
		p = wcsrchr(lpszBuffer, L'\\');
		if (p) {
			p++;
		}
	}
	strResult = p;
	delete[] lpszBuffer;
	return strResult;
}

///////////////////////////////////////////////////////////////////////////////
WideString GetCurrentDirectory()
{
	WideString strResult;
	wchar_t* lpszBuffer = new wchar_t[MAX_PATH_LENGTH];
	if(::GetCurrentDirectory(MAX_PATH_LENGTH, lpszBuffer) != 0){
		strResult = lpszBuffer;
	}
	delete[] lpszBuffer;
	return strResult;
}

///////////////////////////////////////////////////////////////////////////////
WideString GetSystemDirectory()
{
	WideString strResult = L"";
	wchar_t* lpszBuffer = new wchar_t[MAX_PATH_LENGTH];
	if(lpszBuffer != NULL){
		if(::GetSystemDirectory(lpszBuffer, MAX_PATH_LENGTH) != 0){
			strResult = lpszBuffer;
		}
		delete[] lpszBuffer;
	}
	return strResult;
}
