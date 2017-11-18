#pragma once

#include "RecentImp.h"
#include "util/StaticType.h"
#include "config/maxdata.h" // MAX_CMDLEN

typedef StaticString<TCHAR, MAX_CMDLEN> CmdString;

// コマンドの履歴を管理 (RECENT_FOR_CMD)
class RecentCmd : public RecentImp<CmdString, LPCTSTR> {
public:
	// 生成
	RecentCmd();

	// オーバーライド
	int				CompareItem(const CmdString* p1, LPCTSTR p2) const;
	void			CopyItem(CmdString* dst, LPCTSTR src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(LPCTSTR* dst, const CmdString* src) const;
	bool			TextToDataType(CmdString* dst, LPCTSTR pszText) const;
};

