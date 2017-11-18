#include "StdAfx.h"
#include "RecentImp.h"

#include "env/AppNodeManager.h" // EditNode
#include "EditInfo.h" // EditInfo

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	初期生成処理

	@note
	nCmpType = strcmp, stricmp のときに nCmpSize = 0 を指定すると、AppendItem 
	でのデータが文字列であると認識して strcpy をする。
	他の場合は memcpy で nItemSize 分をコピーする。
	
	pnViewCount = nullptr にすると、擬似的に nViewCount == nArrayCount になる。
*/
template <class T, class S>
bool RecentImp<T, S>::Create(
	DataType*	pszItemArray,	// アイテム配列へのポインタ
	size_t*		pnItemCount,	// アイテム個数へのポインタ
	bool*		pbItemFavorite,	// お気に入りへのポインタ(NULL許可)
	size_t		nArrayCount,	// 最大管理可能なアイテム数
	size_t*		pnViewCount		// 表示個数(NULL許可)
	)
{
	Terminate();

	// パラメータチェック
	if (!pszItemArray) return false;
	if (!pnItemCount) return false;
	if (nArrayCount == 0) return false;
	if (pnViewCount && (nArrayCount < *pnViewCount)) return false;

	// 各パラメータ格納
	this->puUserItemData		= pszItemArray;
	this->pnUserItemCount		= pnItemCount;
	this->pbUserItemFavorite	= pbItemFavorite;
	this->nArrayCount			= nArrayCount;
	this->pnUserViewCount		= pnViewCount;
	this->bCreate = true;

	// 個別に操作されていたときのための対応
	UpdateView();

	return true;
}

/*
	終了処理
*/
template <class T, class S>
void RecentImp<T, S>::Terminate()
{
	bCreate = false;

	puUserItemData     = nullptr;
	pnUserItemCount    = nullptr;
	pnUserViewCount    = nullptr;
	pbUserItemFavorite = nullptr;

	nArrayCount  = 0;
}


/*
	初期化済みか調べる。
*/
template <class T, class S>
bool RecentImp<T, S>::IsAvailable() const
{
	if (!bCreate) {
		return false;
	}

	// データ破壊時のリカバリをやってみたりする
	const_cast<RecentImp*>(this)->_Recovery(); 

	return true;
}

// リカバリ
template <class T, class S>
void RecentImp<T, S>::_Recovery()
{
	if (*pnUserItemCount < 0) *pnUserItemCount = 0;
	if (*pnUserItemCount > nArrayCount) *pnUserItemCount = nArrayCount;

	if (pnUserViewCount) {
		if (*pnUserViewCount < 0) *pnUserViewCount = 0;
		if (*pnUserViewCount > nArrayCount) *pnUserViewCount = nArrayCount;
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        お気に入り                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	お気に入り状態を設定する。

	true	設定
	false	解除
*/
template <class T, class S>
bool RecentImp<T, S>::SetFavorite(size_t nIndex, bool bFavorite)
{
	if (! IsAvailable()) return false;
	if (nIndex >= *pnUserItemCount) return false;
	if (!pbUserItemFavorite) return false;

	pbUserItemFavorite[nIndex] = bFavorite;

	return true;
}

/*
	すべてのお気に入り状態を解除する。
*/
template <class T, class S>
void RecentImp<T, S>::ResetAllFavorite()
{
	if (! IsAvailable()) {
		return;
	}

	for (size_t i=0; i<*pnUserItemCount; ++i) {
		SetFavorite(i, false);
	}
}

/*
	お気に入り状態かどうか調べる。

	true	お気に入り
	false	通常
*/
template <class T, class S>
bool RecentImp<T, S>::IsFavorite(size_t nIndex) const
{
	if (! IsAvailable()) return false;
	if (nIndex >= *pnUserItemCount) return false;
	if (!pbUserItemFavorite) return false;

	return pbUserItemFavorite[nIndex];
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       アイテム制御                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムを先頭に追加する。

	@note	すでに登録済みの場合は先頭に移動する。
	@note	いっぱいのときは最古のアイテムを削除する。
	@note	お気に入りは削除されない。
*/
template <class T, class S>
bool RecentImp<T, S>::AppendItem(ReceiveType pItemData)
{
	if (!IsAvailable()) return false;
	if (!pItemData) return false;

	// 登録済みか調べる。
	int	nIndex = FindItem(pItemData);
	if (nIndex >= 0) {
		CopyItem(GetItemPointer(nIndex), pItemData);

		// 先頭に持ってくる。
		MoveItem(nIndex, 0);
		goto reconfigure;
	}

	// いっぱいのときは最古の通常アイテムを削除する。
	if (nArrayCount <= *pnUserItemCount) {
		nIndex = GetOldestItem(*pnUserItemCount - 1, false);
		if (nIndex == -1) {
			return false;
		}

		DeleteItem(nIndex);
	}

	for (size_t i=*pnUserItemCount; i>0; --i) {
		CopyItem(i - 1, i);
	}

	CopyItem(GetItemPointer(0), pItemData);

	//(void)SetFavorite(0, true);
	// 内部処理しないとだめ。
	if (pbUserItemFavorite) {
		pbUserItemFavorite[0] = false;
	}

	*pnUserItemCount += 1;


reconfigure:
	// お気に入りを表示内に移動する。
	if (pnUserViewCount) {
		ChangeViewCount(*pnUserViewCount);
	}
	return true;
}


template <class T, class S>
bool RecentImp<T, S>::AppendItemText(LPCTSTR pText)
{
	DataType data;
	ReceiveType receiveData;
	if (!TextToDataType(&data, pText)) {
		return false;
	}
	if (!DataToReceiveType(&receiveData, &data)) {
		return false;
	}
	int findIndex = FindItem(receiveData);
	if (findIndex != -1) {
		return false;
	}
	return AppendItem(receiveData);
}

template <class T, class S>
bool RecentImp<T, S>::EditItemText(size_t nIndex, LPCTSTR pText)
{
	DataType data;
	ReceiveType receiveData;
	memcpy_raw(&data, GetItemPointer(nIndex), sizeof(data));
	if (!TextToDataType(&data, pText)) {
		return false;
	}
	if (!DataToReceiveType(&receiveData, &data)) {
		return false;
	}
	int findIndex = FindItem(receiveData);
	if (findIndex != -1 && nIndex != findIndex) {
		// 重複不可。ただし同じ場合は大文字小文字の変更かもしれないのでOK
		return false;
	}
	CopyItem(GetItemPointer(nIndex), receiveData);
	return true;
}


/*
	アイテムをゼロクリアする。
*/
template <class T, class S>
void RecentImp<T, S>::ZeroItem(size_t nIndex)
{
	if (! IsAvailable()) {
		return;
	}
	if (nIndex >= nArrayCount) {
		return;
	}

	memset_raw(GetItemPointer(nIndex), 0, sizeof(DataType));

	if (pbUserItemFavorite) {
		pbUserItemFavorite[nIndex] = false;
	}

	return;
}

/*
	アイテムを削除する。
*/
template <class T, class S>
bool RecentImp<T, S>::DeleteItem(size_t nIndex)
{
	if (!IsAvailable()) {
		return false;
	}
	if (nIndex >= *pnUserItemCount) {
		return false;
	}

	ZeroItem(nIndex);

	// 以降のアイテムを前に詰める。
	size_t i;
	for (i=nIndex; i<*pnUserItemCount-1; ++i) {
		CopyItem(i + 1, i);
	}
	ZeroItem(i);

	*pnUserItemCount -= 1;

	return true;
}

/*
	お気に入り以外のアイテムを削除する。
*/
template <class T, class S>
bool RecentImp<T, S>::DeleteItemsNoFavorite()
{
	if (! IsAvailable()) {
		return false;
	}

	bool bDeleted = false;
	for (int i=(int)(*pnUserItemCount)-1; 0<=i; --i) {
		if (!IsFavorite(i)) {
			if (DeleteItem(i)) {
				bDeleted = true;
			}
		}
	}

	return bDeleted;
}

/*
	すべてのアイテムを削除する。

	@note	ゼロクリアを可能とするため、すべてが対象になる。
*/
template <class T, class S>
void RecentImp<T, S>::DeleteAllItem()
{
	if (!IsAvailable()) {
		return;
	}
	
	for (size_t i=0; i<nArrayCount; ++i) {
		ZeroItem(i);
	}
	
	*pnUserItemCount = 0;
	
	return;
}

/*
	アイテムを移動する。
*/
template <class T, class S>
bool RecentImp<T, S>::MoveItem(size_t nSrcIndex, size_t nDstIndex)
{
	bool	bFavorite;

	if (! IsAvailable()) return false;
	if (nSrcIndex >= *pnUserItemCount) return false;
	if (nDstIndex >= *pnUserItemCount) return false;

	if (nSrcIndex == nDstIndex) return true;

	DataType pri;

	// 移動する情報を退避
	memcpy_raw(&pri, GetItemPointer(nSrcIndex), sizeof(pri));
	bFavorite = IsFavorite(nSrcIndex);

	if (nSrcIndex < nDstIndex) {
		for (size_t i=nSrcIndex; i<nDstIndex; ++i) {
			CopyItem(i + 1, i);
		}
	}else {
		for (size_t i=nSrcIndex; i>nDstIndex; --i) {
			CopyItem(i - 1, i);
		}
	}

	// 新しい位置に格納
	memcpy_raw(GetItemPointer(nDstIndex), &pri, sizeof(pri));
	SetFavorite(nDstIndex, bFavorite);
	return true;
}

template <class T, class S>
bool RecentImp<T, S>::CopyItem(size_t nSrcIndex, size_t nDstIndex)
{
	if (!IsAvailable()) return false;
	if (nSrcIndex >= nArrayCount) return false;
	if (nDstIndex >= nArrayCount) return false;

	if (nSrcIndex == nDstIndex) return true;

	memcpy_raw(GetItemPointer(nDstIndex), GetItemPointer(nSrcIndex), sizeof(DataType));

	//(void)SetFavorite(nDstIndex, IsFavorite(nSrcIndex));
	// 内部処理しないとだめ。
	if (pbUserItemFavorite) pbUserItemFavorite[nDstIndex] = pbUserItemFavorite[nSrcIndex];

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       アイテム取得                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

template <class T, class S>
const T* RecentImp<T, S>::GetItem(size_t nIndex) const
{
	if (!IsAvailable() || nIndex >= *pnUserItemCount) return nullptr;
	return &puUserItemData[nIndex];
}

template <class T, class S>
const T* RecentImp<T, S>::GetItemPointer(size_t nIndex) const
{
	if (!IsAvailable() || nIndex >= nArrayCount) return nullptr;
	return &puUserItemData[nIndex];
}

/*
	アイテムを検索する。
*/
template <class T, class S>
int RecentImp<T, S>::FindItem(ReceiveType pItemData) const
{
	if (!IsAvailable()) return -1;
	if (!pItemData) return -1;

	for (size_t i=0; i<*pnUserItemCount; ++i) {
		if (CompareItem(GetItemPointer(i), pItemData) == 0) {
			return (int)i;
		}
	}

	return -1;
}

/*
	アイテムリストからもっとも古い｛お気に入り・通常｝のアイテムを探す。

	bFavorite=true	お気に入りの中から探す
	bFavorite=false	通常の中から探す
*/
template <class T, class S>
int RecentImp<T, S>::GetOldestItem(size_t nIndex, bool bFavorite)
{
	if (!IsAvailable()) return -1;
	if (nIndex >= *pnUserItemCount) nIndex = *pnUserItemCount - 1;

	for (int i=(int)nIndex; i>=0; --i) {
		if (IsFavorite(i) == bFavorite) return i;
	}

	return -1;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          その他                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	管理されているアイテムのうちの表示個数を変更する。

	@note	お気に入りは可能な限り表示内に移動させる。
*/
template <class T, class S>
bool RecentImp<T, S>::ChangeViewCount(size_t nViewCount)
{
	// 範囲外ならエラー
	if (!IsAvailable()) return false;
	if (nViewCount > nArrayCount) return false;

	// 表示個数を更新する。
	if (pnUserViewCount) {
		*pnUserViewCount = nViewCount;
	}

	// 範囲内にすべて収まっているので何もしなくてよい。
	if (nViewCount >= *pnUserItemCount) return true;

	// 最も古いお気に入りを探す。
	int	i = GetOldestItem(*pnUserItemCount - 1, true);
	if (i == -1) return true;	// ないので何もしないで終了

	// 表示外アイテムを表示内に移動する。
	for (; i>=(int)nViewCount; --i) {
		if (IsFavorite(i)) {
			// カレント位置から上に通常アイテムを探す
			int	nIndex = GetOldestItem(i - 1, false);
			if (nIndex == -1) break;	// もう1個もない

			// 見つかったアイテムをカレント位置に移動する
			MoveItem(nIndex, i);
		}
	}

	return true;
}

/*
	リストを更新する。
*/
template <class T, class S>
bool RecentImp<T, S>::UpdateView()
{
	// 範囲外ならエラー
	if (!IsAvailable()) return false;

	size_t nViewCount = pnUserViewCount ? *pnUserViewCount : nArrayCount;
	return ChangeViewCount(nViewCount);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      インスタンス化                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
template class RecentImp<CmdString, LPCTSTR>;
template class RecentImp<EditNode>;
template class RecentImp<EditInfo>;
template class RecentImp<PathString, LPCTSTR>;
#ifndef __MINGW32__
template class RecentImp<MetaPath, LPCTSTR>;
template class RecentImp<GrepFileString, LPCTSTR>;
template class RecentImp<GrepFolderString, LPCTSTR>;
template class RecentImp<SearchString, LPCWSTR>;
template class RecentImp<TagJumpKeywordString, LPCWSTR>;
template class RecentImp<CurDirString, LPCTSTR>;
#endif
#if !defined(__MINGW32__) || (defined(__MINGW32__) && !defined(UNICODE))
template class RecentImp<ReplaceString, LPCWSTR>;
#endif

