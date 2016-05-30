/*!	@file
	@brief ブロックコメントデリミタを管理する

	@author Yazaki
	@date 2002/09/17 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, Moca
	Copyright (C) 2005, D.S.Koba

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "BlockComment.h"
#include "mem/Memory.h"

BlockComment::BlockComment()
{
	szBlockCommentFrom[0] = '\0';
	szBlockCommentTo[0] = '\0';
	nBlockFromLen = 0;
	nBlockToLen = 0;
}

/*!
	ブロックコメントデリミタをコピーする
*/
void BlockComment::SetBlockCommentRule(
	const wchar_t* pszFrom,	// [in] コメント開始文字列
	const wchar_t* pszTo	// [in] コメント終了文字列
	)
{
	size_t nStrLen = wcslen(pszFrom);
	if (0 < nStrLen && nStrLen < BLOCKCOMMENT_BUFFERSIZE) {
		wcscpy(szBlockCommentFrom, pszFrom);
		nBlockFromLen = nStrLen;
	}else {
		szBlockCommentFrom[0] = L'\0';
		nBlockFromLen = 0;
	}
	nStrLen = wcslen(pszTo);
	if (0 < nStrLen && nStrLen < BLOCKCOMMENT_BUFFERSIZE) {
		wcscpy(szBlockCommentTo, pszTo);
		nBlockToLen = nStrLen;
	}else {
		szBlockCommentTo[0] = L'\0';
		nBlockToLen = 0;
	}
}

/*!
	n番目のブロックコメントの、nPosからの文字列が開始文字列(From)に当てはまるか確認する。

	@retval true  一致した
	@retval false 一致しなかった
*/
bool BlockComment::Match_CommentFrom(
	size_t				nPos,	// [in] 探索開始位置
	const StringRef&	str		// [in] 探索対象文字列 ※探索開始位置のポインタではないことに注意
	) const
{
	return (1
		&& szBlockCommentFrom[0] != L'\0'
		&& szBlockCommentTo[0] != L'\0'
		&& (int)nPos <= (int)str.GetLength() - (int)nBlockFromLen 	// ブロックコメントデリミタ(From)
		//&& auto_memicmp(&str.GetPtr()[nPos], szBlockCommentFrom, nBlockFromLen) == 0	// 非ASCIIも大文字小文字を区別しない	//###locale 依存
		&& wmemicmp_ascii(&str.GetPtr()[nPos], szBlockCommentFrom, nBlockFromLen) == 0	// ASCIIのみ大文字小文字を区別しない（高速）
	);
}

/*!
	n番目のブロックコメントの、後者(To)に当てはまる文字列をnPos以降から探す

	@return 当てはまった位置を返すが、当てはまらなかったときは、nLineLenをそのまま返す。
*/
size_t BlockComment::Match_CommentTo(
	size_t				nPos,	// [in] 探索開始位置
	const StringRef&	str		// [in] 探索対象文字列 ※探索開始位置のポインタではないことに注意
	) const
{
	for (int i=(int)nPos; i<=(int)str.GetLength()-(int)nBlockToLen; ++i) {
		//if (auto_memicmp(&str.GetPtr()[i], szBlockCommentTo, nBlockToLen) == 0) {	// 非ASCIIも大文字小文字を区別しない	//###locale 依存
		if (wmemicmp_ascii(&str.GetPtr()[i], szBlockCommentTo, nBlockToLen) == 0) {	// ASCIIのみ大文字小文字を区別しない（高速）
			return i + nBlockToLen;
		}
	}
	return str.GetLength();
}

