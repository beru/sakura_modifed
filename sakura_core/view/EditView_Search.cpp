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

#include "StdAfx.h"
#include <limits.h>
#include "EditView.h"
#include "window/EditWnd.h"
#include "parse/WordParse.h"
#include "util/string_ex2.h"

const int STRNCMP_MAX = 100;	// MAX�L�[���[�h���Fstrnicmp�������r�ő�l(EditView::KeySearchCore) 	// 2006.04.10 fon

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! �L�[���[�h���������̑O������`�F�b�N�ƁA����

	@date 2006.04.10 fon OnTimer, CreatePopUpMenu_R���番��
*/
BOOL EditView::KeywordHelpSearchDict(LID_SKH nID, POINT* po, RECT* rc)
{
	NativeW memCurText;
	// �L�[���[�h�w���v���g�p���邩�H
	if (!m_pTypeData->bUseKeywordHelp)	// �L�[���[�h�w���v�@�\���g�p����	// 2006.04.10 fon
		goto end_of_search;
	// �t�H�[�J�X�����邩�H
	if (!GetCaret().ExistCaretFocus()) 
		goto end_of_search;
	// �E�B���h�E���Ƀ}�E�X�J�[�\�������邩�H
	GetCursorPos(po);
	GetWindowRect(GetHwnd(), rc);
	if (!PtInRect(rc, *po))
		goto end_of_search;
	switch (nID) {
	case LID_SKH_ONTIMER:
		// �E�R�����g�̂P�`�R�łȂ��ꍇ
		if (!(1
				&& !m_bInMenuLoop							// �P�D���j���[ ���[�_�� ���[�v�ɓ����Ă��Ȃ�
				&& m_dwTipTimer != 0						// �Q�D����Tip��\�����Ă��Ȃ�
				&& 300 < ::GetTickCount() - m_dwTipTimer	// �R�D��莞�Ԉȏ�A�}�E�X���Œ肳��Ă���
			)
		)
			goto end_of_search;
		break;
	case LID_SKH_POPUPMENU_R:
		if (!(1
				&& !m_bInMenuLoop							// �P�D���j���[ ���[�_�� ���[�v�ɓ����Ă��Ȃ�
			//	&& m_dwTipTimer != 0			&&			// �Q�D����Tip��\�����Ă��Ȃ�
			//	&& 1000 < ::GetTickCount() - m_dwTipTimer	// �R�D��莞�Ԉȏ�A�}�E�X���Œ肳��Ă���
			)
		)
			goto end_of_search;
		break;
	default:
		PleaseReportToAuthor(NULL, _T("EditView::KeywordHelpSearchDict\nnID=%d"), (int)nID);
	}
	// �I��͈͂̃f�[�^���擾(�����s�I���̏ꍇ�͐擪�̍s�̂�)
	if (GetSelectedDataOne(memCurText, STRNCMP_MAX + 1)) {
	// �L�����b�g�ʒu�̒P����擾���鏈��		2006.03.24 fon
	}else if (GetDllShareData().common.search.bUseCaretKeyword) {
		if (!GetParser().GetCurrentWord(&memCurText))
			goto end_of_search;
	}else
		goto end_of_search;

	if (NativeW::IsEqual(memCurText, m_tipWnd.m_key)	// ���Ɍ����ς݂�
		&& !m_tipWnd.m_KeyWasHit							// �Y������L�[���Ȃ�����
	) {
		goto end_of_search;
	}
	m_tipWnd.m_key = memCurText;

	// �������s
	if (!KeySearchCore(&m_tipWnd.m_key)) {
		goto end_of_search;
	}
	m_dwTipTimer = 0;		// ����Tip��\�����Ă���
	m_poTipCurPos = *po;	// ���݂̃}�E�X�J�[�\���ʒu
	return TRUE;			// �����܂ŗ��Ă���΃q�b�g�E���[�h

	// �L�[���[�h�w���v�\�������I��
end_of_search:
	return FALSE;
}

/*! �L�[���[�h���������������C��

	@date 2006.04.10 fon KeywordHelpSearchDict���番��
*/
bool EditView::KeySearchCore(const NativeW* pMemCurText)
{
	NativeW*	pMemRefKey;
	int			nCmpLen = STRNCMP_MAX; // 2006.04.10 fon
	int			nLine; // 2006.04.10 fon
	
	m_tipWnd.m_info.SetString(_T(""));	// tooltip�o�b�t�@������
	// 1�s�ڂɃL�[���[�h�\���̏ꍇ
	if (m_pTypeData->bUseKeyHelpKeyDisp) {	// �L�[���[�h���\������	// 2006.04.10 fon
		m_tipWnd.m_info.AppendString(_T("["));
		m_tipWnd.m_info.AppendString(pMemCurText->GetStringT());
		m_tipWnd.m_info.AppendString(_T("]"));
	}
	// �r���܂ň�v���g���ꍇ
	if (m_pTypeData->bUseKeyHelpPrefix)
		nCmpLen = wcslen(pMemCurText->GetStringPtr());	// 2006.04.10 fon
	m_tipWnd.m_KeyWasHit = false;
	for (int i=0; i<m_pTypeData->nKeyHelpNum; ++i) {	// �ő吔�FMAX_KEYHELP_FILE
		auto& keyHelpInfo = m_pTypeData->keyHelpArr[i];
		if (keyHelpInfo.bUse) {
			// 2006.04.10 fon (nCmpLen, pMemRefKey,nSearchLine)������ǉ�
			NativeW* pMemRefText;
			int nSearchResult = m_dicMgr.DicMgr::Search(
				pMemCurText->GetStringPtr(),
				nCmpLen,
				&pMemRefKey,
				&pMemRefText,
				keyHelpInfo.szPath,
				&nLine
			);
			if (nSearchResult) {
				// �Y������L�[������
				LPWSTR pszWork = pMemRefText->GetStringPtr();
				// �L���ɂȂ��Ă��鎫����S���Ȃ߂āA�q�b�g�̓s�x�����̌p������
				if (m_pTypeData->bUseKeyHelpAllSearch) {	// �q�b�g�������̎���������	// 2006.04.10 fon
					// �o�b�t�@�ɑO�̃f�[�^���l�܂��Ă�����separator�}��
					if (m_tipWnd.m_info.GetStringLength() != 0)
						m_tipWnd.m_info.AppendString(LS(STR_ERR_DLGEDITVW5));
					else
						m_tipWnd.m_info.AppendString(LS(STR_ERR_DLGEDITVW6));	// �擪�̏ꍇ
					// �����̃p�X�}��
					{
						TCHAR szFile[MAX_PATH];
						// 2013.05.08 �\������̂̓t�@�C����(�g���q�Ȃ�)�݂̂ɂ���
						_tsplitpath(keyHelpInfo.szPath, NULL, NULL, szFile, NULL);
						m_tipWnd.m_info.AppendString(szFile);
					}
					m_tipWnd.m_info.AppendString(_T("\n"));
					// �O����v�Ńq�b�g�����P���}��
					if (m_pTypeData->bUseKeyHelpPrefix) {	// �I��͈͂őO����v����
						m_tipWnd.m_info.AppendString(pMemRefKey->GetStringT());
						m_tipWnd.m_info.AppendString(_T(" >>\n"));
					}// ���������u�Ӗ��v��}��
					m_tipWnd.m_info.AppendStringW(pszWork);
					delete pMemRefText;
					delete pMemRefKey;	// 2006.07.02 genta
					// �^�O�W�����v�p�̏����c��
					if (!m_tipWnd.m_KeyWasHit) {
						m_tipWnd.m_nSearchDict = i;	// �������J���Ƃ��ŏ��Ƀq�b�g�����������J��
						m_tipWnd.m_nSearchLine = nLine;
						m_tipWnd.m_KeyWasHit = true;
					}
				}else {	// �ŏ��̃q�b�g���ڂ̂ݕԂ��ꍇ
					// �L�[���[�h�������Ă�����separator�}��
					if (m_tipWnd.m_info.GetStringLength() != 0)
						m_tipWnd.m_info.AppendString(_T("\n--------------------\n"));
					
					// �O����v�Ńq�b�g�����P���}��
					if (m_pTypeData->bUseKeyHelpPrefix) {	// �I��͈͂őO����v����
						m_tipWnd.m_info.AppendString(pMemRefKey->GetStringT());
						m_tipWnd.m_info.AppendString(_T(" >>\n"));
					}
					
					// ���������u�Ӗ��v��}��
					m_tipWnd.m_info.AppendStringW(pszWork);
					delete pMemRefText;
					delete pMemRefKey;	// 2006.07.02 genta
					// �^�O�W�����v�p�̏����c��
					m_tipWnd.m_nSearchDict = i;
					m_tipWnd.m_nSearchLine = nLine;
					m_tipWnd.m_KeyWasHit = true;
					return true;
				}
			}
		}
	}
	if (m_tipWnd.m_KeyWasHit) {
		return true;
	}
	// �Y������L�[���Ȃ������ꍇ
	return false;
}

bool EditView::MiniMapCursorLineTip(POINT* po, RECT* rc, bool* pbHide)
{
	*pbHide = true;
	if (!m_bMiniMap) {
		return false;
	}
	// �E�B���h�E���Ƀ}�E�X�J�[�\�������邩�H
	GetCursorPos(po);
	GetWindowRect(GetHwnd(), rc);
	rc->right -= ::GetSystemMetrics(SM_CXVSCROLL);
	if (!PtInRect(rc, *po)) {
		return false;
	}
	if (!( !m_bInMenuLoop &&					// �P�D���j���[ ���[�_�� ���[�v�ɓ����Ă��Ȃ�
		300 < ::GetTickCount() - m_dwTipTimer	// �Q�D��莞�Ԉȏ�A�}�E�X���Œ肳��Ă���
	)) {
		return false;
	}
	if (WindowFromPoint(*po) != GetHwnd()) {
		return false;
	}

	Point ptClient(*po);
	ScreenToClient(GetHwnd(), &ptClient);
	LayoutPoint ptNew;
	GetTextArea().ClientToLayout(ptClient, &ptNew);
	// �����s�Ȃ�Ȃɂ����Ȃ�
	if (m_dwTipTimer == 0 && m_tipWnd.m_nSearchLine == (Int)ptNew.y) {
		*pbHide = false; // �\���p��
		return false;
	}
	NativeW memCurText;
	LayoutYInt nTipBeginLine = ptNew.y;
	LayoutYInt nTipEndLine = ptNew.y + LayoutYInt(4);
	for (LayoutYInt nCurLine=nTipBeginLine; nCurLine<nTipEndLine; ++nCurLine) {
		const Layout* pLayout = NULL;
		if (0 <= nCurLine) {
			pLayout = GetDocument()->m_layoutMgr.SearchLineByLayoutY( nCurLine );
		}
		if (pLayout) {
			NativeW memCurLine;
			{
				LogicInt nLineLen = pLayout->GetLengthWithoutEOL();
				const wchar_t* pszData = pLayout->GetPtr();
				int nLimitLength = 80;
				int pre = 0;
				int i = 0;
				int k = 0;
				int charSize = NativeW::GetSizeOfChar( pszData, nLineLen, i );
				int charWidth = t_max(1, (int)(Int)NativeW::GetKetaOfChar( pszData, nLineLen, i ));
				int charType = 0;
				// �A������"\t" " " �� " "1�ɂ���
				// ������nLimitLength�܂ł̕���؂���
				while (i + charSize <= (Int)nLineLen && k + charWidth <= nLimitLength) {
					if (pszData[i] == L'\t' || pszData[i] == L' ') {
						if (charType == 0) {
							memCurLine.AppendString( pszData + pre , i - pre );
							memCurLine.AppendString( L" " );
							charType = 1;
						}
						pre = i + charSize;
						++k;
					}else {
						k += charWidth;
						charType = 0;
					}
					i += charSize;
					charSize = NativeW::GetSizeOfChar( pszData, nLineLen, i );
					charWidth = t_max(1, (int)(Int)NativeW::GetKetaOfChar( pszData, nLineLen, i ));
				}
				memCurLine.AppendString( pszData + pre , i - pre );
			}
			if (nTipBeginLine != nCurLine) {
				memCurText.AppendString( L"\n" );
			}
			memCurLine.Replace( L"\\", L"\\\\" );
			memCurText.AppendNativeData( memCurLine );
		}
	}
	if (memCurText.GetStringLength() <= 0) {
		return false;
	}
	m_tipWnd.m_key = memCurText;
	m_tipWnd.m_info = memCurText.GetStringT();
	m_tipWnd.m_nSearchLine = (Int)ptNew.y;
	m_dwTipTimer = 0;		// ����Tip��\�����Ă���
	m_poTipCurPos = *po;	// ���݂̃}�E�X�J�[�\���ʒu
	return true;			// �����܂ŗ��Ă���΃q�b�g�E���[�h
}

// ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾
void EditView::GetCurrentTextForSearch(NativeW& memCurText, bool bStripMaxPath /* = true */, bool bTrimSpaceTab /* = false */)
{
	NativeW memTopic = L"";

	memCurText.SetString(L"");
	if (GetSelectionInfo().IsTextSelected()) {	// �e�L�X�g���I������Ă��邩
		// �I��͈͂̃f�[�^���擾
		if (GetSelectedDataOne(memCurText, INT_MAX)) {
			// ��������������݈ʒu�̒P��ŏ�����
			if (bStripMaxPath) {
				LimitStringLengthW(memCurText.GetStringPtr(), memCurText.GetStringLength(), _MAX_PATH - 1, memTopic);
			}else {
				memTopic = memCurText;
			}
		}
	}else {
		LogicInt nLineLen;
		const Layout* pLayout;
		const wchar_t* pLine = m_pEditDoc->m_layoutMgr.GetLineStr(GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pLayout);
		if (pLine) {
			// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
			LogicInt nIdx = LineColumnToIndex(pLayout, GetCaret().GetCaretLayoutPos().GetX2());

			// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
			LayoutRange range;
			bool bWhere = m_pEditDoc->m_layoutMgr.WhereCurrentWord(
				GetCaret().GetCaretLayoutPos().GetY2(),
				nIdx,
				&range,
				NULL,
				NULL
			);
			if (bWhere) {
				// �I��͈͂̕ύX
				GetSelectionInfo().m_selectBgn = range;
				GetSelectionInfo().m_select    = range;

				// �I��͈͂̃f�[�^���擾
				if (GetSelectedDataOne(memCurText, INT_MAX)) {
					// ��������������݈ʒu�̒P��ŏ�����
					if (bStripMaxPath) {
						LimitStringLengthW(memCurText.GetStringPtr(), memCurText.GetStringLength(), _MAX_PATH - 1, memTopic);
					}else {
						memTopic = memCurText;
					}
				}
				// ���݂̑I��͈͂��I����Ԃɖ߂�
				GetSelectionInfo().DisableSelectArea(false);
			}
		}
	}

	wchar_t* pTopic2 = memTopic.GetStringPtr();
	if (bTrimSpaceTab) {
		// �O�̃X�y�[�X�E�^�u����菜��
		while (L'\0' != *pTopic2 && (' ' == *pTopic2 || '\t' == *pTopic2)) {
			++pTopic2;
		}
	}
	int nTopic2Len = (int)wcslen(pTopic2);
	// ����������͉��s�܂�
	bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
	int i;
	for (i=0; i<nTopic2Len; ++i) {
		if (WCODE::IsLineDelimiter(pTopic2[i], bExtEol)) {
			break;
		}
	}
	
	if (bTrimSpaceTab) {
		// ���̃X�y�[�X�E�^�u����菜��
		int m = i - 1;
		while (0 <= m &&
		    (L' ' == pTopic2[m] || L'\t' == pTopic2[m])
		) {
			--m;
		}
		if (0 <= m) {
			i = m + 1;
		}
	}
	memCurText.SetString(pTopic2, i);
}


/*!	���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾�i�_�C�A���O�p�j
	@return �l��ݒ肵����
	@date 2006.08.23 ryoji �V�K�쐬
	@date 2014.07.01 Moca bGetHistory�ǉ��A�߂�l��bool�ɕύX
*/
bool EditView::GetCurrentTextForSearchDlg(NativeW& memCurText, bool bGetHistory)
{
	bool bStripMaxPath = false;
	memCurText.SetString(L"");
	
	if (GetSelectionInfo().IsTextSelected()) {	// �e�L�X�g���I������Ă���
		GetCurrentTextForSearch(memCurText, bStripMaxPath);
	}else {	// �e�L�X�g���I������Ă��Ȃ�
		bool bGet = false;
		if (GetDllShareData().common.search.bCaretTextForSearch) {
			GetCurrentTextForSearch(memCurText, bStripMaxPath);	// �J�[�\���ʒu�P����擾
			if (memCurText.GetStringLength() == 0 && bGetHistory) {
				bGet = true;
			}
		}else {
			bGet = true;
		}
		if (bGet) {
			if (1
				&& 0 < GetDllShareData().searchKeywords.searchKeys.size()
				&& m_nCurSearchKeySequence < GetDllShareData().common.search.nSearchKeySequence
			) {
				memCurText.SetString(GetDllShareData().searchKeywords.searchKeys[0]);	// ��������Ƃ��Ă���
				return true; // ""�ł�true	
			}else {
				memCurText.SetString(m_strCurSearchKey.c_str());
				return 0 <= m_nCurSearchKeySequence; // ""�ł�true.���ݒ�̂Ƃ���false

			}
		}
	}
	return 0 < memCurText.GetStringLength();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �`��p����                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ���݈ʒu������������ɊY�����邩
// 2002.02.08 hor
// ���K�\���Ō��������Ƃ��̑��x���P�̂��߁A�}�b�`�擪�ʒu�������ɒǉ�
// Jun. 26, 2001 genta	���K�\�����C�u�����̍����ւ�
/*
	@retval 0
		(�p�^�[��������) �w��ʒu�ȍ~�Ƀ}�b�`�͂Ȃ��B
		(����ȊO) �w��ʒu�͌���������̎n�܂�ł͂Ȃ��B
	@retval 1,2,3,...
		(�p�^�[��������) �w��ʒu�ȍ~�Ƀ}�b�`�����������B
		(�P�ꌟ����) �w��ʒu������������Ɋ܂܂�鉽�Ԗڂ̒P��̎n�܂�ł��邩�B
		(����ȊO) �w��ʒu������������̎n�܂肾�����B
*/
int EditView::IsSearchString(
	const StringRef&	str,
	/*
	const wchar_t*	pszData,
	CLogicInt		nDataLen,
	*/
	LogicInt		nPos,
	LogicInt*		pnSearchStart,
	LogicInt*		pnSearchEnd
	) const
{
	*pnSearchStart = nPos;	// 2002.02.08 hor

	if (m_curSearchOption.bRegularExp) {
		// �s���ł͂Ȃ�?
		// �s�������`�F�b�N�́ACBregexp�N���X�����Ŏ��{����̂ŕs�v 2003.11.01 �����

		/* �ʒu��0��MatchInfo�Ăяo���ƁA�s�������������ɁA�S�� true�@�ƂȂ�A
		** ��ʑS�̂����������񈵂��ɂȂ�s��C��
		** �΍�Ƃ��āA�s���� MacthInfo�ɋ����Ȃ��Ƃ����Ȃ��̂ŁA������̒����E�ʒu����^����`�ɕύX
		** 2003.05.04 �����
		*/
		if (m_curRegexp.Match(str.GetPtr(), str.GetLength(), nPos)) {
			*pnSearchStart = m_curRegexp.GetIndex();	// 2002.02.08 hor
			*pnSearchEnd = m_curRegexp.GetLastIndex();
			return 1;
		}else {
			return 0;
		}
	}else if (m_curSearchOption.bWordOnly) { // �P�ꌟ��
		// �w��ʒu�̒P��͈̔͂𒲂ׂ�
		LogicInt posWordHead, posWordEnd;
		if (!WordParse::WhereCurrentWord_2(str.GetPtr(), LogicInt(str.GetLength()), nPos, &posWordHead, &posWordEnd, NULL, NULL)) {
			return 0; // �w��ʒu�ɒP�ꂪ������Ȃ������B
 		}
		if (nPos != posWordHead) {
			return 0; // �w��ʒu�͒P��̎n�܂�ł͂Ȃ������B
		}
		const LogicInt wordLength = posWordEnd - posWordHead;
		const wchar_t* const pWordHead = str.GetPtr() + posWordHead;

		// ��r�֐�
		int (*const fcmp)(const wchar_t*, const wchar_t*, size_t) = m_curSearchOption.bLoHiCase ? wcsncmp : wcsnicmp;

		// �������P��ɕ������Ȃ���w��ʒu�̒P��Əƍ�����B
		int wordIndex = 0;
		const wchar_t* const searchKeyEnd = m_strCurSearchKey.data() + m_strCurSearchKey.size();
		for (const wchar_t* p=m_strCurSearchKey.data(); p<searchKeyEnd; ) {
			LogicInt begin, end; // ������Ɋ܂܂��P��?�̈ʒu�BWhereCurrentWord_2()�̎d�l�ł͋󔒕�������P��Ɋ܂܂��B
			if (1
				&& WordParse::WhereCurrentWord_2(p, LogicInt(searchKeyEnd - p), LogicInt(0), &begin, &end, NULL, NULL)
				&& begin == 0
				&& begin < end
			) {
				if (!WCODE::IsWordDelimiter(*p)) {
					++wordIndex;
					// p...(p + end) ��������Ɋ܂܂�� wordIndex�Ԗڂ̒P��B(wordIndex�̍ŏ��� 1)
					if (wordLength == end && fcmp(p, pWordHead, wordLength) == 0) {
						*pnSearchStart = posWordHead;
						*pnSearchEnd = posWordEnd;
						return wordIndex;
					}
				}
				p += end;
			}else {
				p += NativeW::GetSizeOfChar(p, searchKeyEnd - p, 0);
			}
		}
		return 0; // �w��ʒu�̒P��ƌ���������Ɋ܂܂��P��͈�v���Ȃ������B
	}else {
		const wchar_t* pHit = SearchAgent::SearchString(str.GetPtr(), str.GetLength(), nPos, m_searchPattern);
		if (pHit) {
			*pnSearchStart = pHit - str.GetPtr();
			*pnSearchEnd = *pnSearchStart + m_searchPattern.GetLen();
			return 1;
		}
		return 0; // ���̍s�̓q�b�g���Ȃ�����
	}
	return 0;
}

