#include "StdAfx.h"
#include "RecentGrepFile.h"
#include <string.h>
#include "env/DllSharedData.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentGrepFile::RecentGrepFile()
{
	Create(
		GetShareData()->searchKeywords.grepFiles.dataPtr(),
		&GetShareData()->searchKeywords.grepFiles._GetSizeRef(),
		nullptr,
		MAX_GREPFILE,
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
const TCHAR* RecentGrepFile::GetItemText(size_t nIndex) const
{
	return *GetItem(nIndex);
}

bool RecentGrepFile::DataToReceiveType(LPCTSTR* dst, const GrepFileString* src) const
{
	*dst = *src;
	return true;
}

bool RecentGrepFile::TextToDataType(GrepFileString* dst, LPCTSTR pszText) const
{
	CopyItem(dst, pszText);
	return true;
}

int RecentGrepFile::CompareItem(const GrepFileString* p1, LPCTSTR p2) const
{
	return _tcsicmp(*p1, p2);
}

void RecentGrepFile::CopyItem(GrepFileString* dst, LPCTSTR src) const
{
	_tcscpy(*dst, src);
}

