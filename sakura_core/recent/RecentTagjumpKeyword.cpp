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

