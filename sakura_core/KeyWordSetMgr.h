/*!	@file
	@brief 強調キーワード管理

	@author Norio Nakatani

	@date 2000.12.01 MIK binary search
	@date 2005.01.26 Moca キーワード数可変化
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, MIK
	Copyright (C) 2001, jepro
	Copyright (C) 2004, Moca
	Copyright (C) 2005, Moca

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

#include <Windows.h>
#include "_main/global.h"// 2002/2/10 aroka

#define		MAX_SETNUM		100	// 2007.12.01 genta 最大値増加
#define		MAX_SETNAMELEN	32


// キーワード総数 (2005.01.27 1セットあたりの数→セット全体の総数へ意味変更)
#define		MAX_KEYWORDNUM	15000
#define		MAX_KEYWORDLEN	63

/*! @brief 強調キーワード管理

	@date 2005.01.27 Moca キーワード数を可変に．
	
	@par キーワード数可変について
	
	従来は各キーワードセット毎に固定サイズを割り当てていたが
	PHPキーワードなど多数のキーワードを登録できない一方で
	少数のキーワード割り当てでは無駄が多かった．
	
	キーワードを全体で1つの配列に入れ，開始位置を別途管理することで
	キーワード総数を全体で管理するように変更した．
	
	セットが複数ある場合に前のセットにキーワードを登録していく場合に
	保管場所が不足するとそれ以降を後ろにずらす必要がある．
	頻繁にずらす操作が発生しないよう，nKeywordSetBlockSize(50個)ずつの
	ブロック単位で場所を確保するようにしている．
*/
class KeywordSetMgr {
public:
	/*
	||  Constructors
	*/
	KeywordSetMgr();
	~KeywordSetMgr();
	
	///	@name キーワードセット操作
	bool AddKeywordSet(							// セットの追加
		const wchar_t*	pszSetName,				// [in] セット名
		bool			bKeywordCase,			// [in] 大文字小文字の区別．true:あり, false:無し
		int				nSize			= -1	// [in] 最初に領域を確保するサイズ．
	);
	bool DelKeywordSet(size_t);			// ｎ番目のセットを削除
	const wchar_t* GetTypeName(size_t);	// ｎ番目のセット名を返す
	const wchar_t* SetTypeName(size_t, const wchar_t*);	// ｎ番目のセット名を設定する // 2005.01.26 Moca
	void SetKeywordCase(size_t, bool);				// ｎ番目のセットの大文字小文字判断をセットする		//MIK
	bool GetKeywordCase(size_t);					// ｎ番目のセットの大文字小文字判断を取得する		//MIK
	void SortKeyword(size_t);						// ｎ番目のセットのキーワードをソートする			//MIK

	// From Here 2004.07.29 Moca 追加 可変長記憶
	size_t SetKeywordArr(size_t, size_t, const wchar_t*);			// iniからキーワードを設定する
	size_t SetKeywordArr(						// キーワードの配列から設定する
		size_t			nIdx,				// [in] キーワードセット番号
		size_t			nSize,				// [in] ppszKeywordArrの要素数
		const wchar_t*	ppszKeywordArr[]	// [in] キーワードの配列(重複・長さ制限等、考慮済みであること)
	);
	// To Here 2004.07.29 Moca
	//@}

	//@{
	///	@name キーワード操作
	size_t GetKeywordNum(size_t);				// ｎ番目のセットのキーワードの数を返す
	const wchar_t* GetKeyword(size_t, size_t);	// ｎ番目のセットのｍ番目のキーワードを返す
	const wchar_t* UpdateKeyword(size_t, size_t, const WCHAR*);	// ｎ番目のセットのｍ番目のキーワードを編集
	size_t AddKeyword(size_t, const wchar_t*);	// ｎ番目のセットにキーワードを追加
	size_t DelKeyword(size_t, size_t);			// ｎ番目のセットのｍ番目のキーワードを削除
	bool CanAddKeyword(int);	// キーワードが追加可能か
	//@}
	
	//@{
	///	@name 検索
	//int SearchKeyword(int , const char*, int);				// ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す
//	BOOL IsModify(KeywordSetMgr&, BOOL* pnModifyFlagArr);	// 変更状況を調査	// Uchi 2010/4/14 実体が無いので削除
	int SearchKeyword2(size_t nIdx , const wchar_t* pszKeyword, size_t nKeywordLen);	// ｎ番目のセットから指定キーワードをバイナリサーチ。見つかれば 0以上を返す	//MIK
	int SearchKeywordSet(const wchar_t* pszKeyword);		// キーワードセット名からセット番号を取得。見つからなければ -1を返す	// Uchi 2010/4/14
	//@}

	// From Here 2004.07.29 Moca 追加 可変長記憶
	size_t CleanKeywords(size_t);			// キーワードの整頓・利用できないキーワードの削除
	size_t GetAllocSize(size_t) const;		// 確保している数を返す
	size_t GetFreeSize() const;			// 未割り当てブロックのキーワード数を返す
	void ResetAllKeywordSet(void);		// 全キーワードセットの削除と初期化
	// To Here 2004.07.29 Moca

	/*
	|| 演算子
	*/
	const KeywordSetMgr& operator = (KeywordSetMgr&);
	/*
	||  Attributes & Operations
	*/
	/*!
		@brief 現在のキーワードセット番号(GUI用)

		本来の処理とは無関係だが，あるウィンドウで選択したセットが
		別のウィンドウの設定画面にも引き継がれるようにするため．
	*/
	size_t	nCurrentKeywordSetIdx;
	size_t	nKeywordSetNum;				// キーワードセット数
	wchar_t	szSetNameArr[MAX_SETNUM][MAX_SETNAMELEN + 1];	// キーワードセット名
	bool	bKeywordCaseArr[MAX_SETNUM];	// キーワードの英大文字小文字区別
	size_t	nKeywordNumArr[MAX_SETNUM];	// キーワードセットに登録されているキーワード数
private:
	// キーワード格納領域
	wchar_t	szKeywordArr[MAX_KEYWORDNUM][MAX_KEYWORDLEN + 1];	
	char	isSorted[MAX_SETNUM];	// ソートしたかどうかのフラグ(INI未保存)	 //MIK

protected:
	// 2004.07.29 Moca 可変長記憶
	/*! キーワードセットの開始位置(INI未保存)
		次の開始位置までが確保済みの領域．
		+1しているのは最後が0で終わるようにするため．
	*/
	size_t	nStartIdx[MAX_SETNUM + 1];
	size_t	nKeywordMaxLenArr[MAX_SETNUM]; // 一番長いキーワードの長さ(ソート後のみ有効)(INI未保存)

protected:
	/*
	||  実装ヘルパ関数
	*/
	//bool KeywordAlloc(int);
	bool KeywordReAlloc(size_t, size_t);
	void KeywordMaxLen( size_t );
};

