#pragma once

#include "RecentImp.h"
#include "util/StaticType.h"

//StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_GREPFOLDER, const TCHAR*>

typedef StaticString<TCHAR, _MAX_PATH> PathString;

// �t�H���_�̗������Ǘ� (RECENT_FOR_FOLDER)
class RecentFolder : public RecentImp<PathString, LPCTSTR> {
public:
	// ����
	RecentFolder();

	// �I�[�o�[���C�h
	int				CompareItem(const PathString* p1, LPCTSTR p2) const;
	void			CopyItem(PathString* dst, LPCTSTR src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(LPCTSTR* dst, const PathString* src) const;
	bool			TextToDataType(PathString* dst, LPCTSTR pszText) const;
};

