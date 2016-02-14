#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Comment.h"
#include "doc/layout/CLayout.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        行コメント                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Color_LineComment::BeginColor(const StringRef& str, int nPos)
{
	if (!str.IsValid()) return false;

	// 行コメント
	return m_pTypeData->m_lineComment.Match(nPos, str);	//@@@ 2002.09.22 YAZAKI
}

bool Color_LineComment::EndColor(const StringRef& str, int nPos)
{
	// 文字列終端
	if (nPos >= str.GetLength()) {
		return true;
	}

	// 改行
	if (WCODE::IsLineDelimiter(str.At(nPos), GetDllShareData().m_common.m_edit.m_bEnableExtEol)) {
		return true;
	}

	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ブロックコメント１                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Color_BlockComment::BeginColor(const StringRef& str, int nPos)
{
	if (!str.IsValid()) return false;

	// ブロックコメント
	if (m_pBlockComment->Match_CommentFrom(nPos, str)	//@@@ 2002.09.22 YAZAKI
	) {
		// この物理行にブロックコメントの終端があるか		//@@@ 2002.09.22 YAZAKI
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
		// この物理行にブロックコメントの終端があるか
		this->m_nCOMMENTEND = m_pBlockComment->Match_CommentTo(
			nPos,
			str
		);
	}else if (nPos == this->m_nCOMMENTEND) {
		return true;
	}
	return false;
}

