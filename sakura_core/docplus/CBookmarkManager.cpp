#include "StdAfx.h"
#include "docplus/CBookmarkManager.h"
#include "doc/logic/CDocLineMgr.h"
#include "doc/logic/CDocLine.h"
#include "CSearchAgent.h"
#include "extmodule/CBregexp.h"


bool CBookmarkGetter::IsBookmarked() const{ return m_pcDocLine->m_sMark.m_cBookmarked; }
void CBookmarkSetter::SetBookmark(bool bFlag){ m_pcDocLine->m_sMark.m_cBookmarked = bFlag; }

//!�u�b�N�}�[�N�̑S����
/*
	@date 2001.12.03 hor
*/
void CBookmarkManager::ResetAllBookMark( void )
{
	CDocLine* pDocLine = m_pcDocLineMgr->GetDocLineTop();
	while (pDocLine) {
		CBookmarkSetter(pDocLine).SetBookmark(false);
		pDocLine = pDocLine->GetNextLine();
	}
}


//! �u�b�N�}�[�N����
/*
	@date 2001.12.03 hor
*/
bool CBookmarkManager::SearchBookMark(
	CLogicInt			nLineNum,		// �����J�n�s
	ESearchDirection	bPrevOrNext,	// 0==�O������ 1==�������
	CLogicInt*			pnLineNum 		// �}�b�`�s
	)
{
	CLogicInt nLinePos = nLineNum;

	// �O������
	if (bPrevOrNext == SEARCH_BACKWARD) {
		nLinePos--;
		CDocLine* pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
		while (pDocLine) {
			if (CBookmarkGetter(pDocLine).IsBookmarked()) {
				*pnLineNum = nLinePos;				// �}�b�`�s
				return true;
			}
			nLinePos--;
			pDocLine = pDocLine->GetPrevLine();
		}
	// �������
	}else {
		nLinePos++;
		CDocLine* pDocLine = m_pcDocLineMgr->GetLine( nLinePos );
		while (pDocLine) {
			if (CBookmarkGetter(pDocLine).IsBookmarked()) {
				*pnLineNum = nLinePos;				// �}�b�`�s
				return true;
			}
			nLinePos++;
			pDocLine = pDocLine->GetNextLine();
		}
	}
	return false;
}

//! �����s�ԍ��̃��X�g����܂Ƃ߂čs�}�[�N
/*
	@date 2002.01.16 hor
*/
void CBookmarkManager::SetBookMarks( wchar_t* pMarkLines )
{
	wchar_t delim[] = L", ";
	wchar_t* p = pMarkLines;
	while (wcstok(p, delim) != NULL) {
		while (wcschr(delim, *p) != NULL) {
			p++;
		}
		CDocLine* pCDocLine = m_pcDocLineMgr->GetLine( CLogicInt(_wtol(p)) );
		if (pCDocLine) {
			CBookmarkSetter(pCDocLine).SetBookmark(true);
		}
		p += wcslen(p) + 1;
	}
}


//! �s�}�[�N����Ă镨���s�ԍ��̃��X�g�����
/*
	@date 2002.01.16 hor
*/
LPCWSTR CBookmarkManager::GetBookMarks()
{
	static wchar_t szText[MAX_MARKLINES_LEN + 1];	//2002.01.17 // Feb. 17, 2003 genta static��
	wchar_t szBuff[10];
	CLogicInt nLinePos = CLogicInt(0);
	CDocLine* pCDocLine = m_pcDocLineMgr->GetLine( nLinePos );
	szText[0] = 0;
	while (pCDocLine) {
		if (CBookmarkGetter(pCDocLine).IsBookmarked()) {
			auto_sprintf_s( szBuff, L"%d,",nLinePos );
			if (wcslen(szBuff)+wcslen(szText)>MAX_MARKLINES_LEN) break;	//2002.01.17
			wcscat( szText, szBuff);
		}
		nLinePos++;
		pCDocLine = pCDocLine->GetNextLine();
	}
	return szText; // Feb. 17, 2003 genta
}


//! ���������ɊY������s�Ƀu�b�N�}�[�N���Z�b�g����
/*
	@date 2002.01.16 hor
*/
void CBookmarkManager::MarkSearchWord(
	const CSearchStringPattern& pattern
)
{
	const SSearchOption& sSearchOption = pattern.GetSearchOption();
	int nLineLen;

	// 1==���K�\��
	if (sSearchOption.bRegularExp) {
		CBregexp* pRegexp = pattern.GetRegexp();
		CDocLine* pDocLine = m_pcDocLineMgr->GetLine( CLogicInt(0) );
		while (pDocLine) {
			if (!CBookmarkGetter(pDocLine).IsBookmarked()) {
				const wchar_t* pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
				// 2005.03.19 ����� �O����v�T�|�[�g�̂��߂̃��\�b�h�ύX
				if (pRegexp->Match( pLine, nLineLen, 0 )) {
					CBookmarkSetter(pDocLine).SetBookmark(true);
				}
			}
			pDocLine = pDocLine->GetNextLine();
		}
	// 1==�P��̂݌���
	}else if (sSearchOption.bWordOnly) {
		const wchar_t* pszPattern = pattern.GetKey();
		const int nPatternLen = pattern.GetLen();
		// �������P��ɕ������� searchWords�Ɋi�[����B
		std::vector<std::pair<const wchar_t*, CLogicInt> > searchWords; // �P��̊J�n�ʒu�ƒ����̔z��B
		CSearchAgent::CreateWordList(searchWords, pszPattern, nPatternLen);

		CDocLine* pDocLine = m_pcDocLineMgr->GetLine( CLogicInt(0) );
		while (pDocLine) {
			if (!CBookmarkGetter(pDocLine).IsBookmarked()) {
				const wchar_t* pLine = pDocLine->GetDocLineStrWithEOL(&nLineLen);
				int nMatchLen;
				if (CSearchAgent::SearchStringWord(pLine, nLineLen, 0, searchWords, sSearchOption.bLoHiCase, &nMatchLen)) {
					CBookmarkSetter(pDocLine).SetBookmark(true);
				}
			}
			// ���̍s�����ɍs��
			pDocLine = pDocLine->GetNextLine();
		}
	}else {
		// ���������̏��
		CDocLine* pDocLine = m_pcDocLineMgr->GetLine( CLogicInt(0) );
		while (pDocLine) {
			if (!CBookmarkGetter(pDocLine).IsBookmarked()) {
				const wchar_t* pLine = pDocLine->GetDocLineStrWithEOL( &nLineLen );
				if (CSearchAgent::SearchString(
					pLine,
					nLineLen,
					0,
					pattern
					)
				) {
					CBookmarkSetter(pDocLine).SetBookmark(true);
				}
			}
			pDocLine = pDocLine->GetNextLine();
		}
	}
}

