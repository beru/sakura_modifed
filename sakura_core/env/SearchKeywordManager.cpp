#include "StdAfx.h"
#include "DllSharedData.h"

#include "SearchKeywordManager.h"
#include "recent/Recent.h"


/*!	aSearchKeys‚ÉpszSearchKey‚ð’Ç‰Á‚·‚é */
void SearchKeywordManager::AddToSearchKeys(const wchar_t* pszSearchKey)
{
	RecentSearch recentSearchKey;
	recentSearchKey.AppendItem(pszSearchKey);
	recentSearchKey.Terminate();
	GetDllShareData().common.search.nSearchKeySequence++;
}

/*!	aReplaceKeys‚ÉpszReplaceKey‚ð’Ç‰Á‚·‚é */
void SearchKeywordManager::AddToReplaceKeys(const wchar_t* pszReplaceKey)
{
	RecentReplace recentReplaceKey;
	recentReplaceKey.AppendItem(pszReplaceKey);
	recentReplaceKey.Terminate();
	GetDllShareData().common.search.nReplaceKeySequence++;

	return;
}

/*!	aGrepFiles‚ÉpszGrepFile‚ð’Ç‰Á‚·‚é */
void SearchKeywordManager::AddToGrepFiles(const TCHAR* pszGrepFile)
{
	RecentGrepFile recentGrepFile;
	recentGrepFile.AppendItem(pszGrepFile);
	recentGrepFile.Terminate();
}

/*!	grepFolders.size()‚ÉpszGrepFolder‚ð’Ç‰Á‚·‚é */
void SearchKeywordManager::AddToGrepFolders(const TCHAR* pszGrepFolder)
{
	RecentGrepFolder recentGrepFolder;
	recentGrepFolder.AppendItem(pszGrepFolder);
	recentGrepFolder.Terminate();
}

