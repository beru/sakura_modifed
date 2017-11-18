#include "StdAfx.h"
#include "view/EditView.h" // ColorStrategyInfo
#include "view/ViewFont.h"
#include "FigureStrategy.h"
#include "doc/layout/Layout.h"
#include "charset/charcode.h"
#include "types/TypeSupport.h"

bool Figure_Text::DrawImp(ColorStrategyInfo& csInfo)
{
	int nIdx = csInfo.GetPosInLogic();
	size_t nLength = NativeW::GetSizeOfChar(
						csInfo.pLineOfLogic,
						csInfo.GetDocLine()->GetLengthWithoutEOL(),
						nIdx
					);
	bool bTrans = csInfo.view.IsBkBitmap() && TypeSupport(csInfo.view, COLORIDX_TEXT).GetBackColor() == csInfo.gr.GetTextBackColor();
	csInfo.view.GetTextDrawer().DispText(
		csInfo.gr,
		csInfo.pDispPos,
		&csInfo.pLineOfLogic[nIdx],
		nLength,
		bTrans
	);
	csInfo.nPosInLogic += (int)nLength;
	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�擝��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      FigureSpace                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
bool FigureSpace::DrawImp(ColorStrategyInfo& csInfo)
{
	bool bTrans = DrawImp_StyleSelect(csInfo);
	DispPos sPos(*csInfo.pDispPos);	// ���݈ʒu���o���Ă���
	DispSpace(csInfo.gr, csInfo.pDispPos, csInfo.view, bTrans);	// �󔒕`��
	DrawImp_StylePop(csInfo);
	DrawImp_DrawUnderline(csInfo, sPos);
	// 1�����O��
	csInfo.nPosInLogic += (int)NativeW::GetSizeOfChar(	// �s���ȊO�͂����ŃX�L�����ʒu���P���i�߂�
		csInfo.pLineOfLogic,
		csInfo.GetDocLine()->GetLengthWithoutEOL(),
		csInfo.GetPosInLogic()
		);
	return true;
}

bool FigureSpace::DrawImp_StyleSelect(ColorStrategyInfo& csInfo)
{
	// ���� DrawImp �͂����i��{�N���X�j�Ńf�t�H���g������������Ă��邪
	// ���z�֐��Ȃ̂Ŕh���N���X���̃I�[�o�[���C�h�ŌʂɎd�l�ύX�\
	auto& view = csInfo.view;

	TypeSupport currentType(view, csInfo.GetCurrentColor());	// ���ӂ̐F�i���݂̎w��F/�I��F�j
	TypeSupport currentType2(view, csInfo.GetCurrentColor2());	// ���ӂ̐F�i���݂̎w��F�j
	TypeSupport textType(view, COLORIDX_TEXT);				// �e�L�X�g�̎w��F
	TypeSupport spaceType(view, GetDispColorIdx());	// �󔒂̎w��F
	TypeSupport currentTypeBg(view, csInfo.GetCurrentColorBg());
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
	bool blendColor = csInfo.GetCurrentColor() != csInfo.GetCurrentColor2() && currentType.GetTextColor() == currentType.GetBackColor(); // �I�������F
	bool bBold;
	if (blendColor) {
		TypeSupport& text = spaceType.GetTextColor() == textType.GetTextColor() ? currentType2 : spaceType;
		TypeSupport& back = spaceType.GetBackColor() == textType.GetBackColor() ? currentType3 : spaceType;
		crText = view.GetTextColorByColorInfo2(currentType.GetColorInfo(), text.GetColorInfo());
		crBack = view.GetBackColorByColorInfo2(currentType.GetColorInfo(), back.GetColorInfo());
		bBold = currentType2.IsBoldFont();
	}else {
		TypeSupport& text = spaceType.GetTextColor() == textType.GetTextColor() ? currentType : spaceType;
		TypeSupport& back = spaceType.GetBackColor() == textType.GetBackColor() ? currentType1 : spaceType;
		crText = text.GetTextColor();
		crBack = back.GetBackColor();
		bBold = currentType.IsBoldFont();
	}
	//spaceType.SetGraphicsState_WhileThisObj(pInfo->gr);

	csInfo.gr.PushTextForeColor(crText);
	csInfo.gr.PushTextBackColor(crBack);
	// Figure�������w��Ȃ炱����ŉ������w��B���̐F�̂ق��������w��Ȃ�ADrawImp_DrawUnderline�ŉ��������w��
	Font font;
	font.fontAttr.bBoldFont = spaceType.IsBoldFont() || bBold;
	font.fontAttr.bUnderLine = spaceType.HasUnderLine();
	font.hFont = csInfo.view.GetFontset().ChooseFontHandle(font.fontAttr);
	csInfo.gr.PushMyFont(font);
	bool bTrans = view.IsBkBitmap() && textType.GetBackColor() == crBack;
	return bTrans;
}

void FigureSpace::DrawImp_StylePop(ColorStrategyInfo& csInfo)
{
	csInfo.gr.PopTextForeColor();
	csInfo.gr.PopTextBackColor();
	csInfo.gr.PopMyFont();
}

void FigureSpace::DrawImp_DrawUnderline(ColorStrategyInfo& csInfo, DispPos& pos)
{
	EditView& view = csInfo.view;

	TypeSupport cCurrentType(view, csInfo.GetCurrentColor());	// ���ӂ̐F
	bool blendColor = csInfo.GetCurrentColor() != csInfo.GetCurrentColor2() && cCurrentType.GetTextColor() == cCurrentType.GetBackColor(); // �I�������F

	TypeSupport colorStyle(view, blendColor ? csInfo.GetCurrentColor2() : csInfo.GetCurrentColor());	// ���ӂ̐F
	TypeSupport cSpaceType(view, GetDispColorIdx());	// �󔒂̎w��F

	if (!cSpaceType.HasUnderLine() && colorStyle.HasUnderLine()) {
		// ���������ӂ̑O�i�F�ŕ`�悷��
		Font font;
		font.fontAttr.bBoldFont = false;
		font.fontAttr.bUnderLine = true;
		font.hFont = csInfo.view.GetFontset().ChooseFontHandle(font.fontAttr);
		csInfo.gr.PushMyFont(font);

		ASSERT_GE(csInfo.pDispPos->GetDrawCol(), (int)pos.GetDrawCol());
		size_t nLength = csInfo.pDispPos->GetDrawCol() - pos.GetDrawCol();
		std::vector<wchar_t> szText(nLength);
		wchar_t* pszText = &szText[0];
		for (size_t i=0; i<nLength; ++i) {
			pszText[i] = L' ';
		}
		csInfo.view.GetTextDrawer().DispText(
			csInfo.gr,
			&pos,
			pszText,
			nLength,
			true		// �w�i�͓���
		);
		csInfo.gr.PopMyFont();
	}
}

