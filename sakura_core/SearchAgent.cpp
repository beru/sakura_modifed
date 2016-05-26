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

#ifdef SEARCH_STRING_SUNDAY_QUICK
// SearchStringPattern
// @date 2010.06.22 Moca
inline
int SearchStringPattern::GetMapIndex(wchar_t c)
{
	// ASCII    => 0x000 - 0x0ff
	// ����ȊO => 0x100 - 0x1ff
	return ((c & 0xff00) ? 0x100 : 0 ) | (c & 0xff);
}
#endif

SearchStringPattern::SearchStringPattern()
	: 
	pszKey(NULL),
	pSearchOption(nullptr),
	pRegexp(nullptr),
	pszCaseKeyRef(NULL),
#ifdef SEARCH_STRING_KMP
	pnNextPossArr(NULL),
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	pnUseCharSkipArr(nullptr),
#endif
	pszPatternCase(NULL)
{
}

SearchStringPattern::SearchStringPattern(
	HWND hwnd,
	const wchar_t* pszPattern,
	size_t nPatternLen,
	const SearchOption& searchOption,
	Bregexp* pRegexp
	)
	:
	pszKey(NULL),
	pSearchOption(nullptr),
	pRegexp(nullptr),
	pszCaseKeyRef(NULL),
#ifdef SEARCH_STRING_KMP
	pnNextPossArr(NULL),
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	pnUseCharSkipArr(nullptr),
#endif
	pszPatternCase(NULL)
{
	SetPattern(hwnd, pszPattern, nPatternLen, searchOption, pRegexp);
}

SearchStringPattern::~SearchStringPattern()
{
	Reset();
}

void SearchStringPattern::Reset() {
	pszKey = NULL;
	pszCaseKeyRef = NULL;
	pSearchOption = nullptr;
	pRegexp = nullptr;

	delete[] pszPatternCase;
	pszPatternCase = NULL;
#ifdef SEARCH_STRING_KMP
	delete[] pnNextPossArr;
	pnNextPossArr = NULL;
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	delete[] pnUseCharSkipArr;
	pnUseCharSkipArr = nullptr;
#endif
}

bool SearchStringPattern::SetPattern(
	HWND hwnd,
	const wchar_t* pszPattern,
	size_t nPatternLen,
	const wchar_t* pszPattern2,
	const SearchOption& searchOption,
	Bregexp* regexp
	)
{
	Reset();
	pszCaseKeyRef = pszKey = pszPattern;
	this->nPatternLen = nPatternLen;
	pSearchOption = &searchOption;
	pRegexp = regexp;
	if (pSearchOption->bRegularExp) {
		if (!pRegexp) {
			return false;
		}
		if (!InitRegexp(hwnd, *pRegexp, true)) {
			return false;
		}
		int nFlag = (GetLoHiCase() ? Bregexp::optCaseSensitive : Bregexp::optNothing);
		// �����p�^�[���̃R���p�C��
		if (pszPattern2) {
			if (!pRegexp->Compile( pszPattern, pszPattern2, nFlag )) {
				return false;
			}
		}else {
			if (!pRegexp->Compile(pszPattern, nFlag)) {
				return false;
			}
		}
	}else if (pSearchOption->bWordOnly) {
	}else {
		if (GetIgnoreCase()) {
			pszPatternCase = new wchar_t[nPatternLen + 1];
			pszCaseKeyRef = pszPatternCase;
			// note: ��������,�T���Q�[�g�́u�啶�����������ꎋ�v���Ή�
			for (size_t i=0; i<nPatternLen; ++i) {
				pszPatternCase[i] = (wchar_t)skr_towlower(pszPattern[i]);
			}
			pszPatternCase[nPatternLen] = L'\0';
		}

#ifdef SEARCH_STRING_KMP
	// "ABCDE" => {-1, 0, 0, 0, 0}
	// "AAAAA" => {-1, 0, 1, 2, 3}
	// "AABAA" => {-1, 0, 0, 0, 0}
	// "ABABA" => {-1, 0, 0, 2, 0}
//	if (GetIgnoreCase()) {
		pnNextPossArr = new int[nPatternLen + 1];
		int* next = pnNextPossArr;
		const wchar_t* key = pszPatternCase;
		for (int i=0, k=-1; i<nPatternLen; ++i, ++k) {
			next[i] = k;
			while (-1 < k && key[i] != key[k]) {
				k = next[k];
			}
		}
//	}
#endif

#ifdef SEARCH_STRING_SUNDAY_QUICK
		const size_t BM_MAPSIZE = 0x200;
		// 64KB �����Ȃ��ŁAISO-8859-1 ����ȊO(�) ��2�̏��̂݋L�^����
		// �u���v�Ɓu���v�@�u���v�Ɓu���v�͒l�����L���Ă���
		pnUseCharSkipArr = new int[BM_MAPSIZE];
		for (size_t n=0; n<BM_MAPSIZE; ++n) {
			pnUseCharSkipArr[n] = nPatternLen + 1;
		}
		for (size_t n=0; n<nPatternLen; ++n) {
			const int index = GetMapIndex(pszCaseKeyRef[n]);
			pnUseCharSkipArr[index] = nPatternLen - n;
		}
#endif
	}
	return true;
}


#define toLoHiLower(bLoHiCase, ch) (bLoHiCase? (ch) : skr_towlower(ch))

/*!
	�����񌟍�
	@return ���������ꏊ�̃|�C���^�B������Ȃ�������NULL�B
*/
const wchar_t* SearchAgent::SearchString(
	const wchar_t* pLine,
	size_t nLineLen,
	int nIdxPos,
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

	// ���`�T��
	const int nCompareTo = nLineLen - nPatternLen;	//	Mar. 4, 2001 genta

#if defined(SEARCH_STRING_SUNDAY_QUICK) && !defined(SEARCH_STRING_KMP)
	// SUNDAY_QUICK�̂ݔ�
	if (!bLoHiCase || nPatternLen > 5) {
#if 1
		if (bLoHiCase) {
			for (int nPos=nIdxPos; nPos<=nCompareTo;) {
				int i;
				for (i = 0; i < nPatternLen && (pLine[nPos + i] == pszPattern[i]); ++i) {
				}
				if (i >= nPatternLen) {
					return &pLine[nPos];
				}
				int index = SearchStringPattern::GetMapIndex(pLine[nPos + nPatternLen]);
				nPos += useSkipMap[index];
			}
		}else {
			for (int nPos=nIdxPos; nPos<=nCompareTo;) {
				int i;
				for (i = 0; i < nPatternLen && ((wchar_t)skr_towlower(pLine[nPos + i]) == pszPattern[i]); ++i) {
				}
				if (i >= nPatternLen) {
					return &pLine[nPos];
				}
				int index = SearchStringPattern::GetMapIndex((wchar_t)skr_towlower(pLine[nPos + nPatternLen]));
				nPos += useSkipMap[index];
			}
		}
#else
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
#endif
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
	// �啶������������ʂ��Ȃ��A���A�����ꂪ5�����ȉ��̏ꍇ�͒ʏ�̌������s��
	// �����łȂ��ꍇ��KMP�{SUNDAY QUICK�A���S���Y�����g�����������s��
	if (!bLoHiCase || nPatternLen > 5) {
		const wchar_t pattern0 = pszPattern[0];
		const int* const nextTable = pattern.GetKMPNextTable();
		for (int nPos=nIdxPos; nPos<=nCompareTo;) {
			if (toLoHiLower(bLoHiCase, pLine[nPos]) != pattern0) {
#ifdef SEARCH_STRING_SUNDAY_QUICK
				int index = SearchStringPattern::GetMapIndex((wchar_t)toLoHiLower(bLoHiCase, pLine[nPos + nPatternLen]));
				nPos += useSkipMap[index];
#else
				++nPos;
#endif
				continue;
			}
			// �r���܂ň�v�Ȃ炸�炵�Čp��(KMP)
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
			assert(i == 0); // -1�`�F�b�N
		}
	}else {
#endif
		// �ʏ��
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

// ���������̏��(�L�[������̑S�p�����p���̔z��)�쐬
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

/*!	�P��P�ʂ̒P�ꃊ�X�g�쐬
*/
void SearchAgent::CreateWordList(
	std::vector<std::pair<const wchar_t*, size_t>>& searchWords,
	const wchar_t* pszPattern,
	size_t nPatternLen
	)
{
	searchWords.clear();
	for (size_t pos=0; pos<nPatternLen; ) {
		size_t begin, end; // ������Ɋ܂܂��P��?�� pos����Ƃ������Έʒu�BWhereCurrentWord_2()�̎d�l�ł͋󔒕�������P��Ɋ܂܂��B
		if (WordParse::WhereCurrentWord_2(pszPattern+pos, nPatternLen-pos, 0, &begin, &end, nullptr, nullptr)
			&& begin == 0 && begin < end
		) {
			if (!WCODE::IsWordDelimiter(pszPattern[pos])) {
				// pszPattern[pos]...pszPattern[pos + end] ��������Ɋ܂܂��P��B
				searchWords.emplace_back(pszPattern + pos, end);
			}
			pos += end;
		}else {
			pos += t_max((size_t)1, NativeW::GetSizeOfChar(pszPattern, nPatternLen, pos));
		}
	}
}


/*!	�P��P�ʌ���
*/
const wchar_t* SearchAgent::SearchStringWord(
	const wchar_t*	pLine,
	size_t			nLineLen,
	size_t			nIdxPos,
	const std::vector<std::pair<const wchar_t*, size_t>>& searchWords,
	bool	 bLoHiCase,
	int*	 pnMatchLen
	)
{
	size_t nNextWordFrom = nIdxPos;
	size_t nNextWordFrom2;
	size_t nNextWordTo2;
	// �������d�����邯�Ǖ��򏜋�
	size_t nSize = searchWords.size();
	if (bLoHiCase) {
		while (WordParse::WhereCurrentWord_2(pLine, nLineLen, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2, nullptr, nullptr)) {
			for (size_t iSW=0; iSW<nSize; ++iSW) {
				auto& searchWord = searchWords[iSW];
				ASSERT_GE(nNextWordTo2, nNextWordFrom2);
				if (searchWord.second == nNextWordTo2 - nNextWordFrom2) {
					if (auto_memcmp(&(pLine[nNextWordFrom2]), searchWord.first, searchWord.second) == 0) {
						*pnMatchLen = searchWord.second;
						return &pLine[nNextWordFrom2];
					}
				}
			}
			if (!WordParse::SearchNextWordPosition(pLine, nLineLen, nNextWordFrom, &nNextWordFrom, false)) {
				break;	//	���̒P�ꂪ�����B
			}
		}
	}else {
		while (WordParse::WhereCurrentWord_2(pLine, nLineLen, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2, nullptr, nullptr)) {
			for (size_t iSW=0; iSW<nSize; ++iSW) {
				auto& searchWord = searchWords[iSW];
				if (searchWord.second == nNextWordTo2 - nNextWordFrom2) {
					if (auto_memicmp(&(pLine[nNextWordFrom2]), searchWord.first, searchWord.second) == 0) {
						*pnMatchLen = searchWord.second;
						return &pLine[nNextWordFrom2];
					}
				}
			}
			if (!WordParse::SearchNextWordPosition(pLine, nLineLen, nNextWordFrom, &nNextWordFrom, false)) {
				break;	//	���̒P�ꂪ�����B
			}
		}
	}
	*pnMatchLen = 0;
	return NULL;
}


// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
// 2001/06/23 N.Nakatani WhereCurrentWord()�ύX WhereCurrentWord_2���R�[������悤�ɂ���
bool SearchAgent::WhereCurrentWord(
	size_t	nLineNum,
	size_t	nIdx,
	size_t*	pnIdxFrom,
	size_t*	pnIdxTo,
	NativeW* pcmcmWord,
	NativeW* pcmcmWordLeft
	)
{
	*pnIdxFrom = nIdx;
	*pnIdxTo = nIdx;
	
	const DocLine* pDocLine = docLineMgr.GetLine(nLineNum);
	if (!pDocLine) {
		return false;
	}
	
	size_t nLineLen;
	const wchar_t*	pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
	
	// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
	return WordParse::WhereCurrentWord_2(pLine, nLineLen, nIdx, pnIdxFrom, pnIdxTo, pcmcmWord, pcmcmWordLeft);
}


// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
bool SearchAgent::PrevOrNextWord(
	size_t	nLineNum,		// �s��
	size_t	nIdx,			// ����
	size_t*	pnColumnNew,	// ���������ʒu
	bool	bLeft,			// true : �O���i���j�֌������Bfalse : ����i�E�j�֌������B
	bool	bStopsBothEnds	// �P��̗��[�Ŏ~�܂�
	)
{
	using namespace WCODE;
	
	const DocLine* pDocLine = docLineMgr.GetLine(nLineNum);
	if (!pDocLine) {
		return false;
	}
	
	size_t nLineLen;
	const wchar_t*	pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
	
	// ABC D[EOF]�ƂȂ��Ă����Ƃ��ɁAD�̌��ɃJ�[�\�������킹�A�P��̍��[�Ɉړ�����ƁAA�ɃJ�[�\���������o�O�C���BYAZAKI
	if (nIdx >= nLineLen) {
		if (bLeft && nIdx == nLineLen) {
		}else {
			// 2011.12.26 EOF���E�֍s�����Ƃ���Ƃ���false��Ԃ��悤��
			// nIdx = nLineLen - LogicInt(1);
			return false;
		}
	}
	// ���݈ʒu�̕����̎�ނɂ���Ă͑I��s�\
	if (!bLeft && WCODE::IsLineDelimiter(pLine[nIdx], GetDllShareData().common.edit.bEnableExtEol)) {
		return false;
	}
	// �O�̒P�ꂩ�H���̒P�ꂩ�H
	if (bLeft) {
		// ���݈ʒu�̕����̎�ނ𒲂ׂ�
		ECharKind	nCharKind = WordParse::WhatKindOfChar(pLine, nLineLen, nIdx);
		if (nIdx == 0) {
			return false;
		}

		// ������ނ��ς��܂őO���փT�[�`
		// �󔒂ƃ^�u�͖�������
		int			nCount = 0;
		int	nIdxNext = nIdx;
		ptrdiff_t nCharChars = &pLine[nIdxNext] - NativeW::GetCharPrev(pLine, nLineLen, &pLine[nIdxNext]);
		while (nCharChars > 0) {
			int nIdxNextPrev = nIdxNext;
			nIdxNext -= nCharChars;
			ECharKind nCharKindNext = WordParse::WhatKindOfChar(pLine, nLineLen, nIdxNext);
			ECharKind nCharKindMerge = WordParse::WhatKindOfTwoChars(nCharKindNext, nCharKind);
			if (nCharKindMerge == CK_NULL) {
				// �T�[�`�J�n�ʒu�̕������󔒂܂��̓^�u�̏ꍇ
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
			nCharChars = &pLine[nIdxNext] - NativeW::GetCharPrev(pLine, nLineLen, &pLine[nIdxNext]);
			++nCount;
		}
		*pnColumnNew = nIdxNext;
	}else {
		WordParse::SearchNextWordPosition(pLine, nLineLen, nIdx, pnColumnNew, bStopsBothEnds);
	}
	return true;
}


/*! �P�ꌟ��
	@date 2003.05.22 ����� �s�������Ȃǌ�����
	@date 2005.11.26 ����� \r��.��\r\n�Ƀq�b�g���Ȃ��悤��
*/
// ������Ȃ��ꍇ�͂O��Ԃ�
int SearchAgent::SearchWord(
	Point ptSerachBegin,	// �����J�n�ʒu
	SearchDirection eDirection,		// ��������
	Range* pMatchRange,	// [out] �}�b�`�͈́B���W�b�N�P�ʁB
	const SearchStringPattern& pattern			// �����p�^�[��
	)
{
	DocLine*	pDocLine;
	int	nLinePos;
	int	nIdxPos;
	int	nIdxPosOld;
	const wchar_t*	pLine;
	size_t nLineLen;
	const wchar_t*	pszRes;
	int	nHitTo;
	ptrdiff_t	nHitPos;
	int	nHitPosOld;
	int			nRetVal = 0;
	const SearchOption&	searchOption = pattern.GetSearchOption();
	Bregexp*	pRegexp = pattern.GetRegexp();
#ifdef MEASURE_SEARCH_TIME
	long clockStart, clockEnd;
	clockStart = clock();
#endif

	// ���K�\��
	if (searchOption.bRegularExp) {
		nLinePos = ptSerachBegin.y;		// �����s�������J�n�s
		pDocLine = docLineMgr.GetLine(nLinePos);
		// �O������
		if (eDirection == SearchDirection::Backward) {
			//
			// �O��(��)����(���K�\��)
			//
			nHitTo = ptSerachBegin.x;				// �����J�n�ʒu
			nIdxPos = 0;
			while (pDocLine) {
				pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				nHitPos = -1;	// -1:���̍s�Ń}�b�`�ʒu�Ȃ�
				for (;;) {
					nHitPosOld = nHitPos;
					nIdxPosOld = nIdxPos;
					// �����O�Ń}�b�`�����̂ŁA���̈ʒu�ōēx�}�b�`���Ȃ��悤�ɁA�P�����i�߂�
					if (nIdxPos == nHitPos) {
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nIdxPos += (NativeW::GetSizeOfChar(pLine, nLineLen, nIdxPos) == 2 ? 2 : 1);
					}
					if (1
						&& nIdxPos <= pDocLine->GetLengthWithoutEOL() 
						&& pRegexp->Match(pLine, nLineLen, nIdxPos)
					) {
						// �����Ƀ}�b�`�����I
						nHitPos = pRegexp->GetIndex();
						nIdxPos = pRegexp->GetLastIndex();
						if (nHitPos >= nHitTo) {
							// �}�b�`�����̂́A�J�[�\���ʒu�ȍ~������
							// ���łɃ}�b�`�����ʒu������΁A�����Ԃ��A�Ȃ���ΑO�̍s��
							break;
						}
					}else {
						// �}�b�`���Ȃ�����
						// ���łɃ}�b�`�����ʒu������΁A�����Ԃ��A�Ȃ���ΑO�̍s��
						break;
					}
				}

				if (nHitPosOld != -1) {
					// ���̍s�Ń}�b�`�����ʒu�����݂���̂ŁA���̍s�Ō����I��
					pMatchRange->SetFromX(nHitPosOld);	// �}�b�`�ʒufrom
					pMatchRange->SetToX  (nIdxPosOld);	// �}�b�`�ʒuto
					break;
				}else {
					// ���̍s�Ń}�b�`�����ʒu�����݂��Ȃ��̂ŁA�O�̍s��������
					--nLinePos;
					pDocLine = pDocLine->GetPrevLine();
					nIdxPos = 0;
					if (pDocLine) {
						nHitTo = pDocLine->GetLengthWithEOL() + 1;		// �O�̍s��NULL����(\0)�ɂ��}�b�`�����邽�߂�+1 2003.05.16 ����� 
					}
				}
			}
		// �������
		}else {
			//
			// �������(���K�\��)
			//
			nIdxPos = ptSerachBegin.x;
			while (pDocLine) {
				pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				if (1
					&& nIdxPos <= pDocLine->GetLengthWithoutEOL() 
					&& pRegexp->Match(pLine, nLineLen, nIdxPos)
				) {
					// �}�b�`����
					pMatchRange->SetFromX(pRegexp->GetIndex());			// �}�b�`�ʒufrom
					pMatchRange->SetToX  (pRegexp->GetLastIndex());		// �}�b�`�ʒuto
					break;
				}
				++nLinePos;
				pDocLine = pDocLine->GetNextLine();
				nIdxPos = 0;
			}
		}
		//
		// ���K�\�������̌㏈��
		if (pDocLine) {
			// �}�b�`�����s������
			pMatchRange->SetFromY(nLinePos); // �}�b�`�s
			pMatchRange->SetToY  (nLinePos); // �}�b�`�s
			nRetVal = 1;
			// ���C�A�E�g�s�ł͉��s�������̈ʒu��\���ł��Ȃ����߁A�}�b�`�J�n�ʒu��␳
			if (pMatchRange->GetFrom().x > pDocLine->GetLengthWithoutEOL()) {
				// \r\n���s����\n�Ƀ}�b�`����ƒu���ł��Ȃ��s��ƂȂ邽��
				// ���s�������Ń}�b�`�����ꍇ�A���s�����̎n�߂���}�b�`�������Ƃɂ���
				pMatchRange->SetFromX(pDocLine->GetLengthWithoutEOL());
			}
		}
	// �P��̂݌���
	}else if (searchOption.bWordOnly) {
		// �������P��ɕ������� searchWords�Ɋi�[����B
		const wchar_t* pszPattern = pattern.GetKey();
		const int	nPatternLen = pattern.GetLen();
		std::vector<std::pair<const wchar_t*, size_t>> searchWords; // �P��̊J�n�ʒu�ƒ����̔z��B
		CreateWordList(searchWords, pszPattern, nPatternLen);
		/*
			2001/06/23 Norio Nakatani
			�P��P�ʂ̌����������I�Ɏ����B�P���WhereCurrentWord()�Ŕ��ʂ��Ă܂��̂ŁA
			�p�P���C/C++���ʎq�Ȃǂ̌��������Ȃ�q�b�g���܂��B
		*/

		// �O������
		if (eDirection == SearchDirection::Backward) {
			nLinePos = ptSerachBegin.y;
			pDocLine = docLineMgr.GetLine(nLinePos);
			size_t nNextWordFrom;
			size_t nNextWordFrom2;
			size_t nNextWordTo2;
			size_t nWork;
			nNextWordFrom = ptSerachBegin.x;
			while (pDocLine) {
				if (PrevOrNextWord(nLinePos, nNextWordFrom, &nWork, true, false)) {
					nNextWordFrom = nWork;
					if (WhereCurrentWord(nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , nullptr, nullptr)) {
						size_t nSize = searchWords.size();
						for (size_t iSW=0; iSW<nSize; ++iSW) {
							auto& searchWord = searchWords[iSW];
							if (searchWord.second == nNextWordTo2 - nNextWordFrom2) {
								const wchar_t* pData = pDocLine->GetPtr();	// 2002/2/10 aroka CMemory�ύX
								// 1 == �啶���������̋��
								if ((!searchOption.bLoHiCase && auto_memicmp(&(pData[nNextWordFrom2]), searchWord.first, searchWord.second) == 0) ||
									(searchOption.bLoHiCase && auto_memcmp(&(pData[nNextWordFrom2]), searchWord.first, searchWord.second) == 0)
								) {
									pMatchRange->SetFromY(nLinePos);	// �}�b�`�s
									pMatchRange->SetToY  (nLinePos);	// �}�b�`�s
									pMatchRange->SetFromX(nNextWordFrom2);						// �}�b�`�ʒufrom
									pMatchRange->SetToX  (pMatchRange->GetFrom().x + searchWord.second);// �}�b�`�ʒuto
									nRetVal = 1;
									goto end_of_func;
								}
							}
						}
						continue;
					}
				}
				// �O�̍s�����ɍs��
				--nLinePos;
				pDocLine = pDocLine->GetPrevLine();
				if (pDocLine) {
					nNextWordFrom = pDocLine->GetLengthWithEOL() - pDocLine->GetEol().GetLen();
					if (0 > nNextWordFrom) {
						nNextWordFrom = 0;
					}
				}
			}
		// �������
		}else {
			nLinePos = ptSerachBegin.y;
			pDocLine = docLineMgr.GetLine(nLinePos);
			int nNextWordFrom = ptSerachBegin.x;
			while (pDocLine) {
				pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				int nMatchLen;
				pszRes = SearchStringWord(pLine, nLineLen, nNextWordFrom, searchWords, searchOption.bLoHiCase, &nMatchLen);
				if (pszRes) {
					pMatchRange->SetFromY(nLinePos);	// �}�b�`�s
					pMatchRange->SetToY  (nLinePos);	// �}�b�`�s
					pMatchRange->SetFromX(pszRes - pLine);						// �}�b�`�ʒufrom
					pMatchRange->SetToX  (pMatchRange->GetFrom().x + nMatchLen);// �}�b�`�ʒuto
					nRetVal = 1;
					goto end_of_func;
				}
				// ���̍s�����ɍs��
				++nLinePos;
				pDocLine = pDocLine->GetNextLine();
				nNextWordFrom = 0;
			}
		}

		nRetVal = 0;
		goto end_of_func;
	// ���ʂ̌��� (���K�\���ł��P��P�ʂł��Ȃ�)
	}else {
		const int nPatternLen = pattern.GetLen();
		// �O������
		if (eDirection == SearchDirection::Backward) {
			nLinePos = ptSerachBegin.y;
			nHitTo = ptSerachBegin.x;

			nIdxPos = 0;
			pDocLine = docLineMgr.GetLine(nLinePos);
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
						nIdxPos = nHitPos + nPatternLen;	// �}�b�`�����񒷐i�߂�悤�ɕύX 2005.10.28 Karoto
						if (nHitPos >= nHitTo) {
							if (nHitPosOld != -1) {
								pMatchRange->SetFromY(nLinePos);	// �}�b�`�s
								pMatchRange->SetToY  (nLinePos);	// �}�b�`�s
								pMatchRange->SetFromX(nHitPosOld);	// �}�b�`�ʒufrom
 								pMatchRange->SetToX  (nIdxPosOld);	// �}�b�`�ʒuto
								nRetVal = 1;
								goto end_of_func;
							}else {
								break;
							}
						}
					}else {
						if (nHitPosOld != -1) {
							pMatchRange->SetFromY(nLinePos);	// �}�b�`�s
							pMatchRange->SetToY  (nLinePos);	// �}�b�`�s
							pMatchRange->SetFromX(nHitPosOld);	// �}�b�`�ʒufrom
							pMatchRange->SetToX  (nIdxPosOld);	// �}�b�`�ʒuto
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
		// �������
		}else {
			nIdxPos = ptSerachBegin.x;
			nLinePos = ptSerachBegin.y;
			pDocLine = docLineMgr.GetLine(nLinePos);
			while (pDocLine) {
				pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				pszRes = SearchString(
					pLine,
					nLineLen,
					nIdxPos,
					pattern
				);
				if (pszRes) {
					pMatchRange->SetFromY(nLinePos);	// �}�b�`�s
					pMatchRange->SetToY  (nLinePos);	// �}�b�`�s
					pMatchRange->SetFromX(pszRes - pLine);							// �}�b�`�ʒufrom (�����P��)
					pMatchRange->SetToX  (pMatchRange->GetFrom().x + nPatternLen);	// �}�b�`�ʒuto   (�����P��)
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


/* �w��͈͂̃f�[�^��u��(�폜 & �f�[�^��}��)
  From���܂ވʒu����To�̒��O���܂ރf�[�^���폜����
  From�̈ʒu�փe�L�X�g��}������
*/
void SearchAgent::ReplaceData(DocLineReplaceArg* pArg)
{
//	MY_RUNNINGTIMER(runningTimer, "CDocLineMgr::ReplaceData()");

	// �}���ɂ���đ������s�̐�
	pArg->nInsLineNum = 0;
	// �폜�����s�̑���
	pArg->nDeletedLineNum = 0;
	// �폜���ꂽ�f�[�^
	if (pArg->pMemDeleted) {
		pArg->pMemDeleted->clear();
	}
	
	DocLine* pDocLine;
	DocLine* pDocLinePrev;
	DocLine* pDocLineNext;
	int nWorkPos;
	int nWorkLen;
	const wchar_t* pLine;
	size_t nLineLen;
	int	nAllLinesOld;
	int nProgress;
	DocLine::MarkType	markNext;
	//	May 15, 2000
	HWND		hwndCancel = NULL;	//	������
	HWND		hwndProgress = NULL;	//	������

	pArg->ptNewPos = pArg->delRange.GetFrom();

	// ��ʂ̃f�[�^�𑀍삷��Ƃ�
	DlgCancel*	pDlgCancel = nullptr;
	class DlgCancelCloser {
		DlgCancel*& pDlg;
	public:
		DlgCancelCloser(DlgCancel*& pDlg): pDlg(pDlg) {}
		~DlgCancelCloser() {
			if (pDlg) {
				// �i���_�C�A���O��\�����Ȃ��ꍇ�Ɠ��������ɂȂ�悤�Ƀ_�C�A���O�͒x���j������
				// ������ pDlgCancel �� delete ����� delete ����߂�܂ł̊Ԃ�
				// �_�C�A���O�j�� -> �ҏW��ʂփt�H�[�J�X�ړ� -> �L�����b�g�ʒu����
				// �܂ň�C�ɓ����̂Ŗ����ȃ��C�A�E�g���Q�Ƃňُ�I�����邱�Ƃ�����
				pDlg->DeleteAsync();	// �����j����x�����s����	// 2008.05.28 ryoji
			}
		}
	};
	DlgCancelCloser closer(pDlgCancel);
	const int nDelLines = pArg->delRange.GetTo().y - pArg->delRange.GetFrom().y;
	const int nEditLines = std::max(1, nDelLines + (pArg->pInsData ? (int)pArg->pInsData->size(): 0));
	if (3000 < nEditLines) {
		// �i���_�C�A���O�̕\��
		pDlgCancel = new DlgCancel;
		if ((hwndCancel = pDlgCancel->DoModeless(::GetModuleHandle(NULL), NULL, IDD_OPERATIONRUNNING))) {
			hwndProgress = ::GetDlgItem(hwndCancel, IDC_PROGRESS);
			Progress_SetRange(hwndProgress, 0, 101);
 			Progress_SetPos(hwndProgress, 0);
		}
	}
	int nProgressOld = 0;

	// �o�b�t�@���m��
	if (pArg->pMemDeleted) {
		pArg->pMemDeleted->reserve(pArg->delRange.GetTo().y + 1 - pArg->delRange.GetFrom().y);
	}

	// 2012.01.10 �s���̍폜&�}���̂Ƃ��̑����1�ɂ���
	bool bChangeOneLine = false;	// �s���̑}��
	bool bInsOneLine = false;
	bool bLastEOLReplace = false;	// �u�Ō���s�v���u�Ō���s�v�Œu��
	if (pArg->pInsData && 0 < pArg->pInsData->size()) {
		const NativeW& memLine = pArg->pInsData->back().memLine;
		size_t nLen = memLine.GetStringLength();
		const wchar_t* pInsLine = memLine.GetStringPtr();
		if (0 < nLen && WCODE::IsLineDelimiter(pInsLine[nLen - 1], GetDllShareData().common.edit.bEnableExtEol)) {
			// �s�}��
			bLastEOLReplace = true; // ���B��ŏC��
		}else {
			if (pArg->pInsData->size() == 1) {
				bChangeOneLine = true; // �uabc\ndef�v=>�u123�v�̂悤�Ȓu����true�Ȃ̂ɒ���
			}
		}
	}
	const wchar_t* pInsData = L"";
	size_t nInsLen = 0;
	int nSetSeq = 0;
	if (bChangeOneLine) {
		nInsLen = pArg->pInsData->back().memLine.GetStringLength();
		pInsData = pArg->pInsData->back().memLine.GetStringPtr();
		nSetSeq = pArg->pInsData->back().nSeq;
	}

	// ���ݍs�̏��𓾂�
	pDocLine = docLineMgr.GetLine(pArg->delRange.GetTo().y);
	int i = pArg->delRange.GetTo().y;
	if (0 < pArg->delRange.GetTo().y && !pDocLine) {
		pDocLine = docLineMgr.GetLine(pArg->delRange.GetTo().y - 1);
		--i;
	}
	bool bFirstLine = true;
	bool bSetMark = false;
	// ��납�珈�����Ă���
	for (; i >= pArg->delRange.GetFrom().y && pDocLine; --i) {
		pLine = pDocLine->GetPtr(); // 2002/2/10 aroka CMemory�ύX
		nLineLen = pDocLine->GetLengthWithEOL(); // 2002/2/10 aroka CMemory�ύX
		pDocLinePrev = pDocLine->GetPrevLine();
		pDocLineNext = pDocLine->GetNextLine();
		// ���ݍs�̍폜�J�n�ʒu�𒲂ׂ�
		if (i == pArg->delRange.GetFrom().y) {
			nWorkPos = pArg->delRange.GetFrom().x;
		}else {
			nWorkPos = 0;
		}
		// ���ݍs�̍폜�f�[�^���𒲂ׂ�
		if (i == pArg->delRange.GetTo().y) {
			nWorkLen = pArg->delRange.GetTo().x - nWorkPos;
		}else {
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory�ύX
		}

		if (nWorkLen == 0) {
			// �O�̍s��
			goto prev_line;
		}
		// ���s���폜����񂩂��̂��E�E�E�H
		if (pDocLine->GetEol() != EolType::None &&
			nWorkPos + nWorkLen > nLineLen - pDocLine->GetEol().GetLen() // 2002/2/10 aroka CMemory�ύX
		) {
			// �폜���钷���ɉ��s���܂߂�
			nWorkLen = nLineLen - nWorkPos; // 2002/2/10 aroka CMemory�ύX
		}

		// �s�S�̂̍폜
		if (nWorkLen >= nLineLen) { // 2002/2/10 aroka CMemory�ύX
			// �폜�����s�̑���
			++(pArg->nDeletedLineNum);
			// �s�I�u�W�F�N�g�̍폜�A���X�g�ύX�A�s��--
			if (pArg->pMemDeleted) {
				LineData tmp;
				pArg->pMemDeleted->push_back(tmp);
				LineData& delLine = pArg->pMemDeleted->back();
				delLine.memLine.swap(pDocLine->_GetDocLineData()); // DocLine��������
				delLine.nSeq = ModifyVisitor().GetLineModifiedSeq(pDocLine);
			}
			docLineMgr.DeleteLine(pDocLine);
			pDocLine = nullptr;
		// ���̍s�ƘA������悤�ȍ폜
		}else if (nWorkPos + nWorkLen >= nLineLen) { // 2002/2/10 aroka CMemory�ύX
			if (pArg->pMemDeleted) {
				if (pDocLineNext && pArg->pMemDeleted->size() == 0) {
					// 1�s�ȓ��̍s���폜�̂Ƃ������A���̍s��seq���ۑ�����Ȃ��̂ŕK�v
					// 2014.01.07 �Ōオ���s�͈̔͂��Ōオ���s�̃f�[�^�Œu�������ꍇ��ύX
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

			// ���̍s������
			if (pDocLineNext) {
				// ���̍s�̃f�[�^���Ō�ɒǉ�
				// ���s���폜����悤�Ȓu��
				int nNewLen = nWorkPos + pDocLineNext->GetLengthWithEOL() + nInsLen;
				if (nWorkLen <= nWorkPos && nLineLen <= nNewLen + 10) {
					// �s��A������1�s�ɂ���悤�ȑ���̍�����
					// �폜�����f�[�^�̗L�����ȉ��ōs�̒������L�т邩��������ꍇrealloc�����݂�
					static DocLine* pDocLinePrevAccess = nullptr;
					static int nAccessCount = 0;
					int nBufferReserve = nNewLen;
					if (pDocLinePrevAccess == pDocLine) {
						if (100 < nAccessCount) {
							if (1000 < nNewLen) {
								int n = 1000;
								while (n < nNewLen) {
									n += n / 5; // 20%�ÂL�΂�
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
					// �폜�����s�̃}�[�N�ނ�ۑ�
					markNext = pDocLineNext->mark;
					bSetMark = true;
				}

				// ���̍s �s�I�u�W�F�N�g�̍폜
				docLineMgr.DeleteLine(pDocLineNext);
				pDocLineNext = nullptr;

				// �폜�����s�̑���
				++(pArg->nDeletedLineNum);
			}else {
				// �s���f�[�^�폜
				NativeW tmp;
				tmp.SetString(pLine, nWorkPos);
				pDocLine->SetDocLineStringMove(&tmp);
				ModifyVisitor().SetLineModified(pDocLine, pArg->nDelSeq);	// �ύX�t���O
			}
		}else {
			// �s�������̍폜
			if (pArg->pMemDeleted) {
				pArg->pMemDeleted->emplace_back();
				LineData& delLine =  pArg->pMemDeleted->back();
				delLine.memLine.SetString(&pLine[nWorkPos], nWorkLen);
				delLine.nSeq = ModifyVisitor().GetLineModifiedSeq(pDocLine);
			}
			{// 20020119 aroka �u���b�N���� pWork ������߂�
				// 2002/2/10 aroka CMemory�ύX ���x�� GetLength,GetPtr ����΂Ȃ��B
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
							nBufferSize += nBufferSize / 20; // 5%�ÂL�΂�
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
		// ���O�̍s�̃I�u�W�F�N�g�̃|�C���^
		pDocLine = pDocLinePrev;
		// �ŋߎQ�Ƃ����s�ԍ��ƍs�f�[�^
		--docLineMgr.nPrevReferLine;
		docLineMgr.pCodePrevRefer = pDocLine;

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
		// ������i�[����Ă���̂łЂ�����Ԃ�
		std::reverse(pArg->pMemDeleted->begin(), pArg->pMemDeleted->end());
	}
	if (bInsOneLine) {
		// �}���ς�
		return;
	}

	// �f�[�^�}������
	if (!pArg->pInsData || pArg->pInsData->size() == 0) {
		pArg->nInsSeq = 0;
		return;
	}
	nAllLinesOld= docLineMgr.GetLineCount();
	pArg->ptNewPos.y = pArg->delRange.GetFrom().y;	// �}�����ꂽ�����̎��̈ʒu�̍s
	pArg->ptNewPos.x = 0;	// �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu

	// �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^
	pDocLine = docLineMgr.GetLine(pArg->delRange.GetFrom().y);

	size_t nInsSize = pArg->pInsData->size();
	bool bInsertLineMode = false;
	bool bLastInsert = false;
	{
		NativeW& memLine = pArg->pInsData->back().memLine;
		size_t nLen = memLine.GetStringLength();
		const wchar_t* pInsLine = memLine.GetStringPtr();
		if (0 < nLen && WCODE::IsLineDelimiter(pInsLine[nLen - 1], GetDllShareData().common.edit.bEnableExtEol)) {
			if (pArg->delRange.GetFrom().x == 0) {
				// �}���f�[�^�̍Ōオ���s�ōs���ɑ}������Ƃ��A���ݍs���ێ�����
				bInsertLineMode = true;
				if (pDocLine && docLineMgr.pCodePrevRefer == pDocLine) {
					docLineMgr.pCodePrevRefer = pDocLine->GetPrevLine();
					if (docLineMgr.pCodePrevRefer) {
						docLineMgr.nPrevReferLine--;
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
		// ������NULL���A���Ă���Ƃ������Ƃ́A
		// �S�e�L�X�g�̍Ō�̎��̍s��ǉ����悤�Ƃ��Ă��邱�Ƃ�����
		pArg->nInsSeq = 0;
	}else {
		// 2002/2/10 aroka ���x�� GetPtr ���Ă΂Ȃ�
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
	size_t nCount;
	for (nCount=0; nCount<nInsSize; ++nCount) {
		NativeW& memLine = (*pArg->pInsData)[nCount].memLine;
#ifdef _DEBUG
		size_t nLen = memLine.GetStringLength();
		const wchar_t* pInsLine = memLine.GetStringPtr();
		assert( 0 < nLen && WCODE::IsLineDelimiter(pInsLine[nLen - 1], GetDllShareData().common.edit.bEnableExtEol) );
#endif
		{
			if (!pDocLine) {
				DocLine* pDocLineNew = docLineMgr.AddNewLine();

				// �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^
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
				// �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^
				if (nCount == 0 && !bInsertLineMode) {
					if (memCurLine.GetStringLength() - prevLine.GetLength() < memCurLine.GetStringLength() / 100
						&& prevLine.GetLength() + memLine.GetStringLength() <= memCurLine.GetStringLength()
						&& memCurLine.capacity() / 2 <= prevLine.GetLength() + memLine.GetStringLength()
					) {
						// �s�̂���Next�ɂȂ�̂�1%�ȉ��ōs���Z���Ȃ�Ȃ�ė��p����(������s�𕪊�����ꍇ�̍œK��)
						NativeW tmp; // Next��ޔ�
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
					DocLine* pDocLineNew = docLineMgr.InsertNewLine(pDocLine);	// pDocLine�̑O�ɑ}��
					pDocLineNew->SetDocLineStringMove(&memLine);
					ModifyVisitor().SetLineModified(pDocLineNew, (*pArg->pInsData)[nCount].nSeq);
				}
			}

			// �}���f�[�^���s�I�[�ŋ�؂����s���J�E���^
			++(pArg->ptNewPos.y);	// �}�����ꂽ�����̎��̈ʒu�̍s
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
		size_t nLen = memLine.GetStringLength();
		NativeW tmp;
		tmp.AllocStringBuffer(prevLine2.GetLength() + memLine.GetStringLength() + nextLine.GetLength());
		tmp.AppendString(prevLine2.GetPtr(), prevLine2.GetLength());
		tmp.AppendNativeData(memLine);
		tmp.AppendString(nextLine.GetPtr(), nextLine.GetLength());
		if (!pDocLine) {
			DocLine* pDocLineNew = docLineMgr.AddNewLine();	// �����ɒǉ�
			pDocLineNew->SetDocLineStringMove(&tmp);
			pDocLineNew->mark = markNext;
			if (!bLastEOLReplace || !bSetMark) {
				ModifyVisitor().SetLineModified(pDocLineNew, nSeq);
			}
			pArg->ptNewPos.x = nLen;	// �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu
		}else {
			if (nCount == 0) {
				// �s�̒��Ԃɑ}��(�폜�f�[�^���Ȃ������B1�������͂Ȃ�)
			}else {
				// �����s�}���̍Ō�̍s
				pDocLine = docLineMgr.InsertNewLine(pDocLine);	// pDocLine�̑O�ɑ}��
				pDocLine->mark = markNext;
			}
			pDocLine->SetDocLineStringMove(&tmp);
			if (!bLastEOLReplace || !bSetMark) {
				ModifyVisitor().SetLineModified(pDocLine, nSeq);
			}
			pArg->ptNewPos.x = prevLine2.GetLength() + nLen;	// �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu
		}
	}
	pArg->nInsLineNum = docLineMgr.GetLineCount() - nAllLinesOld;
	return;
}

