#pragma once

// 要先行定義
// #include "DllSharedData.h"

// 共有メモリ内構造体
struct Share_SearchKeywords {
	// -- -- 検索キー -- -- //
	StaticVector< StaticString<wchar_t, _MAX_PATH>, MAX_SEARCHKEY,  const wchar_t*>	searchKeys;
	StaticVector< StaticString<wchar_t, _MAX_PATH>, MAX_REPLACEKEY, const wchar_t*>	replaceKeys;
	StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_GREPFILE,   const TCHAR*>	grepFiles;
	StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_GREPFOLDER, const TCHAR*>	grepFolders;
};

// 検索キーワード管理
class SearchKeywordManager {
public:
	SearchKeywordManager() {
		pShareData = &GetDllShareData();
	}
	//@@@ 2002.2.2 YAZAKI
	void AddToSearchKeys(const wchar_t* pszSearchKey);		// searchKeys に pszSearchKey を追加する
	void AddToReplaceKeys(const wchar_t* pszReplaceKey);	// replaceKeys に pszReplaceKey を追加する
	void AddToGrepFiles(const TCHAR* pszGrepFile);			// grepFiles に pszGrepFile を追加する
	void AddToGrepFolders(const TCHAR* pszGrepFolder);		// grepFolders.size() に pszGrepFolder を追加する
private:
	DllSharedData* pShareData;
};

