#pragma once

#include "RecentImp.h"
#include "util/StaticType.h"

typedef StaticString<TCHAR, _MAX_PATH> MetaPath;

// フォルダの履歴を管理 (RECENT_FOR_FOLDER)
class RecentExceptMRU : public RecentImp<MetaPath, LPCTSTR> {
public:
	// 生成
	RecentExceptMRU();

	// オーバーライド
	int				CompareItem(const MetaPath* p1, LPCTSTR p2) const;
	void			CopyItem(MetaPath* dst, LPCTSTR src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(LPCTSTR* dst, const MetaPath* src) const;
	bool			TextToDataType(MetaPath* dst, LPCTSTR pszText) const;
};

