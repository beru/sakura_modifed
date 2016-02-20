/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "view/EditView.h" // ColorStrategyInfo
#include "view/ViewFont.h"
#include "FigureStrategy.h"
#include "doc/layout/Layout.h"
#include "charset/charcode.h"
#include "types/TypeSupport.h"

bool Figure_Text::DrawImp(ColorStrategyInfo* pInfo)
{
	int nIdx = pInfo->GetPosInLogic();
	int nLength = NativeW::GetSizeOfChar(	// �T���Q�[�g�y�A�΍�	2008.10.12 ryoji
						pInfo->m_pLineOfLogic,
						pInfo->GetDocLine()->GetLengthWithoutEOL(),
						nIdx
					);
	bool bTrans = pInfo->m_pView->IsBkBitmap() && TypeSupport(pInfo->m_pView, COLORIDX_TEXT).GetBackColor() == pInfo->m_gr.GetTextBackColor();
	pInfo->m_pView->GetTextDrawer().DispText(
		pInfo->m_gr,
		pInfo->m_pDispPos,
		&pInfo->m_pLineOfLogic[nIdx],
		nLength,
		bTrans
	);
	pInfo->m_nPosInLogic += nLength;
	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�擝��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      FigureSpace                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
bool FigureSpace::DrawImp(ColorStrategyInfo* pInfo)
{
	bool bTrans = DrawImp_StyleSelect(pInfo);
	DispPos sPos(*pInfo->m_pDispPos);	// ���݈ʒu���o���Ă���
	DispSpace(pInfo->m_gr, pInfo->m_pDispPos, pInfo->m_pView, bTrans);	// �󔒕`��
	DrawImp_StylePop(pInfo);
	DrawImp_DrawUnderline(pInfo, sPos);
	// 1�����O��
	pInfo->m_nPosInLogic += NativeW::GetSizeOfChar(	// �s���ȊO�͂����ŃX�L�����ʒu���P���i�߂�
		pInfo->m_pLineOfLogic,
		pInfo->GetDocLine()->GetLengthWithoutEOL(),
		pInfo->GetPosInLogic()
		);
	return true;
}

bool FigureSpace::DrawImp_StyleSelect(ColorStrategyInfo* pInfo)
{
	// ���� DrawImp �͂����i��{�N���X�j�Ńf�t�H���g������������Ă��邪
	// ���z�֐��Ȃ̂Ŕh���N���X���̃I�[�o�[���C�h�ŌʂɎd�l�ύX�\
	EditView* pView = pInfo->m_pView;

	TypeSupport currentType(pView, pInfo->GetCurrentColor());	// ���ӂ̐F�i���݂̎w��F/�I��F�j
	TypeSupport currentType2(pView, pInfo->GetCurrentColor2());	// ���ӂ̐F�i���݂̎w��F�j
	TypeSupport textType(pView, COLORIDX_TEXT);				// �e�L�X�g�̎w��F
	TypeSupport spaceType(pView, GetDispColorIdx());	// �󔒂̎w��F
	TypeSupport currentTypeBg(pView, pInfo->GetCurrentColorBg());
	TypeSupport& currentType1 = (currentType.GetBackColor() == textType.GetBackColor() ? currentTypeBg: currentType);
	TypeSupport& currentType3 = (currentType2.GetBackColor() == textType.GetBackColor() ? currentTypeBg: currentType2);

	// �󔒋L���ނ͓��ɖ����w�肵�������ȊO�͂Ȃ�ׂ����ӂ̎w��ɍ��킹��悤�ɂ��Ă݂�	// 2009.05.30 ryoji
	// �Ⴆ�΁A�������w�肵�Ă��Ȃ��ꍇ�A���K�\���L�[���[�h���Ȃ琳�K�\���L�[���[�h���̉����w��ɏ]���ق������R�ȋC������B
	// �i���̂ق����󔒋L���́u�\���v���`�F�b�N���Ă��Ȃ��ꍇ�̕\���ɋ߂��j
	//
	// �O�i�F�E�w�i�F�̈���
	// �E�ʏ�e�L�X�g�Ƃ͈قȂ�F���w�肳��Ă���ꍇ�͋󔒋L���̑��̎w��F���g��
	// �E�ʏ�e�L�X�g�Ɠ����F���w�肳��Ă���ꍇ�͎��ӂ̐F�w��ɍ��킹��
	// �����̈���
	// �E�󔒋L�������ӂ̂ǂ��炩����ł������w�肳��Ă���΁u�O�i�F�E�w�i�F�̈����v�Ō��肵���O�i�F�ő����ɂ���
	// �����̈���
	// �E�󔒋L���ŉ����w�肳��Ă���΁u�O�i�F�E�w�i�F�̈����v�Ō��肵���O�i�F�ŉ���������
	// �E�󔒋L���ŉ����w�肳��Ă��炸���ӂŉ����w�肳��Ă���Ύ��ӂ̑O�i�F�ŉ���������
	// [�I��]�����_�����O��
	// �E�����F�̏ꍇ�͏]���ʂ�B
	COLORREF crText;
	COLORREF crBack;
	bool blendColor = pInfo->GetCurrentColor() != pInfo->GetCurrentColor2() && currentType.GetTextColor() == currentType.GetBackColor(); // �I�������F
	bool bBold;
	if (blendColor) {
		TypeSupport& text = spaceType.GetTextColor() == textType.GetTextColor() ? currentType2 : spaceType;
		TypeSupport& back = spaceType.GetBackColor() == textType.GetBackColor() ? currentType3 : spaceType;
		crText = pView->GetTextColorByColorInfo2(currentType.GetColorInfo(), text.GetColorInfo());
		crBack = pView->GetBackColorByColorInfo2(currentType.GetColorInfo(), back.GetColorInfo());
		bBold = currentType2.IsBoldFont();
	}else {
		TypeSupport& text = spaceType.GetTextColor() == textType.GetTextColor() ? currentType : spaceType;
		TypeSupport& back = spaceType.GetBackColor() == textType.GetBackColor() ? currentType1 : spaceType;
		crText = text.GetTextColor();
		crBack = back.GetBackColor();
		bBold = currentType.IsBoldFont();
	}
	//spaceType.SetGraphicsState_WhileThisObj(pInfo->gr);

	pInfo->m_gr.PushTextForeColor(crText);
	pInfo->m_gr.PushTextBackColor(crBack);
	// Figure�������w��Ȃ炱����ŉ������w��B���̐F�̂ق��������w��Ȃ�ADrawImp_DrawUnderline�ŉ��������w��
	Font sFont;
	sFont.m_fontAttr.bBoldFont = spaceType.IsBoldFont() || bBold;
	sFont.m_fontAttr.bUnderLine = spaceType.HasUnderLine();
	sFont.m_hFont = pInfo->m_pView->GetFontset().ChooseFontHandle(sFont.m_fontAttr);
	pInfo->m_gr.PushMyFont(sFont);
	bool bTrans = pView->IsBkBitmap() && textType.GetBackColor() == crBack;
	return bTrans;
}

void FigureSpace::DrawImp_StylePop(ColorStrategyInfo* pInfo)
{
	pInfo->m_gr.PopTextForeColor();
	pInfo->m_gr.PopTextBackColor();
	pInfo->m_gr.PopMyFont();
}

void FigureSpace::DrawImp_DrawUnderline(ColorStrategyInfo* pInfo, DispPos& pos)
{
	EditView* pView = pInfo->m_pView;

	TypeSupport cCurrentType(pView, pInfo->GetCurrentColor());	// ���ӂ̐F
	bool blendColor = pInfo->GetCurrentColor() != pInfo->GetCurrentColor2() && cCurrentType.GetTextColor() == cCurrentType.GetBackColor(); // �I�������F

	TypeSupport colorStyle(pView, blendColor ? pInfo->GetCurrentColor2() : pInfo->GetCurrentColor());	// ���ӂ̐F
	TypeSupport cSpaceType(pView, GetDispColorIdx());	// �󔒂̎w��F

	if (!cSpaceType.HasUnderLine() && colorStyle.HasUnderLine()) {
		// ���������ӂ̑O�i�F�ŕ`�悷��
		Font sFont;
		sFont.m_fontAttr.bBoldFont = false;
		sFont.m_fontAttr.bUnderLine = true;
		sFont.m_hFont = pInfo->m_pView->GetFontset().ChooseFontHandle(sFont.m_fontAttr);
		pInfo->m_gr.PushMyFont(sFont);

		int nLength = (Int)(pInfo->m_pDispPos->GetDrawCol() - pos.GetDrawCol());
		std::vector<wchar_t> szText(nLength);
		wchar_t* pszText = &szText[0];
		for (int i=0; i<nLength; ++i)
			pszText[i] = L' ';
		pInfo->m_pView->GetTextDrawer().DispText(
			pInfo->m_gr,
			&pos,
			pszText,
			nLength,
			true		// �w�i�͓���
		);
		pInfo->m_gr.PopMyFont();
	}
}

