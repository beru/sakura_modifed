#include "StdAfx.h"
#include "RecentSearch.h"
#include "config/maxdata.h"
#include "env/DllSharedData.h"
#include <string.h>


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentSearch::RecentSearch()
{
	Create(
		GetShareData()->searchKeywords.searchKeys.dataPtr(),
		&GetShareData()->searchKeywords.searchKeys._GetSizeRef(),
		nullptr,
		MAX_SEARCHKEY,
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
const TCHAR* RecentSearch::GetItemText(size_t nIndex) const
{
	return to_tchar(*GetItem(nIndex));
}

bool RecentSearch::DataToReceiveType(LPCWSTR* dst, const SearchString* src) const
{
	*dst = *src;
	return true;
}

bool RecentSearch::TextToDataType(SearchString* dst, LPCTSTR pszText) const
{
	CopyItem(dst, to_wchar(pszText));
	return true;
}

int RecentSearch::CompareItem(const SearchString* p1, LPCWSTR p2) const
{
	return wcscmp(*p1, p2);
}

void RecentSearch::CopyItem(SearchString* dst, LPCWSTR src) const
{
	wcscpy(*dst, src);
}

