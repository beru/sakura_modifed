#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Url.h"
#include "parse/CWordParse.h"
#include "doc/CEditDoc.h"
#include "doc/layout/CLayout.h"
#include "types/CTypeSupport.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           URL                               //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Color_Url::BeginColor(const StringRef& str, int nPos)
{
	if (!str.IsValid()) return false;

	int	nUrlLen;

	if (_IsPosKeywordHead(str, nPos) // URL��\������
		&& IsURL(str.GetPtr() + nPos, str.GetLength() - nPos, &nUrlLen)	// �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ�
	) {
		this->m_nCOMMENTEND = nPos + nUrlLen;
		return true;
	}
	return false;
}

bool Color_Url::EndColor(const StringRef& str, int nPos)
{
	return (nPos == this->m_nCOMMENTEND);
}

