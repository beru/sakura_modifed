/*
	Copyright (C) 2008, kobake

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

#include "_main/global.h" // SearchDirection, SearchOption

class DocLine;
class DocLineMgr;
class Bregexp;

#include "SearchAgent.h"

// 行に付加するブックマーク情報
class LineBookmarked {
public:
	LineBookmarked() : bBookmarked(false) { }
	operator bool() const { return bBookmarked; }
	LineBookmarked& operator = (bool b) { bBookmarked = b; return *this; }
private:
	bool bBookmarked;
};

// 行のブックマーク情報の取得
class BookmarkGetter {
public:
	BookmarkGetter(const DocLine* pDocLine) : pDocLine(pDocLine) { }
	bool IsBookmarked() const;
private:
	const DocLine* pDocLine;
};

// 行のブックマーク情報の取得・設定
class BookmarkSetter : public BookmarkGetter {
public:
	BookmarkSetter(DocLine* pDocLine) : BookmarkGetter(pDocLine), pDocLine(pDocLine) { }
	void SetBookmark(bool bFlag);
private:
	DocLine* pDocLine;
};

// 行全体のブックマーク情報の管理
class BookmarkManager {
public:
	BookmarkManager(DocLineMgr& docLineMgr) : docLineMgr(docLineMgr) { }

	void ResetAllBookMark();												// ブックマークの全解除
	bool SearchBookMark(int nLineNum, SearchDirection, int* pnLineNum);		// ブックマーク検索
	void SetBookMarks(wchar_t*);											// 物理行番号のリストからまとめて行マーク
	LPCWSTR GetBookMarks();													// 行マークされてる物理行番号のリストを作る
	void MarkSearchWord(const SearchStringPattern&);			// 検索条件に該当する行にブックマークをセットする

private:
	DocLineMgr& docLineMgr;
};

