#include "StdAfx.h"
#include "docplus/BookmarkManager.h"
#include "doc/logic/DocLineMgr.h"
#include "doc/logic/DocLine.h"
#include "SearchAgent.h"
#include "extmodule/Bregexp.h"


bool BookmarkGetter::IsBookmarked() const { return pDocLine->mark.bookmarked; }
void BookmarkSetter::SetBookmark(bool bFlag) { pDocLine->mark.bookmarked = bFlag; }

// ブックマークの全解除
/*
	@date 2001.12.03 hor
*/
void BookmarkManager::ResetAllBookMark(void)
{
	DocLine* pDocLine = docLineMgr.GetDocLineTop();
	while (pDocLine) {
		BookmarkSetter(pDocLine).SetBookmark(false);
		pDocLine = pDocLine->GetNextLine();
	}
}


// ブックマーク検索
/*
	@date 2001.12.03 hor
*/
bool BookmarkManager::SearchBookMark(
	LogicInt			nLineNum,		// 検索開始行
	SearchDirection		bPrevOrNext,	// 0==前方検索 1==後方検索
	LogicInt*			pnLineNum 		// マッチ行
	)
{
	LogicInt nLinePos = nLineNum;
	
	// 前方検索
	if (bPrevOrNext == SearchDirection::Backward) {
		--nLinePos;
		DocLine* pDocLine = docLineMgr.GetLine(nLinePos);
		while (pDocLine) {
			if (BookmarkGetter(pDocLine).IsBookmarked()) {
				*pnLineNum = nLinePos;				// マッチ行
				return true;
			}
			--nLinePos;
			pDocLine = pDocLine->GetPrevLine();
		}
	// 後方検索
	}else {
		++nLinePos;
		DocLine* pDocLine = docLineMgr.GetLine(nLinePos);
		while (pDocLine) {
			if (BookmarkGetter(pDocLine).IsBookmarked()) {
				*pnLineNum = nLinePos;				// マッチ行
				return true;
			}
			++nLinePos;
			pDocLine = pDocLine->GetNextLine();
		}
	}
	return false;
}

// 物理行番号のリストからまとめて行マーク
/*
	@date 2002.01.16 hor
*/
void BookmarkManager::SetBookMarks(wchar_t* pMarkLines)
{
	DocLine* pDocLine;
	wchar_t delim[] = L", ";
	wchar_t* p = pMarkLines;
	if (p[0] == L':') {
		if (p[1] == L'0') {
			// ver2 形式 [0-9a-v] 0-31(終端バージョン) [w-zA-Z\+\-] 0-31
			// 2番目以降は、数値+1+ひとつ前の値
			// :00123x0 => 0,1,2,3,x0 => 0,(1+1),(2+2+1),(3+5+1),(32+9+1) => 0,2,5,9,42 => 1,3,6,10,43行目
			// :0a => a, => 10, 11
			p += 2;
			int nLineNum = 0;
			int nLineTemp = 0;
			while (*p != L'\0') {
				bool bSeparete = false;
				if (L'0' <= *p && *p <= L'9') {
					nLineTemp += (*p - L'0');
					bSeparete = true;
				}else if (L'a' <= *p && *p <= L'v') {
					nLineTemp += (*p - L'a') + 10;
					bSeparete = true;
				}else if (L'w' <= *p && *p <= L'z') {
					nLineTemp += (*p - L'w');
				}else if (L'A' <= *p && *p <= L'Z') {
					nLineTemp += (*p - L'A') + 4;
				}else if (*p == L'+') {
					nLineTemp += 30;
				}else if (*p == L'-') {
					nLineTemp += 31;
				}else {
					break;
				}
				if (bSeparete) {
					nLineNum += nLineTemp;
					pDocLine = docLineMgr.GetLine(LogicInt(nLineNum));
					if (pDocLine) {
						BookmarkSetter(pDocLine).SetBookmark(true);
					}
					++nLineNum;
					nLineTemp = 0;
				}else {
					nLineTemp *= 32;
				}
				++p;
			}
		}else {
			// 不明なバージョン
		}
	}else {
		// 旧形式 行番号,区切り
		while (wcstok(p, delim)) {
			while (wcschr(delim, *p)) {
				++p;
			}
			pDocLine = docLineMgr.GetLine(LogicInt(_wtol(p)));
			if (pDocLine) {
				BookmarkSetter(pDocLine).SetBookmark(true);
			}
			p += wcslen(p) + 1;
		}
	}
}


// 行マークされてる物理行番号のリストを作る
/*
	@date 2002.01.16 hor
	@date 2014.04.24 Moca ver2 差分32進数方式に変更
*/
LPCWSTR BookmarkManager::GetBookMarks()
{
	static wchar_t szText[MAX_MARKLINES_LEN + 1];	// 2002.01.17 // Feb. 17, 2003 genta staticに
	wchar_t szBuff[10];
	wchar_t szBuff2[10];
	LogicInt nLinePos = LogicInt(0);
	LogicInt nLinePosOld = LogicInt(-1);
	size_t nTextLen = 2;
	DocLine* pDocLine = docLineMgr.GetLine(nLinePos);
	wcscpy(szText, L":0");
	while (pDocLine) {
		if (BookmarkGetter(pDocLine).IsBookmarked()) {
			LogicInt nDiff = nLinePos - nLinePosOld - LogicInt(1);
			nLinePosOld = nLinePos;
			if (nDiff == LogicInt(0)) {
				szBuff2[0] = L'0';
				szBuff2[1] = L'\0';
			}else {
				int nColumn = 0;
				while (nDiff) {
					LogicInt nKeta = nDiff % 32;
					wchar_t c;
					if (nColumn == 0) {
						if (nKeta <= 9) {
							c = (wchar_t)((Int)nKeta + L'0');
						}else {
							c = (wchar_t)((Int)nKeta - 10 + L'a');
						}
					}else {
						if (nKeta <= 3) {
							c = (wchar_t)((Int)nKeta + L'w');
						}else if (nKeta <= 29) {
							c = (wchar_t)((Int)nKeta - 4 + L'A');
						}else if (nKeta == 30) {
							c = L'+';
						}else { // 31
							c = L'-';
						}
					}
					szBuff[nColumn] = c;
					++nColumn;
					nDiff /= 32;
				}
				for (int i=0; i<nColumn; ++i) {
					szBuff2[i] = szBuff[nColumn - 1 - i];
				}
				szBuff2[nColumn] = L'\0';
			}
			size_t nBuff2Len = wcslen(szBuff2);
			if (nBuff2Len + nTextLen > MAX_MARKLINES_LEN) {
				break;	//2002.01.17
			}
			wcscpy(szText + nTextLen, szBuff2);
			nTextLen += nBuff2Len;
		}
		++nLinePos;
		pDocLine = pDocLine->GetNextLine();
	}
	return szText; // Feb. 17, 2003 genta
}


// 検索条件に該当する行にブックマークをセットする
/*
	@date 2002.01.16 hor
*/
void BookmarkManager::MarkSearchWord(
	const SearchStringPattern& pattern
	)
{
	const SearchOption& searchOption = pattern.GetSearchOption();
	int nLineLen;

	// 1 == 正規表現
	if (searchOption.bRegularExp) {
		Bregexp* pRegexp = pattern.GetRegexp();
		DocLine* pDocLine = docLineMgr.GetLine(LogicInt(0));
		while (pDocLine) {
			if (!BookmarkGetter(pDocLine).IsBookmarked()) {
				const wchar_t* pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				// 2005.03.19 かろと 前方一致サポートのためのメソッド変更
				if (pRegexp->Match(pLine, nLineLen, 0)) {
					BookmarkSetter(pDocLine).SetBookmark(true);
				}
			}
			pDocLine = pDocLine->GetNextLine();
		}
	// 1 == 単語のみ検索
	}else if (searchOption.bWordOnly) {
		const wchar_t* pszPattern = pattern.GetKey();
		const int nPatternLen = pattern.GetLen();
		// 検索語を単語に分割して searchWordsに格納する。
		std::vector<std::pair<const wchar_t*, LogicInt>> searchWords; // 単語の開始位置と長さの配列。
		SearchAgent::CreateWordList(searchWords, pszPattern, nPatternLen);
		DocLine* pDocLine = docLineMgr.GetLine(LogicInt(0));
		while (pDocLine) {
			if (!BookmarkGetter(pDocLine).IsBookmarked()) {
				const wchar_t* pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				int nMatchLen;
				if (SearchAgent::SearchStringWord(pLine, nLineLen, 0, searchWords, searchOption.bLoHiCase, &nMatchLen)) {
					BookmarkSetter(pDocLine).SetBookmark(true);
				}
			}
			// 次の行を見に行く
			pDocLine = pDocLine->GetNextLine();
		}
	}else {
		// 検索条件の情報
		DocLine* pDocLine = docLineMgr.GetLine(LogicInt(0));
		while (pDocLine) {
			if (!BookmarkGetter(pDocLine).IsBookmarked()) {
				const wchar_t* pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				if (SearchAgent::SearchString(
					pLine,
					nLineLen,
					0,
					pattern
					)
				) {
					BookmarkSetter(pDocLine).SetBookmark(true);
				}
			}
			pDocLine = pDocLine->GetNextLine();
		}
	}
}

