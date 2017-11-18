/*!	@file
	@brief プラグインマクロマネージャクラス
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
PluginMacroManager::PluginMacroManager(const wchar_t* ext, Plug* plug)
	:
	ext(ext),
	plug(plug)
{
}

///////////////////////////////////////////////////////////////////////////////
PluginMacroManager::~PluginMacroManager()
{
}

///////////////////////////////////////////////////////////////////////////////
// マクロを実行する
bool PluginMacroManager::ExecKeyMacro(EditView& editView, int flags) const
{
	bool result = false;
	WSHIfObj::List params;
	MacroIfObj* objMacro = new MacroIfObj(MacroIfObj::MACRO_MODE_EXEC, ext.c_str(), flags, source.c_str());
	if (objMacro) {
		objMacro->AddRef();
		params.push_back(objMacro);
		if (plug) {
			objMacro->SetMatch(1);	// Run macro mode
			plug->Invoke(editView, params);
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
	source = L"";
	TextInputStream in(Path);
	if (in) {
		while (in) {
			source += in.ReadLineW() + L"\r\n";
		}
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// 文字列からマクロを読み込む
bool PluginMacroManager::LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* Code)
{
	source = to_wchar(Code);
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// 拡張子が一致したらオブジェクトを生成する
MacroManagerBase* PluginMacroManager::Creator(EditView& view, const TCHAR* ext)
{
	WSHIfObj::List params;
	MacroIfObj* objMacro = new MacroIfObj(MacroIfObj::MACRO_MODE_CREATOR, ext, 0, L"");
	objMacro->AddRef();
	params.push_back(objMacro);

	Plug::Array plugs;
	JackManager::getInstance().GetUsablePlug(PP_MACRO, 0, &plugs);
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		objMacro->SetMatch(0);	// Check macro ext mode
		(*it)->Invoke(view, params);
		if (objMacro->IsMatch()) {
			objMacro->Release();
			return new PluginMacroManager(ext, *it);
		}
	}
	objMacro->Release();
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Register plugin macro manager
void PluginMacroManager::Declare(void)
{
	MacroFactory::getInstance().RegisterCreator(Creator);
}

