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
class CPluginService : public CBasePluginService
{
public:
	CPluginService();
	virtual ~CPluginService();

protected:
	virtual LPCWSTR GetPluginName(){ return L"HelloComplementPlugin"; }
	virtual LPCWSTR GetMacroExt(){ return L""; }

public:
	//PP_COMPLEMENT
	virtual void OnPluginComplement(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_COMPLEMENT_GLOBAL
	virtual void OnPluginComplementGlobal(SAKURA_DLL_PLUGIN_OBJ* obj);

private:
	void DoHelloComplement(const int nType);
};

///////////////////////////////////////////////////////////////////////////////
extern CPluginService thePluginService;

#endif	//CPLUGINSERVICE_H
