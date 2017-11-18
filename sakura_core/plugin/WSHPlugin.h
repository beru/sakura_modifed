/*!	@file
	@brief WSHプラグインクラス
*/
#pragma once

#include "plugin/Plugin.h"
#include "macro/WSHManager.h"

#define PII_WSH						L"Wsh"			// WSHセクション
#define PII_WSH_USECACHE			L"UseCache"		// 読み込んだスクリプトを再利用する

class WSHPlug : public Plug {
public:
	WSHPlug(Plugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel) :
		Plug(plugin, id, sJack, sHandler, sLabel)
	{
		wsh = nullptr;
	}
	virtual ~WSHPlug() {
		if (wsh) {
			delete wsh;
			wsh = nullptr;
		}
	}
	WSHMacroManager* wsh;
};

class WSHPlugin : public Plugin {
	// コンストラクタ
public:
	WSHPlugin(const tstring& sBaseDir) : Plugin(sBaseDir) {
		bUseCache = false;
	}

	// デストラクタ
public:
	~WSHPlugin(void);

	// 操作
	// Plugインスタンスの作成。ReadPluginDefPlug/Command から呼ばれる。
	virtual Plug* CreatePlug(Plugin& plugin, PlugId id, wstring sJack, wstring sHandler, wstring sLabel) {
		return new WSHPlug(plugin, id, sJack, sHandler, sLabel);
	}

	// 実装
public:
	bool ReadPluginDef(DataProfile& profile, DataProfile* profileMlang);
	bool ReadPluginOption(DataProfile& profile);
	Plug::Array GetPlugs() const {
		return plugs;
	}
	bool InvokePlug(EditView& view, Plug& plug, WSHIfObj::List& params);

	// メンバ変数
private:
	bool bUseCache;

};

