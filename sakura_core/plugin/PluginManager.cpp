/*!	@file
	@brief プラグイン管理クラス

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
#include "plugin/PluginManager.h"
#include "plugin/JackManager.h"
#include "plugin/WSHPlugin.h"
#include "plugin/DllPlugin.h"
#include "util/module.h"
#include "io/ZipFile.h"

// コンストラクタ
PluginManager::PluginManager()
{
	// pluginsフォルダの場所を取得
	TCHAR szPluginPath[_MAX_PATH];
	GetInidir(szPluginPath, _T("plugins\\"));	// iniと同じ階層のpluginsフォルダを検索
	sBaseDir.append(szPluginPath);

	// Exeフォルダ配下pluginsフォルダのパスを取得
	TCHAR	szPath[_MAX_PATH];
	TCHAR	szFolder[_MAX_PATH];
	TCHAR	szFname[_MAX_PATH];

	::GetModuleFileName(NULL, szPath, _countof(szPath));
	SplitPath_FolderAndFile(szPath, szFolder, szFname);
	Concat_FolderAndFile(szFolder, _T("plugins\\"), szPluginPath);

	sExePluginDir.append(szPluginPath);
}

// 全プラグインを解放する
void PluginManager::UnloadAllPlugin()
{
	for (auto it=plugins.begin(); it!=plugins.end(); ++it) {
		UnRegisterPlugin(*it);
	}

	for (auto it=plugins.begin(); it!=plugins.end(); ++it) {
		delete *it;
	}
	
	// 2010.08.04 Moca plugins.claerする
	plugins.clear();
}

// 新規プラグインを追加する
bool PluginManager::SearchNewPlugin(
	CommonSetting& common,
	HWND hWndOwner
	)
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter SearchNewPlugin\n"));
#endif

	HANDLE hFind;
	ZipFile	zipFile;

	// プラグインフォルダの配下を検索
	WIN32_FIND_DATA wf;
	hFind = FindFirstFile((sBaseDir + _T("*")).c_str(), &wf);
	if (hFind == INVALID_HANDLE_VALUE) {
		// プラグインフォルダが存在しない
		if (!CreateDirectory(sBaseDir.c_str(), NULL)) {
			InfoMessage(hWndOwner, _T("%ts"), LS(STR_PLGMGR_FOLDER));
			return true;
		}
	}
	::FindClose(hFind);

	bool	bCancel = false;
	// プラグインフォルダの配下を検索
	bool bFindNewDir = SearchNewPluginDir(common, hWndOwner, sBaseDir, bCancel);
	if (!bCancel && sBaseDir != sExePluginDir) {
		bFindNewDir |= SearchNewPluginDir(common, hWndOwner, sExePluginDir, bCancel);
	}
	if (!bCancel && zipFile.IsOk()) {
		bFindNewDir |= SearchNewPluginZip(common, hWndOwner, sBaseDir, bCancel);
		if (!bCancel && sBaseDir != sExePluginDir) {
			bFindNewDir |= SearchNewPluginZip(common, hWndOwner, sExePluginDir, bCancel);
		}
	}

	if (bCancel) {
		InfoMessage(hWndOwner, _T("%ts"), LS(STR_PLGMGR_CANCEL));
	}else if (!bFindNewDir) {
		InfoMessage(hWndOwner, _T("%ts"), LS(STR_PLGMGR_NEWPLUGIN));
	}

	return true;
}


// 新規プラグインを追加する(下請け)
bool PluginManager::SearchNewPluginDir(
	CommonSetting& common,
	HWND hWndOwner,
	const tstring& sSearchDir,
	bool& bCancel
	)
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter SearchNewPluginDir\n"));
#endif

	PluginRec* pluginTable = common.plugin.pluginTable;
	HANDLE hFind;

	WIN32_FIND_DATA wf;
	hFind = FindFirstFile((sSearchDir + _T("*")).c_str(), &wf );
	if (hFind == INVALID_HANDLE_VALUE) {
		// プラグインフォルダが存在しない
		return false;
	}
	bool bFindNewDir = false;
	do {
		if ((wf.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY &&
			(wf.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == 0 &&
			_tcscmp(wf.cFileName, _T(".")) != 0 &&
			_tcscmp(wf.cFileName, _T("..")) != 0 &&
			auto_stricmp(wf.cFileName, _T("unuse")) != 0
		) {
			// インストール済みチェック。フォルダ名＝プラグインテーブルの名前ならインストールしない
			// 2010.08.04 大文字小文字同一視にする
			bool isNotInstalled = true;
			for (int iNo=0; iNo<MAX_PLUGIN; ++iNo) {
				if (auto_stricmp(wf.cFileName, to_tchar(pluginTable[iNo].szName)) == 0) {
					isNotInstalled = false;
					break;
				}
			}
			if (!isNotInstalled) { continue; }

			// 2011.08.20 syat plugin.defが存在しないフォルダは飛ばす
			if (!IsFileExists((sSearchDir + wf.cFileName + _T("\\") + PII_FILENAME).c_str(), true)) {
				continue;
			}

			bFindNewDir = true;
			int nRes = Select3Message(hWndOwner, LS(STR_PLGMGR_INSTALL), wf.cFileName);
			if (nRes == IDYES) {
				std::wstring errMsg;
				int pluginNo = InstallPlugin(common, wf.cFileName, hWndOwner, errMsg);
				if (pluginNo < 0) {
					WarningMessage(hWndOwner, LS(STR_PLGMGR_INSTALL_ERR),
						wf.cFileName, errMsg.c_str()
					);
				}
			}else if (nRes == IDCANCEL) {
				bCancel = true;
				break;	// for loop
			}
		}
	}while (FindNextFile(hFind, &wf));

	FindClose(hFind);
	return bFindNewDir;
}


// 新規プラグインを追加する(下請け)Zip File
bool PluginManager::SearchNewPluginZip(
	CommonSetting& common,
	HWND hWndOwner,
	const tstring& sSearchDir,
	bool& bCancel
	)
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter SearchNewPluginZip\n"));
#endif

	HANDLE hFind;

	WIN32_FIND_DATA wf;
	bool	bNewPlugin = false;
	bool	bFound;
	ZipFile	zipFile;

	hFind = INVALID_HANDLE_VALUE;

	// Zip File 検索解凍
	if (zipFile.IsOk()) {
		hFind = FindFirstFile((sSearchDir + _T("*.zip")).c_str(), &wf);

		for (bFound = (hFind != INVALID_HANDLE_VALUE); bFound; bFound = (FindNextFile(hFind, &wf) != 0)) {
			if ((wf.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_HIDDEN)) == 0) {
				bNewPlugin |= InstZipPluginSub(common, hWndOwner, sSearchDir + wf.cFileName, wf.cFileName, true, bCancel);
				if (bCancel) {
					break;
				}
			}
		}
	}

	FindClose(hFind);
	return bNewPlugin;
}


// Zipプラグインを導入する
bool PluginManager::InstZipPlugin(
	CommonSetting& common,
	HWND hWndOwner,
	const tstring& sZipFile,
	bool bInSearch
	)
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Entry InstZipPlugin\n"));
#endif

	ZipFile	zipFile;
	TCHAR	msg[512];

	// ZIPファイルが扱えるか
	if (!zipFile.IsOk()) {
		auto_snprintf_s(msg, _countof(msg), LS(STR_PLGMGR_ERR_ZIP));
		InfoMessage(hWndOwner, _T("%ts"), msg);
		return false;
	}

	// プラグインフォルダの存在を確認
	WIN32_FIND_DATA wf;
	HANDLE hFind;
	if ((hFind = ::FindFirstFile((sBaseDir + _T("*")).c_str(), &wf )) == INVALID_HANDLE_VALUE) {
		// プラグインフォルダが存在しない
		if (sBaseDir == sExePluginDir) {
			InfoMessage(hWndOwner, LS(STR_PLGMGR_ERR_FOLDER));
			::FindClose(hFind);
			return false;
		}else {
			if (!CreateDirectory(sBaseDir.c_str(), NULL)) {
				WarningMessage(hWndOwner, LS(STR_PLGMGR_ERR_CREATEDIR));
				::FindClose(hFind);
				return false;
			}
		}
	}
	::FindClose(hFind);

	bool bCancel;
	return PluginManager::InstZipPluginSub(common, hWndOwner, sZipFile, sZipFile, false, bCancel);
}

// Zipプラグインを導入する(下請け)
bool PluginManager::InstZipPluginSub(
	CommonSetting& common,
	HWND hWndOwner,
	const tstring& sZipFile,
	const tstring& sDispName,
	bool bInSearch,
	bool& bCancel
	)
{
	PluginRec*		pluginTable = common.plugin.pluginTable;
	ZipFile			zipFile;
	std::tstring	sFolderName;
	TCHAR			msg[512];
	std::wstring	errMsg;
	bool			bOk = true;
	bool			bSkip = false;
	bool			bNewPlugin = false;

	// Plugin フォルダ名の取得,定義ファイルの確認
	if (bOk && !zipFile.SetZip(sZipFile)) {
		auto_snprintf_s(msg, _countof(msg), LS(STR_PLGMGR_INST_ZIP_ACCESS), sDispName.c_str());
		bOk = false;
		bSkip = bInSearch;
	}

	// Plgin フォルダ名の取得,定義ファイルの確認
	if (bOk && !zipFile.ChkPluginDef(PII_FILENAME, sFolderName)) {
		auto_snprintf_s(msg, _countof(msg), LS(STR_PLGMGR_INST_ZIP_DEF), sDispName.c_str());
		bOk = false;
		bSkip = bInSearch;
	}

	if (!bInSearch) {
		// 単独インストール
		// インストール済みチェック。
		bool	isNotInstalled = true;
		int		iNo;
		if (bOk) {
			for (iNo=0; iNo<MAX_PLUGIN; ++iNo) {
				if (auto_stricmp(to_wchar(sFolderName.c_str()), to_wchar(pluginTable[iNo].szName)) == 0) {
					isNotInstalled = false;
					break;
				}
			}
			if (isNotInstalled) {
				bNewPlugin = true;
			}else {
				if (ConfirmMessage(
						hWndOwner,
						LS(STR_PLGMGR_INST_ZIP_ALREADY),
						sDispName.c_str()
					) != IDYES
				) {
					// Yesで無いなら終了
					return false;
				}
			}
		}
	}else {
		// pluginsフォルダ検索中
		// フォルダ チェック。すでに解凍されていたならインストールしない(前段でインストール済み或は可否を確認済み)
		if (bOk && fexist(to_tchar((sBaseDir + to_tchar(sFolderName.c_str())).c_str()))
			|| fexist(to_tchar((sExePluginDir + to_tchar(sFolderName.c_str())).c_str()))
		) {
			bOk = false;
			bSkip = true;
		}
		if (bOk) {
			bNewPlugin= true;
			int nRes = Select3Message(
				hWndOwner, LS(STR_PLGMGR_INST_ZIP_INST),
				sDispName.c_str(), sFolderName.c_str()
			);
			switch (nRes) {
			case IDCANCEL:
				bCancel = true;
				// through
			case IDNO:
				bOk = false;
				bSkip = true;
				break;
			}
		}
	}

	// Zip解凍
	if (bOk && !zipFile.Unzip(sBaseDir)) {
		auto_snprintf_s(msg, _countof(msg), LS(STR_PLGMGR_INST_ZIP_UNZIP), sDispName.c_str());
		bOk = false;
	}
	if (bOk) {
		int pluginNo = InstallPlugin(common, to_tchar(sFolderName.c_str()), hWndOwner, errMsg, true);
		if (pluginNo < 0) {
			auto_snprintf_s(msg, _countof(msg), LS(STR_PLGMGR_INST_ZIP_ERR), sDispName.c_str(), errMsg.c_str());
			bOk = false;
		}
	}

	if (!bOk && !bSkip) {
		// エラーメッセージ出力
		WarningMessage(hWndOwner, _T("%s"), msg);
	}

	return bNewPlugin;
}

// プラグインの初期導入をする
//	common			共有設定変数
//	pszPluginName	プラグイン名
//	hWndOwner		
//	errorMsg		エラーメッセージを返す
//	bUodate			すでに登録していた場合、確認せず上書きする
int PluginManager::InstallPlugin(
	CommonSetting& common,
	const TCHAR* pszPluginName,
	HWND hWndOwner,
	std::wstring& errorMsg,
	bool bUpdate
	)
{
	DataProfile profDef;				// プラグイン定義ファイル

	// プラグイン定義ファイルを読み込む
	profDef.SetReadingMode();
	if (!profDef.ReadProfile((sBaseDir + pszPluginName + _T("\\") + PII_FILENAME).c_str())
		&& !profDef.ReadProfile((sExePluginDir + pszPluginName + _T("\\") + PII_FILENAME).c_str()) 
	) {
		errorMsg = LSW(STR_PLGMGR_INST_DEF);
		return -1;
	}

	std::wstring sId;
	profDef.IOProfileData(PII_PLUGIN, PII_PLUGIN_ID, sId);
	if (sId.length() == 0) {
		errorMsg = LSW(STR_PLGMGR_INST_ID);
		return -1;
	}
	// 2010.08.04 ID使用不可の文字を確認
	//  後々ファイル名やiniで使うことを考えていくつか拒否する
	static const WCHAR szReservedChars[] = L"/\\,[]*?<>&|;:=\" \t";
	for (size_t x=0; x<_countof(szReservedChars); ++x) {
		if (sId.npos != sId.find(szReservedChars[x])) {
			errorMsg = std::wstring(LSW(STR_PLGMGR_INST_RESERVE1)) + szReservedChars + LSW(STR_PLGMGR_INST_RESERVE2);
			return -1;
		}
	}
	if (WCODE::Is09(sId[0])) {
		errorMsg = LSW(STR_PLGMGR_INST_IDNUM);
		return -1;
	}

	// ID重複・テーブル空きチェック
	PluginRec* pluginTable = common.plugin.pluginTable;
	int nEmpty = -1;
	bool isDuplicate = false;
	for (int iNo=0; iNo<MAX_PLUGIN; ++iNo) {
		if (nEmpty == -1 && pluginTable[iNo].state == PLS_NONE) {
			nEmpty = iNo;
			// break してはいけない。後ろで同一IDがあるかも
		}
		if (wcscmp(sId.c_str(), pluginTable[iNo].szId) == 0) {	// ID一致
			if (!bUpdate) {
				const TCHAR* msg = LS(STR_PLGMGR_INST_NAME);
				// 2010.08.04 削除中のIDは元の位置へ追加(復活させる)
				if (pluginTable[iNo].state != PLS_DELETED &&
					ConfirmMessage(hWndOwner, msg, static_cast<const TCHAR*>(pszPluginName), static_cast<const WCHAR*>(pluginTable[iNo].szName)) != IDYES
				) {
					errorMsg = LSW(STR_PLGMGR_INST_USERCANCEL);
					return -1;
				}
			}
			nEmpty = iNo;
			isDuplicate = pluginTable[iNo].state != PLS_DELETED;
			break;
		}
	}

	if (nEmpty == -1) {
		errorMsg = LSW(STR_PLGMGR_INST_MAX);
		return -1;
	}

	wcsncpy(pluginTable[nEmpty].szName, to_wchar(pszPluginName), MAX_PLUGIN_NAME);
	pluginTable[nEmpty].szName[MAX_PLUGIN_NAME-1] = '\0';
	wcsncpy(pluginTable[nEmpty].szId, sId.c_str(), MAX_PLUGIN_ID);
	pluginTable[nEmpty].szId[MAX_PLUGIN_ID-1] = '\0';
	pluginTable[nEmpty].state = isDuplicate ? PLS_UPDATED : PLS_INSTALLED;

	// コマンド数の設定	2010/7/11 Uchi
	int			i;
	WCHAR		szPlugKey[10];
	wstring		sPlugCmd;

	pluginTable[nEmpty].nCmdNum = 0;
	for (i=1; i<MAX_PLUG_CMD; ++i) {
		auto_sprintf_s(szPlugKey, L"C[%d]", i);
		sPlugCmd.clear();
		profDef.IOProfileData(PII_COMMAND, szPlugKey, sPlugCmd);
		if (sPlugCmd == L"") {
			break;
		}
		pluginTable[nEmpty].nCmdNum = i;
	}

	return nEmpty;
}

// 全プラグインを読み込む
bool PluginManager::LoadAllPlugin(CommonSetting* common)
{
#ifdef _UNICODE
	DEBUG_TRACE(_T("Enter LoadAllPlugin\n"));
#endif
	CommonSetting_Plugin& pluginSetting = (common ? common->plugin : GetDllShareData().common.plugin);

	if (!pluginSetting.bEnablePlugin) {
		return true;
	}

	std::tstring szLangName;
	{
		std::tstring szDllName = GetDllShareData().common.window.szLanguageDll;
		if (szDllName == _T("")) {
			szLangName = _T("ja_JP");
		}else {
			// "sakura_lang_*.dll"
			size_t nStartPos = 0;
			size_t nEndPos = szDllName.length();
			if (szDllName.substr(0, 12) == _T("sakura_lang_")) {
				nStartPos = 12;
			}
			if (4 < szDllName.length() && szDllName.substr(szDllName.length() - 4, 4) == _T(".dll")) {
				nEndPos = szDllName.length() - 4;
			}
			szLangName = szDllName.substr(nStartPos, nEndPos - nStartPos);
		}
		DEBUG_TRACE(_T("lang = %ts\n"), szLangName.c_str());
	}

	// プラグインテーブルに登録されたプラグインを読み込む
	PluginRec* pluginTable = pluginSetting.pluginTable;
	for (int iNo=0; iNo<MAX_PLUGIN; ++iNo) {
		if (pluginTable[iNo].szName[0] == '\0') {
			continue;
		}
		// 2010.08.04 削除状態を見る(今のところ保険)
		if (pluginTable[iNo].state == PLS_DELETED) {
			continue;
		}
		if (GetPlugin(iNo)) {
			continue; // 2013.05.31 読み込み済み
		}
		std::tstring name = to_tchar(pluginTable[iNo].szName);
		Plugin* plugin = LoadPlugin(sBaseDir.c_str(), name.c_str(), szLangName.c_str());
		if (!plugin) {
			plugin = LoadPlugin(sExePluginDir.c_str(), name.c_str(), szLangName.c_str());
		}
		if (plugin) {
			// 要検討：plugin.defのidとsakuraw.iniのidの不一致処理
			assert_warning(auto_strcmp(pluginTable[iNo].szId, plugin->sId.c_str()) == 0);
			plugin->id = iNo;		// プラグインテーブルの行番号をIDとする
			plugins.push_back(plugin);
			pluginTable[iNo].state = PLS_LOADED;
			// コマンド数設定
			pluginTable[iNo].nCmdNum = plugin->GetCommandCount();
			RegisterPlugin(plugin);
		}
	}
	
	return true;
}

// プラグインを読み込む
Plugin* PluginManager::LoadPlugin(
	const TCHAR* pszPluginDir,
	const TCHAR* pszPluginName,
	const TCHAR* pszLangName
	)
{
	TCHAR pszBasePath[_MAX_PATH];
	TCHAR pszPath[_MAX_PATH];
	std::tstring strMlang;
	DataProfile profDef;				// プラグイン定義ファイル
	DataProfile profDefMLang;			// プラグイン定義ファイル(L10N)
	DataProfile* pProfDefMLang = &profDefMLang; 
	DataProfile profOption;			// オプションファイル
	Plugin* plugin = nullptr;

#ifdef _UNICODE
	DEBUG_TRACE(_T("Load Plugin %ts\n"),  pszPluginName );
#endif
	// プラグイン定義ファイルを読み込む
	Concat_FolderAndFile(pszPluginDir, pszPluginName, pszBasePath);
	Concat_FolderAndFile(pszBasePath, PII_FILENAME, pszPath);
	profDef.SetReadingMode();
	if (!profDef.ReadProfile(pszPath)) {
		// プラグイン定義ファイルが存在しない
		return nullptr;
	}
#ifdef _UNICODE
	DEBUG_TRACE(_T("  定義ファイル読込 %ts\n"),  pszPath );
#endif

	// L10N定義ファイルを読む
	// プラグイン定義ファイルを読み込む base\pluginname\local\plugin_en_us.def
	strMlang = std::tstring(pszBasePath) + _T("\\") + PII_L10NDIR + _T("\\") + PII_L10NFILEBASE + pszLangName + PII_L10NFILEEXT;
	profDefMLang.SetReadingMode();
	if (!profDefMLang.ReadProfile(strMlang.c_str())) {
		// プラグイン定義ファイルが存在しない
		pProfDefMLang = nullptr;
#ifdef _UNICODE
		DEBUG_TRACE(_T("  L10N定義ファイル読込 %ts Not Found\n"),  strMlang.c_str() );
#endif
	}else {
#ifdef _UNICODE
		DEBUG_TRACE(_T("  L10N定義ファイル読込 %ts\n"),  strMlang.c_str() );
#endif
	}

	std::wstring sPlugType;
	profDef.IOProfileData(PII_PLUGIN, PII_PLUGIN_PLUGTYPE, sPlugType);

	if (wcsicmp(sPlugType.c_str(), L"wsh") == 0) {
		plugin = new WSHPlugin(tstring(pszBasePath));
	}else if (wcsicmp(sPlugType.c_str(), L"dll") == 0) {
		plugin = new DllPlugin(tstring(pszBasePath));
	}else {
		return nullptr;
	}
	plugin->sOptionDir = sBaseDir + pszPluginName;
	plugin->sLangName = pszLangName;
	plugin->ReadPluginDef(profDef, pProfDefMLang);
#ifdef _UNICODE
	DEBUG_TRACE(_T("  プラグインタイプ %ls\n"), sPlugType.c_str() );
#endif

	// オプションファイルを読み込む
	profOption.SetReadingMode();
	if (profOption.ReadProfile(plugin->GetOptionPath().c_str())) {
		// オプションファイルが存在する場合、読み込む
		plugin->ReadPluginOption(profOption);
	}
#ifdef _UNICODE
	DEBUG_TRACE(_T("  オプションファイル読込 %ts\n"),  plugin->GetOptionPath().c_str() );
#endif

	return plugin;
}

// プラグインをJackManagerに登録する
bool PluginManager::RegisterPlugin(Plugin* plugin)
{
	auto& jackMgr = JackManager::getInstance();
	Plug::Array plugs = plugin->GetPlugs();

	for (auto plug=plugs.begin(); plug!=plugs.end(); ++plug) {
		jackMgr.RegisterPlug((*plug)->sJack.c_str(), *plug);
	}

	return true;
}

// プラグインのJackManagerの登録を解除する
bool PluginManager::UnRegisterPlugin(Plugin* plugin)
{
	auto& jackMgr = JackManager::getInstance();
	Plug::Array plugs = plugin->GetPlugs();

	for (auto plug=plugs.begin(); plug!=plugs.end(); ++plug) {
		jackMgr.UnRegisterPlug((*plug)->sJack.c_str(), *plug);
	}

	return true;
}

// プラグインを取得する
Plugin* PluginManager::GetPlugin(int id)
{
	for (auto plugin=plugins.begin(); plugin!=plugins.end(); ++plugin) {
		if ((*plugin)->id == id) {
			return *plugin;
		}
	}
	return nullptr;
}

// プラグインを削除する
void PluginManager::UninstallPlugin(CommonSetting& common, int id)
{
	PluginRec* pluginTable = common.plugin.pluginTable;

	// 2010.08.04 ここではIDを保持する。後で再度追加するときに同じ位置に追加
	// PLS_DELETEDのszId/szNameはiniを保存すると削除されます
//	pluginTable[id].szId[0] = '\0';
	pluginTable[id].szName[0] = '\0';
	pluginTable[id].state = PLS_DELETED;
	pluginTable[id].nCmdNum = 0;
}

