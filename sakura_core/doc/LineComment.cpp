#include "StdAfx.h"
#include "LineComment.h"

LineComment::LineComment()
{
	for (size_t i=0; i<COMMENT_DELIMITER_NUM; ++i) {
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
void LineComment::CopyTo(size_t n, const wchar_t* buffer, int commentPos)
{
	size_t nStrLen = wcslen(buffer);
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
	for (size_t i=0; i<COMMENT_DELIMITER_NUM; ++i) {
		if (1
			&& pszLineComment[i][0] != L'\0'	// 行コメントデリミタ
			&& (lineCommentPos[i] < 0 || lineCommentPos[i] == pos)	// 位置指定ON.
			&& pos <= (int)str.GetLength() - lineCommentLen[i]	// 行コメントデリミタ
			//&& auto_memicmp(&str.GetPtr()[pos], pszLineComment[i], lineCommentLen[i]) == 0	// 非ASCIIも大文字小文字を区別しない	//###locale 依存
			&& wmemicmp_ascii(&str.GetPtr()[pos], pszLineComment[i], lineCommentLen[i]) == 0	// ASCIIのみ大文字小文字を区別しない（高速）
		) {
			return true;
		}
	}
	return false;
}

