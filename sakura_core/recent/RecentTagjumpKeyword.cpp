#include "StdAfx.h"
#include "RecentTagjumpKeyword.h"
#include "env/DllSharedData.h"
#include <string.h>


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentTagJumpKeyword::RecentTagJumpKeyword()
{
	Create(
		GetShareData()->tagJump.aTagJumpKeywords.dataPtr(),
		&GetShareData()->tagJump.aTagJumpKeywords._GetSizeRef(),
		nullptr,
		MAX_TAGJUMP_KEYWORD,
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
const TCHAR* RecentTagJumpKeyword::GetItemText(size_t nIndex) const
{
	return to_tchar(*GetItem(nIndex));
}

bool RecentTagJumpKeyword::DataToReceiveType(LPCWSTR* dst, const TagJumpKeywordString* src) const
{
	*dst = *src;
	return true;
}

bool RecentTagJumpKeyword::TextToDataType(TagJumpKeywordString* dst, LPCTSTR pszText) const
{
	CopyItem(dst, to_wchar(pszText));
	return true;
}

int RecentTagJumpKeyword::CompareItem(const TagJumpKeywordString* p1, LPCWSTR p2) const
{
	return wcscmp(*p1, p2);
}

void RecentTagJumpKeyword::CopyItem(TagJumpKeywordString* dst, LPCWSTR src) const
{
	wcscpy(*dst, src);
}

