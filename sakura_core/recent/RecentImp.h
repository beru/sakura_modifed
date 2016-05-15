// 各Recent実装クラスのベースクラス

// エディタ系ファイルからincludeするときは Recent.h をinclude
/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#pragma once

#include "recent/Recent.h"


template <class DATA_TYPE, class RECEIVE_TYPE = const DATA_TYPE*>
class RecentImp : public Recent {
private:
	typedef RecentImp<DATA_TYPE, RECEIVE_TYPE>	Me;
	typedef DATA_TYPE							DataType;
	typedef RECEIVE_TYPE						ReceiveType;

public:
	RecentImp() { Terminate(); }
	virtual ~RecentImp() { Terminate(); }

protected:
	// 生成
	bool Create(
		DataType*	pszItemArray,	// アイテム配列へのポインタ
		int*		pnItemCount,	// アイテム個数へのポインタ
		bool*		pbItemFavorite,	// お気に入りへのポインタ(NULL許可)
		int			nArrayCount,	// 最大管理可能なアイテム数
		int*		pnViewCount		// 表示個数(NULL許可)
	);
public:
	void Terminate();
	bool IsAvailable() const;
	void _Recovery();

	// 更新
	bool ChangeViewCount(int nViewCount);	// 表示数の変更
	bool UpdateView();

	// プロパティ取得系
	int GetArrayCount() const { return nArrayCount; }	// 最大要素数
	int GetItemCount() const { return (IsAvailable() ? *pnUserItemCount : 0); }	// 登録アイテム数
	int GetViewCount() const { return (IsAvailable() ? (pnUserViewCount ? *pnUserViewCount : nArrayCount) : 0); }	// 表示数

	// お気に入り制御系
	bool SetFavorite(int nIndex, bool bFavorite = true);	// お気に入りに設定
	bool ResetFavorite(int nIndex) { return SetFavorite(nIndex, false); }	// お気に入りを解除
	void ResetAllFavorite();				// お気に入りをすべて解除
	bool IsFavorite(int nIndex) const;	// お気に入りか調べる

	// アイテム制御
	bool AppendItem(ReceiveType pItemData);	// アイテムを先頭に追加
	bool AppendItemText(LPCTSTR pszText);
	bool EditItemText(int nIndex, LPCTSTR pszText);
	bool DeleteItem(int nIndex);				// アイテムをクリア
	bool DeleteItem(ReceiveType pItemData) {
		return DeleteItem(FindItem(pItemData));
	}
	bool DeleteItemsNoFavorite();			// お気に入り以外のアイテムをクリア
	void DeleteAllItem();					// アイテムをすべてクリア

	// アイテム取得
	const DataType* GetItem(int nIndex) const;
	DataType* GetItem(int nIndex) { return const_cast<DataType*>(static_cast<const Me*>(this)->GetItem(nIndex)); }
	int FindItem(ReceiveType pItemData) const;
	bool MoveItem(int nSrcIndex, int nDstIndex);	// アイテムを移動

	// オーバーライド用インターフェース
	virtual int  CompareItem(const DataType* p1, ReceiveType p2) const = 0;
	virtual void CopyItem(DataType* dst, ReceiveType src) const = 0;
	virtual bool DataToReceiveType(ReceiveType* dst, const DataType* src) const = 0;
	virtual bool TextToDataType(DataType* dst, LPCTSTR pszText) const = 0;

	// 実装補助
private:
	const DataType* GetItemPointer(int nIndex) const;
	DataType* GetItemPointer(int nIndex) { return const_cast<DataType*>(static_cast<const Me*>(this)->GetItemPointer(nIndex)); }
	void   ZeroItem(int nIndex);	// アイテムをゼロクリアする
	int    GetOldestItem(int nIndex, bool bFavorite);	// 最古のアイテムを探す
	bool   CopyItem(int nSrcIndex, int nDstIndex);

protected:
	// 内部フラグ
	bool		bCreate;				// Create済みか

	// 外部参照
	DataType*	puUserItemData;			// アイテム配列へのポインタ
	int*		pnUserItemCount;		// アイテム個数へのポインタ
	bool*		pbUserItemFavorite;		// お気に入りへのポインタ (nullptr許可)
	int			nArrayCount;			// 最大管理可能なアイテム数
	int*		pnUserViewCount;		// 表示個数 (nullptr許可)
};

#include "RecentFile.h"
#include "RecentFolder.h"
#include "RecentExceptMru.h"
#include "RecentSearch.h"
#include "RecentReplace.h"
#include "RecentGrepFile.h"
#include "RecentGrepFolder.h"
#include "RecentCmd.h"
#include "RecentCurDir.h"
#include "RecentEditNode.h"
#include "RecentTagjumpKeyword.h"

