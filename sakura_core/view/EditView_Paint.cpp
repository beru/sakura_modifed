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
#include <vector>
#include <limits.h>
#include "view/EditView_Paint.h"
#include "view/EditView.h"
#include "view/ViewFont.h"
#include "view/Ruler.h"
#include "view/colors/ColorStrategy.h"
#include "view/colors/Color_Found.h"
#include "view/figures/FigureManager.h"
#include "types/TypeSupport.h"
#include "doc/EditDoc.h"
#include "doc/layout/Layout.h"
#include "window/EditWnd.h"
#include "parse/WordParse.h"
#include "util/string_ex2.h"
#ifdef USE_SSE2
#ifdef __MINGW32__
#include <x86intrin.h>
#else
#include <intrin.h>
#endif
#endif

void _DispWrap(Graphics& gr, DispPos* pDispPos, const EditView* pView, LayoutYInt nLineNum);

/*
	PaintAreaType::LineNumber = (1<<0), // 行番号
	PaintAreaType::Ruler      = (1<<1), // ルーラー
	PaintAreaType::Body       = (1<<2), // 本文
*/

void EditView_Paint::Call_OnPaint(
	int nPaintFlag,   // 描画する領域を選択する
	bool bUseMemoryDC // メモリDCを使用する
	)
{
	EditView* pView = GetEditView();

	auto& textArea = pView->GetTextArea();
	// 各要素
	Rect rcLineNumber(0, textArea.GetAreaTop(), textArea.GetAreaLeft(), textArea.GetAreaBottom());
	Rect rcRuler(textArea.GetAreaLeft(), 0, textArea.GetAreaRight(), textArea.GetAreaTop());
	Rect rcBody(textArea.GetAreaLeft(), textArea.GetAreaTop(), textArea.GetAreaRight(), textArea.GetAreaBottom());

	// 領域を作成 -> rc
	std::vector<Rect> rcs;
	if (nPaintFlag & (int)PaintAreaType::LineNumber) rcs.push_back(rcLineNumber);
	if (nPaintFlag & (int)PaintAreaType::Ruler) rcs.push_back(rcRuler);
	if (nPaintFlag & (int)PaintAreaType::Body) rcs.push_back(rcBody);
	if (rcs.size() == 0) return;
	Rect rc = rcs[0];
	int nSize = (int)rcs.size();
	for (int i=1; i<nSize; ++i) {
		rc = MergeRect(rc, rcs[i]);
	}

	// 描画
	PAINTSTRUCT	ps;
	ps.rcPaint = rc;
	HDC hdc = pView->GetDC();
	pView->OnPaint(hdc, &ps, bUseMemoryDC);
	pView->ReleaseDC(hdc);
}


/* フォーカス移動時の再描画

	@date 2001/06/21 asa-o 「スクロールバーの状態を更新する」「カーソル移動」削除
*/
void EditView::RedrawAll()
{
	if (!GetHwnd()) {
		return;
	}
	
	if (GetDrawSwitch()) {
		// ウィンドウ全体を再描画
		PAINTSTRUCT	ps;
		HDC hdc = ::GetDC(GetHwnd());
		::GetClientRect(GetHwnd(), &ps.rcPaint);
		OnPaint(hdc, &ps, FALSE);
		::ReleaseDC(GetHwnd(), hdc);
	}

	// キャレットの表示
	GetCaret().ShowEditCaret();

	// キャレットの行桁位置を表示する
	GetCaret().ShowCaretPosInfo();

	// 親ウィンドウのタイトルを更新
	m_pEditWnd->UpdateCaption();

	//	Jul. 9, 2005 genta	選択範囲の情報をステータスバーへ表示
	GetSelectionInfo().PrintSelectionInfoMsg();

	// スクロールバーの状態を更新する
	AdjustScrollBars();
}

// 2001/06/21 Start by asa-o 再描画
void EditView::Redraw()
{
	if (!GetHwnd()) {
		return;
	}
	if (!GetDrawSwitch()) {
		return;
	}

	HDC hdc = ::GetDC(GetHwnd());
	PAINTSTRUCT	ps;
	::GetClientRect(GetHwnd(), &ps.rcPaint);
	OnPaint(hdc, &ps, FALSE);
	::ReleaseDC(GetHwnd(), hdc);
}
// 2001/06/21 End

void EditView::RedrawLines(LayoutYInt top, LayoutYInt bottom)
{
	if (!GetHwnd()) {
		return;
	}
	if (!GetDrawSwitch()) {
		return;
	}

	if (bottom < GetTextArea().GetViewTopLine()) {
		return;
	}
	if (GetTextArea().GetBottomLine() <= top) {
		return;
	}
	HDC hdc = GetDC();
	PAINTSTRUCT	ps;
	ps.rcPaint.left = 0;
	ps.rcPaint.right = GetTextArea().GetAreaRight();
	ps.rcPaint.top = GetTextArea().GenerateYPx(top);
	ps.rcPaint.bottom = GetTextArea().GenerateYPx(bottom);
	OnPaint(hdc, &ps, FALSE);
	ReleaseDC(hdc);
}

void MyFillRect(HDC hdc, RECT& re)
{
	::ExtTextOut(hdc, re.left, re.top, ETO_OPAQUE|ETO_CLIPPED, &re, _T(""), 0, NULL);
}

void EditView::DrawBackImage(HDC hdc, RECT& rcPaint, HDC hdcBgImg)
{
#if 0
//	テスト背景パターン
	static int testColorIndex = 0;
	testColorIndex = testColorIndex % 7;
	COLORREF cols[7] = {RGB(255,255,255),
		RGB(200,255,255),RGB(255,200,255),RGB(255,255,200),
		RGB(200,200,255),RGB(255,200,200),RGB(200,255,200),
	};
	COLORREF colorOld = ::SetBkColor(hdc, cols[testColorIndex]);
	MyFillRect(hdc, rcPaint);
	::SetBkColor(hdc, colorOld);
	++testColorIndex;
#else
	TypeSupport textType(this, COLORIDX_TEXT);
	COLORREF colorOld = ::SetBkColor(hdc, textType.GetBackColor());
	const TextArea& area = GetTextArea();
	const EditDoc& doc  = *m_pEditDoc;
	const TypeConfig& typeConfig = doc.m_docType.GetDocumentAttribute();

	Rect rcImagePos;
	switch (typeConfig.backImgPos) {
	case BackgroundImagePosType::TopLeft:
	case BackgroundImagePosType::BottomLeft:
	case BackgroundImagePosType::CenterLeft:
		rcImagePos.left = area.GetAreaLeft();
		break;
	case BackgroundImagePosType::TopRight:
	case BackgroundImagePosType::BottomRight:
	case BackgroundImagePosType::CenterRight:
		rcImagePos.left = area.GetAreaRight() - doc.m_nBackImgWidth;
		break;
	case BackgroundImagePosType::TopCenter:
	case BackgroundImagePosType::BottomCenter:
	case BackgroundImagePosType::Center:
		rcImagePos.left = area.GetAreaLeft() + area.GetAreaWidth()/2 - doc.m_nBackImgWidth/2;
		break;
	default:
		assert_warning(false);
		break;
	}
	switch (typeConfig.backImgPos) {
	case BackgroundImagePosType::TopLeft:
	case BackgroundImagePosType::TopRight:
	case BackgroundImagePosType::TopCenter:
		rcImagePos.top  = area.GetAreaTop();
		break;
	case BackgroundImagePosType::BottomLeft:
	case BackgroundImagePosType::BottomRight:
	case BackgroundImagePosType::BottomCenter:
		rcImagePos.top  = area.GetAreaBottom() - doc.m_nBackImgHeight;
		break;
	case BackgroundImagePosType::CenterLeft:
	case BackgroundImagePosType::CenterRight:
	case BackgroundImagePosType::Center:
		rcImagePos.top  = area.GetAreaTop() + area.GetAreaHeight()/2 - doc.m_nBackImgHeight/2;
		break;
	default:
		assert_warning(false);
		break;
	}
	rcImagePos.left += typeConfig.backImgPosOffset.x;
	rcImagePos.top  += typeConfig.backImgPosOffset.y;
	// スクロール時の画面の端を作画するときの位置あたりへ移動
	if (typeConfig.backImgScrollX) {
		int tile = typeConfig.backImgRepeatX ? doc.m_nBackImgWidth : INT_MAX;
		Int posX = (area.GetViewLeftCol() % tile) * GetTextMetrics().GetHankakuDx();
		rcImagePos.left -= posX % tile;
	}
	if (typeConfig.backImgScrollY) {
		int tile = typeConfig.backImgRepeatY ? doc.m_nBackImgHeight : INT_MAX;
		Int posY = (area.GetViewTopLine() % tile) * GetTextMetrics().GetHankakuDy();
		rcImagePos.top -= posY % tile;
	}
	if (typeConfig.backImgRepeatX) {
		if (0 < rcImagePos.left) {
			// rcImagePos.left = rcImagePos.left - (rcImagePos.left / doc.m_nBackImgWidth + 1) * doc.m_nBackImgWidth;
			rcImagePos.left = rcImagePos.left % doc.m_nBackImgWidth - doc.m_nBackImgWidth;
		}
	}
	if (typeConfig.backImgRepeatY) {
		if (0 < rcImagePos.top) {
			// rcImagePos.top = rcImagePos.top - (rcImagePos.top / doc.m_nBackImgHeight + 1) * doc.m_nBackImgHeight;
			rcImagePos.top = rcImagePos.top % doc.m_nBackImgHeight - doc.m_nBackImgHeight;
		}
	}
	rcImagePos.SetSize(doc.m_nBackImgWidth, doc.m_nBackImgHeight);
	
	RECT rc = rcPaint;
	// rc.left = t_max((int)rc.left, area.GetAreaLeft());
	rc.top  = t_max((int)rc.top,  area.GetRulerHeight()); // ルーラーを除外
	const int nXEnd = area.GetAreaRight();
	const int nYEnd = area.GetAreaBottom();
	Rect rcBltAll;
	rcBltAll.SetLTRB(INT_MAX, INT_MAX, -INT_MAX, -INT_MAX);
	Rect rcImagePosOrg = rcImagePos;
	for (; rcImagePos.top<=nYEnd; ) {
		for (; rcImagePos.left<=nXEnd; ) {
			Rect rcBlt;
			if (::IntersectRect(&rcBlt, &rc, &rcImagePos)) {
				::BitBlt(
					hdc,
					rcBlt.left,
					rcBlt.top,
					rcBlt.right  - rcBlt.left,
					rcBlt.bottom - rcBlt.top,
					hdcBgImg,
					rcBlt.left - rcImagePos.left,
					rcBlt.top - rcImagePos.top,
					SRCCOPY
				);
				rcBltAll.left   = t_min(rcBltAll.left,   rcBlt.left);
				rcBltAll.top    = t_min(rcBltAll.top,    rcBlt.top);
				rcBltAll.right  = t_max(rcBltAll.right,  rcBlt.right);
				rcBltAll.bottom = t_max(rcBltAll.bottom, rcBlt.bottom);
			}
			rcImagePos.left  += doc.m_nBackImgWidth;
			rcImagePos.right += doc.m_nBackImgWidth;
			if (!typeConfig.backImgRepeatX) {
				break;
			}
		}
		rcImagePos.left  = rcImagePosOrg.left;
		rcImagePos.right = rcImagePosOrg.right;
		rcImagePos.top    += doc.m_nBackImgHeight;
		rcImagePos.bottom += doc.m_nBackImgHeight;
		if (!typeConfig.backImgRepeatY) {
			break;
		}
	}
	if (rcBltAll.left != INT_MAX) {
		// 上下左右ななめの隙間を埋める
		Rect rcFill;
		LONG& x1 = rc.left;
		LONG& x2 = rcBltAll.left;
		LONG& x3 = rcBltAll.right;
		LONG& x4 = rc.right;
		LONG& y1 = rc.top;
		LONG& y2 = rcBltAll.top;
		LONG& y3 = rcBltAll.bottom;
		LONG& y4 = rc.bottom;
		if (y1 < y2) {
			rcFill.SetLTRB(x1,y1, x4,y2); MyFillRect(hdc, rcFill);
		}
		if (x1 < x2) {
			rcFill.SetLTRB(x1,y2, x2,y3); MyFillRect(hdc, rcFill);
		}
		if (x3 < x4) {
			rcFill.SetLTRB(x3,y2, x4,y3); MyFillRect(hdc, rcFill);
		}
		if (y3 < y4) {
			rcFill.SetLTRB(x1,y3, x4,y4); MyFillRect(hdc, rcFill);
		}
	}else {
		MyFillRect(hdc, rc);
	}
	::SetBkColor(hdc, colorOld);
#endif
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          色設定                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! 指定位置のColorIndexの取得
	EditView::DrawLogicLineを元にしたためEditView::DrawLogicLineに
	修正があった場合は、ここも修正が必要。
*/
Color3Setting EditView::GetColorIndex(
	const Layout*			pLayout,
	LayoutYInt				nLineNum,
	int						nIndex,
	ColorStrategyInfo*	 	pInfo,			// 2010.03.31 ryoji 追加
	bool					bPrev			// 指定位置の色変更直前まで	2010.06.19 ryoji 追加
	)
{
	EColorIndexType eRet = COLORIDX_TEXT;

	if (!pLayout) {
		Color3Setting cColor = { COLORIDX_TEXT, COLORIDX_TEXT, COLORIDX_TEXT };
		return cColor;
	}
	// 2014.12.30 Skipモードの時もCOLORIDX_TEXT
	if (ColorStrategyPool::getInstance()->IsSkipBeforeLayout()) {
		Color3Setting cColor = { COLORIDX_TEXT, COLORIDX_TEXT, COLORIDX_TEXT };
		return cColor;
	}

	const LayoutColorInfo* colorInfo;
	const Layout* pLayoutLineFirst = pLayout;
	LayoutYInt nLineNumFirst = nLineNum;
	{
		// 2002/2/10 aroka CMemory変更
		pInfo->pLineOfLogic = pLayout->GetDocLineRef()->GetPtr();

		// 論理行の最初のレイアウト情報を取得 -> pLayoutLineFirst
		while (pLayoutLineFirst->GetLogicOffset() != 0) {
			pLayoutLineFirst = pLayoutLineFirst->GetPrevLayout();
			--nLineNumFirst;

			// 論理行の先頭まで戻らないと確実には正確な色は得られない
			// （正規表現キーワードにマッチした長い強調表示がその位置のレイアウト行頭をまたいでいる場合など）
			//if (pLayout->GetLogicOffset() - pLayoutLineFirst->GetLogicOffset() > 260)
			//	break;
		}

		// 2005.11.20 Moca 色が正しくないことがある問題に対処
		eRet = pLayoutLineFirst->GetColorTypePrev();	// 現在の色を指定	// 02/12/18 ai
		colorInfo = pLayoutLineFirst->GetColorInfo();
		pInfo->nPosInLogic = pLayoutLineFirst->GetLogicOffset();

		// ColorStrategyPool初期化
		ColorStrategyPool* pool = ColorStrategyPool::getInstance();
		pool->SetCurrentView(this);
		pool->NotifyOnStartScanLogic();

		// 2009.02.07 ryoji この関数では pInfo->CheckChangeColor() で色を調べるだけなので以下の処理は不要
		//
		////############超仮。本当はVisitorを使うべき
		//class TmpVisitor{
		//public:
		//	static int CalcLayoutIndex(const Layout* pLayout)
		//	{
		//		int n = -1;
		//		while (pLayout) {
		//			pLayout = pLayout->GetPrevLayout(); // prev or null
		//			++n;
		//		}
		//		return n;
		//	}
		//};
		//pInfo->pDispPos->SetLayoutLineRef(LayoutInt(TmpVisitor::CalcLayoutIndex(pLayout)));
		// 2013.12.11 Moca カレント行の色替えで必要になりました
		pInfo->pDispPos->SetLayoutLineRef(nLineNumFirst);
	}

	// 文字列参照
	const DocLine* pDocLine = pLayout->GetDocLineRef();
	StringRef lineStr(pDocLine->GetPtr(), pDocLine->GetLengthWithEOL());

	// color strategy
	ColorStrategyPool* pool = ColorStrategyPool::getInstance();
	pInfo->pStrategy = pool->GetStrategyByColor(eRet);
	if (pInfo->pStrategy) {
		pInfo->pStrategy->InitStrategyStatus();
		pInfo->pStrategy->SetStrategyColorInfo(colorInfo);
	}

	const Layout* pLayoutNext = pLayoutLineFirst->GetNextLayout();
	LayoutYInt nLineNumScan = nLineNumFirst;
	int nPosTo = pLayout->GetLogicOffset() + t_min(nIndex, (int)pLayout->GetLengthWithEOL() - 1);
	while (pInfo->nPosInLogic <= nPosTo) {
		if (bPrev && pInfo->nPosInLogic == nPosTo)
			break;

		// 色切替
		pInfo->CheckChangeColor(lineStr);

		// 1文字進む
		pInfo->nPosInLogic += NativeW::GetSizeOfChar(
									lineStr.GetPtr(),
									lineStr.GetLength(),
									pInfo->nPosInLogic
								);
		if (pLayoutNext && pLayoutNext->GetLogicOffset() <= pInfo->nPosInLogic) {
			++nLineNumScan;
			pInfo->pDispPos->SetLayoutLineRef(nLineNumScan);
			pLayoutNext = pLayoutNext->GetNextLayout();
		}
	}

	Color3Setting cColor;
	pInfo->DoChangeColor(&cColor);

	return cColor;
}

/*! 現在の色を指定
	@param eColorIndex   選択を含む現在の色
	@param eColorIndex2  選択以外の現在の色
	@param eColorIndexBg 背景色

	@date 2013.05.08 novice 範囲外チェック削除
*/
void EditView::SetCurrentColor(
	Graphics& gr,
	EColorIndexType eColorIndex,
	EColorIndexType eColorIndex2,
	EColorIndexType eColorIndexBg
	)
{
	// インデックス決定
	int nColorIdx = ToColorInfoArrIndex(eColorIndex);
	int nColorIdx2 = ToColorInfoArrIndex(eColorIndex2);
	int nColorIdxBg = ToColorInfoArrIndex(eColorIndexBg);

	// 実際に色を設定
	const ColorInfo& info  = m_pTypeData->colorInfoArr[nColorIdx];
	const ColorInfo& info2 = m_pTypeData->colorInfoArr[nColorIdx2];
	const ColorInfo& infoBg = m_pTypeData->colorInfoArr[nColorIdxBg];
	COLORREF fgcolor = GetTextColorByColorInfo2(info, info2);
	gr.SetTextForeColor(fgcolor);
	// 2012.11.21 背景色がテキストとおなじなら背景色はカーソル行背景
	const ColorInfo& info3 = (info2.colorAttr.cBACK == m_crBack ? infoBg : info2);
	COLORREF bkcolor = (nColorIdx == nColorIdx2) ? info3.colorAttr.cBACK : GetBackColorByColorInfo2(info, info3);
	gr.SetTextBackColor(bkcolor);
	Font font;
	font.fontAttr = (info.colorAttr.cTEXT != info.colorAttr.cBACK) ? info.fontAttr : info2.fontAttr;
	font.hFont = GetFontset().ChooseFontHandle(font.fontAttr);
	gr.SetMyFont(font);
}

inline COLORREF MakeColor2(COLORREF a, COLORREF b, int alpha)
{
#ifdef USE_SSE2
	// (a * alpha + b * (256 - alpha)) / 256 -> ((a - b) * alpha) / 256 + b
	__m128i xmm0, xmm1, xmm2, xmm3;
	COLORREF color;
	xmm0 = _mm_setzero_si128();
	xmm1 = _mm_cvtsi32_si128(a);
	xmm2 = _mm_cvtsi32_si128(b);
	xmm3 = _mm_cvtsi32_si128(alpha);

	xmm1 = _mm_unpacklo_epi8(xmm1, xmm0); // a:a:a:a
	xmm2 = _mm_unpacklo_epi8(xmm2, xmm0); // b:b:b:b
	xmm3 = _mm_shufflelo_epi16(xmm3, 0); // alpha:alpha:alpha:alpha

	xmm1 = _mm_sub_epi16(xmm1, xmm2); // (a - b)
	xmm1 = _mm_mullo_epi16(xmm1, xmm3); // (a - b) * alpha
	xmm1 = _mm_srli_epi16(xmm1, 8); // ((a - b) * alpha) / 256
	xmm1 = _mm_add_epi8(xmm1, xmm2); // ((a - b) * alpha) / 256 + b

	xmm1 = _mm_packus_epi16(xmm1, xmm0);
	color = _mm_cvtsi128_si32(xmm1);

	return color;
#else
	const int ap = alpha;
	const int bp = 256 - ap;
	BYTE valR = (BYTE)((GetRValue(a) * ap + GetRValue(b) * bp) / 256);
	BYTE valG = (BYTE)((GetGValue(a) * ap + GetGValue(b) * bp) / 256);
	BYTE valB = (BYTE)((GetBValue(a) * ap + GetBValue(b) * bp) / 256);
	return RGB(valR, valG, valB);
#endif
}

COLORREF EditView::GetTextColorByColorInfo2(const ColorInfo& info, const ColorInfo& info2)
{
	if (info.colorAttr.cTEXT != info.colorAttr.cBACK) {
		return info.colorAttr.cTEXT;
	}
	// 反転表示
	if (info.colorAttr.cBACK == m_crBack) {
		return  info2.colorAttr.cTEXT ^ 0x00FFFFFF;
	}
	int alpha = 255*30/100; // 30%
	return MakeColor2(info.colorAttr.cTEXT, info2.colorAttr.cTEXT, alpha);
}

COLORREF EditView::GetBackColorByColorInfo2(const ColorInfo& info, const ColorInfo& info2)
{
	if (info.colorAttr.cTEXT != info.colorAttr.cBACK) {
		return info.colorAttr.cBACK;
	}
	// 反転表示
	if (info.colorAttr.cBACK == m_crBack) {
		return  info2.colorAttr.cBACK ^ 0x00FFFFFF;
	}
	int alpha = 255*30/100; // 30%
	return MakeColor2(info.colorAttr.cBACK, info2.colorAttr.cBACK, alpha);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           描画                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void EditView::OnPaint(HDC _hdc, PAINTSTRUCT *pPs, BOOL bDrawFromComptibleBmp)
{
	bool bChangeFont = m_bMiniMap;
	if (bChangeFont) {
		SelectCharWidthCache(CharWidthFontMode::MiniMap, CharWidthCacheMode::Local);
	}
	OnPaint2(_hdc, pPs, bDrawFromComptibleBmp);
	if (bChangeFont) {
		SelectCharWidthCache(CharWidthFontMode::Edit, m_pEditWnd->GetLogfontCacheMode());
	}
}

/*! 通常の描画処理 new 
	@param pPs  pPs.rcPaint は正しい必要がある
	@param bDrawFromComptibleBmp  TRUE 画面バッファからhdcに作画する(コピーするだけ)。
			TRUEの場合、pPs.rcPaint領域外は作画されないが、FALSEの場合は作画される事がある。
			互換DC/BMPが無い場合は、普通の作画処理をする。

	@date 2007.09.09 Moca 元々無効化されていた第三パラメータのbUseMemoryDCをbDrawFromComptibleBmpに変更。
	@date 2009.03.26 ryoji 行番号のみ描画を通常の行描画と分離（効率化）
*/
void EditView::OnPaint2(HDC _hdc, PAINTSTRUCT *pPs, BOOL bDrawFromComptibleBmp)
{
//	MY_RUNNINGTIMER(runningTimer, "EditView::OnPaint");
	Graphics gr(_hdc);

	// 2004.01.28 Moca デスクトップに作画しないように
	if (!GetHwnd() || !_hdc) {
		return;
	}

	if (!GetDrawSwitch()) return;
	//@@@
#if 0
	::MYTRACE(_T("OnPaint(%d,%d)-(%d,%d) : %d\n"),
		pPs->rcPaint.left,
		pPs->rcPaint.top,
		pPs->rcPaint.right,
		pPs->rcPaint.bottom,
		bDrawFromComptibleBmp
		);
#endif
	
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	// 互換BMPからの転送のみによる作画
	if (bDrawFromComptibleBmp
		&& m_hdcCompatDC
		&& m_hbmpCompatBMP
	) {
		::BitBlt(
			gr,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			pPs->rcPaint.right - pPs->rcPaint.left,
			pPs->rcPaint.bottom - pPs->rcPaint.top,
			m_hdcCompatDC,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			SRCCOPY
		);
		if (m_pEditWnd->GetActivePane() == m_nMyIndex) {
			// アクティブペインは、アンダーライン描画
			GetCaret().m_underLine.CaretUnderLineON(true, false);
		}
		return;
	}
	if (m_hdcCompatDC && !m_hbmpCompatBMP
		 || m_nCompatBMPWidth < (pPs->rcPaint.right - pPs->rcPaint.left)
		 || m_nCompatBMPHeight < (pPs->rcPaint.bottom - pPs->rcPaint.top)
	) {
		RECT rect;
		::GetWindowRect(this->GetHwnd(), &rect);
		CreateOrUpdateCompatibleBitmap(rect.right - rect.left, rect.bottom - rect.top);
	}
	// To Here 2007.09.09 Moca

	// キャレットを隠す
	bool bCaretShowFlag_Old = GetCaret().GetCaretShowFlag();	// 2008.06.09 ryoji
	GetCaret().HideCaret_(this->GetHwnd()); // 2002/07/22 novice

	RECT			rc;
	int				nLineHeight = GetTextMetrics().GetHankakuDy();
	int				nCharDx = GetTextMetrics().GetHankakuDx();

	// サポート
	TypeSupport textType(this, COLORIDX_TEXT);

//@@@ 2001.11.17 add start MIK
	// 変更があればタイプ設定を行う。
	if (m_pTypeData->bUseRegexKeyword || m_pRegexKeyword->m_bUseRegexKeyword) { // OFFなのに前回のデータが残ってる
		// タイプ別設定をする。設定済みかどうかは呼び先でチェックする。
		m_pRegexKeyword->RegexKeySetTypes(m_pTypeData);
	}
//@@@ 2001.11.17 add end MIK

	bool bTransText = IsBkBitmap();
	// メモリＤＣを利用した再描画の場合は描画先のＤＣを切り替える
	HDC hdcOld = 0;
	// 2007.09.09 Moca bUseMemoryDCを有効化。
	// bUseMemoryDC = FALSE;
	bool bUseMemoryDC = (m_hdcCompatDC != NULL);
	assert_warning(gr != m_hdcCompatDC);
	if (bUseMemoryDC) {
		hdcOld = gr;
		gr = m_hdcCompatDC;
	}else {
		if (bTransText || pPs->rcPaint.bottom - pPs->rcPaint.top <= 2 || pPs->rcPaint.left - pPs->rcPaint.right <= 2) {
			// 透過処理の場合フォントの輪郭が重ね塗りになるため自分でクリッピング領域を設定
			// 2以下はたぶんアンダーライン・カーソル行縦線の作画
			// MemoryDCの場合は転送が矩形クリッピングの代わりになっている
			gr.SetClipping(pPs->rcPaint);
		}
	}

	// 03/02/18 対括弧の強調表示(消去) ai
	if (!bUseMemoryDC) {
		// MemoryDCだとスクロール時に先に括弧だけ表示されて不自然なので後でやる。
		DrawBracketPair(false);
	}

	EditView& activeView = m_pEditWnd->GetActiveView();
	m_nPageViewTop = activeView.GetTextArea().GetViewTopLine();
	m_nPageViewBottom = activeView.GetTextArea().GetBottomLine();

	// 背景の表示
	if (bTransText) {
		HDC hdcBgImg = CreateCompatibleDC(gr);
		HBITMAP hOldBmp = (HBITMAP)::SelectObject(hdcBgImg, m_pEditDoc->m_hBackImg);
		DrawBackImage(gr, pPs->rcPaint, hdcBgImg);
		SelectObject(hdcBgImg, hOldBmp);
		DeleteObject(hdcBgImg);
	}

	// ルーラーとテキストの間の余白
	//@@@ 2002.01.03 YAZAKI 余白が0のときは無駄でした。
	if (GetTextArea().GetTopYohaku()) {
		if (!bTransText) {
			rc.left   = 0;
			rc.top    = GetTextArea().GetRulerHeight();
			rc.right  = GetTextArea().GetAreaRight();
			rc.bottom = GetTextArea().GetAreaTop();
			textType.FillBack(gr, rc);
		}
	}

	// 行番号の表示
	//	From Here Sep. 7, 2001 genta
	//	Sep. 23, 2002 genta 行番号非表示でも行番号色の帯があるので隙間を埋める
	if (GetTextArea().GetTopYohaku()) {
		if (bTransText && m_pTypeData->colorInfoArr[COLORIDX_GYOU].colorAttr.cBACK == textType.GetBackColor()) {
		}else {
			rc.left   = 0;
			rc.top    = GetTextArea().GetRulerHeight();
			rc.right  = GetTextArea().GetLineNumberWidth(); //	Sep. 23 ,2002 genta 余白はテキスト色のまま残す
			rc.bottom = GetTextArea().GetAreaTop();
			gr.SetTextBackColor(m_pTypeData->colorInfoArr[COLORIDX_GYOU].colorAttr.cBACK);
			gr.FillMyRectTextBackColor(rc);
		}
	}
	//	To Here Sep. 7, 2001 genta

	::SetBkMode(gr, TRANSPARENT);

	textType.SetGraphicsState_WhileThisObj(gr);


	int nTop = pPs->rcPaint.top;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//           描画開始レイアウト絶対行 -> nLayoutLine             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LayoutInt nLayoutLine;
	if (0 > nTop - GetTextArea().GetAreaTop()) {
		nLayoutLine = GetTextArea().GetViewTopLine(); // ビュー上部から描画
	}else {
		nLayoutLine = GetTextArea().GetViewTopLine() + LayoutInt((nTop - GetTextArea().GetAreaTop()) / nLineHeight); // ビュー途中から描画
	}

	// ※ ここにあった描画範囲の 260 文字ロールバック処理は GetColorIndex() に吸収	// 2009.02.11 ryoji

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          描画終了レイアウト絶対行 -> nLayoutLineTo            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LayoutInt nLayoutLineTo = GetTextArea().GetViewTopLine()
		+ LayoutInt((pPs->rcPaint.bottom - GetTextArea().GetAreaTop() + (nLineHeight - 1)) / nLineHeight) - 1;	// 2007.02.17 ryoji 計算を精密化


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         描画座標                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	DispPos pos(GetTextMetrics().GetHankakuDx(), GetTextMetrics().GetHankakuDy());
	pos.InitDrawPos(Point(
		GetTextArea().GetAreaLeft() - (Int)GetTextArea().GetViewLeftCol() * nCharDx,
		GetTextArea().GetAreaTop() + (Int)(nLayoutLine - GetTextArea().GetViewTopLine()) * nLineHeight
	));
	pos.SetLayoutLineRef(nLayoutLine);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      全部の行を描画                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	// 必要な行を描画する	// 2009.03.26 ryoji 行番号のみ描画を通常の行描画と分離（効率化）
	if (pPs->rcPaint.right <= GetTextArea().GetAreaLeft()) {
		while (pos.GetLayoutLineRef() <= nLayoutLineTo) {
			if (!pos.GetLayoutRef())
				break;

			// 1行描画（行番号のみ）
			GetTextDrawer().DispLineNumber(
				gr,
				pos.GetLayoutLineRef(),
				pos.GetDrawPos().y
			);
			// 行を進める
			pos.ForwardDrawLine(1);		// 描画Y座標＋＋
			pos.ForwardLayoutLineRef(1);	// レイアウト行＋＋
		}
	}else {
		while (pos.GetLayoutLineRef() <= nLayoutLineTo) {
			// 描画X位置リセット
			pos.ResetDrawCol();

			// 1行描画
			bool bDispResult = DrawLogicLine(
				gr,
				&pos,
				nLayoutLineTo
			);

//			if (bDispResult) {
//				pPs->rcPaint.bottom += nLineHeight;	// EOF再描画対応
//				break;
//			}
		}
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//              テキストの無い部分の塗りつぶし                 //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if (!bTransText && pos.GetDrawPos().y < pPs->rcPaint.bottom) {
		RECT rcBack;
		rcBack.left   = pPs->rcPaint.left;
		rcBack.right  = pPs->rcPaint.right;
		rcBack.top    = pos.GetDrawPos().y;
		rcBack.bottom = pPs->rcPaint.bottom;

		textType.FillBack(gr, rcBack);
	}
	
	{
		if (!m_bMiniMap) {
			GetTextDrawer().DispNoteLine(gr, pos.GetDrawPos().y, pPs->rcPaint.bottom, pPs->rcPaint.left, pPs->rcPaint.right);
		}
		// 2006.04.29 行部分は行ごとに作画し、ここでは縦線の残りを作画
		GetTextDrawer().DispVerticalLines(gr, pos.GetDrawPos().y, pPs->rcPaint.bottom, LayoutInt(0), LayoutInt(-1));
		GetTextDrawer().DispWrapLine(gr, pos.GetDrawPos().y, pPs->rcPaint.bottom);	// 2009.10.24 ryoji
	}

	textType.RewindGraphicsState(gr);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       ルーラー描画                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if (pPs->rcPaint.top < GetTextArea().GetRulerHeight()) { // ルーラーが再描画範囲にあるときのみ再描画する 2002.02.25 Add By KK
		GetRuler().SetRedrawFlag(); // 2002.02.25 Add By KK ルーラー全体を描画。
		GetRuler().DispRuler(gr);
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     その他後始末など                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// メモリＤＣを利用した再描画の場合はメモリＤＣに描画した内容を画面へコピーする
	if (bUseMemoryDC) {
		// 2010.10.11 先に描くと背景固定のスクロールなどでの表示が不自然になる
		DrawBracketPair(false);

		::BitBlt(
			hdcOld,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			pPs->rcPaint.right - pPs->rcPaint.left,
			pPs->rcPaint.bottom - pPs->rcPaint.top,
			gr,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			SRCCOPY
		);
	}

	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	//     アンダーライン描画をメモリDCからのコピー前処理から後に移動
	if (m_pEditWnd->GetActivePane() == m_nMyIndex) {
		// アクティブペインは、アンダーライン描画
		GetCaret().m_underLine.CaretUnderLineON(true, false);
	}
	// To Here 2007.09.09 Moca

	// 03/02/18 対括弧の強調表示(描画) ai
	DrawBracketPair(true);

	// キャレットを現在位置に表示します
	if (bCaretShowFlag_Old)	// 2008.06.09 ryoji
		GetCaret().ShowCaret_(this->GetHwnd()); // 2002/07/22 novice
	return;
}

/*!
	行のテキスト／選択状態の描画
	1回で1ロジック行分を作画する。

	@return EOFを作画したらtrue

	@date 2001.02.17 MIK
	@date 2001.12.21 YAZAKI 改行記号の描きかたを変更
	@date 2007.08.31 kobake 引数 bDispBkBitmap を削除
*/
bool EditView::DrawLogicLine(
	HDC				_hdc,			// [in]     作画対象
	DispPos*		_pDispPos,		// [in/out] 描画する箇所、描画元ソース
	LayoutInt		nLineTo			// [in]     作画終了するレイアウト行番号
	)
{
//	MY_RUNNINGTIMER(runningTimer, "EditView::DrawLogicLine");
	bool bDispEOF = false;
	ColorStrategyInfo _sInfo;
	ColorStrategyInfo* pInfo = &_sInfo;
	pInfo->gr.Init(_hdc);
	pInfo->pDispPos = _pDispPos;
	pInfo->pView = this;

	// ColorStrategyPool初期化
	ColorStrategyPool* pool = ColorStrategyPool::getInstance();
	pool->SetCurrentView(this);
	pool->NotifyOnStartScanLogic();
	bool bSkipBeforeLayout = pool->IsSkipBeforeLayout();

	// DispPosを保存しておく
	pInfo->dispPosBegin = *pInfo->pDispPos;

	// 処理する文字位置
	pInfo->nPosInLogic = LogicInt(0); // ☆開始

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          論理行データの取得 -> pLine, pLineLen              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 前行の最終設定色
	{
		const Layout* pLayout = pInfo->pDispPos->GetLayoutRef();
		if (bSkipBeforeLayout) {
			EColorIndexType eRet = COLORIDX_TEXT;
			const LayoutColorInfo* colorInfo = NULL;
			if (pLayout) {
				eRet = pLayout->GetColorTypePrev(); // COLORIDX_TEXTのはず
				colorInfo = pLayout->GetColorInfo();
			}
			pInfo->pStrategy = pool->GetStrategyByColor(eRet);
			if (pInfo->pStrategy) {
				pInfo->pStrategy->InitStrategyStatus();
				pInfo->pStrategy->SetStrategyColorInfo(colorInfo);
			}
		}else {
			Color3Setting cColor = GetColorIndex(pLayout, pInfo->pDispPos->GetLayoutLineRef(), 0, pInfo, true);
			SetCurrentColor(pInfo->gr, cColor.eColorIndex, cColor.eColorIndex2, cColor.eColorIndexBg);
		}
	}

	// 開始ロジック位置を算出
	{
		const Layout* pLayout = pInfo->pDispPos->GetLayoutRef();
		pInfo->nPosInLogic = pLayout ? pLayout->GetLogicOffset() : LogicInt(0);
	}

	for (;;) {
		// 対象行が描画範囲外だったら終了
		if (GetTextArea().GetBottomLine() < pInfo->pDispPos->GetLayoutLineRef()) {
			pInfo->pDispPos->SetLayoutLineRef(nLineTo + LayoutInt(1));
			break;
		}
		if (nLineTo < pInfo->pDispPos->GetLayoutLineRef()) {
			break;
		}

		// レイアウト行を1行描画
		bDispEOF = DrawLayoutLine(pInfo);

		// 行を進める
		LogicInt nOldLogicLineNo = pInfo->pDispPos->GetLayoutRef()->GetLogicLineNo();
		pInfo->pDispPos->ForwardDrawLine(1);		// 描画Y座標＋＋
		pInfo->pDispPos->ForwardLayoutLineRef(1);	// レイアウト行＋＋

		// ロジック行を描画し終わったら抜ける
		if (pInfo->pDispPos->GetLayoutRef()->GetLogicLineNo() != nOldLogicLineNo) {
			break;
		}

		// nLineToを超えたら抜ける
		if (pInfo->pDispPos->GetLayoutLineRef() >= nLineTo + LayoutInt(1)) {
			break;
		}
	}

	return bDispEOF;
}

/*!
	レイアウト行を1行描画
*/
// 改行記号を描画した場合はtrueを返す？
bool EditView::DrawLayoutLine(ColorStrategyInfo* pInfo)
{
	bool bDispEOF = false;
	TypeSupport textType(this, COLORIDX_TEXT);

	const Layout* pLayout = pInfo->pDispPos->GetLayoutRef(); //m_pEditDoc->m_layoutMgr.SearchLineByLayoutY(pInfo->pDispPos->GetLayoutLineRef());

	// レイアウト情報
	if (pLayout) {
		pInfo->pLineOfLogic = pLayout->GetDocLineRef()->GetPtr();
	}else {
		pInfo->pLineOfLogic = NULL;
	}

	// 文字列参照
	const DocLine* pDocLine = pInfo->GetDocLine();
	StringRef lineStr = pDocLine->GetStringRefWithEOL();

	// 描画範囲外の場合は色切替だけで抜ける
	if (pInfo->pDispPos->GetDrawPos().y < GetTextArea().GetAreaTop()) {
		if (pLayout) {
			bool bChange = false;
			int nPosTo = pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL();
			Color3Setting cColor;
			while (pInfo->nPosInLogic < nPosTo) {
				// 色切替
				bChange |= pInfo->CheckChangeColor(lineStr);

				// 1文字進む
				pInfo->nPosInLogic += NativeW::GetSizeOfChar(
											lineStr.GetPtr(),
											lineStr.GetLength(),
											pInfo->nPosInLogic
										);
			}
			if (bChange) {
				pInfo->DoChangeColor(&cColor);
				SetCurrentColor(pInfo->gr, cColor.eColorIndex, cColor.eColorIndex2, cColor.eColorIndexBg);
			}
		}
		return false;
	}

	// コンフィグ
	int nLineHeight = GetTextMetrics().GetHankakuDy();  // 行の縦幅？
	TypeSupport	caretLineBg(this, COLORIDX_CARETLINEBG);
	TypeSupport	evenLineBg(this, COLORIDX_EVENLINEBG);
	TypeSupport	pageViewBg(this, COLORIDX_PAGEVIEW);
	EditView& activeView = m_pEditWnd->GetActiveView();
	TypeSupport&	backType = (caretLineBg.IsDisp() &&
		GetCaret().GetCaretLayoutPos().GetY() == pInfo->pDispPos->GetLayoutLineRef() && !m_bMiniMap
			? caretLineBg
			: evenLineBg.IsDisp() && pInfo->pDispPos->GetLayoutLineRef() % 2 == 1 && !m_bMiniMap
				? evenLineBg
				: (pageViewBg.IsDisp() && m_bMiniMap
					&& activeView.GetTextArea().GetViewTopLine() <= pInfo->pDispPos->GetLayoutLineRef()
					&& pInfo->pDispPos->GetLayoutLineRef() < activeView.GetTextArea().GetBottomLine())
						? pageViewBg
						: textType);
	bool bTransText = IsBkBitmap();
	if (bTransText) {
		bTransText = backType.GetBackColor() == textType.GetBackColor();
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        行番号描画                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	GetTextDrawer().DispLineNumber(
		pInfo->gr,
		pInfo->pDispPos->GetLayoutLineRef(),
		pInfo->pDispPos->GetDrawPos().y
	);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       本文描画開始                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pInfo->pDispPos->ResetDrawCol();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                 行頭(インデント)背景描画                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if (pLayout && pLayout->GetIndent() != 0) {
		RECT rcClip;
		if (!bTransText && GetTextArea().GenerateClipRect(&rcClip, *pInfo->pDispPos,(Int)pLayout->GetIndent())) {
			backType.FillBack(pInfo->gr, rcClip);
		}
		// 描画位置進める
		pInfo->pDispPos->ForwardDrawCol((Int)pLayout->GetIndent());
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         本文描画                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool bSkipRight = false; // 続きを描画しなくていい場合はスキップする
	if (pLayout) {
		const Layout* pLayoutNext = pLayout->GetNextLayout();
		if (!pLayoutNext) {
			bSkipRight = true;
		}else if (pLayoutNext->GetLogicOffset() == 0) {
			bSkipRight = true; // 次の行は別のロジック行なのでスキップ可能
		}
		if (!bSkipRight) {
			bSkipRight = ColorStrategyPool::getInstance()->IsSkipBeforeLayout();
		}
	}
	// 行終端または折り返しに達するまでループ
	if (pLayout) {
		int nPosTo = pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL();
		FigureManager* pFigureManager = FigureManager::getInstance();
		while (pInfo->nPosInLogic < nPosTo) {
			// 色切替
			if (pInfo->CheckChangeColor(lineStr)) {
				Color3Setting cColor;
				pInfo->DoChangeColor(&cColor);
				SetCurrentColor(pInfo->gr, cColor.eColorIndex, cColor.eColorIndex2, cColor.eColorIndexBg);
			}

			// 1文字情報取得 $$高速化可能
			Figure& figure = pFigureManager->GetFigure(&lineStr.GetPtr()[pInfo->GetPosInLogic()],
				lineStr.GetLength() - pInfo->GetPosInLogic());

			// 1文字描画
			figure.DrawImp(pInfo);
			if (bSkipRight && GetTextArea().GetAreaRight() < pInfo->pDispPos->GetDrawPos().x) {
				pInfo->nPosInLogic = nPosTo;
				break;
			}
		}
	}

	// 必要ならEOF描画
	void _DispEOF(Graphics& gr, DispPos* pDispPos, const EditView* pView);
	if (pLayout && !pLayout->GetNextLayout() && pLayout->GetLayoutEol().GetLen() == 0) {
		// 有文字行のEOF
		_DispEOF(pInfo->gr, pInfo->pDispPos, this);
		bDispEOF = true;
	}else if (!pLayout && pInfo->pDispPos->GetLayoutLineRef() == m_pEditDoc->m_layoutMgr.GetLineCount()) {
		// 空行のEOF
		const Layout* pBottom = m_pEditDoc->m_layoutMgr.GetBottomLayout();
		if (!pBottom || (pBottom && pBottom->GetLayoutEol().GetLen())) {
			_DispEOF(pInfo->gr,pInfo->pDispPos,this);
			bDispEOF = true;
		}
	}

	// 必要なら折り返し記号描画
	if (pLayout && pLayout->GetLayoutEol().GetLen() == 0 && pLayout->GetNextLayout()) {
		_DispWrap(pInfo->gr, pInfo->pDispPos, this, pInfo->pDispPos->GetLayoutLineRef());
	}

	// 行末背景描画
	RECT rcClip;
	bool rcClipRet = GetTextArea().GenerateClipRectRight(&rcClip, *pInfo->pDispPos);
	if (rcClipRet) {
		if (!bTransText) {
			backType.FillBack(pInfo->gr, rcClip);
		}
		TypeSupport selectType(this, COLORIDX_SELECT);
		if (GetSelectionInfo().IsTextSelected() && selectType.IsDisp()) {
			// 選択範囲の指定色：必要ならテキストのない部分の矩形選択を作画
			LayoutRange selectArea = GetSelectionInfo().GetSelectAreaLine(pInfo->pDispPos->GetLayoutLineRef(), pLayout);
			// 2010.10.04 スクロール分の足し忘れ
			int nSelectFromPx = GetTextMetrics().GetHankakuDx() * (Int)(selectArea.GetFrom().x - GetTextArea().GetViewLeftCol());
			int nSelectToPx   = GetTextMetrics().GetHankakuDx() * (Int)(selectArea.GetTo().x - GetTextArea().GetViewLeftCol());
			if (nSelectFromPx < nSelectToPx && selectArea.GetTo().x != INT_MAX) {
				RECT rcSelect; // Pixel
				rcSelect.top    = pInfo->pDispPos->GetDrawPos().y;
				rcSelect.bottom = pInfo->pDispPos->GetDrawPos().y + GetTextMetrics().GetHankakuDy();
				rcSelect.left   = GetTextArea().GetAreaLeft() + nSelectFromPx;
				rcSelect.right  = GetTextArea().GetAreaLeft() + nSelectToPx;
				RECT rcDraw;
				if (::IntersectRect(&rcDraw, &rcClip, &rcSelect)) {
					COLORREF color = GetBackColorByColorInfo2(selectType.GetColorInfo(), backType.GetColorInfo());
					if (color != backType.GetBackColor()) {
						pInfo->gr.FillSolidMyRect(rcDraw, color);
					}
				}
			}
		}
	}

	// ノート線描画
	if (!m_bMiniMap) {
		GetTextDrawer().DispNoteLine(
			pInfo->gr,
			pInfo->pDispPos->GetDrawPos().y,
			pInfo->pDispPos->GetDrawPos().y + nLineHeight,
			GetTextArea().GetAreaLeft(),
			GetTextArea().GetAreaRight()
		);
	}

	// 指定桁縦線描画
	GetTextDrawer().DispVerticalLines(
		pInfo->gr,
		pInfo->pDispPos->GetDrawPos().y,
		pInfo->pDispPos->GetDrawPos().y + nLineHeight,
		LayoutInt(0),
		LayoutInt(-1)
	);

	// 折り返し桁縦線描画
	if (!m_bMiniMap) {
		GetTextDrawer().DispWrapLine(
			pInfo->gr,
			pInfo->pDispPos->GetDrawPos().y,
			pInfo->pDispPos->GetDrawPos().y + nLineHeight
		);
	}

	// 反転描画
	if (pLayout && GetSelectionInfo().IsTextSelected()) {
		DispTextSelected(
			pInfo->gr,
			pInfo->pDispPos->GetLayoutLineRef(),
			Point(pInfo->dispPosBegin.GetDrawPos().x, pInfo->pDispPos->GetDrawPos().y),
			pLayout->CalcLayoutWidth(EditDoc::GetInstance(0)->m_layoutMgr) + LayoutInt(pLayout->GetLayoutEol().GetLen() ? 1 : 0)
		);
	}

	return bDispEOF;
}



/* テキスト反転

	@param hdc      
	@param nLineNum 
	@param x        
	@param y        
	@param nX       

	@note
	CCEditView::DrawLogicLine() での作画(WM_PAINT)時に、1レイアウト行をまとめて反転処理するための関数。
	範囲選択の随時更新は、EditView::DrawSelectArea() が選択・反転解除を行う。
	
*/
void EditView::DispTextSelected(
	HDC				hdc,		// 作画対象ビットマップを含むデバイス
	LayoutInt		nLineNum,	// 反転処理対象レイアウト行番号(0開始)
	const Point&	ptXY,		// (相対レイアウト0桁目の左端座標, 対象行の上端座標)
	LayoutInt		nX_Layout	// 対象行の終了桁位置。　[ABC\n]なら改行の後ろで4
)
{
	LayoutInt	nSelectFrom;
	LayoutInt	nSelectTo;
	RECT		rcClip;
	int			nLineHeight = GetTextMetrics().GetHankakuDy();
	int			nCharWidth = GetTextMetrics().GetHankakuDx();
	HRGN		hrgnDraw;
	const Layout* pLayout = m_pEditDoc->m_layoutMgr.SearchLineByLayoutY(nLineNum);
	LayoutRange& select = GetSelectionInfo().m_select;

	// 選択範囲内の行かな
//	if (IsTextSelected()) {
		if (nLineNum >= select.GetFrom().y && nLineNum <= select.GetTo().y) {
			LayoutRange selectArea = GetSelectionInfo().GetSelectAreaLine(nLineNum, pLayout);
			nSelectFrom = selectArea.GetFrom().x;
			nSelectTo   = selectArea.GetTo().x;
			if (nSelectFrom == INT_MAX) {
				nSelectFrom = nX_Layout;
			}
			if (nSelectTo == INT_MAX) {
				nSelectTo = nX_Layout;
			}

			// 2006.03.28 Moca 表示域外なら何もしない
			if (GetTextArea().GetRightCol() < nSelectFrom) {
				return;
			}
			if (nSelectTo < GetTextArea().GetViewLeftCol()) {	// nSelectTo == GetTextArea().GetViewLeftCol()のケースは後で０文字マッチでないことを確認してから抜ける
				return;
			}

			if (nSelectFrom < GetTextArea().GetViewLeftCol()) {
				nSelectFrom = GetTextArea().GetViewLeftCol();
			}
			rcClip.left   = ptXY.x + (Int)nSelectFrom * nCharWidth;
			rcClip.right  = ptXY.x + (Int)nSelectTo   * nCharWidth;
			rcClip.top    = ptXY.y;
			rcClip.bottom = ptXY.y + nLineHeight;

			bool bOMatch = false;

			// 2005/04/02 かろと ０文字マッチだと反転幅が０となり反転されないので、1/3文字幅だけ反転させる
			// 2005/06/26 zenryaku 選択解除でキャレットの残骸が残る問題を修正
			// 2005/09/29 ryoji スクロール時にキャレットのようなゴミが表示される問題を修正
			if (GetSelectionInfo().IsTextSelected() && rcClip.right == rcClip.left &&
				select.IsLineOne() &&
				select.GetFrom().x >= GetTextArea().GetViewLeftCol()
			) {
				HWND hWnd = ::GetForegroundWindow();
				if (hWnd && (hWnd == m_pEditWnd->m_dlgFind.GetHwnd() || hWnd == m_pEditWnd->m_dlgReplace.GetHwnd())) {
					rcClip.right = rcClip.left + (nCharWidth/3 == 0 ? 1 : nCharWidth/3);
					bOMatch = true;
				}
			}
			if (rcClip.right == rcClip.left) {
				return;	// ０文字マッチによる反転幅拡張なし
			}

			// 2006.03.28 Moca ウィンドウ幅が大きいと正しく反転しない問題を修正
			if (rcClip.right > GetTextArea().GetAreaRight()) {
				rcClip.right = GetTextArea().GetAreaRight();
			}
			
			// 選択色表示なら反転しない
			if (!bOMatch && TypeSupport(this, COLORIDX_SELECT).IsDisp()) {
				return;
			}
			
			HBRUSH hBrush    = ::CreateSolidBrush(SELECTEDAREA_RGB);

			int    nROP_Old  = ::SetROP2(hdc, SELECTEDAREA_ROP2);
			HBRUSH hBrushOld = (HBRUSH)::SelectObject(hdc, hBrush);
			hrgnDraw = ::CreateRectRgn(rcClip.left, rcClip.top, rcClip.right, rcClip.bottom);
			::PaintRgn(hdc, hrgnDraw);
			::DeleteObject(hrgnDraw);

			SetROP2(hdc, nROP_Old);
			SelectObject(hdc, hBrushOld);
			DeleteObject(hBrush);
		}
//	}
	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       画面バッファ                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*!
	画面の互換ビットマップを作成または更新する。
		必要の無いときは何もしない。
	
	@param cx ウィンドウの高さ
	@param cy ウィンドウの幅
	@return true: ビットマップを利用可能 / false: ビットマップの作成・更新に失敗

	@date 2007.09.09 Moca EditView::OnSizeから分離。
		単純に生成するだけだったものを、仕様変更に従い内容コピーを追加。
		サイズが同じときは何もしないように変更

	@par 互換BMPにはキャレット・カーソル位置横縦線・対括弧以外の情報を全て書き込む。
		選択範囲変更時の反転処理は、画面と互換BMPの両方を別々に変更する。
		カーソル位置横縦線変更時には、互換BMPから画面に元の情報を復帰させている。

*/
bool EditView::CreateOrUpdateCompatibleBitmap(int cx, int cy)
{
	if (!m_hdcCompatDC) {
		return false;
	}
	// サイズを64の倍数で整列
	int nBmpWidthNew  = ((cx + 63) & (0x7fffffff - 63));
	int nBmpHeightNew = ((cy + 63) & (0x7fffffff - 63));
	if (nBmpWidthNew != m_nCompatBMPWidth || nBmpHeightNew != m_nCompatBMPHeight) {
#if 0
	MYTRACE(_T("EditView::CreateOrUpdateCompatibleBitmap(%d, %d): resized\n"), cx, cy);
#endif
		HDC	hdc = ::GetDC(GetHwnd());
		HBITMAP hBitmapNew = NULL;
		if (m_hbmpCompatBMP) {
			// BMPの更新
			HDC hdcTemp = ::CreateCompatibleDC(hdc);
			hBitmapNew = ::CreateCompatibleBitmap(hdc, nBmpWidthNew, nBmpHeightNew);
			if (hBitmapNew) {
				HBITMAP hBitmapOld = (HBITMAP)::SelectObject(hdcTemp, hBitmapNew);
				// 前の画面内容をコピーする
				::BitBlt(hdcTemp, 0, 0,
					t_min(nBmpWidthNew, m_nCompatBMPWidth),
					t_min(nBmpHeightNew, m_nCompatBMPHeight),
					m_hdcCompatDC, 0, 0, SRCCOPY);
				::SelectObject(hdcTemp, hBitmapOld);
				::SelectObject(m_hdcCompatDC, m_hbmpCompatBMPOld);
				::DeleteObject(m_hbmpCompatBMP);
			}
			::DeleteDC(hdcTemp);
		}else {
			// BMPの新規作成
			hBitmapNew = ::CreateCompatibleBitmap(hdc, nBmpWidthNew, nBmpHeightNew);
		}
		if (hBitmapNew) {
			m_hbmpCompatBMP = hBitmapNew;
			m_nCompatBMPWidth = nBmpWidthNew;
			m_nCompatBMPHeight = nBmpHeightNew;
			m_hbmpCompatBMPOld = (HBITMAP)::SelectObject(m_hdcCompatDC, m_hbmpCompatBMP);
		}else {
			// 互換BMPの作成に失敗
			// 今後も失敗を繰り返す可能性が高いので
			// m_hdcCompatDCをNULLにすることで画面バッファ機能をこのウィンドウのみ無効にする。
			//	2007.09.29 genta 関数化．既存のBMPも解放
			UseCompatibleDC(FALSE);
		}
		::ReleaseDC(GetHwnd(), hdc);
	}
	return m_hbmpCompatBMP != NULL;
}


/*!
	互換メモリBMPを削除

	@note 分割ビューが非表示になった場合と
		親ウィンドウが非表示・最小化された場合に削除される。
	@date 2007.09.09 Moca 新規作成 
*/
void EditView::DeleteCompatibleBitmap()
{
	if (m_hbmpCompatBMP) {
		::SelectObject(m_hdcCompatDC, m_hbmpCompatBMPOld);
		::DeleteObject(m_hbmpCompatBMP);
		m_hbmpCompatBMP = NULL;
		m_hbmpCompatBMPOld = NULL;
		m_nCompatBMPWidth = -1;
		m_nCompatBMPHeight = -1;
	}
}


/** 画面キャッシュ用CompatibleDCを用意する

	@param[in] TRUE: 画面キャッシュON

	@date 2007.09.30 genta 関数化
*/
void EditView::UseCompatibleDC(BOOL fCache)
{
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	if (fCache) {
		if (!m_hdcCompatDC) {
			HDC hdc = ::GetDC(GetHwnd());
			m_hdcCompatDC = ::CreateCompatibleDC(hdc);
			::ReleaseDC(GetHwnd(), hdc);
			DEBUG_TRACE(_T("EditView::UseCompatibleDC: Created\n"), fCache);
		}else {
			DEBUG_TRACE(_T("EditView::UseCompatibleDC: Reused\n"), fCache);
		}
	}else {
		//	CompatibleBitmapが残っているかもしれないので最初に削除
		DeleteCompatibleBitmap();
		if (m_hdcCompatDC) {
			::DeleteDC(m_hdcCompatDC);
			DEBUG_TRACE(_T("EditView::UseCompatibleDC: Deleted.\n"));
			m_hdcCompatDC = NULL;
		}
	}
}

