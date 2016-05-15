/*!	@file
	@brief 行コメントデリミタを管理する

	@author Yazaki
	@date 2002/09/17 新規作成
*/
/*
	Copyright (C) 2002, Yazaki

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

// sakura
#include "_main/global.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
#define COMMENT_DELIMITER_NUM	3
#define COMMENT_DELIMITER_BUFFERSIZE	16

/*! 行コメントデリミタを管理する

	@note LineCommentは、共有メモリSTypeConfigに含まれるので、メンバ変数は常に実体を持っていなければならない。
*/
class LineComment {
public:
	/*
	||  Constructors：コンパイラ標準を使用。
	*/
	LineComment();

	void CopyTo(const int n, const wchar_t* buffer, int commentPos);	// 行コメントデリミタをコピーする
	bool Match(int pos, const StringRef& str) const;				// 行コメントに値するか確認する

	const wchar_t* getLineComment(const int n) {
		return pszLineComment[n];
	}
	int getLineCommentPos(const int n) const {
		return lineCommentPos[n];
	}

private:
	wchar_t	pszLineComment[COMMENT_DELIMITER_NUM][COMMENT_DELIMITER_BUFFERSIZE];	// 行コメントデリミタ
	int		lineCommentPos[COMMENT_DELIMITER_NUM];	// 行コメントの開始位置(負数は指定無し)
	int		lineCommentLen[COMMENT_DELIMITER_NUM];	// 行コメント文字列の長さ
};

