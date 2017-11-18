#include "StdAfx.h"
#include "RecentReplace.h"
#include <string.h>
#include "env/DllSharedData.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentReplace::RecentReplace()
{
	Create(
		GetShareData()->searchKeywords.replaceKeys.dataPtr(),
		&GetShareData()->searchKeywords.replaceKeys._GetSizeRef(),
		nullptr,
		MAX_REPLACEKEY,
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
const TCHAR* RecentReplace::GetItemText(size_t nIndex) const
{
	return to_tchar(*GetItem(nIndex));
}

bool RecentReplace::DataToReceiveType(LPCWSTR* dst, const ReplaceString* src) const
{
	*dst = *src;
	return true;
}

bool RecentReplace::TextToDataType(ReplaceString* dst, LPCTSTR pszText) const
{
	CopyItem(dst, to_wchar(pszText));
	return true;
}


int RecentReplace::CompareItem(const ReplaceString* p1, LPCWSTR p2) const
{
	return wcscmp(*p1, p2);
}

void RecentReplace::CopyItem(ReplaceString* dst, LPCWSTR src) const
{
	wcscpy(*dst, src);
}

