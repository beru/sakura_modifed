#pragma once

// 強調キーワード管理

#include <Windows.h>
#include "_main/global.h"

#define		MAX_SETNUM		100
#define		MAX_SETNAMELEN	32


// キーワード総数
#define		MAX_KEYWORDNUM	15000
#define		MAX_KEYWORDLEN	63

/*! @brief 強調キーワード管理

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
	const wchar_t* SetTypeName(size_t, const wchar_t*);	// ｎ番目のセット名を設定する
	void SetKeywordCase(size_t, bool);				// ｎ番目のセットの大文字小文字判断をセットする
	bool GetKeywordCase(size_t);					// ｎ番目のセットの大文字小文字判断を取得する
	void SortKeyword(size_t);						// ｎ番目のセットのキーワードをソートする

	size_t SetKeywordArr(size_t, size_t, const wchar_t*);			// iniからキーワードを設定する
	size_t SetKeywordArr(						// キーワードの配列から設定する
		size_t			nIdx,				// [in] キーワードセット番号
		size_t			nSize,				// [in] ppszKeywordArrの要素数
		const wchar_t*	ppszKeywordArr[]	// [in] キーワードの配列(重複・長さ制限等、考慮済みであること)
	);
	//@}

	//@{
	///	@name キーワード操作
	size_t GetKeywordNum(size_t);				// ｎ番目のセットのキーワードの数を返す
	const wchar_t* GetKeyword(size_t, size_t);	// ｎ番目のセットのｍ番目のキーワードを返す
	const wchar_t* UpdateKeyword(size_t, size_t, const wchar_t*);	// ｎ番目のセットのｍ番目のキーワードを編集
	size_t AddKeyword(size_t, const wchar_t*);	// ｎ番目のセットにキーワードを追加
	size_t DelKeyword(size_t, size_t);			// ｎ番目のセットのｍ番目のキーワードを削除
	bool CanAddKeyword(size_t);	// キーワードが追加可能か
	//@}
	
	//@{
	///	@name 検索
	//int SearchKeyword(int , const char*, int);				// ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す
	int SearchKeyword2(size_t nIdx , const wchar_t* pszKeyword, size_t nKeywordLen);	// ｎ番目のセットから指定キーワードをバイナリサーチ。見つかれば 0以上を返す
	int SearchKeywordSet(const wchar_t* pszKeyword);		// キーワードセット名からセット番号を取得。見つからなければ -1を返す
	//@}

	size_t CleanKeywords(size_t);			// キーワードの整頓・利用できないキーワードの削除
	size_t GetAllocSize(size_t) const;		// 確保している数を返す
	size_t GetFreeSize() const;			// 未割り当てブロックのキーワード数を返す
	void ResetAllKeywordSet(void);		// 全キーワードセットの削除と初期化

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
	char	isSorted[MAX_SETNUM];	// ソートしたかどうかのフラグ(INI未保存)

protected:
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

