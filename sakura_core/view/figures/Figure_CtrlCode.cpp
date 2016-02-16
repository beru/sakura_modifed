#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Figure_CtrlCode.h"
#include "types/TypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     Figure_CtrlCode                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Figure_CtrlCode::Match(const wchar_t* pText, int nTextLen) const
{
	// 当面はASCII制御文字（C0 Controls, IsHankaku()で半角扱い）だけを制御文字表示にする
	// そうしないと IsHankaku(0x0600) == false なのに iswcntrl(0x0600) != 0 のようなケースで表示桁がずれる
	// U+0600: ARABIC NUMBER SIGN
	return (!(pText[0] & 0xFF80) && WCODE::IsControlCode(pText[0]));
}

void Figure_CtrlCode::DispSpace(Graphics& gr, DispPos* pDispPos, EditView* pView, bool bTrans) const
{
	// クリッピング矩形を計算。画面外なら描画しない
	RECT rc;
	if (pView->GetTextArea().GenerateClipRect(&rc, *pDispPos, 1)) {
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			L"･",
			1,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}

	// 位置進める
	pDispPos->ForwardDrawCol(1);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     Figure_HanBinary                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Figure_HanBinary::Match(const wchar_t* pText, int nTextLen) const
{
	int nLen = pText[1]? 2:1;	// ※ pText は常に終端よりも手前
	if (NativeW::GetKetaOfChar(pText, nLen, 0) == 1) {	// 半角
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
	// クリッピング矩形を計算。画面外なら描画しない
	RECT rc;
	if (pView->GetTextArea().GenerateClipRect(&rc, *pDispPos, 1)) {
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			L"〓",
			1,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}

	// 位置進める
	pDispPos->ForwardDrawCol(1);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     Figure_ZenBinary                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Figure_ZenBinary::Match(const wchar_t* pText, int nTextLen) const
{
	int nLen = pText[1]? 2:1;	// ※ pText は常に終端よりも手前
	if (NativeW::GetKetaOfChar(pText, nLen, 0) > 1) {	// 全角
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
	// クリッピング矩形を計算。画面外なら描画しない
	RECT rc;
	if (pView->GetTextArea().GenerateClipRect(&rc, *pDispPos, 2)) {
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			L"〓",
			1,
			pView->GetTextMetrics().GetDxArray_AllZenkaku()
		);
	}

	// 位置進める
	pDispPos->ForwardDrawCol(2);
}


