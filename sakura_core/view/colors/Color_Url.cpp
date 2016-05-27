#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Color_Url.h"
#include "parse/WordParse.h"
#include "doc/EditDoc.h"
#include "doc/layout/Layout.h"
#include "types/TypeSupport.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           URL                               //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Color_Url::BeginColor(const StringRef& str, size_t nPos)
{
	if (!str.IsValid()) return false;

	size_t nUrlLen;
	if (_IsPosKeywordHead(str, nPos) // URL��\������
		&& IsURL(str.GetPtr() + nPos, str.GetLength() - nPos, &nUrlLen)	// �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ�
	) {
		this->nCommentEnd = nPos + nUrlLen;
		return true;
	}
	return false;
}

bool Color_Url::EndColor(const StringRef& str, size_t nPos)
{
	return (nPos == this->nCommentEnd);
}

