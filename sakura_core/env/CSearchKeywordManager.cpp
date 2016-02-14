/*
	2008.05.18 kobake CShareData ���番��
*/
/*
	Copyright (C) 2008, kobake

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
#include "DLLSHAREDATA.h"

#include "CSearchKeywordManager.h"
#include "recent/CRecent.h"


/*!	m_aSearchKeys��pszSearchKey��ǉ�����B
	YAZAKI
*/
void SearchKeywordManager::AddToSearchKeyArr(const wchar_t* pszSearchKey)
{
	RecentSearch recentSearchKey;
	recentSearchKey.AppendItem(pszSearchKey);
	recentSearchKey.Terminate();
	GetDllShareData().m_common.m_search.m_nSearchKeySequence++;
}

/*!	m_aReplaceKeys��pszReplaceKey��ǉ�����
	YAZAKI
*/
void SearchKeywordManager::AddToReplaceKeyArr(const wchar_t* pszReplaceKey)
{
	RecentReplace recentReplaceKey;
	recentReplaceKey.AppendItem(pszReplaceKey);
	recentReplaceKey.Terminate();
	GetDllShareData().m_common.m_search.m_nReplaceKeySequence++;

	return;
}

/*!	m_aGrepFiles��pszGrepFile��ǉ�����
	YAZAKI
*/
void SearchKeywordManager::AddToGrepFileArr(const TCHAR* pszGrepFile)
{
	RecentGrepFile recentGrepFile;
	recentGrepFile.AppendItem(pszGrepFile);
	recentGrepFile.Terminate();
}

/*!	m_aGrepFolders.size()��pszGrepFolder��ǉ�����
	YAZAKI
*/
void SearchKeywordManager::AddToGrepFolderArr(const TCHAR* pszGrepFolder)
{
	RecentGrepFolder recentGrepFolder;
	recentGrepFolder.AppendItem(pszGrepFolder);
	recentGrepFolder.Terminate();
}

