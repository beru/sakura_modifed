#include "StdAfx.h"
#include "DllSharedData.h"

#include "SearchKeywordManager.h"
#include "recent/Recent.h"


/*!	aSearchKeys��pszSearchKey��ǉ����� */
void SearchKeywordManager::AddToSearchKeys(const wchar_t* pszSearchKey)
{
	RecentSearch recentSearchKey;
	recentSearchKey.AppendItem(pszSearchKey);
	recentSearchKey.Terminate();
	GetDllShareData().common.search.nSearchKeySequence++;
}

/*!	aReplaceKeys��pszReplaceKey��ǉ����� */
void SearchKeywordManager::AddToReplaceKeys(const wchar_t* pszReplaceKey)
{
	RecentReplace recentReplaceKey;
	recentReplaceKey.AppendItem(pszReplaceKey);
	recentReplaceKey.Terminate();
	GetDllShareData().common.search.nReplaceKeySequence++;

	return;
}

/*!	aGrepFiles��pszGrepFile��ǉ����� */
void SearchKeywordManager::AddToGrepFiles(const TCHAR* pszGrepFile)
{
	RecentGrepFile recentGrepFile;
	recentGrepFile.AppendItem(pszGrepFile);
	recentGrepFile.Terminate();
}

/*!	grepFolders.size()��pszGrepFolder��ǉ����� */
void SearchKeywordManager::AddToGrepFolders(const TCHAR* pszGrepFolder)
{
	RecentGrepFolder recentGrepFolder;
	recentGrepFolder.AppendItem(pszGrepFolder);
	recentGrepFolder.Terminate();
}

