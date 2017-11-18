#pragma once

#include "RecentImp.h"
#include "util/StaticType.h"
#include "config/maxdata.h" // MAX_CMDLEN

typedef StaticString<TCHAR, MAX_CMDLEN> CmdString;

// �R�}���h�̗������Ǘ� (RECENT_FOR_CMD)
class RecentCmd : public RecentImp<CmdString, LPCTSTR> {
public:
	// ����
	RecentCmd();

	// �I�[�o�[���C�h
	int				CompareItem(const CmdString* p1, LPCTSTR p2) const;
	void			CopyItem(CmdString* dst, LPCTSTR src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(LPCTSTR* dst, const CmdString* src) const;
	bool			TextToDataType(CmdString* dst, LPCTSTR pszText) const;
};

