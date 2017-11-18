#include "StdAfx.h"
#include <string.h>
#include "RecentGrepFolder.h"
#include "env/DllSharedData.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentGrepFolder::RecentGrepFolder()
{
	Create(
		GetShareData()->searchKeywords.grepFolders.dataPtr(),
		&GetShareData()->searchKeywords.grepFolders._GetSizeRef(),
		nullptr,
		MAX_GREPFOLDER,
		nullptr
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      オーバーライド                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const TCHAR* RecentGrepFolder::GetItemText(size_t nIndex) const
{
	return *GetItem(nIndex);
}

bool RecentGrepFolder::DataToReceiveType(LPCTSTR* dst, const GrepFolderString* src) const
{
	*dst = *src;
	return true;
}

bool RecentGrepFolder::TextToDataType(GrepFolderString* dst, LPCTSTR pszText) const
{
	CopyItem(dst, pszText);
	return true;
}

int RecentGrepFolder::CompareItem(const GrepFolderString* p1, LPCTSTR p2) const
{
	return _tcsicmp(*p1, p2);
}

void RecentGrepFolder::CopyItem(GrepFolderString* dst, LPCTSTR src) const
{
	_tcscpy(*dst, src);
}

