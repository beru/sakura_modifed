#include "StdAfx.h"
#include "ViewFont.h"

/*! �t�H���g�쐬
*/
void ViewFont::CreateFont(const LOGFONT* plf)
{
	LOGFONT	lf;
	auto& csWindow = GetDllShareData().common.window;
	int miniSize = csWindow.nMiniMapFontSize;
	int quality = csWindow.nMiniMapQuality;
	int outPrec = OUT_TT_ONLY_PRECIS;	// FixedSys����MiniMap�̃t�H���g���������Ȃ�Ȃ��C��

	// �t�H���g�쐬
	lf = *plf;
	if (bMiniMap) {
		lf.lfHeight = miniSize;
		lf.lfQuality = quality;
		lf.lfOutPrecision = outPrec;
	}
	hFont_HAN = CreateFontIndirect(&lf);
	logFont = lf;

	// �����t�H���g�쐬
	lf = *plf;
	if (bMiniMap) {
		lf.lfHeight = miniSize;
		lf.lfQuality = quality;
		lf.lfOutPrecision = outPrec;
	}
	lf.lfWeight += 300;
	if (1000 < lf.lfWeight) {
		lf.lfWeight = 1000;
	}
	hFont_HAN_BOLD = CreateFontIndirect(&lf);

	// �����t�H���g�쐬
	lf = *plf;
	if (bMiniMap) {
		lf.lfHeight = miniSize;
		lf.lfQuality = quality;
		lf.lfOutPrecision = outPrec;
	}
	
	lf.lfUnderline = TRUE;
	hFont_HAN_UL = CreateFontIndirect(&lf);

	// ���������t�H���g�쐬
	lf = *plf;
	if (bMiniMap) {
		lf.lfHeight = miniSize;
		lf.lfQuality = quality;
		lf.lfOutPrecision = outPrec;
	}
	lf.lfUnderline = TRUE;
	lf.lfWeight += 300;
	if (1000 < lf.lfWeight) {
		lf.lfWeight = 1000;
	}
	hFont_HAN_BOLD_UL = CreateFontIndirect(&lf);
}

// �t�H���g�폜
void ViewFont::DeleteFont()
{
	DeleteObject(hFont_HAN);
	DeleteObject(hFont_HAN_BOLD);
	DeleteObject(hFont_HAN_UL);
	DeleteObject(hFont_HAN_BOLD_UL);
}

/*! �t�H���g��I��
	@param bBoldFont true�ő���
	@param bUnderLine true�ŉ���
*/
HFONT ViewFont::ChooseFontHandle(FontAttr fontAttr) const
{
	if (fontAttr.bBoldFont) {		// ������
		if (fontAttr.bUnderLine) {	// ������
			return hFont_HAN_BOLD_UL;
		}else {
			return hFont_HAN_BOLD;
		}
	}else {
		if (fontAttr.bUnderLine) {	// ������
			return hFont_HAN_UL;
		}else {
			return hFont_HAN;
		}
	}
}

