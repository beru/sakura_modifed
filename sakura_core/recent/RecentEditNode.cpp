#include "StdAfx.h"
#include "RecentEditNode.h"
#include <string.h>
#include "env/DllSharedData.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

RecentEditNode::RecentEditNode()
{
	Create(
		GetShareData()->nodes.pEditArr,
		&GetShareData()->nodes.nEditArrNum,
		nullptr,
		MAX_EDITWINDOWS,
		nullptr
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      オーバーライド                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const TCHAR* RecentEditNode::GetItemText(size_t nIndex) const
{
	return _T("WIN"); // ※テキスト情報は無い (GetWindowTextしてあげても良いけど、この関数は実行されないので、意味は無い)
}

bool RecentEditNode::DataToReceiveType(const EditNode** dst, const EditNode* src) const
{
	*dst = src;
	return true;
}

bool RecentEditNode::TextToDataType(EditNode* dst, LPCTSTR pszText) const
{
	return false;
}

int RecentEditNode::CompareItem(const EditNode* p1, const EditNode* p2) const
{
	return p1->hWnd - p2->hWnd;
}

void RecentEditNode::CopyItem(EditNode* dst, const EditNode* src) const
{
	*dst = *src;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   固有インターフェース                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int RecentEditNode::FindItemByHwnd(HWND hwnd) const
{
	size_t n = GetItemCount();
	for (size_t i=0; i<n; ++i) {
		if (GetItem(i)->hWnd == hwnd) {
			return (int)i;
		}
	}
	return -1;
}

void RecentEditNode::DeleteItemByHwnd(HWND hwnd)
{
	int n = FindItemByHwnd(hwnd);
	if (n != -1) {
		DeleteItem(n);
	}else {
		DEBUG_TRACE(_T("DeleteItemByHwnd失敗\n"));
	}
}

