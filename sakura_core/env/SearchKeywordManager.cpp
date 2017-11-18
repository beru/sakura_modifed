#include "StdAfx.h"
#include "DllSharedData.h"

#include "SearchKeywordManager.h"
#include "recent/Recent.h"


/*!	aSearchKeysÇ…pszSearchKeyÇí«â¡Ç∑ÇÈÅB
	YAZAKI
*/
void SearchKeywordManager::AddToSearchKeys(const wchar_t* pszSearchKey)
{
	RecentSearch recentSearchKey;
	recentSearchKey.AppendItem(pszSearchKey);
	recentSearchKey.Terminate();
	GetDllShareData().common.search.nSearchKeySequence++;
}

/*!	aReplaceKeysÇ…pszReplaceKeyÇí«â¡Ç∑ÇÈ
	YAZAKI
*/
void SearchKeywordManager::AddToReplaceKeys(const wchar_t* pszReplaceKey)
{
	RecentReplace recentReplaceKey;
	recentReplaceKey.AppendItem(pszReplaceKey);
	recentReplaceKey.Terminate();
	GetDllShareData().common.search.nReplaceKeySequence++;

	return;
}

/*!	aGrepFilesÇ…pszGrepFileÇí«â¡Ç∑ÇÈ
	YAZAKI
*/
void SearchKeywordManager::AddToGrepFiles(const TCHAR* pszGrepFile)
{
	RecentGrepFile recentGrepFile;
	recentGrepFile.AppendItem(pszGrepFile);
	recentGrepFile.Terminate();
}

/*!	grepFolders.size()Ç…pszGrepFolderÇí«â¡Ç∑ÇÈ
	YAZAKI
*/
void SearchKeywordManager::AddToGrepFolders(const TCHAR* pszGrepFolder)
{
	RecentGrepFolder recentGrepFolder;
	recentGrepFolder.AppendItem(pszGrepFolder);
	recentGrepFolder.Terminate();
}

