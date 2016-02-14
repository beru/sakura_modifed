#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Comment.h"
#include "doc/layout/CLayout.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �s�R�����g                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Color_LineComment::BeginColor(const StringRef& str, int nPos)
{
	if (!str.IsValid()) return false;

	// �s�R�����g
	return m_pTypeData->m_lineComment.Match(nPos, str);	//@@@ 2002.09.22 YAZAKI
}

bool Color_LineComment::EndColor(const StringRef& str, int nPos)
{
	// ������I�[
	if (nPos >= str.GetLength()) {
		return true;
	}

	// ���s
	if (WCODE::IsLineDelimiter(str.At(nPos), GetDllShareData().m_common.m_edit.m_bEnableExtEol)) {
		return true;
	}

	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    �u���b�N�R�����g�P                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Color_BlockComment::BeginColor(const StringRef& str, int nPos)
{
	if (!str.IsValid()) return false;

	// �u���b�N�R�����g
	if (m_pBlockComment->Match_CommentFrom(nPos, str)	//@@@ 2002.09.22 YAZAKI
	) {
		// ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩		//@@@ 2002.09.22 YAZAKI
		this->m_nCOMMENTEND = m_pBlockComment->Match_CommentTo(
			nPos + m_pBlockComment->getBlockFromLen(),
			str
		);

		return true;
	}
	return false;
}

bool Color_BlockComment::EndColor(const StringRef& str, int nPos)
{
	if (this->m_nCOMMENTEND == 0) {
		// ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩
		this->m_nCOMMENTEND = m_pBlockComment->Match_CommentTo(
			nPos,
			str
		);
	}else if (nPos == this->m_nCOMMENTEND) {
		return true;
	}
	return false;
}

