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

///////////////////////////////////////////////////////////////////////////////
#ifdef PRAGMA_PUSH4
#pragma pack(push, 4)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#ifdef PRAGMA_PUSH4
#pragma pack(pop)
#endif

///////////////////////////////////////////////////////////////////////////////
#define RUBY_SCRIPT_PLUGIN			L"RubyScriptPlugin"
#define RUBY_SCRIPT_EXT				L"mrb"
#define RUBY_PROFILE_MODULE_PATH	L"RubyModulePath"
#define RUBY_DEFAULT_MODULE_NAME	L"msvcrt-ruby200.dll"

///////////////////////////////////////////////////////////////////////////////
class CPluginService : public CBasePluginService
{
public:
	CPluginService();
	virtual ~CPluginService();

	WideString			m_strRubyModulePath;	//!< msvcrt-ruby200.dllファイルパス指定
	CRITICAL_SECTION	m_CriticalSection;		//!< 再入時のマップロック

public:
	virtual LPCWSTR GetPluginName(){ return RUBY_SCRIPT_PLUGIN; }
	virtual LPCWSTR GetMacroExt(){ return RUBY_SCRIPT_EXT; }
	virtual void RunMacro(LPCWSTR Source);

	void EnterCriticalSection();
	void LeaveCriticalSection();

public:
	BOOL InitializePlugin(SAKURA_DLL_PLUGIN_OBJ* obj);
	void ReadProfile();
};

///////////////////////////////////////////////////////////////////////////////
extern CPluginService thePluginService;

#endif	//CPLUGINSERVICE_H
