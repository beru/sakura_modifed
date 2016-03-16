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
#include "PluginMacroManager.h"
#include "macro/MacroFactory.h"
#include "view/EditView.h"
#include "io/TextStream.h"
#include "macro/MacroIfObj.h"
#include "macro/EditorIfObj.h"
#include "plugin/JackManager.h"

///////////////////////////////////////////////////////////////////////////////
PluginMacroManager::PluginMacroManager(const WCHAR* Ext, Plug* plug)
{
	m_Ext = Ext;
	m_Plug = plug;
}

///////////////////////////////////////////////////////////////////////////////
PluginMacroManager::~PluginMacroManager()
{
}

///////////////////////////////////////////////////////////////////////////////
// マクロを実行する
bool PluginMacroManager::ExecKeyMacro(EditView* EditView, int flags) const
{
	bool result = false;
	WSHIfObj::List params;
	MacroIfObj* objMacro = new MacroIfObj(MacroIfObj::MACRO_MODE_EXEC, m_Ext.c_str(), flags, m_source.c_str());
	if (objMacro) {
		objMacro->AddRef();
		params.push_back(objMacro);
		if (m_Plug) {
			objMacro->SetMatch(1);	// Run macro mode
			m_Plug->Invoke(EditView, params);
			result = true;
		}
		objMacro->Release();
	}
	return result;
}

///////////////////////////////////////////////////////////////////////////////
//	ファイルからマクロを読み込む
bool PluginMacroManager::LoadKeyMacro(HINSTANCE hInstance, const TCHAR* Path)
{
	m_source = L"";
	TextInputStream in(Path);
	if (in) {
		while (in) {
			m_source += in.ReadLineW() + L"\r\n";
		}
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// 文字列からマクロを読み込む
bool PluginMacroManager::LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* Code)
{
	m_source = to_wchar(Code);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// 拡張子が一致したらオブジェクトを生成する
MacroManagerBase* PluginMacroManager::Creator(const TCHAR* Ext)
{
	WSHIfObj::List params;
#ifdef _UNICODE
	MacroIfObj* objMacro = new MacroIfObj(MacroIfObj::MACRO_MODE_CREATOR, Ext, 0, L"");
#else
	NativeW szWideExt(to_wchar(Ext));
	MacroIfObj* objMacro = new MacroIfObj(MacroIfObj::MACRO_MODE_CREATOR, szWideExt.GetStringPtr(), 0, L"");
#endif
	objMacro->AddRef();
	params.push_back(objMacro);

	Plug::Array plugs;
	JackManager::getInstance().GetUsablePlug(PP_MACRO, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		objMacro->SetMatch(0);	// Check macro ext mode
		(*it)->Invoke(NULL, params);
		if (objMacro->IsMatch()) {
			objMacro->Release();
#ifdef _UNICODE
			return new PluginMacroManager(Ext, *it);
#else
			return new PluginMacroManager(szWideExt.GetStringPtr(), *it);
#endif
		}
	}
	objMacro->Release();
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Register plugin macro manager
void PluginMacroManager::declare(void)
{
	MacroFactory::getInstance().RegisterCreator(Creator);
}

