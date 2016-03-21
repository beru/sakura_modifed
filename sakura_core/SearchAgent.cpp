#include "StdAfx.h"

#include <vector>
#include <utility>
#include "SearchAgent.h"
#include "doc/logic/DocLineMgr.h"
#include "doc/logic/DocLine.h"
#include "Ope.h"
#include "dlg/DlgCancel.h"
#include "util/string_ex.h"
#include <algorithm>
#include "sakura_rc.h"

//#define MEASURE_SEARCH_TIME
#ifdef MEASURE_SEARCH_TIME
#include <time.h>
#endif

// SearchStringPattern
// @date 2010.06.22 Moca
inline
int SearchStringPattern::GetMapIndex(wchar_t c)
{
	// ASCII    => 0x000 - 0x0ff
	// それ以外 => 0x100 - 0x1ff
	return ((c & 0xff00) ? 0x100 : 0 ) | (c & 0xff);
}

SearchStringPattern::SearchStringPattern()
	: 
	m_pszKey(NULL),
	m_pSearchOption(NULL),
	m_pRegexp(NULL),
	m_pszCaseKeyRef(NULL),
	m_pszPatternCase(NULL),
#ifdef SEARCH_STRING_KMP
	m_pnNextPossArr(NULL),
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	m_pnUseCharSkipArr(NULL)
#endif
{
}

SearchStringPattern::SearchStringPattern(
	HWND hwnd,
	const wchar_t* pszPattern,
	int nPatternLen,
	const SearchOption& searchOption,
	Bregexp* pRegexp
	)
	:
	m_pszKey(NULL),
	m_pSearchOption(NULL),
	m_pRegexp(NULL),
	m_pszCaseKeyRef(NULL),
	m_pszPatternCase(NULL),
#ifdef SEARCH_STRING_KMP
	m_pnNextPossArr(NULL),
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	m_pnUseCharSkipArr(NULL)
#endif
{
	SetPattern(hwnd, pszPattern, nPatternLen, searchOption, pRegexp);
}

SearchStringPattern::~SearchStringPattern()
{
	Reset();
}

void SearchStringPattern::Reset() {
	m_pszKey = NULL;
	m_pszCaseKeyRef = NULL;
	m_pSearchOption = NULL;
	m_pRegexp = NULL;

	delete[] m_pszPatternCase;
	m_pszPatternCase = NULL;
#ifdef SEARCH_STRING_KMP
	delete[] m_pnNextPossArr;
	m_pnNextPossArr = NULL;
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	delete[] m_pnUseCharSkipArr;
	m_pnUseCharSkipArr = NULL;
#endif
}

bool SearchStringPattern::SetPattern(
	HWND hwnd,
	const wchar_t* pszPattern,
	int nPatternLen,
	const wchar_t* pszPattern2,
	const SearchOption& searchOption,
	Bregexp* regexp
	)
{
	Reset();
	m_pszCaseKeyRef = m_pszKey = pszPattern;
	m_nPatternLen = nPatternLen;
	m_pSearchOption = &searchOption;
	m_pRegexp = regexp;
	if (m_pSearchOption->bRegularExp) {
		if (!m_pRegexp) {
			return false;
		}
		if (!InitRegexp(hwnd, *m_pRegexp, true)) {
			return false;
		}
		int nFlag = (GetLoHiCase() ? Bregexp::optCaseSensitive : Bregexp::optNothing);
		// 検索パターンのコンパイル
		if (pszPattern2) {
			if (!m_pRegexp->Compile( pszPattern, pszPattern2, nFlag )) {
				return false;
			}
		}else {
			if (!m_pRegexp->Compile(pszPattern, nFlag)) {
				return false;
			}
		}
	}else if (m_pSearchOption->bWordOnly) {
	}else {
		if (GetIgnoreCase()) {
			m_pszPatternCase = new wchar_t[nPatternLen + 1];
			m_pszCaseKeyRef = m_pszPatternCase;
			// note: 合成文字,サロゲートの「大文字小文字同一視」未対応
			for (int i=0; i<m_nPatternLen; ++i) {
				m_pszPatternCase[i] = (wchar_t)skr_towlower(pszPattern[i]);
			}
			m_pszPatternCase[nPatternLen] = L'\0';
		}

#ifdef SEARCH_STRING_KMP
	// "ABCDE" => {-1, 0, 0, 0, 0}
	// "AAAAA" => {-1, 0, 1, 2, 3}
	// "AABAA" => {-1, 0, 0, 0, 0}
	// "ABABA" => {-1, 0, 0, 2, 0}
//	if (GetIgnoreCase()) {
		m_pnNextPossArr = new int[nPatternLen + 1];
		int* next = m_pnNextPossArr;
		const wchar_t* key = m_pszPatternCase;
		for (int i=0, k=-1; i<nPatternLen; ++i, ++k) {
			next[i] = k;
			while (-1 < k && key[i] != key[k]) {
				k = next[k];
			}
		}
//	}
#endif

#ifdef SEARCH_STRING_SUNDAY_QUICK
		const int BM_MAPSIZE = 0x200;
		// 64KB も作らないで、ISO-8859-1 それ以外(包括) の2つの情報のみ記録する
		// 「あ」と「乂」　「ぅ」と「居」は値を共有している
		m_pnUseCharSkipArr = new int[BM_MAPSIZE];
		for (int n=0; n<BM_MAPSIZE; ++n) {
			m_pnUseCharSkipArr[n] = nPatternLen + 1;
		}
		for (int n=0; n<nPatternLen; ++n) {
			const int index = GetMapIndex(m_pszCaseKeyRef[n]);
			m_pnUseCharSkipArr[index] = nPatternLen - n;
		}
#endif
	}
	return true;
}


#define toLoHiLower(bLoHiCase, ch) (bLoHiCase? (ch) : skr_towlower(ch))

/*!
	文字列検索
	@return 見つかった場所のポインタ。見つからなかったらNULL。
*/
const wchar_t* SearchAgent::SearchString(
	const wchar_t*	pLine,
	int				nLineLen,
	int				nIdxPos,
	const SearchStringPattern& pattern
	)
{
	const int      nPatternLen = pattern.GetLen();
	const wchar_t* pszPattern  = pattern.GetCaseKey();
#ifdef SEARCH_STRING_SUNDAY_QUICK
	const int* const useSkipMap = pattern.GetUseCharSkipMap();
#endif
	bool bLoHiCase = pattern.GetLoHiCase();

	if (nLineLen < nPatternLen) {
		return NULL;
	}
	if (0 >= nPatternLen || 0 >= nLineLen) {
		return NULL;
	}

	// 線形探索
	const int nCompareTo = nLineLen - nPatternLen;	//	Mar. 4, 2001 genta

#if defined(SEARCH_STRING_SUNDAY_QUICK) && !defined(SEARCH_STRING_KMP)
	// SUNDAY_QUICKのみ版
	if (!bLoHiCase || nPatternLen > 5) {
		for (int nPos=nIdxPos; nPos<=nCompareTo;) {
			int i;
			for (i = 0; i < nPatternLen && toLoHiLower(bLoHiCase, pLine[nPos + i]) == pszPattern[i]; ++i) {
			}
			if (i >= nPatternLen) {
				return &pLine[nPos];
			}
			int index = SearchStringPattern::GetMapIndex((wchar_t)toLoHiLower(bLoHiCase, pLine[nPos + nPatternLen]));
			nPos += useSkipMap[index];
		}
	}else {
		for (int nPos=nIdxPos; nPos<=nCompareTo;) {
			int n = wmemcmp(&pLine[nPos], pszPattern, nPatternLen);
			if (n == 0) {
				return &pLine[nPos];
			}
			int index = SearchStringPattern::GetMapIndex(pLine[nPos + nPatternLen]);
			nPos += useSkipMap[index];
		}
	}
#else
#ifdef SEARCH_STRING_KMP
	// 大文字小文字を区別しない、かつ、検索語が5文字以下の場合は通常の検索を行う
	// そうでない場合はKMP＋SUNDAY QUICKアルゴリズムを使った検索を行う
	if (!bLoHiCase || nPatternLen > 5) {
		const wchar_t pattern0 = pszPattern[0];
		const int* const nextTable = pattern.GetKMPNextTable();
		for (int nPos=nIdxPos; nPos<=nCompareTo;) {
			if (toLoHiLower(bLoHiCase, pLine[nPos]) != pattern0) {
#ifdef SEARCH_STRING_SUNDAY_QUICK
				int index = SearchStringPattern::GetMapIndex((wchar_t)toLoHiLower(bLoHiCase, pLine[nPos + nPatternLen));
				nPos += useSkipMap[index];
#else
				++nPos;
#endif
				continue;
			}
			// 途中まで一致ならずらして継続(KMP)
			int i = 1;
			++nPos;
			while (0 < i) {
				while (i < nPatternLen && toLoHiLower(bLoHiCase, pLine[nPos]) == pszPattern[i]) {
					++i;
					++nPos;
				}
				if (i >= nPatternLen) {
					return &pLine[nPos - nPatternLen];
				}
				i = nextTable[i];
			}
			assert(i == 0); // -1チェック
		}
	}else {
#endif
		// 通常版
		int	nPos;
		for (nPos = nIdxPos; nPos <= nCompareTo; nPos += NativeW::GetSizeOfChar(pLine, nLineLen, nPos)) {
			int n = bLoHiCase ?
						wmemcmp(&pLine[nPos], pszPattern, nPatternLen):
						wmemicmp(&pLine[nPos], pszPattern, nPatternLen);
			if (n == 0) {
				return &pLine[nPos];
			}
		}
#ifdef SEARCH_STRING_KMP
	}
#endif
#endif // defined(SEARCH_STRING_) && !defined(SEARCH_STRING_KMP)
	return NULL;
}

// 検索条件の情報(キー文字列の全角か半角かの配列)作成
void SearchAgent::CreateCharCharsArr(
	const wchar_t*	pszPattern,
	int				nSrcLen,
	int**			ppnCharCharsArr
	)
{
	int* pnCharCharsArr = new int[nSrcLen];
	for (int i=0; i<nSrcLen; /*++i*/) {
		// 2005-09-02 D.S.Koba GetSizeOfChar
		pnCharCharsArr[i] = NativeW::GetSizeOfChar(pszPattern, nSrcLen, i);
		if (pnCharCharsArr[i] == 0) {
			pnCharCharsArr[i] = 1;
		}
		if (pnCharCharsArr[i] == 2) {
			pnCharCharsArr[i + 1] = pnCharCharsArr[i];
		}
		i += pnCharCharsArr[i];
	}
	*ppnCharCharsArr = pnCharCharsArr;
	return;
}

/*!	単語単位の単語リスト作成
*/
void SearchAgent::CreateWordList(
	std::vector<std::pair<const wchar_t*, LogicInt>>& searchWords,
	const wchar_t* pszPattern,
	int	nPatternLen
	)
{
	for (LogicInt pos=LogicInt(0); pos<nPatternLen; ) {
		LogicInt begin, end; // 検索語に含まれる単語?の posを基準とした相対位置。WhereCurrentWord_2()の仕様では空白文字列も単語に含まれる。
		if (WordParse::WhereCurrentWord_2(pszPattern+pos, nPatternLen-pos, LogicInt(0), &begin, &end, NULL, NULL)
			&& begin == 0 && begin < end
		) {
			if (!WCODE::IsWordDelimiter(pszPattern[pos])) {
				// pszPattern[pos]...pszPattern[pos + end] が検索語に含まれる単語。
				searchWords.emplace_back(pszPattern + pos, end);
			}
			pos += end;
		}else {
			pos += t_max(LogicInt(1), NativeW::GetSizeOfChar(pszPattern, nPatternLen, pos));
		}
	}
}


/*!	単語単位検索
*/
const wchar_t* SearchAgent::SearchStringWord(
	const wchar_t*	pLine,
	int				nLineLen,
	int				nIdxPos,
	const std::vector<std::pair<const wchar_t*, LogicInt>>& searchWords,
	bool	 bLoHiCase,
	int*	 pnMatchLen
	)
{
	LogicInt nNextWordFrom = LogicInt(nIdxPos);
	LogicInt nNextWordFrom2;
	LogicInt nNextWordTo2;
	while (WordParse::WhereCurrentWord_2(pLine, LogicInt(nLineLen), nNextWordFrom, &nNextWordFrom2, &nNextWordTo2, NULL, NULL)) {
		size_t nSize = searchWords.size();
		for (size_t iSW=0; iSW<nSize; ++iSW) {
			if (searchWords[iSW].second == nNextWordTo2 - nNextWordFrom2) {
				// 1 == 大文字小文字の区別
				if ((!bLoHiCase && auto_memicmp(&(pLine[nNextWordFrom2]), searchWords[iSW].first, searchWords[iSW].second) == 0) ||
					(bLoHiCase && auto_memcmp(&(pLine[nNextWordFrom2]), searchWords[iSW].first, searchWords[iSW].second) == 0)
				) {
					*pnMatchLen = searchWords[iSW].second;
					return &pLine[nNextWordFrom2];
				}
			}
		}
		if (!WordParse::SearchNextWordPosition(pLine, LogicInt(nLineLen), nNextWordFrom, &nNextWordFrom, false)) {
			break;	//	次の単語が無い。
		}
	}
	*pnMatchLen = 0;
	return NULL;
}


// 現在位置の単語の範囲を調べる
// 2001/06/23 N.Nakatani WhereCurrentWord()変更 WhereCurrentWord_2をコールするようにした
bool SearchAgent::WhereCurrentWord(
	LogicInt	nLineNum,
	LogicInt	nIdx,
	LogicInt*	pnIdxFrom,
	LogicInt*	pnIdxTo,
	NativeW*	pcmcmWord,
	NativeW*	pcmcmWordLeft
	)
{
	*pnIdxFrom = nIdx;
	*pnIdxTo = nIdx;
	
	const DocLine* pDocLine = m_docLineMgr.GetLine(nLineNum);
	if (!pDocLine) {
		return false;
	}
	
	LogicInt		nLineLen;
	const wchar_t*	pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
	
	// 現在位置の単語の範囲を調べる
	return WordParse::WhereCurrentWord_2(pLine, nLineLen, nIdx, pnIdxFrom, pnIdxTo, pcmcmWord, pcmcmWordLeft);
}


// 現在位置の左右の単語の先頭位置を調べる
bool SearchAgent::PrevOrNextWord(
	LogicInt	nLineNum,		// 行数
	LogicInt	nIdx,			// 桁数
	LogicInt*	pnColumnNew,	// 見つかった位置
	bool		bLeft,			// true : 前方（左）へ向かう。false : 後方（右）へ向かう。
	bool		bStopsBothEnds	// 単語の両端で止まる
	)
{
	using namespace WCODE;
	
	const DocLine*	pDocLine = m_docLineMgr.GetLine( nLineNum );
	if (!pDocLine) {
		return false;
	}
	
	LogicInt		nLineLen;
	const wchar_t*	pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
	
	// ABC D[EOF]となっていたときに、Dの後ろにカーソルを合わせ、単語の左端に移動すると、Aにカーソルがあうバグ修正。YAZAKI
	if (nIdx >= nLineLen) {
		if (bLeft && nIdx == nLineLen) {
		}else {
			// 2011.12.26 EOFより右へ行こうとするときもfalseを返すように
			// nIdx = nLineLen - LogicInt(1);
			return false;
		}
	}
	// 現在位置の文字の種類によっては選択不能
	if (!bLeft && WCODE::IsLineDelimiter(pLine[nIdx], GetDllShareData().common.edit.bEnableExtEol)) {
		return false;
	}
	// 前の単語か？後ろの単語か？
	if (bLeft) {
		// 現在位置の文字の種類を調べる
		ECharKind	nCharKind = WordParse::WhatKindOfChar(pLine, nLineLen, nIdx);
		if (nIdx == 0) {
			return false;
		}

		// 文字種類が変わるまで前方へサーチ
		// 空白とタブは無視する
		int			nCount = 0;
		LogicInt	nIdxNext = nIdx;
		LogicInt	nCharChars = LogicInt(&pLine[nIdxNext] - NativeW::GetCharPrev(pLine, nLineLen, &pLine[nIdxNext]));
		while (nCharChars > 0) {
			LogicInt nIdxNextPrev = nIdxNext;
			nIdxNext -= nCharChars;
			ECharKind nCharKindNext = WordParse::WhatKindOfChar(pLine, nLineLen, nIdxNext);
			ECharKind nCharKindMerge = WordParse::WhatKindOfTwoChars(nCharKindNext, nCharKind);
			if (nCharKindMerge == CK_NULL) {
				// サーチ開始位置の文字が空白またはタブの場合
				if (nCharKind == CK_TAB	|| nCharKind == CK_SPACE) {
					if (bStopsBothEnds && nCount) {
						nIdxNext = nIdxNextPrev;
						break;
					}
					nCharKindMerge = nCharKindNext;
				}else {
					if (nCount == 0) {
						nCharKindMerge = nCharKindNext;
					}else {
						nIdxNext = nIdxNextPrev;
						break;
					}
				}
			}
			nCharKind = nCharKindMerge;
			nCharChars = LogicInt(&pLine[nIdxNext] - NativeW::GetCharPrev(pLine, nLineLen, &pLine[nIdxNext]));
			++nCount;
		}
		*pnColumnNew = nIdxNext;
	}else {
		WordParse::SearchNextWordPosition(pLine, nLineLen, nIdx, pnColumnNew, bStopsBothEnds);
	}
	return true;
}


/*! 単語検索
	@date 2003.05.22 かろと 行頭処理など見直し
	@date 2005.11.26 かろと \rや.が\r\nにヒットしないように
*/
// 見つからない場合は０を返す
int SearchAgent::SearchWord(
	LogicPoint					ptSerachBegin,	// 検索開始位置
	SearchDirection				eDirection,		// 検索方向
	LogicRange*					pMatchRange,	// [out] マッチ範囲。ロジック単位。
	const SearchStringPattern&	pattern			// 検索パターン
	)
{
	DocLine*	pDocLine;
	LogicInt	nLinePos;
	LogicInt	nIdxPos;
	LogicInt	nIdxPosOld;
	const wchar_t*	pLine;
	int			nLineLen;
	const wchar_t*	pszRes;
	LogicInt	nHitTo;
	LogicInt	nHitPos;
	LogicInt	nHitPosOld;
	int			nRetVal = 0;
	const SearchOption&	searchOption = pattern.GetSearchOption();
	Bregexp*	pRegexp = pattern.GetRegexp();
#ifdef MEASURE_SEARCH_TIME
	long clockStart, clockEnd;
	clockStart = clock();
#endif

	// 正規表現
	if (searchOption.bRegularExp) {
		nLinePos = ptSerachBegin.GetY2();		// 検索行＝検索開始行
		pDocLine = m_docLineMgr.GetLine(nLinePos);
		// 前方検索
		if (eDirection == SearchDirection::Backward) {
			//
			// 前方(↑)検索(正規表現)
			//
			nHitTo = ptSerachBegin.x;				// 検索開始位置
			nIdxPos = 0;
			while (pDocLine) {
				pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				nHitPos = -1;	// -1:この行でマッチ位置なし
				for (;;) {
					nHitPosOld = nHitPos;
					nIdxPosOld = nIdxPos;
					// 長さ０でマッチしたので、この位置で再度マッチしないように、１文字進める
					if (nIdxPos == nHitPos) {
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nIdxPos += (NativeW::GetSizeOfChar(pLine, nLineLen, nIdxPos) == 2 ? 2 : 1);
					}
					if (1
						&& nIdxPos <= pDocLine->GetLengthWithoutEOL() 
						&& pRegexp->Match(pLine, nLineLen, nIdxPos)
					) {
						// 検索にマッチした！
						nHitPos = pRegexp->GetIndex();
						nIdxPos = pRegexp->GetLastIndex();
						if (nHitPos >= nHitTo) {
							// マッチしたのは、カーソル位置以降だった
							// すでにマッチした位置があれば、それを返し、なければ前の行へ
							break;
						}
					}else {
						// マッチしなかった
						// すでにマッチした位置があれば、それを返し、なければ前の行へ
						break;
					}
				}

				if (nHitPosOld != -1) {
					// この行でマッチした位置が存在するので、この行で検索終了
					pMatchRange->SetFromX(nHitPosOld);	// マッチ位置from
					pMatchRange->SetToX  (nIdxPosOld);	// マッチ位置to
					break;
				}else {
					// この行でマッチした位置が存在しないので、前の行を検索へ
					--nLinePos;
					pDocLine = pDocLine->GetPrevLine();
					nIdxPos = 0;
					if (pDocLine) {
						nHitTo = pDocLine->GetLengthWithEOL() + 1;		// 前の行のNULL文字(\0)にもマッチさせるために+1 2003.05.16 かろと 
					}
				}
			}
		// 後方検索
		}else {
			//
			// 後方検索(正規表現)
			//
			nIdxPos = ptSerachBegin.x;
			while (pDocLine) {
				pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				if (1
					&& nIdxPos <= pDocLine->GetLengthWithoutEOL() 
					&& pRegexp->Match(pLine, nLineLen, nIdxPos)
				) {
					// マッチした
					pMatchRange->SetFromX(pRegexp->GetIndex());			// マッチ位置from
					pMatchRange->SetToX  (pRegexp->GetLastIndex());		// マッチ位置to
					break;
				}
				++nLinePos;
				pDocLine = pDocLine->GetNextLine();
				nIdxPos = 0;
			}
		}
		//
		// 正規表現検索の後処理
		if (pDocLine) {
			// マッチした行がある
			pMatchRange->SetFromY(nLinePos); // マッチ行
			pMatchRange->SetToY  (nLinePos); // マッチ行
			nRetVal = 1;
			// レイアウト行では改行文字内の位置を表現できないため、マッチ開始位置を補正
			if (pMatchRange->GetFrom().x > pDocLine->GetLengthWithoutEOL()) {
				// \r\n改行時に\nにマッチすると置換できない不具合となるため
				// 改行文字内でマッチした場合、改行文字の始めからマッチしたことにする
				pMatchRange->SetFromX(pDocLine->GetLengthWithoutEOL());
			}
		}
	// 単語のみ検索
	}else if (searchOption.bWordOnly) {
		// 検索語を単語に分割して searchWordsに格納する。
		const wchar_t* pszPattern = pattern.GetKey();
		const int	nPatternLen = pattern.GetLen();
		std::vector<std::pair<const wchar_t*, LogicInt>> searchWords; // 単語の開始位置と長さの配列。
		CreateWordList(searchWords, pszPattern, nPatternLen);
		/*
			2001/06/23 Norio Nakatani
			単語単位の検索を試験的に実装。単語はWhereCurrentWord()で判別してますので、
			英単語やC/C++識別子などの検索条件ならヒットします。
		*/

		// 前方検索
		if (eDirection == SearchDirection::Backward) {
			nLinePos = ptSerachBegin.GetY2();
			pDocLine = m_docLineMgr.GetLine(nLinePos);
			LogicInt nNextWordFrom;
			LogicInt nNextWordFrom2;
			LogicInt nNextWordTo2;
			LogicInt nWork;
			nNextWordFrom = ptSerachBegin.GetX2();
			while (pDocLine) {
				if (PrevOrNextWord(nLinePos, nNextWordFrom, &nWork, true, false)) {
					nNextWordFrom = nWork;
					if (WhereCurrentWord(nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL)) {
						size_t nSize = searchWords.size();
						for (size_t iSW=0; iSW<nSize; ++iSW) {
							if (searchWords[iSW].second == nNextWordTo2 - nNextWordFrom2) {
								const wchar_t* pData = pDocLine->GetPtr();	// 2002/2/10 aroka CMemory変更
								// 1 == 大文字小文字の区別
								if ((!searchOption.bLoHiCase && auto_memicmp(&(pData[nNextWordFrom2]), searchWords[iSW].first, searchWords[iSW].second) == 0) ||
									(searchOption.bLoHiCase && auto_memcmp(&(pData[nNextWordFrom2]), searchWords[iSW].first, searchWords[iSW].second) == 0)
								) {
									pMatchRange->SetFromY(nLinePos);	// マッチ行
									pMatchRange->SetToY  (nLinePos);	// マッチ行
									pMatchRange->SetFromX(nNextWordFrom2);						// マッチ位置from
									pMatchRange->SetToX  (pMatchRange->GetFrom().x + searchWords[iSW].second);// マッチ位置to
									nRetVal = 1;
									goto end_of_func;
								}
							}
						}
						continue;
					}
				}
				// 前の行を見に行く
				--nLinePos;
				pDocLine = pDocLine->GetPrevLine();
				if (pDocLine) {
					nNextWordFrom = pDocLine->GetLengthWithEOL() - pDocLine->GetEol().GetLen();
					if (0 > nNextWordFrom) {
						nNextWordFrom = LogicInt(0);
					}
				}
			}
		// 後方検索
		}else {
			nLinePos = ptSerachBegin.GetY2();
			pDocLine = m_docLineMgr.GetLine(nLinePos);
			LogicInt nNextWordFrom = ptSerachBegin.GetX2();
			while (pDocLine) {
				pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				int nMatchLen;
				pszRes = SearchStringWord(pLine, nLineLen, nNextWordFrom, searchWords, searchOption.bLoHiCase, &nMatchLen);
				if (pszRes) {
					pMatchRange->SetFromY(nLinePos);	// マッチ行
					pMatchRange->SetToY  (nLinePos);	// マッチ行
					pMatchRange->SetFromX(LogicInt(pszRes - pLine));						// マッチ位置from
					pMatchRange->SetToX  (pMatchRange->GetFrom().x + nMatchLen);// マッチ位置to
					nRetVal = 1;
					goto end_of_func;
				}
				// 次の行を見に行く
				++nLinePos;
				pDocLine = pDocLine->GetNextLine();
				nNextWordFrom = LogicInt(0);
			}
		}

		nRetVal = 0;
		goto end_of_func;
	// 普通の検索 (正規表現でも単語単位でもない)
	}else {
		const int nPatternLen = pattern.GetLen();
		// 前方検索
		if (eDirection == SearchDirection::Backward) {
			nLinePos = ptSerachBegin.GetY2();
			nHitTo = ptSerachBegin.x;

			nIdxPos = 0;
			pDocLine = m_docLineMgr.GetLine(nLinePos);
			while (pDocLine) {
				pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				nHitPos = -1;
				for (;;) {
					nHitPosOld = nHitPos;
					nIdxPosOld = nIdxPos;
					pszRes = SearchString(
						pLine,
						nLineLen,
						nIdxPos,
						pattern
					);
					if (pszRes) {
						nHitPos = pszRes - pLine;
						nIdxPos = nHitPos + nPatternLen;	// マッチ文字列長進めるように変更 2005.10.28 Karoto
						if (nHitPos >= nHitTo) {
							if (nHitPosOld != -1) {
								pMatchRange->SetFromY(nLinePos);	// マッチ行
								pMatchRange->SetToY  (nLinePos);	// マッチ行
								pMatchRange->SetFromX(nHitPosOld);	// マッチ位置from
 								pMatchRange->SetToX  (nIdxPosOld);	// マッチ位置to
								nRetVal = 1;
								goto end_of_func;
							}else {
								break;
							}
						}
					}else {
						if (nHitPosOld != -1) {
							pMatchRange->SetFromY(nLinePos);	// マッチ行
							pMatchRange->SetToY  (nLinePos);	// マッチ行
							pMatchRange->SetFromX(nHitPosOld);	// マッチ位置from
							pMatchRange->SetToX  (nIdxPosOld);	// マッチ位置to
							nRetVal = 1;
							goto end_of_func;
						}else {
							break;
						}
					}
				}
				--nLinePos;
				pDocLine = pDocLine->GetPrevLine();
				nIdxPos = 0;
				if (pDocLine) {
					nHitTo = pDocLine->GetLengthWithEOL();
				}
			}
			nRetVal = 0;
			goto end_of_func;
		// 後方検索
		}else {
			nIdxPos = ptSerachBegin.x;
			nLinePos = ptSerachBegin.GetY2();
			pDocLine = m_docLineMgr.GetLine(nLinePos);
			while (pDocLine) {
				pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				pszRes = SearchString(
					pLine,
					nLineLen,
					nIdxPos,
					pattern
				);
				if (pszRes) {
					pMatchRange->SetFromY(nLinePos);	// マッチ行
					pMatchRange->SetToY  (nLinePos);	// マッチ行
					pMatchRange->SetFromX(LogicInt(pszRes - pLine));							// マッチ位置from (文字単位)
					pMatchRange->SetToX  (pMatchRange->GetFrom().x + nPatternLen);	// マッチ位置to   (文字単位)
					nRetVal = 1;
					goto end_of_func;
				}
				++nLinePos;
				pDocLine = pDocLine->GetNextLine();
				nIdxPos = 0;
			}
			nRetVal = 0;
			goto end_of_func;
		}
	}
end_of_func:;
#ifdef MEASURE_SEARCH_TIME
	clockEnd = clock();
	TCHAR buf[100] = {0};
	wsprintf(buf, _T("%d"), clockEnd - clockStart);
	::MessageBox(NULL, buf, GSTR_APPNAME, MB_OK);
#endif
	
	return nRetVal;
}


/* 指定範囲のデータを置換(削除 & データを挿入)
  Fromを含む位置からToの直前を含むデータを削除する
  Fromの位置へテキストを挿入する
*/
void SearchAgent::ReplaceData(DocLineReplaceArg* pArg)
{
//	MY_RUNNINGTIMER(runningTimer, "CDocLineMgr::ReplaceData()");

	// 挿入によって増えた行の数
	pArg->nInsLineNum = LogicInt(0);
	// 削除した行の総数
	pArg->nDeletedLineNum = LogicInt(0);
	// 削除されたデータ
	if (pArg->pMemDeleted) {
		pArg->pMemDeleted->clear();
	}
	
	DocLine* pDocLine;
	DocLine* pDocLinePrev;
	DocLine* pDocLineNext;
	int nWorkPos;
	int nWorkLen;
	const wchar_t* pLine;
	int nLineLen;
	LogicInt	nAllLinesOld;
	int			nProgress;
	DocLine::MarkType	markNext;
	//	May 15, 2000
	HWND		hwndCancel = NULL;	//	初期化
	HWND		hwndProgress = NULL;	//	初期化

	pArg->ptNewPos = pArg->delRange.GetFrom();

	// 大量のデータを操作するとき
	DlgCancel*	pDlgCancel = NULL;
	class DlgCancelCloser {
		DlgCancel*& m_pDlg;
	public:
		DlgCancelCloser(DlgCancel*& pDlg): m_pDlg(pDlg) {}
		~DlgCancelCloser() {
			if (m_pDlg) {
				// 進捗ダイアログを表示しない場合と同じ動きになるようにダイアログは遅延破棄する
				// ここで pDlgCancel を delete すると delete から戻るまでの間に
				// ダイアログ破棄 -> 編集画面へフォーカス移動 -> キャレット位置調整
				// まで一気に動くので無効なレイアウト情報参照で異常終了することがある
				m_pDlg->DeleteAsync();	// 自動破棄を遅延実行する	// 2008.05.28 ryoji
			}
		}
	};
	DlgCancelCloser closer(pDlgCancel);
	const LogicInt nDelLines = pArg->delRange.GetTo().y - pArg->delRange.GetFrom().y;
	const LogicInt nEditLines = std::max<LogicInt>(LogicInt(1), nDelLines + LogicInt(pArg->pInsData ? pArg->pInsData->size(): 0));
	if (3000 < nEditLines) {
		// 進捗ダイアログの表示
		pDlgCancel = new DlgCancel;
		if ((hwndCancel = pDlgCancel->DoModeless(::GetModuleHandle(NULL), NULL, IDD_OPERATIONRUNNING))) {
			hwndProgress = ::GetDlgItem(hwndCancel, IDC_PROGRESS);
			Progress_SetRange(hwndProgress, 0, 101);
 			Progress_SetPos(hwndProgress, 0);
		}
	}
	int nProgressOld = 0;

	// バッファを確保
	if (pArg->pMemDeleted) {
		pArg->pMemDeleted->reserve(pArg->delRange.GetTo().y + LogicInt(1) - pArg->delRange.GetFrom().y);
	}

	// 2012.01.10 行内の削除&挿入のときの操作を1つにする
	bool bChangeOneLine = false;	// 行内の挿入
	bool bInsOneLine = false;
	bool bLastEOLReplace = false;	// 「最後改行」を「最後改行」で置換
	if (pArg->pInsData && 0 < pArg->pInsData->size()) {
		const NativeW& memLine = pArg->pInsData->back().memLine;
		int nLen = memLine.GetStringLength();
		const wchar_t* pInsLine = memLine.GetStringPtr();
		if (0 < nLen && WCODE::IsLineDelimiter(pInsLine[nLen - 1], GetDllShareData().common.edit.bEnableExtEol)) {
			// 行挿入
			bLastEOLReplace = true; // 仮。後で修正
		}else {
			if (pArg->pInsData->size() == 1) {
				bChangeOneLine = true; // 「abc\ndef」=>「123」のような置換もtrueなのに注意
			}
		}
	}
	const wchar_t* pInsData = L"";
	int nInsLen = 0;
	int nSetSeq = 0;
	if (bChangeOneLine) {
		nInsLen = pArg->pInsData->back().memLine.GetStringLength();
		pInsData = pArg->pInsData->back().memLine.GetStringPtr();
		nSetSeq = pArg->pInsData->back().nSeq;
	}

	// 現在行の情報を得る
	pDocLine = m_docLineMgr.GetLine(pArg->delRange.GetTo().GetY2());
	int i = pArg->delRange.GetTo().y;
	if (0 < pArg->delRange.GetTo().y && !pDocLine) {
		pDocLine = m_docLineMgr.GetLine(pArg->delRange.GetTo().GetY2() - LogicInt(1));
		--i;
	}
	bool bFirstLine = true;
	bool bSetMark = false;
	// 後ろから処理していく
	for (; i >= pArg->delRange.GetFrom().y && pDocLine; --i) {
		pLine = pDocLine->GetPtr(); // 2002/2/10 aroka CMemory変更
		nLineLen = pDocLine->GetLengthWithEOL(); // 2002/2/10 aroka CMemory変更
		pDocLinePrev = pDocLine->GetPrevLine();
		pDocLineNext = pDocLine->GetNextLine();
		// 現在行の削除開始位置を調べる
		if (i == pArg->delRange.GetFrom().y) {
			nWorkPos = pArg->delRange.GetFrom().x;
		}else {
			nWorkPos = 0;
		}
		// 現在行の削除データ長を調べる
		if (i == pArg->delRange.GetTo().y) {
			nWorkLen = pArg->delRange.GetTo().x - nWorkPos;
		}else {
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory変更
		}

		if (nWorkLen == 0) {
			// 前の行へ
			goto prev_line;
		}
		// 改行も削除するんかぃのぉ・・・？
		if (pDocLine->GetEol() != EolType::None &&
			nWorkPos + nWorkLen > nLineLen - pDocLine->GetEol().GetLen() // 2002/2/10 aroka CMemory変更
		) {
			// 削除する長さに改行も含める
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory変更
		}

		// 行全体の削除
		if (nWorkLen >= nLineLen) { // 2002/2/10 aroka CMemory変更
			// 削除した行の総数
			++(pArg->nDeletedLineNum);
			// 行オブジェクトの削除、リスト変更、行数--
			if (pArg->pMemDeleted) {
				LineData tmp;
				pArg->pMemDeleted->push_back(tmp);
				LineData& delLine = pArg->pMemDeleted->back();
				delLine.memLine.swap(pDocLine->_GetDocLineData()); // DocLine書き換え
				delLine.nSeq = ModifyVisitor().GetLineModifiedSeq(pDocLine);
			}
			m_docLineMgr.DeleteLine(pDocLine);
			pDocLine = NULL;
		// 次の行と連結するような削除
		}else if (nWorkPos + nWorkLen >= nLineLen) { // 2002/2/10 aroka CMemory変更
			if (pArg->pMemDeleted) {
				if (pDocLineNext && pArg->pMemDeleted->size() == 0) {
					// 1行以内の行末削除のときだけ、次の行のseqが保存されないので必要
					// 2014.01.07 最後が改行の範囲を最後が改行のデータで置換した場合を変更
					if (!bLastEOLReplace) {
						pArg->pMemDeleted->emplace_back();
						LineData& delLine =  pArg->pMemDeleted->back();
						delLine.memLine.SetString(L"");
						delLine.nSeq = ModifyVisitor().GetLineModifiedSeq(pDocLineNext);
					}
				}
				pArg->pMemDeleted->emplace_back();
				LineData& delLine = pArg->pMemDeleted->back();
				delLine.memLine.SetString(&pLine[nWorkPos], nWorkLen);
				delLine.nSeq = ModifyVisitor().GetLineModifiedSeq(pDocLine);
			}

			// 次の行がある
			if (pDocLineNext) {
				// 次の行のデータを最後に追加
				// 改行を削除するような置換
				int nNewLen = nWorkPos + pDocLineNext->GetLengthWithEOL() + nInsLen;
				if (nWorkLen <= nWorkPos && nLineLen <= nNewLen + 10) {
					// 行を連結して1行にするような操作の高速化
					// 削除が元データの有効長以下で行の長さが伸びるか少し減る場合reallocを試みる
					static DocLine* pDocLinePrevAccess = NULL;
					static int nAccessCount = 0;
					int nBufferReserve = nNewLen;
					if (pDocLinePrevAccess == pDocLine) {
						if (100 < nAccessCount) {
							if (1000 < nNewLen) {
								int n = 1000;
								while (n < nNewLen) {
									n += n / 5; // 20%づつ伸ばす
								}
								nBufferReserve = n;
							}
						}else {
							++nAccessCount;
						}
					}else {
						pDocLinePrevAccess = pDocLine;
						nAccessCount = 0;
					}
					NativeW& ref = pDocLine->_GetDocLineData();
					ref.AllocStringBuffer(nBufferReserve);
					ref._SetStringLength(nWorkPos);
					ref.AppendString(pInsData, nInsLen);
					ref.AppendNativeData(pDocLineNext->_GetDocLineDataWithEOL());
					pDocLine->SetEol();
				}else {
					NativeW tmp;
					tmp.AllocStringBuffer(nNewLen);
					tmp.AppendString(pLine, nWorkPos);
					tmp.AppendString(pInsData, nInsLen);
					tmp.AppendNativeData(pDocLineNext->_GetDocLineDataWithEOL());
					pDocLine->SetDocLineStringMove(&tmp);
				}
				if (bChangeOneLine) {
					pArg->nInsSeq = ModifyVisitor().GetLineModifiedSeq(pDocLine);
					ModifyVisitor().SetLineModified(pDocLine, nSetSeq);
					if (!bInsOneLine) {
						pArg->ptNewPos.x = pArg->ptNewPos.x + nInsLen;
						bInsOneLine = true;
					}
				}else {
					ModifyVisitor().SetLineModified(pDocLine, pArg->nDelSeq);
					// 削除される行のマーク類を保存
					markNext = pDocLineNext->m_mark;
					bSetMark = true;
				}

				// 次の行 行オブジェクトの削除
				m_docLineMgr.DeleteLine(pDocLineNext);
				pDocLineNext = NULL;

				// 削除した行の総数
				++(pArg->nDeletedLineNum);
			}else {
				// 行内データ削除
				NativeW tmp;
				tmp.SetString(pLine, nWorkPos);
				pDocLine->SetDocLineStringMove(&tmp);
				ModifyVisitor().SetLineModified(pDocLine, pArg->nDelSeq);	// 変更フラグ
			}
		}else {
			// 行内だけの削除
			if (pArg->pMemDeleted) {
				pArg->pMemDeleted->emplace_back();
				LineData& delLine =  pArg->pMemDeleted->back();
				delLine.memLine.SetString(&pLine[nWorkPos], nWorkLen);
				delLine.nSeq = ModifyVisitor().GetLineModifiedSeq(pDocLine);
			}
			{// 20020119 aroka ブロック内に pWork を閉じ込めた
				// 2002/2/10 aroka CMemory変更 何度も GetLength,GetPtr をよばない。
				int nNewLen = nLineLen - nWorkLen + nInsLen;
				int nAfterLen = nLineLen - (nWorkPos + nWorkLen);
				if (1
					&& pDocLine->_GetDocLineData().capacity() * 9 / 10 < nNewLen
					&& nNewLen <= pDocLine->_GetDocLineData().capacity()
				) {
					NativeW& ref = pDocLine->_GetDocLineData();
					WCHAR* pBuf = const_cast<WCHAR*>(ref.GetStringPtr());
					if (nWorkLen != nInsLen) {
						wmemmove(&pBuf[nWorkPos + nInsLen], &pLine[nWorkPos + nWorkLen], nAfterLen);
					}
					wmemcpy(&pBuf[nWorkPos], pInsData, nInsLen);
					ref._SetStringLength(nNewLen);
				}else {
					int nBufferSize = 16;
					if (1000 < nNewLen) {
						nBufferSize = 1000;
						while (nBufferSize < nNewLen) {
							nBufferSize += nBufferSize / 20; // 5%づつ伸ばす
						}
					}
					NativeW tmp;
					tmp.AllocStringBuffer(nBufferSize);
					tmp.AppendString(pLine, nWorkPos);
					tmp.AppendString(pInsData, nInsLen);
					tmp.AppendString(&pLine[nWorkPos + nWorkLen], nAfterLen);
					pDocLine->SetDocLineStringMove(&tmp);
				}
			}
			if (bChangeOneLine) {
				pArg->nInsSeq = ModifyVisitor().GetLineModifiedSeq(pDocLine);
				ModifyVisitor().SetLineModified(pDocLine, nSetSeq);
				pArg->ptNewPos.x = pArg->ptNewPos.x + nInsLen;
				bInsOneLine = true;
				pInsData = L"";
				nInsLen = 0;
			}else {
				ModifyVisitor().SetLineModified(pDocLine, pArg->nDelSeq);
			}
			if (bFirstLine) {
				bLastEOLReplace = false;
			}
		}
		bFirstLine = false;

prev_line:;
		// 直前の行のオブジェクトのポインタ
		pDocLine = pDocLinePrev;
		// 最近参照した行番号と行データ
		--m_docLineMgr.m_nPrevReferLine;
		m_docLineMgr.m_pCodePrevRefer = pDocLine;

		if (hwndCancel) {
			int nLines = pArg->delRange.GetTo().y - i;
			if ((nLines % 32) == 0) {
				nProgress = ::MulDiv(nLines, 100, nEditLines);
				if (nProgressOld != nProgress) {
					nProgressOld = nProgress;
					Progress_SetPos(hwndProgress, nProgress + 1);
					Progress_SetPos(hwndProgress, nProgress);
				}
			}
		}
	}

	if (pArg->pMemDeleted) {
		// 下から格納されているのでひっくり返す
		std::reverse(pArg->pMemDeleted->begin(), pArg->pMemDeleted->end());
	}
	if (bInsOneLine) {
		// 挿入済み
		return;
	}

	// データ挿入処理
	if (!pArg->pInsData || pArg->pInsData->size() == 0) {
		pArg->nInsSeq = 0;
		return;
	}
	nAllLinesOld= m_docLineMgr.GetLineCount();
	pArg->ptNewPos.y = pArg->delRange.GetFrom().y;	// 挿入された部分の次の位置の行
	pArg->ptNewPos.x = 0;	// 挿入された部分の次の位置のデータ位置

	// 挿入データを行終端で区切った行数カウンタ
	pDocLine = m_docLineMgr.GetLine(pArg->delRange.GetFrom().GetY2());

	int nInsSize = pArg->pInsData->size();
	bool bInsertLineMode = false;
	bool bLastInsert = false;
	{
		NativeW& memLine = pArg->pInsData->back().memLine;
		int nLen = memLine.GetStringLength();
		const wchar_t* pInsLine = memLine.GetStringPtr();
		if (0 < nLen && WCODE::IsLineDelimiter(pInsLine[nLen - 1], GetDllShareData().common.edit.bEnableExtEol)) {
			if (pArg->delRange.GetFrom().x == 0) {
				// 挿入データの最後が改行で行頭に挿入するとき、現在行を維持する
				bInsertLineMode = true;
				if (pDocLine && m_docLineMgr.m_pCodePrevRefer == pDocLine) {
					m_docLineMgr.m_pCodePrevRefer = pDocLine->GetPrevLine();
					if (m_docLineMgr.m_pCodePrevRefer) {
						m_docLineMgr.m_nPrevReferLine--;
					}
				}
			}
		}else {
			bLastInsert = true;
			nInsSize--;
		}
	}
	StringRef	prevLine;
	StringRef	nextLine;
	NativeW	memCurLine;
	if (!pDocLine) {
		// ここでNULLが帰ってくるということは、
		// 全テキストの最後の次の行を追加しようとしていることを示す
		pArg->nInsSeq = 0;
	}else {
		// 2002/2/10 aroka 何度も GetPtr を呼ばない
		if (!bInsertLineMode) {
			memCurLine.swap(pDocLine->_GetDocLineData());
			pLine = memCurLine.GetStringPtr(&nLineLen);
			prevLine = StringRef(pLine, pArg->delRange.GetFrom().x);
			nextLine = StringRef(&pLine[pArg->delRange.GetFrom().x], nLineLen - pArg->delRange.GetFrom().x);
			pArg->nInsSeq = ModifyVisitor().GetLineModifiedSeq(pDocLine);
		}else {
			pArg->nInsSeq = 0;
		}
	}
	int nCount;
	for (nCount=0; nCount<nInsSize; ++nCount) {
		NativeW& memLine = (*pArg->pInsData)[nCount].memLine;
#ifdef _DEBUG
		int nLen = memLine.GetStringLength();
		const wchar_t* pInsLine = memLine.GetStringPtr();
		assert( 0 < nLen && WCODE::IsLineDelimiter(pInsLine[nLen - 1], GetDllShareData().common.edit.bEnableExtEol) );
#endif
		{
			if (!pDocLine) {
				DocLine* pDocLineNew = m_docLineMgr.AddNewLine();

				// 挿入データを行終端で区切った行数カウンタ
				if (nCount == 0) {
					NativeW tmp;
					tmp.AllocStringBuffer(prevLine.GetLength() + memLine.GetStringLength());
					tmp.AppendString(prevLine.GetPtr(), prevLine.GetLength());
					tmp.AppendNativeData(memLine);
					pDocLineNew->SetDocLineStringMove(&tmp);
				}else {
					pDocLineNew->SetDocLineStringMove(&memLine);
				}
				ModifyVisitor().SetLineModified(pDocLineNew, (*pArg->pInsData)[nCount].nSeq);
			}else {
				// 挿入データを行終端で区切った行数カウンタ
				if (nCount == 0 && !bInsertLineMode) {
					if (memCurLine.GetStringLength() - prevLine.GetLength() < memCurLine.GetStringLength() / 100
						&& prevLine.GetLength() + memLine.GetStringLength() <= memCurLine.GetStringLength()
						&& memCurLine.capacity() / 2 <= prevLine.GetLength() + memLine.GetStringLength()
					) {
						// 行のうちNextになるのが1%以下で行が短くなるなら再利用する(長い一行を分割する場合の最適化)
						NativeW tmp; // Nextを退避
						tmp.SetString(nextLine.GetPtr(), nextLine.GetLength());
						memCurLine.swap(tmp);
						tmp._SetStringLength(prevLine.GetLength());
						tmp.AppendNativeData(memLine);
						pDocLine->SetDocLineStringMove(&tmp);
						nextLine = StringRef(memCurLine.GetStringPtr(), memCurLine.GetStringLength());
					}else {
						NativeW tmp;
						tmp.AllocStringBuffer(prevLine.GetLength() + memLine.GetStringLength());
						tmp.AppendString(prevLine.GetPtr(), prevLine.GetLength());
						tmp.AppendNativeData(memLine);
						pDocLine->SetDocLineStringMove(&tmp);
					}
					ModifyVisitor().SetLineModified(pDocLine, (*pArg->pInsData)[nCount].nSeq);
					pDocLine = pDocLine->GetNextLine();
				}else {
					DocLine* pDocLineNew = m_docLineMgr.InsertNewLine(pDocLine);	// pDocLineの前に挿入
					pDocLineNew->SetDocLineStringMove(&memLine);
					ModifyVisitor().SetLineModified(pDocLineNew, (*pArg->pInsData)[nCount].nSeq);
				}
			}

			// 挿入データを行終端で区切った行数カウンタ
			++(pArg->ptNewPos.y);	// 挿入された部分の次の位置の行
			if (hwndCancel) {
				if ((nCount % 32) == 0) {
					nProgress = ::MulDiv(nCount + nDelLines, 100, nEditLines);
					if (nProgressOld != nProgress) {
						nProgressOld = nProgress;
						Progress_SetPos(hwndProgress, nProgress + 1);
						Progress_SetPos(hwndProgress, nProgress);
					}
				}
			}
		}
	}
	if (bLastInsert || 0 < nextLine.GetLength()) {
		NativeW mNull;
		StringRef nullStr(L"", 0);
		NativeW& memLine = bLastInsert ? pArg->pInsData->back().memLine : mNull;
		const StringRef& prevLine2 = ((nCount == 0) ? prevLine: nullStr);
		int nSeq = pArg->pInsData->back().nSeq;
		int nLen = memLine.GetStringLength();
		NativeW tmp;
		tmp.AllocStringBuffer(prevLine2.GetLength() + memLine.GetStringLength() + nextLine.GetLength());
		tmp.AppendString(prevLine2.GetPtr(), prevLine2.GetLength());
		tmp.AppendNativeData(memLine);
		tmp.AppendString(nextLine.GetPtr(), nextLine.GetLength());
		if (!pDocLine) {
			DocLine* pDocLineNew = m_docLineMgr.AddNewLine();	// 末尾に追加
			pDocLineNew->SetDocLineStringMove(&tmp);
			pDocLineNew->m_mark = markNext;
			if (!bLastEOLReplace || !bSetMark) {
				ModifyVisitor().SetLineModified(pDocLineNew, nSeq);
			}
			pArg->ptNewPos.x = nLen;	// 挿入された部分の次の位置のデータ位置
		}else {
			if (nCount == 0) {
				// 行の中間に挿入(削除データがなかった。1文字入力など)
			}else {
				// 複数行挿入の最後の行
				pDocLine = m_docLineMgr.InsertNewLine(pDocLine);	// pDocLineの前に挿入
				pDocLine->m_mark = markNext;
			}
			pDocLine->SetDocLineStringMove(&tmp);
			if (!bLastEOLReplace || !bSetMark) {
				ModifyVisitor().SetLineModified(pDocLine, nSeq);
			}
			pArg->ptNewPos.x = prevLine2.GetLength() + nLen;	// 挿入された部分の次の位置のデータ位置
		}
	}
	pArg->nInsLineNum = m_docLineMgr.GetLineCount() - nAllLinesOld;
	return;
}

