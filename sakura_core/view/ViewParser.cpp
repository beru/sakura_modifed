#include "StdAfx.h"
#include "ViewParser.h"
#include "doc/EditDoc.h"
#include "doc/layout/Layout.h"
#include "view/EditView.h"
#include "charset/charcode.h"

/*
	�J�[�\�����O�̒P����擾 �P��̒�����Ԃ��܂�
	�P���؂�
*/
int ViewParser::GetLeftWord(NativeW* pMemWord, int nMaxWordLen) const
{
	size_t nLineLen;
	int	nIdx;
	int	nIdxTo;

	int nCharChars;
	const Layout* pLayout;
	auto& caret = editView.GetCaret();
	auto& layoutMgr = editView.pEditDoc->layoutMgr;
	int nCurLine = caret.GetCaretLayoutPos().GetY2();
	const wchar_t* pLine = layoutMgr.GetLineStr(nCurLine, &nLineLen, &pLayout);
	if (!pLine) {
//		return 0;
		nIdxTo = 0;
	}else {
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1
		nIdxTo = editView.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().GetX2());
	}
	if (nIdxTo == 0 || !pLine) {
		if (nCurLine <= 0) {
			return 0;
		}
		--nCurLine;
		pLine = layoutMgr.GetLineStr(nCurLine, &nLineLen);
		if (!pLine) {
			return 0;
		}
		bool bExtEol = GetDllShareData().common.edit.bEnableExtEol;
		if (WCODE::IsLineDelimiter(pLine[nLineLen - 1], bExtEol)) {
			return 0;
		}

		nCharChars = &pLine[nLineLen] - NativeW::GetCharPrev(pLine, nLineLen, &pLine[nLineLen]);
		if (nCharChars == 0) {
			return 0;
		}
		nIdxTo = nLineLen;
		nIdx = nIdxTo - nCharChars;
	}else {
		nCharChars = &pLine[nIdxTo] - NativeW::GetCharPrev(pLine, nLineLen, &pLine[nIdxTo]);
		if (nCharChars == 0) {
			return 0;
		}
		nIdx = nIdxTo - nCharChars;
	}

	if (nCharChars == 1) {
		if (WCODE::IsWordDelimiter(pLine[nIdx])) {
			return 0;
		}
	}

	// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
	NativeW memWord;
	LayoutRange range;
	bool bResult = layoutMgr.WhereCurrentWord(
		nCurLine,
		nIdx,
		&range,
		&memWord,
		pMemWord
	);
	if (bResult) {
		pMemWord->AppendString(&pLine[nIdx], nCharChars);
		return pMemWord->GetStringLength();
	}else {
		return 0;
	}
}


/*!
	�L�����b�g�ʒu�̒P����擾
	�P���؂�

	@param[out] pMemWord �L�����b�g�ʒu�̒P��
	@return true: �����Cfalse: ���s
	
	@date 2006.03.24 fon (CEditView::Command_SelectWord�𗬗p)
*/
bool ViewParser::GetCurrentWord(
	NativeW* pMemWord
	) const
{
	auto& layoutMgr = editView.pEditDoc->layoutMgr;
	auto& caret = editView.GetCaret();
	const Layout* pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().GetY2());
	if (!pLayout) {
		return false;	// �P��I���Ɏ��s
	}
	
	// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
	int nIdx = editView.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().GetX2());
	
	// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
	LayoutRange range;
	bool bResult = layoutMgr.WhereCurrentWord(
		caret.GetCaretLayoutPos().GetY2(),
		nIdx,
		&range,
		pMemWord,
		NULL
	);

	return bResult;
}

