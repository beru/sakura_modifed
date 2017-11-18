#pragma once

#include "RecentImp.h"
#include "EditInfo.h" //EditInfo

// EditInfoの履歴を管理 (RECENT_FOR_FILE)
class RecentFile : public RecentImp<EditInfo> {
public:
	// 生成
	RecentFile();

	// オーバーライド
	int				CompareItem(const EditInfo* p1, const EditInfo* p2) const;
	void			CopyItem(EditInfo* dst, const EditInfo* src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(const EditInfo** dst, const EditInfo* src) const;
	bool			TextToDataType(EditInfo* dst, LPCTSTR pszText) const;
	// 固有インターフェース
	int FindItemByPath(const TCHAR* pszPath) const;
};

