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

typedef StaticString<TCHAR, _MAX_PATH> GrepFileString;

// GREPファイルの履歴を管理 (RECENT_FOR_GREP_FILE)
class RecentGrepFile : public RecentImp<GrepFileString, LPCTSTR> {
public:
	// 生成
	RecentGrepFile();

	// オーバーライド
	int				CompareItem(const GrepFileString* p1, LPCTSTR p2) const;
	void			CopyItem(GrepFileString* dst, LPCTSTR src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(LPCTSTR* dst, const GrepFileString* src) const;
	bool			TextToDataType(GrepFileString* dst, LPCTSTR pszText) const;
};

