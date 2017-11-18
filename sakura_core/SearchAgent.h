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
	const uint16_t* GetUseCharSkipMap() const { return pnUseCharSkipArr; }

	static int GetMapIndex(wchar_t c);
#endif

private:
	// �O���ˑ�
	const wchar_t* pszKey;
	const SearchOption* pSearchOption;
	mutable Bregexp* pRegexp;

	const wchar_t* pszCaseKeyRef;

	// �����o�b�t�@
	wchar_t* pszPatternCase;
	size_t nPatternLen;
#ifdef SEARCH_STRING_KMP
	int* pnNextPossArr;
#endif
#ifdef SEARCH_STRING_SUNDAY_QUICK
	uint16_t* pnUseCharSkipArr;
#endif

private:
	DISALLOW_COPY_AND_ASSIGN(SearchStringPattern);
};

class SearchAgent {
public:
	// �����񌟍�
	static
	const wchar_t* SearchString(
		const wchar_t* pLine,
		size_t nLineLen,
		size_t nIdxPos,
		const SearchStringPattern& pattern
	);
	// �P��P�ʂŕ����񌟍�
	static
	const wchar_t* SearchStringWord(
		const wchar_t*	pLine,
		size_t	nLineLen,
		size_t	nIdxPos,
		const std::vector<std::pair<const wchar_t*, size_t>>& searchWords,
		bool	bLoHiCase,
		size_t*	pnMatchLen
	);
	
	static
	void CreateWordList(
		std::vector<std::pair<const wchar_t*, size_t>>& searchWords,
		const wchar_t* pszPattern,
		size_t nPatternLen
	);

public:
	SearchAgent(DocLineMgr& docLineMgr) : docLineMgr(docLineMgr) { }

	bool WhereCurrentWord(size_t, size_t, size_t* , size_t*, NativeW*, NativeW*);	// ���݈ʒu�̒P��͈̔͂𒲂ׂ�

	bool PrevOrNextWord(size_t, size_t, size_t* , bool bLEFT, bool bStopsBothEnds);	// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
	//	Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
	bool SearchWord(Point ptSerachBegin, SearchDirection eDirection, Range* pMatchRange, const SearchStringPattern& pattern); // �P�ꌟ��

	void ReplaceData(DocLineReplaceArg*);
private:
	DocLineMgr& docLineMgr;
};

