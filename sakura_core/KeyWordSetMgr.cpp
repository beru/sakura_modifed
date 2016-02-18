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
	m_nCurrentKeywordSetIdx = 0;
	m_nKeywordSetNum = 0;
	m_nStartIdx[0] = 0;
	m_nStartIdx[1] = 0;
	m_nStartIdx[MAX_SETNUM] = 0;
	return;
}

KeywordSetMgr::~KeywordSetMgr(void)
{
	m_nKeywordSetNum = 0;
	return;
}

/*!
	@brief 全キーワードセットの削除と初期化

	キーワードセットのインデックスを全て0とする．
	
	@date 2004.07.29 Moca 可変長記憶
*/
void KeywordSetMgr::ResetAllKeywordSet(void)
{
	m_nKeywordSetNum = 0;
	for (int i=0; i<MAX_SETNUM+1; ++i) {
		m_nStartIdx[i] = 0;
	}
	for (int i=0; i<MAX_SETNUM; ++i) {
		m_nKeywordNumArr[i] = 0;
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
	m_nCurrentKeywordSetIdx = KeywordSetMgr.m_nCurrentKeywordSetIdx;
	m_nKeywordSetNum = KeywordSetMgr.m_nKeywordSetNum;
	// 配列まるごとコピー
	memcpy_raw(m_szSetNameArr   , KeywordSetMgr.m_szSetNameArr   , sizeof(m_szSetNameArr)		);
	memcpy_raw(m_bKeywordCaseArr, KeywordSetMgr.m_bKeywordCaseArr, sizeof(m_bKeywordCaseArr)	);
	memcpy_raw(m_nStartIdx      , KeywordSetMgr.m_nStartIdx      , sizeof(m_nStartIdx	)		); // 2004.07.29 Moca
	memcpy_raw(m_nKeywordNumArr , KeywordSetMgr.m_nKeywordNumArr , sizeof(m_nKeywordNumArr)		);
	memcpy_raw(m_szKeywordArr   , KeywordSetMgr.m_szKeywordArr   , sizeof(m_szKeywordArr)		);
	memcpy_raw(m_isSorted       , KeywordSetMgr.m_isSorted       , sizeof(m_isSorted)			); // MIK 2000.12.01 binary search
	memcpy_raw(m_nKeywordMaxLenArr, KeywordSetMgr.m_nKeywordMaxLenArr, sizeof(m_nKeywordMaxLenArr) ); //2014.05.04 Moca
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
	if (MAX_SETNUM <= m_nKeywordSetNum) {
		return false;
	}
	int nIdx = m_nKeywordSetNum;	// 追加位置
	m_nStartIdx[++m_nKeywordSetNum] = m_nStartIdx[nIdx];// サイズ0でセット追加

	if (!KeywordReAlloc(nIdx, nSize)) {
		--m_nKeywordSetNum;	//	キーワードセットの追加をキャンセルする
		return false;
	}
	wcsncpy( m_szSetNameArr[nIdx], pszSetName, _countof(m_szSetNameArr[nIdx]) - 1 );
	m_szSetNameArr[nIdx][_countof(m_szSetNameArr[nIdx]) - 1] = L'\0';
	m_bKeywordCaseArr[nIdx] = bKeywordCase;
	m_nKeywordNumArr[nIdx] = 0;
	m_isSorted[nIdx] = 0;	// MIK 2000.12.01 binary search
	return true;
}

// ｎ番目のセットを削除
bool KeywordSetMgr::DelKeywordSet(int nIdx)
{
	if (m_nKeywordSetNum <= nIdx ||
		0 > nIdx
	) {
		return false;
	}
	// キーワード領域を開放
	KeywordReAlloc(nIdx, 0);
	
	for (int i=nIdx; i<m_nKeywordSetNum-1; ++i) {
		// 配列まるごとコピー
		memcpy_raw(m_szSetNameArr[i], m_szSetNameArr[i + 1], sizeof(m_szSetNameArr[0]));
		m_bKeywordCaseArr[i] = m_bKeywordCaseArr[i + 1];
		m_nKeywordNumArr[i] = m_nKeywordNumArr[i + 1];
		m_nStartIdx[i] = m_nStartIdx[i + 1];	//	2004.07.29 Moca 可変長記憶
		m_isSorted[i] = m_isSorted[i + 1];	// MIK 2000.12.01 binary search
		m_nKeywordMaxLenArr[i] = m_nKeywordMaxLenArr[i+1];	// 2014.05.04 Moca
	}
	m_nStartIdx[m_nKeywordSetNum - 1] = m_nStartIdx[m_nKeywordSetNum];	// 2007.07.14 ryoji これが無いと末尾＝最終セットの先頭になってしまう
	--m_nKeywordSetNum;
	if (m_nKeywordSetNum <= m_nCurrentKeywordSetIdx) {
		m_nCurrentKeywordSetIdx = m_nKeywordSetNum - 1;
// セットが無くなったとき、m_nCurrentKeywordSetIdxをわざと-1にするため、コメント化
//		if (0 > m_nCurrentKeywordSetIdx) {
//			m_nCurrentKeywordSetIdx = 0;
//		}
	}
	return true;
}



/*! ｎ番目のセットのセット名を返す

	@param nIdx [in] セット番号 0～キーワードセット数-1
*/
const wchar_t* KeywordSetMgr::GetTypeName(int nIdx)
{
	if (nIdx < 0 || m_nKeywordSetNum <= nIdx) {
		return NULL;
	}
	return m_szSetNameArr[nIdx];
}

/*! ｎ番目のセットのセット名を再設定

	@date 2005.01.26 Moca 新規作成
*/
const wchar_t* KeywordSetMgr::SetTypeName(int nIdx, const wchar_t* name)
{
	if (!name || nIdx < 0 || m_nKeywordSetNum <= nIdx) {
		return NULL;
	}
	wcsncpy(m_szSetNameArr[nIdx], name, MAX_SETNAMELEN);
	m_szSetNameArr[nIdx][MAX_SETNAMELEN] = L'\0';
	return m_szSetNameArr[nIdx];
}

// ｎ番目のセットのキーワードの数を返す
int KeywordSetMgr::GetKeywordNum(int nIdx)
{
	if (nIdx < 0 || m_nKeywordSetNum <= nIdx) {
		return 0;
	}
	return m_nKeywordNumArr[nIdx];
}

/*! ｎ番目のセットのｍ番目のキーワードを返す

	@param nIdx [in] キーワードセット番号
	@param nIdx2 [in] キーワード番号
*/
const wchar_t* KeywordSetMgr::GetKeyword(int nIdx, int nIdx2)
{
	if (nIdx < 0 || m_nKeywordSetNum <= nIdx) {
		return NULL;
	}
	if (nIdx2 < 0 || m_nKeywordNumArr[nIdx] <= nIdx2) {
		return NULL;
	}
	return m_szKeywordArr[m_nStartIdx[nIdx] + nIdx2];
}

// ｎ番目のセットのｍ番目のキーワードを編集
const wchar_t* KeywordSetMgr::UpdateKeyword(
	int				nIdx,		// [in] キーワードセット番号
	int				nIdx2,		// [in] キーワード番号
	const WCHAR*	pszKeyword	// [in] 設定するキーワード
	)
{
	if (nIdx < 0 || m_nKeywordSetNum <= nIdx) {
		return NULL;
	}
	if (nIdx2 < 0 || m_nKeywordNumArr[nIdx] <= nIdx2) {
		return NULL;
	}
	// 0バイトの長さのキーワードは編集しない
	if (pszKeyword[0] == L'\0') {
		return NULL;
	}
	// 重複したキーワードは編集しない
	for (int i=m_nStartIdx[nIdx]; i<m_nStartIdx[nIdx]+m_nKeywordNumArr[nIdx]; ++i) {
		if (wcscmp(m_szKeywordArr[i], pszKeyword) == 0) {
			return NULL;
		}
	}
	m_isSorted[nIdx] = 0;	// MIK 2000.12.01 binary search
	wchar_t* p = m_szKeywordArr[m_nStartIdx[nIdx] + nIdx2];
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
	if (m_nKeywordSetNum <= nIdx) {
		return 1;
	}
// 2004.07.29 Moca
	if (!KeywordReAlloc(nIdx, m_nKeywordNumArr[nIdx] + 1)) {
		return 2;
	}
//	if (MAX_KEYWORDNUM <= m_nKeywordNumArr[nIdx]) {
//		return FALSE;
//	}

	// 0バイトの長さのキーワードは登録しない
	if (pszKeyword[0] == L'\0') {
		return 3;
	}
	// 重複したキーワードは登録しない
	for (int i=m_nStartIdx[nIdx]; i<m_nStartIdx[nIdx]+m_nKeywordNumArr[nIdx]; ++i) {
		if (wcscmp(m_szKeywordArr[i], pszKeyword) == 0) {
			return 4;
		}
	}
	// MAX_KEYWORDLENより長いキーワードは切り捨てる
	if (MAX_KEYWORDLEN < wcslen(pszKeyword)) {
		wmemcpy(m_szKeywordArr[m_nStartIdx[nIdx] + m_nKeywordNumArr[nIdx]], pszKeyword, MAX_KEYWORDLEN);
		m_szKeywordArr[m_nStartIdx[nIdx] + m_nKeywordNumArr[nIdx]][MAX_KEYWORDLEN] = L'\0';
	}else {
		wcscpy(m_szKeywordArr[m_nStartIdx[nIdx] + m_nKeywordNumArr[nIdx]], pszKeyword);
	}
	m_nKeywordNumArr[nIdx]++;
	m_isSorted[nIdx] = 0;	// MIK 2000.12.01 binary search
	return 0;
}


/*! ｎ番目のセットのｍ番目のキーワードを削除

	@param nIdx [in] キーワードセット番号
	@param nIdx2 [in] キーワード番号
*/
int KeywordSetMgr::DelKeyword(int nIdx, int nIdx2)
{
	if (nIdx < 0 || m_nKeywordSetNum <= nIdx) {
		return 1;
	}
	if (nIdx2 < 0 ||  m_nKeywordNumArr[nIdx] <= nIdx2) {
		return 2;
	}
	if (0 >= m_nKeywordNumArr[nIdx]) {
		return 3;	//	登録数が0なら上の条件で引っかかるのでここには来ない？
	}
	int nDelKeywordLen = wcslen( m_szKeywordArr[m_nStartIdx[nIdx] + nIdx2] );
	int endPos = m_nStartIdx[nIdx] + m_nKeywordNumArr[nIdx] - 1;
	for (int i=m_nStartIdx[nIdx]+nIdx2; i<endPos; ++i) {
		wcscpy(m_szKeywordArr[i], m_szKeywordArr[i + 1]);
	}
	m_nKeywordNumArr[nIdx]--;

	// 2005.01.26 Moca 1つずらすだけなので、ソートの状態は保持される
	// m_isSorted[nIdx] = 0;	// MIK 2000.12.01 binary search
	KeywordReAlloc(nIdx, m_nKeywordNumArr[nIdx]);	// 2004.07.29 Moca

	// 2014.05.04 Moca キーワード長の再計算
	if (nDelKeywordLen == m_nKeywordMaxLenArr[nIdx]) {
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
	if (m_bKeywordCaseArr[nIdx]) {
		qsort(
			m_szKeywordArr[m_nStartIdx[nIdx]],
			m_nKeywordNumArr[nIdx],
			sizeof(m_szKeywordArr[0]),
			(qsort_callback)wcscmp
		);
	}else {
		qsort(
			m_szKeywordArr[m_nStartIdx[nIdx]],
			m_nKeywordNumArr[nIdx],
			sizeof(m_szKeywordArr[0]),
			(qsort_callback)wcsicmp
		);
	}
	KeywordMaxLen(nIdx);
	m_isSorted[nIdx] = 1;
	return;
}

void KeywordSetMgr::KeywordMaxLen(int nIdx)
{
	int nMaxLen = 0;
	const int nEnd = m_nStartIdx[nIdx] + m_nKeywordNumArr[nIdx];
	for (int i=m_nStartIdx[nIdx]; i<nEnd; ++i) {
		int len = wcslen( m_szKeywordArr[i] );
		if (nMaxLen < len) {
			nMaxLen = len;
		}
	}
	m_nKeywordMaxLenArr[nIdx] = nMaxLen;
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
	if (m_isSorted[nIdx] == 0) {
		SortKeyword(nIdx);
	}

	if (m_nKeywordMaxLenArr[nIdx] < nKeywordLen) {
		return -1; // 字数オーバー。
	}

	int result = -1;
	int pl = m_nStartIdx[nIdx];
	int pr = m_nStartIdx[nIdx] + m_nKeywordNumArr[nIdx] - 1;
	int pc = (pr + 1 - pl) / 2 + pl;
	int (*const cmp)(const wchar_t*, const wchar_t*, size_t) = m_bKeywordCaseArr[nIdx] ? wcsncmp : wcsnicmp;
	while (pl <= pr) {
		const int ret = cmp(pszKeyword, m_szKeywordArr[pc], nKeywordLen);
		if (0 < ret) {
			pl = pc + 1;
		}else if (ret < 0) {
			pr = pc - 1;
		}else {
			if (wcslen(m_szKeywordArr[pc]) > static_cast<size_t>(nKeywordLen)) {
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
				result = pc - m_nStartIdx[nIdx];
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
	m_bKeywordCaseArr[nIdx] = bCase;
	m_isSorted[nIdx] = 0;
	return;
}

bool KeywordSetMgr::GetKeywordCase(int nIdx)
{
	return m_bKeywordCaseArr[nIdx];
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
	for (cnt = 0, i = m_nStartIdx[nIdx];
		i < m_nStartIdx[nIdx] + nSize && *ptr != L'\0';
		++cnt, ++i
	) {
		//	May 25, 2003 キーワードの区切りとして\0以外にTABを受け付けるようにする
		const wchar_t* pTop = ptr;	// キーワードの先頭位置を保存
		while (*ptr != L'\t' && *ptr != L'\0') {
			++ptr;
		}
		int kwlen = ptr - pTop;
		wmemcpy(m_szKeywordArr[i], pTop, kwlen);
		m_szKeywordArr[i][kwlen] = L'\0';
		++ptr;
	}
	m_nKeywordNumArr[nIdx] = cnt;
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
	for (int cnt = 0, i = m_nStartIdx[nIdx];
		i < m_nStartIdx[nIdx] + nSize;
		++cnt, ++i
	) {
		wcscpy_s(m_szKeywordArr[i], ppszKeywordArr[cnt]);
	}
	m_nKeywordNumArr[nIdx] = nSize;
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
	if (m_isSorted[nIdx] == 0) {
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
			if (m_bKeywordCaseArr[nIdx]) {
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
	if (m_nKeywordNumArr[nIdx] < nSizeOld) {
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
	m_nKeywordSetNumは、呼び出し側が、呼び出した後に + 1する
*/
bool KeywordSetMgr::KeywordAlloc(int nSize)
{
	// assert(m_nKeywordSetNum < MAX_SETNUM);
	// assert(0 <= nSize);

	// ブロックのサイズで整列
	int nAllocSize = GetAlignmentSize(nSize);

	if (GetFreeSize() < nAllocSize) {
		// メモリ不足
		return false;
	}
	m_nStartIdx[m_nKeywordSetNum + 1] = m_nStartIdx[m_nKeywordSetNum] + nAllocSize;
	for (int i=m_nKeywordSetNum+1; i<MAX_SETNUM; ++i) {
		m_nStartIdx[i + 1] = m_nStartIdx[i];
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
	// assert(0 <= nIdx && nIdx < m_nKeywordSetNum);

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
	if (nIdx + 1 < m_nKeywordSetNum) {
		int nKeywordIdx = m_nStartIdx[nIdx + 1];
		int nKeywordNum = m_nStartIdx[m_nKeywordSetNum] - m_nStartIdx[nIdx + 1];
		memmove(
			m_szKeywordArr[nKeywordIdx + nDiffSize],
			m_szKeywordArr[nKeywordIdx],
			nKeywordNum * sizeof(m_szKeywordArr[0])
		);
	}
	for (int i=nIdx+1; i<=m_nKeywordSetNum; ++i) {
		m_nStartIdx[i] += nDiffSize;
	}
	return true;
}

/*!	@brief 割り当て済みキーワード数 

	@param nIdx [in] キーワードセット番号
	@return キーワードセットに割り当て済みのキーワード数
*/
int KeywordSetMgr::GetAllocSize(int nIdx) const
{
	return m_nStartIdx[nIdx + 1] - m_nStartIdx[nIdx];
}

/*! 共有空きスペース

	@date 2004.07.29 Moca 新規作成
	
	@return 共有空き領域(キーワード数)
*/
int KeywordSetMgr::GetFreeSize(void) const 
{
	return MAX_KEYWORDNUM - m_nStartIdx[m_nKeywordSetNum];
}

// To Here 2004.07.29 Moca

// キーワードセット名からセット番号を取得。見つからなければ -1
//	Uchi 2010/4/14
int  KeywordSetMgr::SearchKeywordSet(const wchar_t* pszKeyword)
{
	int nIdx = -1;
	for (int i=0; i<m_nKeywordSetNum; ++i) {
		if (wcscmp(m_szSetNameArr[i], pszKeyword) == 0) {
			nIdx = i;
			break;
		}
	}
	return nIdx;
}


