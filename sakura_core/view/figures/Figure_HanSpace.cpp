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
//                         描画実装                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 半角スペース描画
void Figure_HanSpace::DispSpace(Graphics& gr, DispPos* pDispPos, EditView* pView, bool bTrans) const
{
	// クリッピング矩形を計算。画面外なら描画しない
	Rect rcClip;
	if (pView->GetTextArea().GenerateClipRect(&rcClip, *pDispPos, 1)) {
		// 小文字"o"の下半分を出力
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
		
		// 上半分は普通の空白で出力（"o"の上半分を消す）
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

	// 位置進める
	pDispPos->ForwardDrawCol(1);
}

