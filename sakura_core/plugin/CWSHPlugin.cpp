/*!	@file
	@brief WSHプラグインクラス

*/
/*
	Copyright (C) 2009, syat

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
#include "plugin/CWSHPlugin.h"
#include "plugin/CPluginIfObj.h"
#include "macro/CWSHManager.h"

// デストラクタ
WSHPlugin::~WSHPlugin(void)
{
	for (auto it=m_plugs.begin(); it!=m_plugs.end(); ++it) {
		delete *it;
	}
}

// プラグイン定義ファイルを読み込む
bool WSHPlugin::ReadPluginDef(
	DataProfile* profile,
	DataProfile* profileMlang
	)
{
	ReadPluginDefCommon(profile, profileMlang);

	// WSHセクションの読み込み
	profile->IOProfileData<bool>(PII_WSH, PII_WSH_USECACHE, m_bUseCache);

	// プラグの読み込み
	ReadPluginDefPlug(profile, profileMlang);

	// コマンドの読み込み
	ReadPluginDefCommand(profile, profileMlang);

	// オプション定義の読み込み	// 2010/3/24 Uchi
	ReadPluginDefOption(profile, profileMlang);

	// 文字列定義の読み込み
	ReadPluginDefString(profile, profileMlang);

	return true;
}

// オプションファイルを読み込む
bool WSHPlugin::ReadPluginOption(
	DataProfile* profile
	)
{
	return true;
}

// プラグを実行する
bool WSHPlugin::InvokePlug(
	EditView* view,
	Plug& plug,
	WSHIfObj::List& params
	)
{
	WSHPlug& wshPlug = static_cast<WSHPlug&>(plug);
	WSHMacroManager* pWsh = NULL;

	if (!m_bUseCache || !wshPlug.m_Wsh) {
		FilePath path(plug.m_plugin.GetFilePath(to_tchar(plug.m_sHandler.c_str())).c_str());

		pWsh = (WSHMacroManager*)WSHMacroManager::Creator(path.GetExt(true));
		if (!pWsh) {
			return false;
		}

		bool bLoadResult = pWsh->LoadKeyMacro(G_AppInstance(), path);
		if (!bLoadResult) {
			ErrorMessage(NULL, LS(STR_WSHPLUG_LOADMACRO), static_cast<const TCHAR*>(path));
			delete pWsh;
			return false;
		}

	}else {
		pWsh = wshPlug.m_Wsh;
	}

	PluginIfObj cPluginIfo(*this);		// Pluginオブジェクトを追加
	cPluginIfo.AddRef();
	cPluginIfo.SetPlugIndex(plug.m_id);	// 実行中プラグ番号を提供
	pWsh->AddParam(&cPluginIfo);
	pWsh->AddParam(params);			// パラメータを追加
	pWsh->ExecKeyMacro2(view, FA_NONRECORD | FA_FROMMACRO);
	pWsh->ClearParam();

	if (m_bUseCache) {
		wshPlug.m_Wsh = pWsh;
	}else {
		// 終わったら開放
		delete pWsh;
	}

	return true;
}

