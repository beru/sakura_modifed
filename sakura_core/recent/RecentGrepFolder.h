#pragma once

#include "RecentImp.h"
#include "util/StaticType.h"

typedef StaticString<TCHAR, _MAX_PATH> GrepFolderString;

// GREPフォルダの履歴を管理 (RECENT_FOR_GREP_FOLDER)
class RecentGrepFolder : public RecentImp<GrepFolderString, LPCTSTR> {
public:
	// 生成
	RecentGrepFolder();

	// オーバーライド
	int				CompareItem(const GrepFolderString* p1, LPCTSTR p2) const;
	void			CopyItem(GrepFolderString* dst, LPCTSTR src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(LPCTSTR* dst, const GrepFolderString* src) const;
	bool			TextToDataType(GrepFolderString* dst, LPCTSTR pszText) const;
};

