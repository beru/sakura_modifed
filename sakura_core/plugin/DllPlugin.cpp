/*!	@file
	@brief DLLプラグインクラス
*/
#include "StdAfx.h"
#include "plugin/DllPlugin.h"
#include "view/EditView.h"

// デストラクタ
DllPlugin::~DllPlugin(void)
{
	for (auto it=plugs.begin(); it!=plugs.end(); ++it) {
		delete (DllPlug*)(*it);
	}
}

// プラグの生成
// Plugの代わりにDllPlugを作成する
Plug* DllPlugin::CreatePlug(
	Plugin& plugin,
	PlugId id,
	const wstring& sJack,
	const wstring& sHandler,
	const wstring& sLabel
	)
{
	DllPlug* newPlug = new DllPlug(plugin, id, sJack, sHandler, sLabel);
	return newPlug;
}

// プラグイン定義ファイルの読み込み
bool DllPlugin::ReadPluginDef(
	DataProfile& profile,
	DataProfile* profileMlang
	)
{
	ReadPluginDefCommon(profile, profileMlang);

	// DLL名の読み込み
	profile.IOProfileData(PII_DLL, PII_DLL_NAME, sDllName);

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

// プラグ実行
bool DllPlugin::InvokePlug(
	EditView& view,
	Plug& plug_raw,
	WSHIfObj::List& params
	)
{
	tstring dllPath = GetFilePath(to_tchar(sDllName.c_str()));
	InitDllResultType resInit = InitDll(to_tchar(dllPath.c_str()));
	if (resInit != InitDllResultType::Success) {
		::MYMESSAGEBOX(view.hwndParent, MB_OK, LS(STR_DLLPLG_TITLE), LS(STR_DLLPLG_INIT_ERR1), dllPath.c_str(), sName.c_str());
		return false;
	}

	DllPlug& plug = *(static_cast<DllPlug*>(&plug_raw));
	if (!plug.handler) {
		// DLL関数の取得
		ImportTable imp[2] = {
			{ &plug.handler, to_achar(plug.sHandler.c_str()) },
			{ NULL, 0 }
		};
		if (!RegisterEntries(imp)) {
//			DWORD err = GetLastError();
			::MYMESSAGEBOX(NULL, MB_OK, LS(STR_DLLPLG_TITLE), LS(STR_DLLPLG_INIT_ERR2));
			return false;
		}
	}
	MacroBeforeAfter ba;
	int flags = FA_NONRECORD | FA_FROMMACRO;
	ba.ExecKeyMacroBefore(view, flags);
	// DLL関数の呼び出し
	plug.handler();
	ba.ExecKeyMacroAfter(view, flags, true);
	
	return true;
}
