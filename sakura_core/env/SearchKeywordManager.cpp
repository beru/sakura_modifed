#include "StdAfx.h"
#include "DllSharedData.h"

#include "SearchKeywordManager.h"
#include "recent/Recent.h"


/*!	aSearchKeysにpszSearchKeyを追加する */
void SearchKeywordManager::AddToSearchKeys(const wchar_t* pszSearchKey)
{
	RecentSearch recentSearchKey;
	recentSearchKey.AppendItem(pszSearchKey);
	recentSearchKey.Terminate();
	GetDllShareData().common.search.nSearchKeySequence++;
}

/*!	aReplaceKeysにpszReplaceKeyを追加する */
void SearchKeywordManager::AddToReplaceKeys(const wchar_t* pszReplaceKey)
{
	RecentReplace recentReplaceKey;
	recentReplaceKey.AppendItem(pszReplaceKey);
	recentReplaceKey.Terminate();
	GetDllShareData().common.search.nReplaceKeySequence++;

	return;
}

/*!	aGrepFilesにpszGrepFileを追加する */
void SearchKeywordManager::AddToGrepFiles(const TCHAR* pszGrepFile)
{
	RecentGrepFile recentGrepFile;
	recentGrepFile.AppendItem(pszGrepFile);
	recentGrepFile.Terminate();
}

/*!	grepFolders.size()にpszGrepFolderを追加する */
void SearchKeywordManager::AddToGrepFolders(const TCHAR* pszGrepFolder)
{
	RecentGrepFolder recentGrepFolder;
	recentGrepFolder.AppendItem(pszGrepFolder);
	recentGrepFolder.Terminate();
}

