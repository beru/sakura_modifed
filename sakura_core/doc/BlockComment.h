/*!	@file
	@brief ブロックコメントデリミタを管理する
*/
#pragma once

// sakura
#include "_main/global.h"

enum class CommentType {
	Zero	= 0,
	One		= 1,
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*! ブロックコメントデリミタを管理する

	@note CBlockCommentGroupは、共有メモリSTypeConfigに含まれるので、メンバ変数は常に実体を持っていなければならない。
*/
#define BLOCKCOMMENT_NUM	2
#define BLOCKCOMMENT_BUFFERSIZE	16

// 2005.11.10 Moca アクセス関数追加
class BlockComment {
public:
	// 生成と破棄
	BlockComment();

	// 設定
	void SetBlockCommentRule(const wchar_t* pszFrom, const wchar_t* pszTo);	// 行コメントデリミタをコピーする

	// 判定
	bool Match_CommentFrom(size_t nPos, const StringRef& str) const;			// 行コメントに値するか確認する
	size_t Match_CommentTo(size_t nPos, const StringRef& str) const;			// 行コメントに値するか確認する

	// 取得
	const wchar_t* getBlockCommentFrom() const { return szBlockCommentFrom; }
	const wchar_t* getBlockCommentTo() const { return szBlockCommentTo; }
	size_t getBlockFromLen() const { return nBlockFromLen; }
	size_t getBlockToLen() const { return nBlockToLen; }

private:
	wchar_t	szBlockCommentFrom[BLOCKCOMMENT_BUFFERSIZE]; // ブロックコメントデリミタ(From)
	wchar_t	szBlockCommentTo[BLOCKCOMMENT_BUFFERSIZE];   // ブロックコメントデリミタ(To)
	size_t	nBlockFromLen;
	size_t	nBlockToLen;
};

