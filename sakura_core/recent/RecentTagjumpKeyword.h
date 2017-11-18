#pragma once

#include "RecentImp.h"
#include "util/StaticType.h"

typedef StaticString<wchar_t, _MAX_PATH> TagJumpKeywordString;

// タグジャンプキーワードの履歴を管理 (RECENT_FOR_TAGJUMP_KEYWORD)
class RecentTagJumpKeyword : public RecentImp<TagJumpKeywordString, LPCWSTR> {
public:
	// 生成
	RecentTagJumpKeyword();

	// オーバーライド
	int				CompareItem(const TagJumpKeywordString* p1, LPCWSTR p2) const;
	void			CopyItem(TagJumpKeywordString* dst, LPCWSTR src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(LPCWSTR* dst, const TagJumpKeywordString* src) const;
	bool			TextToDataType(TagJumpKeywordString* dst, LPCTSTR pszText) const;
};

