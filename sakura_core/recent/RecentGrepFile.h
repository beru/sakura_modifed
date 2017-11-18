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

