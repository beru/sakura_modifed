#pragma once

// �v��s��`
// #include "DllSharedData.h"

// ���L���������\����
struct Share_SearchKeywords {
	// -- -- �����L�[ -- -- //
	StaticVector< StaticString<wchar_t, _MAX_PATH>, MAX_SEARCHKEY,  const wchar_t*>	searchKeys;
	StaticVector< StaticString<wchar_t, _MAX_PATH>, MAX_REPLACEKEY, const wchar_t*>	replaceKeys;
	StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_GREPFILE,   const TCHAR*>	grepFiles;
	StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_GREPFOLDER, const TCHAR*>	grepFolders;
};

// �����L�[���[�h�Ǘ�
class SearchKeywordManager {
public:
	SearchKeywordManager() {
		pShareData = &GetDllShareData();
	}
	//@@@ 2002.2.2 YAZAKI
	void AddToSearchKeys(const wchar_t* pszSearchKey);		// searchKeys �� pszSearchKey ��ǉ�����
	void AddToReplaceKeys(const wchar_t* pszReplaceKey);	// replaceKeys �� pszReplaceKey ��ǉ�����
	void AddToGrepFiles(const TCHAR* pszGrepFile);			// grepFiles �� pszGrepFile ��ǉ�����
	void AddToGrepFolders(const TCHAR* pszGrepFolder);		// grepFolders.size() �� pszGrepFolder ��ǉ�����
private:
	DllSharedData* pShareData;
};

