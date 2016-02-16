#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Figure_ZenSpace.h"
#include "types/TypeSupport.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      Figure_ZenSpace                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Figure_ZenSpace::Match(const wchar_t* pText, int nTextLen) const
{
	return (pText[0] == L'　');
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         描画実装                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 全角スペース描画
void Figure_ZenSpace::DispSpace(Graphics& gr, DispPos* pDispPos, EditView* pView, bool bTrans) const
{
	// クリッピング矩形を計算。画面外なら描画しない
	RECT rc;
	if (pView->GetTextArea().GenerateClipRect(&rc, *pDispPos, 2)) {
		// 描画
		const wchar_t* szZenSpace =
			TypeSupport(pView, COLORIDX_ZENSPACE).IsDisp() ? L"□" : L"　";
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			szZenSpace,
			wcslen(szZenSpace),
			pView->GetTextMetrics().GetDxArray_AllZenkaku()
		);
	}

	// 位置進める
	pDispPos->ForwardDrawCol(2);
}

