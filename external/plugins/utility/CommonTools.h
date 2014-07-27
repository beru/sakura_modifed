/*
	Copyright (C) 2013, Plugins developers

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

#ifndef CPLUGIN_COMMON_TOOLS_H
#define CPLUGIN_COMMON_TOOLS_H

#include "CPluginCommon.h"

#define MAX_PATH_LENGTH	65536

BOOL GetOpenFolderNameDialog(HWND hWnd, LPCWSTR lpszTitle, LPCWSTR lpszInitFolder, WideString& strFolder);
BOOL GetOpenFileNameDialog(HINSTANCE hInstance, HWND hWnd, LPCWSTR lpszTitle, LPCWSTR lpszInitFolder, LPCWSTR lpszFilter, WideString& strFileName);
BOOL GetOpenFileNameDialog(HINSTANCE hInstance, HWND hWnd, LPCWSTR lpszTitle, LPCWSTR lpszInitFolder, LPCWSTR lpszFilter, WideString& strFileName);
WideString GetParentFolder(WideString& strPath);
WideString GetTempFileName(LPCWSTR lpszPathName, LPCWSTR lpszPrefix, BOOL bFullPath = TRUE);
WideString GetCurrentDirectory();
WideString GetSystemDirectory();

#endif	//CPLUGIN_COMMON_TOOLS_H
