#pragma once

#include "RecentImp.h"
#include "util/StaticType.h"

typedef StaticString<TCHAR, _MAX_PATH> CurDirString;

// コマンドの履歴を管理 (RECENT_FOR_CUR_DIR)
class RecentCurDir : public RecentImp<CurDirString, LPCTSTR> {
public:
	// 生成
	RecentCurDir();

	// オーバーライド
	int				CompareItem(const CurDirString* p1, LPCTSTR p2) const;
	void			CopyItem(CurDirString* dst, LPCTSTR src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(LPCTSTR* dst, const CurDirString* src) const;
	bool			TextToDataType(CurDirString* dst, LPCTSTR pszText) const;
};

