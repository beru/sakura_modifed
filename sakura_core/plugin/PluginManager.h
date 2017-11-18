/*!	@file
	@brief プラグイン管理クラス
*/
#pragma once

#include "plugin/Plugin.h"
#include <list>
#include <string>

class PluginManager : public TSingleton<PluginManager> {
	friend class TSingleton<PluginManager>;
	PluginManager();

	// 型定義
private:
	typedef std::wstring wstring;
	typedef std::string string;

	// 操作
public:
	bool LoadAllPlugin(CommonSetting* common = nullptr);				// 全プラグインを読み込む
	void UnloadAllPlugin();				// 全プラグインを解放する
	bool SearchNewPlugin(CommonSetting& common, HWND hWndOwner);		// 新規プラグインを導入する
	int InstallPlugin(CommonSetting& common, const TCHAR* pszPluginName, HWND hWndOwner, wstring& errorMsg, bool bUpdate = false);	// プラグインの初期導入をする
	bool InstZipPlugin(CommonSetting& common, HWND hWndOwner, const tstring& sZipName, bool bInSearch=false);		// Zipプラグインを追加する
	Plugin* GetPlugin(int id);		// プラグインを取得する
	void UninstallPlugin(CommonSetting& common, int id);		// プラグインを削除する

private:
	Plugin* LoadPlugin(const TCHAR* pszPluginDir, const TCHAR* pszPluginName, const TCHAR* pszLangName);	// プラグインを読み込む
	bool RegisterPlugin(Plugin* plugin);		// プラグインをCJackManagerに登録する
	bool UnRegisterPlugin(Plugin* plugin);	// プラグインのCJackManagerの登録を解除する

	// 属性
public:
	// pluginsフォルダのパス
	const tstring GetBaseDir() { return sBaseDir; }
	const tstring GetExePluginDir() { return sExePluginDir; }
	bool SearchNewPluginDir(CommonSetting& common, HWND hWndOwner, const tstring& sSearchDir, bool& bCancel);		// 新規プラグインを追加する(下請け)
	bool SearchNewPluginZip(CommonSetting& common, HWND hWndOwner, const tstring& sSearchDir, bool& bCancel);		// 新規プラグインを追加する(下請け)Zip File
	bool InstZipPluginSub(CommonSetting& common, HWND hWndOwner, const tstring& sZipName, const tstring& sDispName, bool bInSearch, bool& bCancel);		// Zipプラグインを導入する(下請け)

	// メンバ変数
private:
	Plugin::List plugins;
	tstring sBaseDir;					// pluginsフォルダのパス
	tstring sExePluginDir;			// Exeフォルダ配下pluginsフォルダのパス

};

