/*!	@file
	@brief WSHプラグインクラス
*/
#include "StdAfx.h"
#include "plugin/WSHPlugin.h"
#include "plugin/PluginIfObj.h"
#include "macro/WSHManager.h"

// デストラクタ
WSHPlugin::~WSHPlugin(void)
{
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		delete *it;
	}
}

// プラグイン定義ファイルを読み込む
bool WSHPlugin::ReadPluginDef(
	DataProfile& profile,
	DataProfile* profileMlang
	)
{
	ReadPluginDefCommon(profile, profileMlang);

	// WSHセクションの読み込み
	profile.IOProfileData<bool>(PII_WSH, PII_WSH_USECACHE, bUseCache);

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
	DataProfile& profile
	)
{
	return true;
}

// プラグを実行する
bool WSHPlugin::InvokePlug(
	EditView& view,
	Plug& plug,
	WSHIfObj::List& params
	)
{
	WSHPlug& wshPlug = static_cast<WSHPlug&>(plug);
	WSHMacroManager* pWsh = nullptr;

	if (!bUseCache || !wshPlug.wsh) {
		FilePath path(plug.plugin.GetFilePath(to_tchar(plug.sHandler.c_str())).c_str());

		pWsh = (WSHMacroManager*)WSHMacroManager::Creator(view, path.GetExt(true));
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
		pWsh = wshPlug.wsh;
	}

	PluginIfObj pluginIfo(*this);		// Pluginオブジェクトを追加
	pluginIfo.AddRef();
	pluginIfo.SetPlugIndex(plug.id);	// 実行中プラグ番号を提供
	pWsh->AddParam(&pluginIfo);
	pWsh->AddParam(params);			// パラメータを追加
	pWsh->ExecKeyMacro2(view, FA_NONRECORD | FA_FROMMACRO);
	pWsh->ClearParam();

	if (bUseCache) {
		wshPlug.wsh = pWsh;
	}else {
		// 終わったら開放
		delete pWsh;
	}

	return true;
}

