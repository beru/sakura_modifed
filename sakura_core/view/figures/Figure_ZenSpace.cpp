#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Figure_ZenSpace.h"
#include "types/TypeSupport.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      Figure_ZenSpace                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Figure_ZenSpace::Match(const wchar_t* pText, int nTextLen) const
{
	return (pText[0] == L'�@');
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�����                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �S�p�X�y�[�X�`��
void Figure_ZenSpace::DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool bTrans) const
{
	// �N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	RECT rc;
	if (view.GetTextArea().GenerateClipRect(&rc, *pDispPos, 2)) {
		// �`��
		const wchar_t* szZenSpace =
			TypeSupport(view, COLORIDX_ZENSPACE).IsDisp() ? L"��" : L"�@";
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			szZenSpace,
			wcslen(szZenSpace),
			view.GetTextMetrics().GetDxArray_AllZenkaku()
		);
	}

	// �ʒu�i�߂�
	pDispPos->ForwardDrawCol(2);
}

