/*!	@file
	@brief プラグインマクロマネージャクラス
*/
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
#include "StdAfx.h"
#include "CPluginMacroManager.h"
#include "macro/CMacroFactory.h"
#include "view/CEditView.h"
#include "io/CTextStream.h"
#include "macro/CMacroIfObj.h"
#include "macro/CEditorIfObj.h"
#include "plugin/CJackManager.h"

///////////////////////////////////////////////////////////////////////////////
CPluginMacroManager::CPluginMacroManager(const WCHAR* Ext, CPlug* plug)
{
	m_Ext = Ext;
	m_Plug = plug;
}

///////////////////////////////////////////////////////////////////////////////
CPluginMacroManager::~CPluginMacroManager()
{
}

///////////////////////////////////////////////////////////////////////////////
//!	マクロを実行する
bool CPluginMacroManager::ExecKeyMacro(CEditView* EditView, int flags) const
{
	bool result = false;
	CWSHIfObj::List params;
	CMacroIfObj* objMacro = new CMacroIfObj(CMacroIfObj::MACRO_MODE_EXEC, m_Ext.c_str(), flags, m_Source.c_str());
	if (objMacro != NULL) {
		objMacro->AddRef();
		params.push_back(objMacro);
		if (m_Plug != NULL) {
			objMacro->SetMatch(1);	// Run macro mode
			m_Plug->Invoke(EditView, params);
			result = true;
		}
		objMacro->Release();
	}
	return result;
}

///////////////////////////////////////////////////////////////////////////////
//!	ファイルからマクロを読み込む
BOOL CPluginMacroManager::LoadKeyMacro(HINSTANCE hInstance, const TCHAR* Path)
{
	m_Source = L"";
	CTextInputStream in(Path);
	if (in) {
		while (in) {
			m_Source += in.ReadLineW() + L"\r\n";
		}
		return TRUE;
	}
	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//!	文字列からマクロを読み込む
BOOL CPluginMacroManager::LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* Code)
{
	m_Source = to_wchar(Code);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//!	拡張子が一致したらオブジェクトを生成する
CMacroManagerBase* CPluginMacroManager::Creator(const TCHAR* Ext)
{
	CWSHIfObj::List params;
#ifdef _UNICODE
	CMacroIfObj* objMacro = new CMacroIfObj(CMacroIfObj::MACRO_MODE_CREATOR, Ext, 0, L"");
#else
	CNativeW szWideExt(to_wchar(Ext));
	CMacroIfObj* objMacro = new CMacroIfObj(CMacroIfObj::MACRO_MODE_CREATOR, szWideExt.GetStringPtr(), 0, L"");
#endif
	objMacro->AddRef();
	params.push_back(objMacro);

	CPlug::Array plugs;
	CJackManager::getInstance()->GetUsablePlug(PP_MACRO, 0, &plugs);
	for (auto it = plugs.begin(); it != plugs.end(); it++) {
		objMacro->SetMatch(0);	//Check macro ext mode
		(*it)->Invoke(NULL, params);
		if (objMacro->IsMatch()) {
			objMacro->Release();
#ifdef _UNICODE
			return new CPluginMacroManager(Ext, *it);
#else
			return new CPluginMacroManager(szWideExt.GetStringPtr(), *it);
#endif
		}
	}
	objMacro->Release();
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//!	Register plugin macro manager
void CPluginMacroManager::declare(void)
{
	CMacroFactory::getInstance()->RegisterCreator(Creator);
}

