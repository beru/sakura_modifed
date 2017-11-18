#pragma once

#include "RecentImp.h"
struct EditNode;

// EditNode(ウィンドウリスト)の履歴を管理 (RECENT_FOR_EDITNODE)
class RecentEditNode : public RecentImp<EditNode> {
public:
	// 生成
	RecentEditNode();

	// オーバーライド
	int				CompareItem(const EditNode* p1, const EditNode* p2) const;
	void			CopyItem(EditNode* dst, const EditNode* src) const;
	const TCHAR*	GetItemText(size_t nIndex) const;
	bool			DataToReceiveType(const EditNode** dst, const EditNode* src) const;
	bool			TextToDataType(EditNode* dst, LPCTSTR pszText) const;
	// 固有インターフェース
	int FindItemByHwnd(HWND hwnd) const;
	void DeleteItemByHwnd(HWND hwnd);
};

