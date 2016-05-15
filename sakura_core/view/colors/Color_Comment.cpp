#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Color_Comment.h"
#include "doc/layout/Layout.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        行コメント                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Color_LineComment::BeginColor(const StringRef& str, int nPos)
{
	if (!str.IsValid()) return false;

	// 行コメント
	return pTypeData->lineComment.Match(nPos, str);	//@@@ 2002.09.22 YAZAKI
}

bool Color_LineComment::EndColor(const StringRef& str, int nPos)
{
	// 文字列終端
	if (nPos >= str.GetLength()) {
		return true;
	}

	// 改行
	if (WCODE::IsLineDelimiter(str.At(nPos), GetDllShareData().common.edit.bEnableExtEol)) {
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
	if (pBlockComment->Match_CommentFrom(nPos, str)	//@@@ 2002.09.22 YAZAKI
	) {
		// この物理行にブロックコメントの終端があるか		//@@@ 2002.09.22 YAZAKI
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
		// この物理行にブロックコメントの終端があるか
		this->nCommentEnd = pBlockComment->Match_CommentTo(
			nPos,
			str
		);
	}else if (nPos == this->nCommentEnd) {
		return true;
	}
	return false;
}

