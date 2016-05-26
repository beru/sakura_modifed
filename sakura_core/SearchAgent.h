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

#include "_main/global.h"

class DocLineMgr;
struct DocLineReplaceArg;
class Bregexp;

//#define SEARCH_STRING_KMP
#define SEARCH_STRING_SUNDAY_QUICK

class SearchStringPattern {
public:
	SearchStringPattern();
	SearchStringPattern(HWND, const wchar_t* pszPattern, size_t nPatternLen, const SearchOption& searchOption, Bregexp* pRegexp);
	~SearchStringPattern();
	void Reset();
	bool SetPattern(HWND hwnd, const wchar_t* pszPattern, size_t nPatternLen, const SearchOption& searchOption, Bregexp* pRegexp) {
		return SetPattern(hwnd, pszPattern, nPatternLen, NULL, searchOption, pRegexp);
	}
	bool SetPattern(HWND, const wchar_t* pszPattern, size_t nPatternLen, const wchar_t* pszPattern2, const SearchOption& searchOption, Bregexp* pRegexp);
	const wchar_t* GetKey() const { return pszKey; }
	const wchar_t* GetCaseKey() const { return pszCaseKeyRef; }
	size_t GetLen() const { return nPatternLen; }
	bool GetIgnoreCase() const { return !pSearchOption->bLoHiCase; }
	bool GetLoHiCase() const { return pSearchOption->bLoHiCase; }
	const SearchOption& GetSearchOption() const { return *pSearchOption; }
	Bregexp* GetRegexp() const { return pRegexp; }
#ifdef SEARCH_STRING_KMP
	const int* GetKMPNextTable() const { return pnNextPossArr; }
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	const int* GetUseCharSkipMap() const { return pnUseCharSkipArr; }

	static int GetMapIndex(wchar_t c);
#endif

private:
	// 外部依存
	const wchar_t* pszKey;
	const SearchOption* pSearchOption;
	mutable Bregexp* pRegexp;

	const wchar_t* pszCaseKeyRef;

	// 内部バッファ
	wchar_t* pszPatternCase;
	size_t nPatternLen;
#ifdef SEARCH_STRING_KMP
	int* pnNextPossArr;
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	int* pnUseCharSkipArr;
#endif

private:
	DISALLOW_COPY_AND_ASSIGN(SearchStringPattern);
};

class SearchAgent {
public:
	// 文字列検索
	static
	const wchar_t* SearchString(
		const wchar_t* pLine,
		size_t nLineLen,
		int nIdxPos,
		const SearchStringPattern& pattern
	);
	// 単語単位で文字列検索
	static
	const wchar_t* SearchStringWord(
		const wchar_t*	pLine,
		size_t			nLineLen,
		size_t			nIdxPos,
		const std::vector<std::pair<const wchar_t*, size_t>>& searchWords,
		bool	bLoHiCase,
		int*	pnMatchLen
	);
	
	// 検索条件の情報
	static
	void CreateCharCharsArr(
		const wchar_t*	pszPattern,
		int				nSrcLen,
		int**			ppnCharCharsArr
	);
	
	static
	void CreateWordList(
		std::vector<std::pair<const wchar_t*, size_t>>& searchWords,
		const wchar_t* pszPattern,
		size_t nPatternLen
	);

public:
	SearchAgent(DocLineMgr& docLineMgr) : docLineMgr(docLineMgr) { }

	bool WhereCurrentWord(size_t, size_t, size_t* , size_t*, NativeW*, NativeW*);	// 現在位置の単語の範囲を調べる

	bool PrevOrNextWord(size_t, size_t, size_t* , bool bLEFT, bool bStopsBothEnds);	// 現在位置の左右の単語の先頭位置を調べる
	//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	int SearchWord(Point ptSerachBegin, SearchDirection eDirection, Range* pMatchRange, const SearchStringPattern& pattern); // 単語検索

	void ReplaceData(DocLineReplaceArg*);
private:
	DocLineMgr& docLineMgr;
};

