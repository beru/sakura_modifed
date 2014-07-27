/*
	Copyright (C) 2014, Plugins developers

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

#ifndef CPLUGINSERVICE_H
#define CPLUGINSERVICE_H

#include <windows.h>
#include "CBasePluginService.h"

///////////////////////////////////////////////////////////////////////////////
//#define DEFAULT_CSC_PATH L"C:\\Windows\\Microsoft.NET\\Framework\\v2.0.50727\\csc.exe"
//#define DEFAULT_CSC_PATH L"C:\\Windows\\Microsoft.NET\\Framework\\v3.5\\csc.exe"
#define DEFAULT_CSC_PATH L"C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319\\csc.exe"

///////////////////////////////////////////////////////////////////////////////
#ifdef PRAGMA_PUSH4
#pragma pack(push, 4)
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLL_EXPORT void WINAPI PluginCscCompile(SAKURA_DLL_PLUGIN_OBJ* obj);
DLL_EXPORT void WINAPI PluginCscCompileAndRun(SAKURA_DLL_PLUGIN_OBJ* obj);

#ifdef __cplusplus
}
#endif

#ifdef PRAGMA_PUSH4
#pragma pack(pop)
#endif

///////////////////////////////////////////////////////////////////////////////
class CPluginService : public CBasePluginService
{
public:
	CPluginService();
	virtual ~CPluginService();

public:
	virtual LPCWSTR GetPluginName(){ return L"CSharpPlugin"; }
	void OnPluginCscCompile(SAKURA_DLL_PLUGIN_OBJ* obj);
	void OnPluginCscCompileAndRun(SAKURA_DLL_PLUGIN_OBJ* obj);

	WideString	m_strCommandPath;
	DWORD		m_dwMode;
	WideString	m_strCsFileName;
	WideString	m_strCompileLogFileName;
	WideString	m_strRunLogFileName;
	WideString	m_strRunFileName;

protected:
	void ReadProfile();
	HANDLE ExecuteCommandCompile(LPCWSTR lpszCsFileName, LPCWSTR lpszCompileLogFileName);
	HANDLE ExecuteCommandRun(LPCWSTR lpszRunFileName, LPCWSTR lpszRunLogFileName);
	void OnPluginCscCompileSub(const int nType);
	BOOL SaveCsFile(LPCWSTR lpszCsFileName, LPCWSTR lpszText);
	void DisplayResult(LPCWSTR lpszLogFileName);
};

///////////////////////////////////////////////////////////////////////////////
extern CPluginService thePluginService;

#endif	//CPLUGINSERVICE_H
