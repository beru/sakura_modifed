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
	int nLength = NativeW::GetSizeOfChar(	// サロゲートペア対策	2008.10.12 ryoji
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
//                         描画統合                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      FigureSpace                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
bool FigureSpace::DrawImp(ColorStrategyInfo* pInfo)
{
	bool bTrans = DrawImp_StyleSelect(pInfo);
	DispPos sPos(*pInfo->m_pDispPos);	// 現在位置を覚えておく
	DispSpace(pInfo->m_gr, pInfo->m_pDispPos, pInfo->m_pView, bTrans);	// 空白描画
	DrawImp_StylePop(pInfo);
	DrawImp_DrawUnderline(pInfo, sPos);
	// 1文字前提
	pInfo->m_nPosInLogic += NativeW::GetSizeOfChar(	// 行末以外はここでスキャン位置を１字進める
		pInfo->m_pLineOfLogic,
		pInfo->GetDocLine()->GetLengthWithoutEOL(),
		pInfo->GetPosInLogic()
		);
	return true;
}

bool FigureSpace::DrawImp_StyleSelect(ColorStrategyInfo* pInfo)
{
	// この DrawImp はここ（基本クラス）でデフォルト動作を実装しているが
	// 仮想関数なので派生クラス側のオーバーライドで個別に仕様変更可能
	EditView* pView = pInfo->m_pView;

	TypeSupport currentType(pView, pInfo->GetCurrentColor());	// 周辺の色（現在の指定色/選択色）
	TypeSupport currentType2(pView, pInfo->GetCurrentColor2());	// 周辺の色（現在の指定色）
	TypeSupport textType(pView, COLORIDX_TEXT);				// テキストの指定色
	TypeSupport spaceType(pView, GetDispColorIdx());	// 空白の指定色
	TypeSupport currentTypeBg(pView, pInfo->GetCurrentColorBg());
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
	bool blendColor = pInfo->GetCurrentColor() != pInfo->GetCurrentColor2() && currentType.GetTextColor() == currentType.GetBackColor(); // 選択混合色
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
	// Figureが下線指定ならこちらで下線を指定。元の色のほうが下線指定なら、DrawImp_DrawUnderlineで下線だけ指定
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

	TypeSupport cCurrentType(pView, pInfo->GetCurrentColor());	// 周辺の色
	bool blendColor = pInfo->GetCurrentColor() != pInfo->GetCurrentColor2() && cCurrentType.GetTextColor() == cCurrentType.GetBackColor(); // 選択混合色

	TypeSupport colorStyle(pView, blendColor ? pInfo->GetCurrentColor2() : pInfo->GetCurrentColor());	// 周辺の色
	TypeSupport cSpaceType(pView, GetDispColorIdx());	// 空白の指定色

	if (!cSpaceType.HasUnderLine() && colorStyle.HasUnderLine()) {
		// 下線を周辺の前景色で描画する
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
			true		// 背景は透明
		);
		pInfo->m_gr.PopMyFont();
	}
}

