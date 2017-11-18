#pragma once

#include "RecentImp.h"
#include "util/StaticType.h"

typedef StaticString<wchar_t, _MAX_PATH> SearchString;

// �����̗������Ǘ� (RECENT_FOR_SEARCH)
class RecentSearch : public RecentImp<SearchString, LPCWSTR> {
public:
	// ����
	RecentSearch();

	// �I�[�o�[���C�h
	int				CompareItem(const SearchString* p1, LPCWSTR p2) const;
	void			CopyItem(SearchString* dst, LPCWSTR src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(LPCWSTR* dst, const SearchString* src) const;
	bool			TextToDataType(SearchString* dst, LPCTSTR pszText) const;
};

