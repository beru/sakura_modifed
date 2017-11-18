#pragma once

#include "_main/global.h"
#include "env/DllSharedData.h"

// 最近使ったリスト

class Recent {
public:
	virtual ~Recent() {}

	// インスタンス管理
	virtual void Terminate() = 0;

	// アイテム
	virtual const TCHAR*	GetItemText(size_t nIndex) const = 0;
	virtual size_t			GetArrayCount() const = 0;
	virtual size_t			GetItemCount() const = 0;
	virtual void			DeleteAllItem() = 0;
	virtual bool			DeleteItemsNoFavorite() = 0;
	virtual bool			DeleteItem(size_t nIndex) = 0;	// アイテムをクリア
	virtual bool			AppendItemText(const TCHAR* pszText) = 0;
	virtual bool			EditItemText(size_t nIndex, const TCHAR* pszText) = 0;

	int FindItemByText(const TCHAR* pszText) const {
		size_t n = GetItemCount();
		for (size_t i=0; i<n; ++i) {
			if (_tcscmp(GetItemText(i), pszText) == 0) {
				return (int)i;
			}
		}
		return -1;
	}

	// お気に入り
	virtual bool	SetFavorite(size_t nIndex, bool bFavorite = true) = 0;		// お気に入りに設定
	virtual bool	IsFavorite(size_t nIndex) const = 0;						// お気に入りか調べる

	// その他
	virtual size_t	GetViewCount() const = 0;
	virtual bool	UpdateView() = 0;

	// 共有メモリアクセス
	DllSharedData*	GetShareData() {
		return &GetDllShareData();
	}
};

#include "RecentImp.h"

