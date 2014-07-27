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

DLL_EXPORT void WINAPI PluginHello(SAKURA_DLL_PLUGIN_OBJ* obj);

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
	virtual LPCWSTR GetPluginName(){ return L"HelloPlugin"; }
	virtual LPCWSTR GetMacroExt(){ return L"hello"; }
	virtual void RunMacro(LPCWSTR Source);

public:
	//PP_EDITOR_START
	virtual void OnPluginEditorStart(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_EDITOR_END
	virtual void OnPluginEditorEnd(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_DOCUMENT_OPEN
	virtual void OnPluginDocumentOpen(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_DOCUMENT_CLOSE
	virtual void OnPluginDocumentClose(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_DOCUMENT_BEFORE_SAVE
	virtual void OnPluginDocumentBeforeSave(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_DOCUMENT_AFTER_SAVE
	virtual void OnPluginDocumentAfterSave(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_OUTLINE
	//virtual void OnPluginOutline(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_SMARTINDENT
	//virtual void OnPluginSmartIndent(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_COMPLEMENT
	//virtual void OnPluginComplement(SAKURA_DLL_PLUGIN_OBJ* obj);
	//PP_COMPLEMENT_GLOBAL
	//virtual void OnPluginComplementGlobal(SAKURA_DLL_PLUGIN_OBJ* obj);

	void OnPluginHello(SAKURA_DLL_PLUGIN_OBJ* obj, const int nCommandNo);
};

///////////////////////////////////////////////////////////////////////////////
extern CPluginService thePluginService;

#endif	//CPLUGINSERVICE_H
