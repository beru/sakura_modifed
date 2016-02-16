#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo

#include "Figure_HanSpace.h"
#include "types/TypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     Figure_HanSpace                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Figure_HanSpace::Match(const wchar_t* pText, int nTextLen) const
{
	return (pText[0] == L' ');
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�����                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ���p�X�y�[�X�`��
void Figure_HanSpace::DispSpace(Graphics& gr, DispPos* pDispPos, EditView* pView, bool bTrans) const
{
	// �N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	Rect rcClip;
	if (pView->GetTextArea().GenerateClipRect(&rcClip, *pDispPos, 1)) {
		// ������"o"�̉��������o��
		Rect rcClipBottom = rcClip;
		rcClipBottom.top = rcClip.top + pView->GetTextMetrics().GetHankakuHeight() / 2;
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClipBottom,
			L"o",
			1,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);
		
		// �㔼���͕��ʂ̋󔒂ŏo�́i"o"�̏㔼���������j
		Rect rcClipTop = rcClip;
		rcClipTop.bottom = rcClip.top + pView->GetTextMetrics().GetHankakuHeight() / 2;
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClipTop,
			L" ",
			1,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}

	// �ʒu�i�߂�
	pDispPos->ForwardDrawCol(1);
}

