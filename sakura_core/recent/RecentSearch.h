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
#pragma once

#include "RecentImp.h"
#include "util/StaticType.h"

typedef StaticString<WCHAR, _MAX_PATH> SearchString;

// 検索の履歴を管理 (RECENT_FOR_SEARCH)
class RecentSearch : public RecentImp<SearchString, LPCWSTR> {
public:
	// 生成
	RecentSearch();

	// オーバーライド
	int				CompareItem(const SearchString* p1, LPCWSTR p2) const;
	void			CopyItem(SearchString* dst, LPCWSTR src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(LPCWSTR* dst, const SearchString* src) const;
	bool			TextToDataType(SearchString* dst, LPCTSTR pszText) const;
};

