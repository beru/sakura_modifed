#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Color_Comment.h"
#include "doc/layout/Layout.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �s�R�����g                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Color_LineComment::BeginColor(const StringRef& str, int nPos)
{
	if (!str.IsValid()) return false;

	// �s�R�����g
	return pTypeData->lineComment.Match(nPos, str);	//@@@ 2002.09.22 YAZAKI
}

bool Color_LineComment::EndColor(const StringRef& str, int nPos)
{
	// ������I�[
	if (nPos >= str.GetLength()) {
		return true;
	}

	// ���s
	if (WCODE::IsLineDelimiter(str.At(nPos), GetDllShareData().common.edit.bEnableExtEol)) {
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
	if (pBlockComment->Match_CommentFrom(nPos, str)	//@@@ 2002.09.22 YAZAKI
	) {
		// ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩		//@@@ 2002.09.22 YAZAKI
		this->nCommentEnd = pBlockComment->Match_CommentTo(
			nPos + pBlockComment->getBlockFromLen(),
			str
		);

		return true;
	}
	return false;
}

bool Color_BlockComment::EndColor(const StringRef& str, int nPos)
{
	if (this->nCommentEnd == 0) {
		// ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩
		this->nCommentEnd = pBlockComment->Match_CommentTo(
			nPos,
			str
		);
	}else if (nPos == this->nCommentEnd) {
		return true;
	}
	return false;
}

