// 各Recent実装クラスのベースクラス
// エディタ系ファイルからincludeするときは Recent.h をinclude
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
		size_t*		pnItemCount,	// アイテム個数へのポインタ
		bool*		pbItemFavorite,	// お気に入りへのポインタ(NULL許可)
		size_t		nArrayCount,	// 最大管理可能なアイテム数
		size_t*		pnViewCount		// 表示個数(NULL許可)
	);
public:
	void Terminate();
	bool IsAvailable() const;
	void _Recovery();

	// 更新
	bool ChangeViewCount(size_t nViewCount);	// 表示数の変更
	bool UpdateView();

	// プロパティ取得系
	size_t GetArrayCount() const { return nArrayCount; }	// 最大要素数
	size_t GetItemCount() const { return (IsAvailable() ? *pnUserItemCount : 0); }	// 登録アイテム数
	size_t GetViewCount() const { return (IsAvailable() ? (pnUserViewCount ? *pnUserViewCount : nArrayCount) : 0); }	// 表示数

	// お気に入り制御系
	bool SetFavorite(size_t nIndex, bool bFavorite = true);	// お気に入りに設定
	bool ResetFavorite(size_t nIndex) { return SetFavorite(nIndex, false); }	// お気に入りを解除
	void ResetAllFavorite();			// お気に入りをすべて解除
	bool IsFavorite(size_t nIndex) const;	// お気に入りか調べる

	// アイテム制御
	bool AppendItem(ReceiveType pItemData);	// アイテムを先頭に追加
	bool AppendItemText(LPCTSTR pszText);
	bool EditItemText(size_t nIndex, LPCTSTR pszText);
	bool DeleteItem(size_t nIndex);				// アイテムをクリア
	bool DeleteItem(ReceiveType pItemData) {
		return DeleteItem(FindItem(pItemData));
	}
	bool DeleteItemsNoFavorite();			// お気に入り以外のアイテムをクリア
	void DeleteAllItem();					// アイテムをすべてクリア

	// アイテム取得
	const DataType* GetItem(size_t nIndex) const;
	DataType* GetItem(size_t nIndex) { return const_cast<DataType*>(static_cast<const Me*>(this)->GetItem(nIndex)); }
	int FindItem(ReceiveType pItemData) const;
	bool MoveItem(size_t nSrcIndex, size_t nDstIndex);	// アイテムを移動

	// オーバーライド用インターフェース
	virtual int  CompareItem(const DataType* p1, ReceiveType p2) const = 0;
	virtual void CopyItem(DataType* dst, ReceiveType src) const = 0;
	virtual bool DataToReceiveType(ReceiveType* dst, const DataType* src) const = 0;
	virtual bool TextToDataType(DataType* dst, LPCTSTR pszText) const = 0;

	// 実装補助
private:
	const DataType* GetItemPointer(size_t nIndex) const;
	DataType* GetItemPointer(size_t nIndex) { return const_cast<DataType*>(static_cast<const Me*>(this)->GetItemPointer(nIndex)); }
	void   ZeroItem(size_t nIndex);	// アイテムをゼロクリアする
	int    GetOldestItem(size_t nIndex, bool bFavorite);	// 最古のアイテムを探す
	bool   CopyItem(size_t nSrcIndex, size_t nDstIndex);

protected:
	// 内部フラグ
	bool		bCreate;				// Create済みか

	// 外部参照
	DataType*	puUserItemData;			// アイテム配列へのポインタ
	size_t*		pnUserItemCount;		// アイテム個数へのポインタ
	bool*		pbUserItemFavorite;		// お気に入りへのポインタ (nullptr許可)
	size_t		nArrayCount;			// 最大管理可能なアイテム数
	size_t*		pnUserViewCount;		// 表示個数 (nullptr許可)
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

