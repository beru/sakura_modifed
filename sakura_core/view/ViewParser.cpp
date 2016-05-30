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
size_t ViewParser::GetLeftWord(NativeW* pMemWord, int nMaxWordLen) const
{
	size_t nLineLen;
	size_t nIdx;
	size_t nIdxTo;

	ptrdiff_t nCharChars;
	const Layout* pLayout;
	auto& caret = editView.GetCaret();
	auto& layoutMgr = editView.pEditDoc->layoutMgr;
	size_t nCurLine = caret.GetCaretLayoutPos().y;
	const wchar_t* pLine = layoutMgr.GetLineStr(nCurLine, &nLineLen, &pLayout);
	if (!pLine) {
//		return 0;
		nIdxTo = 0;
	}else {
		// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1
		nIdxTo = editView.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().x);
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
		ASSERT_GE((int)nIdxTo, nCharChars);
		nIdx = nIdxTo - nCharChars;
	}else {
		nCharChars = &pLine[nIdxTo] - NativeW::GetCharPrev(pLine, nLineLen, &pLine[nIdxTo]);
		if (nCharChars == 0) {
			return 0;
		}
		ASSERT_GE((int)nIdxTo, nCharChars);
		nIdx = nIdxTo - nCharChars;
	}

	if (nCharChars == 1) {
		if (WCODE::IsWordDelimiter(pLine[nIdx])) {
			return 0;
		}
	}

	// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
	NativeW memWord;
	Range range;
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
	const Layout* pLayout = layoutMgr.SearchLineByLayoutY(caret.GetCaretLayoutPos().y);
	if (!pLayout) {
		return false;	// �P��I���Ɏ��s
	}
	
	// �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ�
	size_t nIdx = editView.LineColumnToIndex(pLayout, caret.GetCaretLayoutPos().x);
	
	// ���݈ʒu�̒P��͈̔͂𒲂ׂ�
	Range range;
	bool bResult = layoutMgr.WhereCurrentWord(
		caret.GetCaretLayoutPos().y,
		nIdx,
		&range,
		pMemWord,
		NULL
	);

	return bResult;
}

