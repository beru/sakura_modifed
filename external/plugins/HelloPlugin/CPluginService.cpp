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

#include "stdafx.h"
#include <windows.h>
#include "CPluginService.h"
#include "plugin/SakuraPlugin.h"

///////////////////////////////////////////////////////////////////////////////
CPluginService::CPluginService()
{
}

///////////////////////////////////////////////////////////////////////////////
CPluginService::~CPluginService()
{
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::RunMacro(LPCWSTR Source)
{
	::MessageBox(NULL, Source, GetPluginName(), MB_ICONINFORMATION | MB_OK);
}

///////////////////////////////////////////////////////////////////////////////
void CPluginService::OnPluginHello(SAKURA_DLL_PLUGIN_OBJ* obj, const int nCommandNo)
{
	switch(nCommandNo){
	case 0:
		{
			WideString msg;
			msg = L"GetPluginDir = " + Plugin.GetPluginDir();
			::MessageBox(NULL, msg.c_str(), GetPluginName(), MB_ICONINFORMATION | MB_OK);
		}
		break;
	default:
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
//PP_EDITOR_START
void CPluginService::OnPluginEditorStart(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	::MessageBox(NULL, _T("OnPluginEditorStart"), GetPluginName(), MB_ICONINFORMATION | MB_OK);
}

///////////////////////////////////////////////////////////////////////////////
//PP_EDITOR_END
void CPluginService::OnPluginEditorEnd(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	::MessageBox(NULL, _T("OnPluginEditorEnd"), GetPluginName(), MB_ICONINFORMATION | MB_OK);
}

///////////////////////////////////////////////////////////////////////////////
//PP_DOCUMENT_OPEN
void CPluginService::OnPluginDocumentOpen(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	::MessageBox(NULL, _T("OnPluginDocumentOpen"), GetPluginName(), MB_ICONINFORMATION | MB_OK);
}

///////////////////////////////////////////////////////////////////////////////
//PP_DOCUMENT_CLOSE
void CPluginService::OnPluginDocumentClose(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	::MessageBox(NULL, _T("OnPluginDocumentClose"), GetPluginName(), MB_ICONINFORMATION | MB_OK);
}

///////////////////////////////////////////////////////////////////////////////
//PP_DOCUMENT_BEFORE_SAVE
void CPluginService::OnPluginDocumentBeforeSave(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	::MessageBox(NULL, _T("OnPluginDocumentBeforeSave"), GetPluginName(), MB_ICONINFORMATION | MB_OK);
}

///////////////////////////////////////////////////////////////////////////////
//PP_DOCUMENT_AFTER_SAVE
void CPluginService::OnPluginDocumentAfterSave(SAKURA_DLL_PLUGIN_OBJ* obj)
{
	::MessageBox(NULL, _T("OnPluginDocumentAfterSave"), GetPluginName(), MB_ICONINFORMATION | MB_OK);
}
