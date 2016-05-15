/*!	@file
	@brief 行コメントデリミタを管理する

	@author Yazaki
	@date 2002/09/17 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Yazaki, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "LineComment.h"

LineComment::LineComment()
{
	for (int i=0; i<COMMENT_DELIMITER_NUM; ++i) {
		pszLineComment[i][0] = '\0';
		lineCommentPos[i] = -1;
	}
}

/*!
	行コメントデリミタをコピーする
	@param n [in]           コピー対象のコメント番号
	@param buffer [in]      コメント文字列
	@param commentPos [in] コメント位置．-1のときは指定無し．
*/
void LineComment::CopyTo(const int n, const wchar_t* buffer, int commentPos)
{
	int nStrLen = wcslen(buffer);
	if (0 < nStrLen && nStrLen < COMMENT_DELIMITER_BUFFERSIZE) {
		wcscpy(pszLineComment[n], buffer);
		lineCommentPos[n] = commentPos;
		lineCommentLen[n] = nStrLen;
	}else {
		pszLineComment[n][0] = L'\0';
		lineCommentPos[n] = -1;
		lineCommentLen[n] = 0;
	}
}

bool LineComment::Match(int pos, const StringRef& str) const
{
	for (int i=0; i<COMMENT_DELIMITER_NUM; ++i) {
		if (1
			&& pszLineComment[i][0] != L'\0'	// 行コメントデリミタ
			&& (lineCommentPos[i] < 0 || lineCommentPos[i] == pos)	// 位置指定ON.
			&& pos <= str.GetLength() - lineCommentLen[i]	// 行コメントデリミタ
			//&& auto_memicmp(&str.GetPtr()[pos], pszLineComment[i], lineCommentLen[i]) == 0	// 非ASCIIも大文字小文字を区別しない	//###locale 依存
			&& wmemicmp_ascii(&str.GetPtr()[pos], pszLineComment[i], lineCommentLen[i]) == 0	// ASCIIのみ大文字小文字を区別しない（高速）
		) {
			return true;
		}
	}
	return false;
}

