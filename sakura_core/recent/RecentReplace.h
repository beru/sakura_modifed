#pragma once

#include "RecentImp.h"
#include "util/StaticType.h"

typedef StaticString<wchar_t, _MAX_PATH> ReplaceString;

// �u���̗������Ǘ� (RECENT_FOR_REPLACE)
class RecentReplace : public RecentImp<ReplaceString, LPCWSTR> {
public:
	// ����
	RecentReplace();

	// �I�[�o�[���C�h
	int				CompareItem(const ReplaceString* p1, LPCWSTR p2) const;
	void			CopyItem(ReplaceString* dst, LPCWSTR src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(LPCWSTR* dst, const ReplaceString* src) const;
	bool			TextToDataType(ReplaceString* dst, LPCTSTR pszText) const;
};

