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
		m_Wsh = NULL;
	}
	virtual ~WSHPlug() {
		if (m_Wsh) {
			delete m_Wsh;
			m_Wsh = NULL;
		}
	}
	WSHMacroManager* m_Wsh;
};

class WSHPlugin : public Plugin {
	// コンストラクタ
public:
	WSHPlugin(const tstring& sBaseDir) : Plugin(sBaseDir) {
		m_bUseCache = false;
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
	bool ReadPluginDef(DataProfile* profile, DataProfile* profileMlang);
	bool ReadPluginOption(DataProfile* profile);
	Plug::Array GetPlugs() const {
		return m_plugs;
	}
	bool InvokePlug(EditView* view, Plug& plug, WSHIfObj::List& params);

	// メンバ変数
private:
	bool m_bUseCache;

};

