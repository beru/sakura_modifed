/*!	@file
	@brief DLLプラグインクラス
*/
#pragma once

#include "Plugin.h"

#define	PII_DLL							L"Dll"			// DLL情報
#define	PII_DLL_NAME					L"Name"			// 名前

typedef void (*DllPlugHandler)();

class DllPlug : public Plug {
public:
	DllPlug(Plugin& plugin, PlugId id, const wstring& sJack, const wstring& sHandler, const wstring& sLabel)
		:
		Plug(plugin, id, sJack, sHandler, sLabel),
		handler(NULL)
	{
	}
public:
	DllPlugHandler handler;
};

class DllPlugin :
	public Plugin,
	public DllImp
{
	// コンストラクタ
public:
	DllPlugin(const tstring& sBaseDir)
		:
		Plugin(sBaseDir),
		DllImp()
	{
	}

	// デストラクタ
public:
	~DllPlugin(void);

	// 実装
public:
	bool ReadPluginDef(DataProfile& profile, DataProfile* profileMlang);
	bool ReadPluginOption(DataProfile& profile) {
		return true;
	}
	Plug* CreatePlug(Plugin& plugin, PlugId id, const wstring& sJack, const wstring& sHandler, const wstring& sLabel);
	Plug::Array GetPlugs() const {
		return plugs;
	}
	bool InvokePlug(EditView& view, Plug& plug, WSHIfObj::List& params);

	bool InitDllImp() {
		return true;
	}
	LPCTSTR GetDllNameImp(int nIndex) {
		return _T("");
	}

	// メンバ変数
private:
	wstring sDllName;

};

