#pragma once

#include "RecentImp.h"
#include "util/StaticType.h"

typedef StaticString<TCHAR, _MAX_PATH> MetaPath;

// �t�H���_�̗������Ǘ� (RECENT_FOR_FOLDER)
class RecentExceptMRU : public RecentImp<MetaPath, LPCTSTR> {
public:
	// ����
	RecentExceptMRU();

	// �I�[�o�[���C�h
	int				CompareItem(const MetaPath* p1, LPCTSTR p2) const;
	void			CopyItem(MetaPath* dst, LPCTSTR src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(LPCTSTR* dst, const MetaPath* src) const;
	bool			TextToDataType(MetaPath* dst, LPCTSTR pszText) const;
};

