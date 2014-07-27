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

#ifndef CPLUGINSERVICE_H
#define CPLUGINSERVICE_H

#include <windows.h>
#include "CBasePluginService.h"

class CCppCheckDialog;

///////////////////////////////////////////////////////////////////////////////
#ifdef PRAGMA_PUSH4
#pragma pack(push, 4)
#endif

#ifdef __cplusplus
extern "C" {
#endif

DLL_EXPORT void WINAPI PluginCppCheckDialog(SAKURA_DLL_PLUGIN_OBJ* obj);

#ifdef __cplusplus
}
#endif

#ifdef PRAGMA_PUSH4
#pragma pack(pop)
#endif

///////////////////////////////////////////////////////////////////////////////
#define PROFILE_KEY_CPPCHECK_EXE_PATH	L"CppCheckExePath"
#define PROFILE_DEF_CPPCHECK_EXE_PATH	L"cppcheck.exe"
#define PROFILE_KEY_PLATFORM			L"Platform"
#define PROFILE_DEF_PLATFORM			L"win32W"

///////////////////////////////////////////////////////////////////////////////
class CPluginService : public CBasePluginService
{
public:
	CPluginService();
	virtual ~CPluginService();

	WideString			m_strCppCheckExePath;	//!< cppcheck.exeパス
	WideString			m_strPlatform;			//!< platform(unix32, unix64, win32A, win32W, win64)
	WideString			m_strResultFile;		//!< cppcheck結果ファイル
	WideString			m_strTargetName;
	CCppCheckDialog*	m_lpCppCheckDialog;		//!< モードレスが閉じないようここで管理する

public:
	virtual LPCWSTR GetPluginName(){ return L"CppCheckPlugin"; }

public:
	void OnPluginCppCheckDialog(SAKURA_DLL_PLUGIN_OBJ* obj);

	void ReadProfile();
};

///////////////////////////////////////////////////////////////////////////////
extern CPluginService thePluginService;

#endif	//CPLUGINSERVICE_H
