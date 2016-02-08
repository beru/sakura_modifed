#include "StdAfx.h"
#include "CViewParser.h"
#include "doc/CEditDoc.h"
#include "doc/layout/CLayout.h"
#include "view/CEditView.h"
#include "charset/charcode.h"

/*
	�J�[�\�����O�̒P����擾 �P��̒�����Ԃ��܂�
	�P���؂�
*/
int CViewParser::GetLeftWord(CNativeW* pcmemWord, int nMaxWordLen) const
{
	LogicInt	nLineLen;
	LogicInt	nIdx;
	LogicInt	nIdxTo;

	int			nCharChars;
	const Layout* pcLayout;

	LayoutInt nCurLine = m_pEditView->GetCaret().GetCaretLayoutPos().GetY2();
	const wchar_t* pLine = m_pEditView->m_pcEditDoc->m_cLayoutMgr.GetLineStr(nCurLine, &nLineLen, &pcLayout);
	if (!pLine) {
//		return 0;
		nIdxTo = LogicInt(0);
	}else {
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1
		nIdxTo = m_pEditView->LineColumnToIndex(pcLayout, m_pEditView->GetCaret().GetCaretLayoutPos().GetX2());
	}
	if (nIdxTo == 0 || !pLine) {
		if (nCurLine <= 0) {
			return 0;
		}
		--nCurLine;
		pLine = m_pEditView->m_pcEditDoc->m_cLayoutMgr.GetLineStr(nCurLine, &nLineLen);
		if (!pLine) {
			return 0;
		}
		bool bExtEol = GetDllShareData().m_common.m_sEdit.m_bEnableExtEol;
		if (WCODE::IsLineDelimiter(pLine[nLineLen - 1], bExtEol)) {
			return 0;
		}

		nCharChars = &pLine[nLineLen] - CNativeW::GetCharPrev(pLine, nLineLen, &pLine[nLineLen]);
		if (nCharChars == 0) {
			return 0;
		}
		nIdxTo = nLineLen;
		nIdx = nIdxTo - LogicInt(nCharChars);
	}else {
		nCharChars = &pLine[nIdxTo] - CNativeW::GetCharPrev(pLine, nLineLen, &pLine[nIdxTo]);
		if (nCharChars == 0) {
			return 0;
		}
		nIdx = nIdxTo - LogicInt(nCharChars);
	}

	if (nCharChars == 1) {
		if (WCODE::IsWordDelimiter(pLine[nIdx])) {
			return 0;
		}
	}

	// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
	CNativeW cmemWord;
	LayoutRange sRange;
	bool bResult = m_pEditView->m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
		nCurLine,
		nIdx,
		&sRange,
		&cmemWord,
		pcmemWord
	);
	if (bResult) {
		pcmemWord->AppendString(&pLine[nIdx], nCharChars);
		return pcmemWord->GetStringLength();
	}else {
		return 0;
	}
}


/*!
	�L�����b�g�ʒu�̒P����擾
	�P���؂�

	@param[out] pcmemWord �L�����b�g�ʒu�̒P��
	@return true: �����Cfalse: ���s
	
	@date 2006.03.24 fon (CEditView::Command_SELECTWORD�𗬗p)
*/
bool CViewParser::GetCurrentWord(
	CNativeW* pcmemWord
) const
{
	const Layout* pcLayout = m_pEditView->m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY(m_pEditView->GetCaret().GetCaretLayoutPos().GetY2());
	if (!pcLayout) {
		return false;	// �P��I���Ɏ��s
	}
	
	// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
	LogicInt nIdx = m_pEditView->LineColumnToIndex(pcLayout, m_pEditView->GetCaret().GetCaretLayoutPos().GetX2());
	
	// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
	LayoutRange sRange;
	bool bResult = m_pEditView->m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
		m_pEditView->GetCaret().GetCaretLayoutPos().GetY2(),
		nIdx,
		&sRange,
		pcmemWord,
		NULL
	);

	return bResult;
}

