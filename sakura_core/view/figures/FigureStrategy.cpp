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
//                         描画統合                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      FigureSpace                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
bool FigureSpace::DrawImp(ColorStrategyInfo& csInfo)
{
	bool bTrans = DrawImp_StyleSelect(csInfo);
	DispPos sPos(*csInfo.pDispPos);	// 現在位置を覚えておく
	DispSpace(csInfo.gr, csInfo.pDispPos, csInfo.view, bTrans);	// 空白描画
	DrawImp_StylePop(csInfo);
	DrawImp_DrawUnderline(csInfo, sPos);
	// 1文字前提
	csInfo.nPosInLogic += (int)NativeW::GetSizeOfChar(	// 行末以外はここでスキャン位置を１字進める
		csInfo.pLineOfLogic,
		csInfo.GetDocLine()->GetLengthWithoutEOL(),
		csInfo.GetPosInLogic()
		);
	return true;
}

bool FigureSpace::DrawImp_StyleSelect(ColorStrategyInfo& csInfo)
{
	// この DrawImp はここ（基本クラス）でデフォルト動作を実装しているが
	// 仮想関数なので派生クラス側のオーバーライドで個別に仕様変更可能
	auto& view = csInfo.view;

	TypeSupport currentType(view, csInfo.GetCurrentColor());	// 周辺の色（現在の指定色/選択色）
	TypeSupport currentType2(view, csInfo.GetCurrentColor2());	// 周辺の色（現在の指定色）
	TypeSupport textType(view, COLORIDX_TEXT);				// テキストの指定色
	TypeSupport spaceType(view, GetDispColorIdx());	// 空白の指定色
	TypeSupport currentTypeBg(view, csInfo.GetCurrentColorBg());
	TypeSupport& currentType1 = (currentType.GetBackColor() == textType.GetBackColor() ? currentTypeBg: currentType);
	TypeSupport& currentType3 = (currentType2.GetBackColor() == textType.GetBackColor() ? currentTypeBg: currentType2);

	// 空白記号類は特に明示指定した部分以外はなるべく周辺の指定に合わせるようにしてみた	// 2009.05.30 ryoji
	// 例えば、下線を指定していない場合、正規表現キーワード内なら正規表現キーワード側の下線指定に従うほうが自然な気がする。
	// （そのほうが空白記号の「表示」をチェックしていない場合の表示に近い）
	//
	// 前景色・背景色の扱い
	// ・通常テキストとは異なる色が指定されている場合は空白記号の側の指定色を使う
	// ・通常テキストと同じ色が指定されている場合は周辺の色指定に合わせる
	// 太字の扱い
	// ・空白記号か周辺のどちらか一方でも太字指定されていれば「前景色・背景色の扱い」で決定した前景色で太字にする
	// 下線の扱い
	// ・空白記号で下線指定されていれば「前景色・背景色の扱い」で決定した前景色で下線を引く
	// ・空白記号で下線指定されておらず周辺で下線指定されていれば周辺の前景色で下線を引く
	// [選択]レンダリング中
	// ・混合色の場合は従来通り。
	COLORREF crText;
	COLORREF crBack;
	bool blendColor = csInfo.GetCurrentColor() != csInfo.GetCurrentColor2() && currentType.GetTextColor() == currentType.GetBackColor(); // 選択混合色
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
	// Figureが下線指定ならこちらで下線を指定。元の色のほうが下線指定なら、DrawImp_DrawUnderlineで下線だけ指定
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

	TypeSupport cCurrentType(view, csInfo.GetCurrentColor());	// 周辺の色
	bool blendColor = csInfo.GetCurrentColor() != csInfo.GetCurrentColor2() && cCurrentType.GetTextColor() == cCurrentType.GetBackColor(); // 選択混合色

	TypeSupport colorStyle(view, blendColor ? csInfo.GetCurrentColor2() : csInfo.GetCurrentColor());	// 周辺の色
	TypeSupport cSpaceType(view, GetDispColorIdx());	// 空白の指定色

	if (!cSpaceType.HasUnderLine() && colorStyle.HasUnderLine()) {
		// 下線を周辺の前景色で描画する
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
			true		// 背景は透明
		);
		csInfo.gr.PopMyFont();
	}
}

