#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Figure_CtrlCode.h"
#include "types/TypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     Figure_CtrlCode                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Figure_CtrlCode::Match(const wchar_t* pText, int nTextLen) const
{
	// ���ʂ�ASCII���䕶���iC0 Controls, IsHankaku()�Ŕ��p�����j�����𐧌䕶���\���ɂ���
	// �������Ȃ��� IsHankaku(0x0600) == false �Ȃ̂� iswcntrl(0x0600) != 0 �̂悤�ȃP�[�X�ŕ\�����������
	// U+0600: ARABIC NUMBER SIGN
	return (!(pText[0] & 0xFF80) && WCODE::IsControlCode(pText[0]));
}

void Figure_CtrlCode::DispSpace(Graphics& gr, DispPos* pDispPos, EditView* pView, bool bTrans) const
{
	// �N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	RECT rc;
	if (pView->GetTextArea().GenerateClipRect(&rc, *pDispPos, 1)) {
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			L"�",
			1,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}

	// �ʒu�i�߂�
	pDispPos->ForwardDrawCol(1);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     Figure_HanBinary                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Figure_HanBinary::Match(const wchar_t* pText, int nTextLen) const
{
	int nLen = pText[1]? 2:1;	// �� pText �͏�ɏI�[������O
	if (NativeW::GetKetaOfChar(pText, nLen, 0) == 1) {	// ���p
		ECharSet e;
		CheckUtf16leChar(pText, nLen, &e, UC_NONCHARACTER);
		if (e == CHARSET_BINARY) {
			return true;
		}
	}
	return false;
}

void Figure_HanBinary::DispSpace(Graphics& gr, DispPos* pDispPos, EditView* pView, bool bTrans) const
{
	// �N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	RECT rc;
	if (pView->GetTextArea().GenerateClipRect(&rc, *pDispPos, 1)) {
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			L"��",
			1,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}

	// �ʒu�i�߂�
	pDispPos->ForwardDrawCol(1);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     Figure_ZenBinary                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Figure_ZenBinary::Match(const wchar_t* pText, int nTextLen) const
{
	int nLen = pText[1]? 2:1;	// �� pText �͏�ɏI�[������O
	if (NativeW::GetKetaOfChar(pText, nLen, 0) > 1) {	// �S�p
		ECharSet e;
		CheckUtf16leChar(pText, nLen, &e, UC_NONCHARACTER);
		if (e == CHARSET_BINARY) {
			return true;
		}
	}
	return false;
}

void Figure_ZenBinary::DispSpace(Graphics& gr, DispPos* pDispPos, EditView* pView, bool bTrans) const
{
	// �N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	RECT rc;
	if (pView->GetTextArea().GenerateClipRect(&rc, *pDispPos, 2)) {
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			L"��",
			1,
			pView->GetTextMetrics().GetDxArray_AllZenkaku()
		);
	}

	// �ʒu�i�߂�
	pDispPos->ForwardDrawCol(2);
}


