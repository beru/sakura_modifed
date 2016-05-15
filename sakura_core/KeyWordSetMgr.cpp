/*!	@file
	@brief 強調キーワード管理

	@author Norio Nakatani
	
	@date 2000.12.01 MIK binary search
	@date 2004.07.29-2005.01.27 Moca キーワードの可変長記憶
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, MIK
	Copyright (C) 2002, genta, Moca
	Copyright (C) 2004, Moca
	Copyright (C) 2005, Moca, genta

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
#include "StdAfx.h"
#include "KeywordSetMgr.h"
#include <limits>

// 1ブロック当たりのキーワード数
static const int nKeywordSetBlockSize = 50;

// ブロックサイズで整列
inline int GetAlignmentSize(int nSize)
{
	return (nKeywordSetBlockSize - 1 + nSize) / nKeywordSetBlockSize * nKeywordSetBlockSize;
}

/*!
	@note KeywordSetMgrは共有メモリ構造体に埋め込まれているため，
	そのままではコンストラクタが動かないことに注意．
*/
KeywordSetMgr::KeywordSetMgr(void)
{
	nCurrentKeywordSetIdx = 0;
	nKeywordSetNum = 0;
	nStartIdx[0] = 0;
	nStartIdx[1] = 0;
	nStartIdx[MAX_SETNUM] = 0;
	return;
}

KeywordSetMgr::~KeywordSetMgr(void)
{
	nKeywordSetNum = 0;
	return;
}

/*!
	@brief 全キーワードセットの削除と初期化

	キーワードセットのインデックスを全て0とする．
	
	@date 2004.07.29 Moca 可変長記憶
*/
void KeywordSetMgr::ResetAllKeywordSet(void)
{
	nKeywordSetNum = 0;
	for (int i=0; i<MAX_SETNUM+1; ++i) {
		nStartIdx[i] = 0;
	}
	for (int i=0; i<MAX_SETNUM; ++i) {
		nKeywordNumArr[i] = 0;
	}
}

const KeywordSetMgr& KeywordSetMgr::operator = (KeywordSetMgr& KeywordSetMgr)
{
//	int		nDataLen;
//	char*	pData;
//	int		i;
	if (this == &KeywordSetMgr) {
		return *this;
	}
	nCurrentKeywordSetIdx = KeywordSetMgr.nCurrentKeywordSetIdx;
	nKeywordSetNum = KeywordSetMgr.nKeywordSetNum;
	// 配列まるごとコピー
	memcpy_raw(szSetNameArr   , KeywordSetMgr.szSetNameArr   , sizeof(szSetNameArr)		);
	memcpy_raw(bKeywordCaseArr, KeywordSetMgr.bKeywordCaseArr, sizeof(bKeywordCaseArr)	);
	memcpy_raw(nStartIdx      , KeywordSetMgr.nStartIdx      , sizeof(nStartIdx	)		); // 2004.07.29 Moca
	memcpy_raw(nKeywordNumArr , KeywordSetMgr.nKeywordNumArr , sizeof(nKeywordNumArr)		);
	memcpy_raw(szKeywordArr   , KeywordSetMgr.szKeywordArr   , sizeof(szKeywordArr)		);
	memcpy_raw(isSorted       , KeywordSetMgr.isSorted       , sizeof(isSorted)			); // MIK 2000.12.01 binary search
	memcpy_raw(nKeywordMaxLenArr, KeywordSetMgr.nKeywordMaxLenArr, sizeof(nKeywordMaxLenArr) ); //2014.05.04 Moca
	return *this;
}


/*! @brief キーワードセットの追加

	@date 2005.01.26 Moca 新規作成
	@date 2005.01.29 genta サイズ0で作成→reallocするように
*/
bool KeywordSetMgr::AddKeywordSet(
	const wchar_t*	pszSetName,		// [in] セット名
	bool			bKeywordCase,	// [in] 大文字小文字の区別．true:あり, false:無し
	int				nSize			// [in] 最初に領域を確保するサイズ．
	)
{
	if (nSize < 0) {
		nSize = nKeywordSetBlockSize;
	}
	if (MAX_SETNUM <= nKeywordSetNum) {
		return false;
	}
	int nIdx = nKeywordSetNum;	// 追加位置
	nStartIdx[++nKeywordSetNum] = nStartIdx[nIdx];// サイズ0でセット追加

	if (!KeywordReAlloc(nIdx, nSize)) {
		--nKeywordSetNum;	//	キーワードセットの追加をキャンセルする
		return false;
	}
	wcsncpy( szSetNameArr[nIdx], pszSetName, _countof(szSetNameArr[nIdx]) - 1 );
	szSetNameArr[nIdx][_countof(szSetNameArr[nIdx]) - 1] = L'\0';
	bKeywordCaseArr[nIdx] = bKeywordCase;
	nKeywordNumArr[nIdx] = 0;
	isSorted[nIdx] = 0;	// MIK 2000.12.01 binary search
	return true;
}

// ｎ番目のセットを削除
bool KeywordSetMgr::DelKeywordSet(int nIdx)
{
	if (nKeywordSetNum <= nIdx ||
		0 > nIdx
	) {
		return false;
	}
	// キーワード領域を開放
	KeywordReAlloc(nIdx, 0);
	
	for (int i=nIdx; i<nKeywordSetNum-1; ++i) {
		// 配列まるごとコピー
		memcpy_raw(szSetNameArr[i], szSetNameArr[i + 1], sizeof(szSetNameArr[0]));
		bKeywordCaseArr[i] = bKeywordCaseArr[i + 1];
		nKeywordNumArr[i] = nKeywordNumArr[i + 1];
		nStartIdx[i] = nStartIdx[i + 1];	//	2004.07.29 Moca 可変長記憶
		isSorted[i] = isSorted[i + 1];	// MIK 2000.12.01 binary search
		nKeywordMaxLenArr[i] = nKeywordMaxLenArr[i+1];	// 2014.05.04 Moca
	}
	nStartIdx[nKeywordSetNum - 1] = nStartIdx[nKeywordSetNum];	// 2007.07.14 ryoji これが無いと末尾＝最終セットの先頭になってしまう
	--nKeywordSetNum;
	if (nKeywordSetNum <= nCurrentKeywordSetIdx) {
		nCurrentKeywordSetIdx = nKeywordSetNum - 1;
// セットが無くなったとき、nCurrentKeywordSetIdxをわざと-1にするため、コメント化
//		if (0 > nCurrentKeywordSetIdx) {
//			nCurrentKeywordSetIdx = 0;
//		}
	}
	return true;
}



/*! ｎ番目のセットのセット名を返す

	@param nIdx [in] セット番号 0～キーワードセット数-1
*/
const wchar_t* KeywordSetMgr::GetTypeName(int nIdx)
{
	if (nIdx < 0 || nKeywordSetNum <= nIdx) {
		return NULL;
	}
	return szSetNameArr[nIdx];
}

/*! ｎ番目のセットのセット名を再設定

	@date 2005.01.26 Moca 新規作成
*/
const wchar_t* KeywordSetMgr::SetTypeName(int nIdx, const wchar_t* name)
{
	if (!name || nIdx < 0 || nKeywordSetNum <= nIdx) {
		return NULL;
	}
	wcsncpy(szSetNameArr[nIdx], name, MAX_SETNAMELEN);
	szSetNameArr[nIdx][MAX_SETNAMELEN] = L'\0';
	return szSetNameArr[nIdx];
}

// ｎ番目のセットのキーワードの数を返す
int KeywordSetMgr::GetKeywordNum(int nIdx)
{
	if (nIdx < 0 || nKeywordSetNum <= nIdx) {
		return 0;
	}
	return nKeywordNumArr[nIdx];
}

/*! ｎ番目のセットのｍ番目のキーワードを返す

	@param nIdx [in] キーワードセット番号
	@param nIdx2 [in] キーワード番号
*/
const wchar_t* KeywordSetMgr::GetKeyword(int nIdx, int nIdx2)
{
	if (nIdx < 0 || nKeywordSetNum <= nIdx) {
		return NULL;
	}
	if (nIdx2 < 0 || nKeywordNumArr[nIdx] <= nIdx2) {
		return NULL;
	}
	return szKeywordArr[nStartIdx[nIdx] + nIdx2];
}

// ｎ番目のセットのｍ番目のキーワードを編集
const wchar_t* KeywordSetMgr::UpdateKeyword(
	int				nIdx,		// [in] キーワードセット番号
	int				nIdx2,		// [in] キーワード番号
	const WCHAR*	pszKeyword	// [in] 設定するキーワード
	)
{
	if (nIdx < 0 || nKeywordSetNum <= nIdx) {
		return NULL;
	}
	if (nIdx2 < 0 || nKeywordNumArr[nIdx] <= nIdx2) {
		return NULL;
	}
	// 0バイトの長さのキーワードは編集しない
	if (pszKeyword[0] == L'\0') {
		return NULL;
	}
	// 重複したキーワードは編集しない
	for (int i=nStartIdx[nIdx]; i<nStartIdx[nIdx]+nKeywordNumArr[nIdx]; ++i) {
		if (wcscmp(szKeywordArr[i], pszKeyword) == 0) {
			return NULL;
		}
	}
	isSorted[nIdx] = 0;	// MIK 2000.12.01 binary search
	wchar_t* p = szKeywordArr[nStartIdx[nIdx] + nIdx2];
	wcsncpy( p, pszKeyword, MAX_KEYWORDLEN );
	p[MAX_KEYWORDLEN] = L'\0';
	return p;
}


/*! ｎ番目のセットにキーワードを追加

	@param nIdx [in] セット番号
	@param pszKeyword [in] キーワード文字列
	
	@return 0: 成功, 1: セット番号エラー，2: メモリ確保エラー
		3: キーワード不正，4: キーワード重複

*/
int KeywordSetMgr::AddKeyword(int nIdx, const wchar_t* pszKeyword)
{
	if (nKeywordSetNum <= nIdx) {
		return 1;
	}
// 2004.07.29 Moca
	if (!KeywordReAlloc(nIdx, nKeywordNumArr[nIdx] + 1)) {
		return 2;
	}
//	if (MAX_KEYWORDNUM <= nKeywordNumArr[nIdx]) {
//		return FALSE;
//	}

	// 0バイトの長さのキーワードは登録しない
	if (pszKeyword[0] == L'\0') {
		return 3;
	}
	// 重複したキーワードは登録しない
	for (int i=nStartIdx[nIdx]; i<nStartIdx[nIdx]+nKeywordNumArr[nIdx]; ++i) {
		if (wcscmp(szKeywordArr[i], pszKeyword) == 0) {
			return 4;
		}
	}
	// MAX_KEYWORDLENより長いキーワードは切り捨てる
	if (MAX_KEYWORDLEN < wcslen(pszKeyword)) {
		wmemcpy(szKeywordArr[nStartIdx[nIdx] + nKeywordNumArr[nIdx]], pszKeyword, MAX_KEYWORDLEN);
		szKeywordArr[nStartIdx[nIdx] + nKeywordNumArr[nIdx]][MAX_KEYWORDLEN] = L'\0';
	}else {
		wcscpy(szKeywordArr[nStartIdx[nIdx] + nKeywordNumArr[nIdx]], pszKeyword);
	}
	nKeywordNumArr[nIdx]++;
	isSorted[nIdx] = 0;	// MIK 2000.12.01 binary search
	return 0;
}


/*! ｎ番目のセットのｍ番目のキーワードを削除

	@param nIdx [in] キーワードセット番号
	@param nIdx2 [in] キーワード番号
*/
int KeywordSetMgr::DelKeyword(int nIdx, int nIdx2)
{
	if (nIdx < 0 || nKeywordSetNum <= nIdx) {
		return 1;
	}
	if (nIdx2 < 0 ||  nKeywordNumArr[nIdx] <= nIdx2) {
		return 2;
	}
	if (0 >= nKeywordNumArr[nIdx]) {
		return 3;	//	登録数が0なら上の条件で引っかかるのでここには来ない？
	}
	int nDelKeywordLen = wcslen( szKeywordArr[nStartIdx[nIdx] + nIdx2] );
	int endPos = nStartIdx[nIdx] + nKeywordNumArr[nIdx] - 1;
	for (int i=nStartIdx[nIdx]+nIdx2; i<endPos; ++i) {
		wcscpy(szKeywordArr[i], szKeywordArr[i + 1]);
	}
	nKeywordNumArr[nIdx]--;

	// 2005.01.26 Moca 1つずらすだけなので、ソートの状態は保持される
	// isSorted[nIdx] = 0;	// MIK 2000.12.01 binary search
	KeywordReAlloc(nIdx, nKeywordNumArr[nIdx]);	// 2004.07.29 Moca

	// 2014.05.04 Moca キーワード長の再計算
	if (nDelKeywordLen == nKeywordMaxLenArr[nIdx]) {
		KeywordMaxLen(nIdx);
	}
	return 0;
}


// MIK START 2000.12.01 binary search
/*!	キーワードのソートとキーワード長の最大値計算

	@param nIdx [in] キーワードセット番号

*/
typedef int (__cdecl *qsort_callback)(const void *, const void *);
void KeywordSetMgr::SortKeyword(int nIdx)
{
	// nIdxのセットをソートする。
	if (bKeywordCaseArr[nIdx]) {
		qsort(
			szKeywordArr[nStartIdx[nIdx]],
			nKeywordNumArr[nIdx],
			sizeof(szKeywordArr[0]),
			(qsort_callback)wcscmp
		);
	}else {
		qsort(
			szKeywordArr[nStartIdx[nIdx]],
			nKeywordNumArr[nIdx],
			sizeof(szKeywordArr[0]),
			(qsort_callback)wcsicmp
		);
	}
	KeywordMaxLen(nIdx);
	isSorted[nIdx] = 1;
	return;
}

void KeywordSetMgr::KeywordMaxLen(int nIdx)
{
	int nMaxLen = 0;
	const int nEnd = nStartIdx[nIdx] + nKeywordNumArr[nIdx];
	for (int i=nStartIdx[nIdx]; i<nEnd; ++i) {
		int len = wcslen( szKeywordArr[i] );
		if (nMaxLen < len) {
			nMaxLen = len;
		}
	}
	nKeywordMaxLenArr[nIdx] = nMaxLen;
}


/** nIdx番目のキーワードセットから pszKeywordを探す。
	見つかれば 0以上を、見つからなければ負の数を返す。
	@retval 0以上 見つかった。
	@retval -1     見つからなかった。
	@retval -2     見つからなかったが、pszKeywordから始まるキーワードが存在している。
	@retval intmax 見つかったが、pszKeywordから始まる、より長いキーワードが存在している。
*/
int KeywordSetMgr::SearchKeyword2(
	int nIdx,
	const wchar_t* pszKeyword,
	int nKeywordLen
	)
{
	// sort
	if (isSorted[nIdx] == 0) {
		SortKeyword(nIdx);
	}

	if (nKeywordMaxLenArr[nIdx] < nKeywordLen) {
		return -1; // 字数オーバー。
	}

	int result = -1;
	int pl = nStartIdx[nIdx];
	int pr = nStartIdx[nIdx] + nKeywordNumArr[nIdx] - 1;
	int pc = (pr + 1 - pl) / 2 + pl;
	int (*const cmp)(const wchar_t*, const wchar_t*, size_t) = bKeywordCaseArr[nIdx] ? wcsncmp : wcsnicmp;
	while (pl <= pr) {
		const int ret = cmp(pszKeyword, szKeywordArr[pc], nKeywordLen);
		if (0 < ret) {
			pl = pc + 1;
		}else if (ret < 0) {
			pr = pc - 1;
		}else {
			if (wcslen(szKeywordArr[pc]) > static_cast<size_t>(nKeywordLen)) {
				// 始まりは一致したが長さが足りない。
				if (0 <= result) {
					result = std::numeric_limits<int>::max();
					break;
				}
				result = -2;
				// ぴったり一致するキーワードを探すために続ける。
				pr = pc - 1;
			}else {
				// 一致するキーワードが見つかった。
				if (result == -2) {
					result = std::numeric_limits<int>::max();
					break;
				}
				result = pc - nStartIdx[nIdx];
				// より長いキーワードを探すために続ける。
				pl = pc + 1;
			}
		}
		pc = (pr + 1 - pl) / 2 + pl;
	}
	return result;
}
// MIK END

// MIK START 2000.12.01 START
void KeywordSetMgr::SetKeywordCase(int nIdx, bool bCase)
{
	// 大文字小文字判断は１ビットあれば実現できる。
	// 今はint型(sizeof(int) * セット数 = 4 * 100 = 400)だが,
	// char型(sizeof(char) * セット数 = 1 * 100 = 100)で十分だし
	// ビット操作してもいい。
	bKeywordCaseArr[nIdx] = bCase;
	isSorted[nIdx] = 0;
	return;
}

bool KeywordSetMgr::GetKeywordCase(int nIdx)
{
	return bKeywordCaseArr[nIdx];
}
// MIK END

// From Here 2004.07.29 Moca 可変長記憶
/*!	@brief \\0またはTABで区切られた文字列からキーワードを設定

	@return 登録に成功したキーワード数
	
	@author Moca
	@date 2004.07.29 Moca CShareData::ShareData_IO_2内のコードを元に移築・作成
*/
int KeywordSetMgr::SetKeywordArr(
	int				nIdx,			// [in] キーワードセット番号
	int				nSize,			// [in] キーワード数
	const wchar_t*	pszKeywordArr	// [in]「key\\tword\\t\\0」又は「key\\0word\\0\\0」の形式
	)
{
	if (!KeywordReAlloc(nIdx, nSize)) {
		return 0;
	}
	int cnt, i;
	const wchar_t* ptr = pszKeywordArr;
	for (cnt = 0, i = nStartIdx[nIdx];
		i < nStartIdx[nIdx] + nSize && *ptr != L'\0';
		++cnt, ++i
	) {
		//	May 25, 2003 キーワードの区切りとして\0以外にTABを受け付けるようにする
		const wchar_t* pTop = ptr;	// キーワードの先頭位置を保存
		while (*ptr != L'\t' && *ptr != L'\0') {
			++ptr;
		}
		int kwlen = ptr - pTop;
		wmemcpy(szKeywordArr[i], pTop, kwlen);
		szKeywordArr[i][kwlen] = L'\0';
		++ptr;
	}
	nKeywordNumArr[nIdx] = cnt;
	return nSize;
}

/*!
	キーワードリストを設定

	@return 登録したキーワード数．0は失敗．
*/
int KeywordSetMgr::SetKeywordArr(
	int				nIdx,				// [in] キーワードセット番号
	int				nSize,				// [in] ppszKeywordArrの要素数
	const wchar_t*	ppszKeywordArr[]	// [in] キーワードの配列(重複・長さ制限等、考慮済みであること)
	)
{
	if (!KeywordReAlloc(nIdx, nSize)) {
		return 0;
	}
	for (int cnt = 0, i = nStartIdx[nIdx];
		i < nStartIdx[nIdx] + nSize;
		++cnt, ++i
	) {
		wcscpy_s(szKeywordArr[i], ppszKeywordArr[cnt]);
	}
	nKeywordNumArr[nIdx] = nSize;
	return nSize;
}

/*!	@brief キーワードの整理

	重複や使用不可のキーワードを取り除く

	@param nIdx [in] キーワードセット番号
	
	@return 削除したキーワード数
*/
int KeywordSetMgr::CleanKeywords(int nIdx)
{
	// 先にソートしておかないと、後で順番が変わると都合が悪い
	if (isSorted[nIdx] == 0) {
		SortKeyword(nIdx);
	}

	int nDelCount = 0;	// 削除キーワード数
	int i = 0;
	while (i < GetKeywordNum(nIdx) - 1) {
		const wchar_t* p = GetKeyword(nIdx, i);
		bool bDelKey = false;	// trueなら削除対象
		// 重複するキーワードか
		const wchar_t* r = GetKeyword(nIdx, i + 1);
		unsigned int nKeywordLen = wcslen(p);
		if (nKeywordLen == wcslen(r)) {
			if (bKeywordCaseArr[nIdx]) {
				if (auto_memcmp(p, r, nKeywordLen) == 0) {
					bDelKey = true;
				}
			}else {
				if (auto_memicmp(p, r, nKeywordLen) == 0) {
					bDelKey = true;
				}
			}
		}
		if (bDelKey) {
			DelKeyword(nIdx, i);
			++nDelCount;
			// 後ろがずれるので、iを増やさない
		}else {
			++i;
		}
	}
	return nDelCount;
}

/*!	@brief キーワード追加余地の問い合わせ

	@param nIdx [in] キーワードセット番号
	@return true: もう1つ追加可能, false: 追加不可能

	@date 2005.01.26 Moca 新規作成
	@date 2005.01.29 genta 割り当て済みの領域に空きがあれば拡張不能でも追加可能
*/
bool KeywordSetMgr::CanAddKeyword(int nIdx)
{
	//	割り当て済みの領域の空きをまず調べる
	int nSizeOld = GetAllocSize(nIdx);
	if (nKeywordNumArr[nIdx] < nSizeOld) {
		return true;
	}

	//	割り当て済み領域がいっぱいならば，割り当て可能領域の有無を確認
	//	一応割り当て最小単位分残っていることを確認．
	if (GetFreeSize() >= nKeywordSetBlockSize) {
		return true;
	}

	//	それでもだめか
	return false;
}

#if 0
/*!	新しいキーワードセットのキーワード領域を確保する
	nKeywordSetNumは、呼び出し側が、呼び出した後に + 1する
*/
bool KeywordSetMgr::KeywordAlloc(int nSize)
{
	// assert(nKeywordSetNum < MAX_SETNUM);
	// assert(0 <= nSize);

	// ブロックのサイズで整列
	int nAllocSize = GetAlignmentSize(nSize);

	if (GetFreeSize() < nAllocSize) {
		// メモリ不足
		return false;
	}
	nStartIdx[nKeywordSetNum + 1] = m_nStartIdx[m_nKeywordSetNum] + nAllocSize;
	for (int i=nKeywordSetNum+1; i<MAX_SETNUM; ++i) {
		nStartIdx[i + 1] = nStartIdx[i];
	}
	return true;
}
#endif

/*!	初期化済みのキーワードセットのキーワード領域の再割り当て、解放を行う

	@param nIdx [in] キーワードセット番号
	@param nSize [in] 必要なキーワード数 (0～)
*/
bool KeywordSetMgr::KeywordReAlloc(int nIdx, int nSize)
{
	// assert(0 <= nIdx && nIdx < nKeywordSetNum);

	// ブロックのサイズで整列
	int nAllocSize = GetAlignmentSize(nSize);
	int nSizeOld = GetAllocSize(nIdx);

	if (nSize < 0) {
		return false;
	}
	if (nAllocSize == nSizeOld) {
		// サイズ変更なし
		return true;
	}

	int nDiffSize = nAllocSize - nSizeOld;
	if (GetFreeSize() < nDiffSize) {
		// メモリ不足
		return false;
	}
	// 後ろのキーワードセットのキーワードをすべて移動する
	if (nIdx + 1 < nKeywordSetNum) {
		int nKeywordIdx = nStartIdx[nIdx + 1];
		int nKeywordNum = nStartIdx[nKeywordSetNum] - nStartIdx[nIdx + 1];
		memmove(
			szKeywordArr[nKeywordIdx + nDiffSize],
			szKeywordArr[nKeywordIdx],
			nKeywordNum * sizeof(szKeywordArr[0])
		);
	}
	for (int i=nIdx+1; i<=nKeywordSetNum; ++i) {
		nStartIdx[i] += nDiffSize;
	}
	return true;
}

/*!	@brief 割り当て済みキーワード数 

	@param nIdx [in] キーワードセット番号
	@return キーワードセットに割り当て済みのキーワード数
*/
int KeywordSetMgr::GetAllocSize(int nIdx) const
{
	return nStartIdx[nIdx + 1] - nStartIdx[nIdx];
}

/*! 共有空きスペース

	@date 2004.07.29 Moca 新規作成
	
	@return 共有空き領域(キーワード数)
*/
int KeywordSetMgr::GetFreeSize(void) const 
{
	return MAX_KEYWORDNUM - nStartIdx[nKeywordSetNum];
}

// To Here 2004.07.29 Moca

// キーワードセット名からセット番号を取得。見つからなければ -1
//	Uchi 2010/4/14
int  KeywordSetMgr::SearchKeywordSet(const wchar_t* pszKeyword)
{
	int nIdx = -1;
	for (int i=0; i<nKeywordSetNum; ++i) {
		if (wcscmp(szSetNameArr[i], pszKeyword) == 0) {
			nIdx = i;
			break;
		}
	}
	return nIdx;
}


