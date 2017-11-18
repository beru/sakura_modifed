#include "StdAfx.h"
#include "ViewFont.h"

/*! フォント作成
*/
void ViewFont::CreateFont(const LOGFONT* plf)
{
	LOGFONT	lf;
	auto& csWindow = GetDllShareData().common.window;
	int miniSize = csWindow.nMiniMapFontSize;
	int quality = csWindow.nMiniMapQuality;
	int outPrec = OUT_TT_ONLY_PRECIS;	// FixedSys等でMiniMapのフォントが小さくならない修正

	// フォント作成
	lf = *plf;
	if (bMiniMap) {
		lf.lfHeight = miniSize;
		lf.lfQuality = quality;
		lf.lfOutPrecision = outPrec;
	}
	hFont_HAN = CreateFontIndirect(&lf);
	logFont = lf;

	// 太字フォント作成
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

	// 下線フォント作成
	lf = *plf;
	if (bMiniMap) {
		lf.lfHeight = miniSize;
		lf.lfQuality = quality;
		lf.lfOutPrecision = outPrec;
	}
	
	lf.lfUnderline = TRUE;
	hFont_HAN_UL = CreateFontIndirect(&lf);

	// 太字下線フォント作成
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

// フォント削除
void ViewFont::DeleteFont()
{
	DeleteObject(hFont_HAN);
	DeleteObject(hFont_HAN_BOLD);
	DeleteObject(hFont_HAN_UL);
	DeleteObject(hFont_HAN_BOLD_UL);
}

/*! フォントを選ぶ
	@param bBoldFont trueで太字
	@param bUnderLine trueで下線
*/
HFONT ViewFont::ChooseFontHandle(FontAttr fontAttr) const
{
	if (fontAttr.bBoldFont) {		// 太字か
		if (fontAttr.bUnderLine) {	// 下線か
			return hFont_HAN_BOLD_UL;
		}else {
			return hFont_HAN_BOLD;
		}
	}else {
		if (fontAttr.bUnderLine) {	// 下線か
			return hFont_HAN_UL;
		}else {
			return hFont_HAN;
		}
	}
}

