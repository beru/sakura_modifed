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

void _DispWrap(Graphics& gr, DispPos* pDispPos, const EditView& view, int nLineNum);

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
	EditView& view = GetEditView();

	auto& textArea = view.GetTextArea();
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
	HDC hdc = view.GetDC();
	view.OnPaint(hdc, &ps, bUseMemoryDC);
	view.ReleaseDC(hdc);
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
	editWnd.UpdateCaption();

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

void EditView::RedrawLines(int top, int bottom)
{
	if (!GetHwnd()) {
		return;
	}
	if (!GetDrawSwitch()) {
		return;
	}

	auto& textArea = GetTextArea();
	if (bottom < textArea.GetViewTopLine()) {
		return;
	}
	if (textArea.GetBottomLine() <= top) {
		return;
	}
	HDC hdc = GetDC();
	PAINTSTRUCT	ps;
	ps.rcPaint.left = 0;
	ps.rcPaint.right = textArea.GetAreaRight();
	ps.rcPaint.top = textArea.GenerateYPx(top);
	ps.rcPaint.bottom = textArea.GenerateYPx(bottom);
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
	TypeSupport textType(*this, COLORIDX_TEXT);
	COLORREF colorOld = ::SetBkColor(hdc, textType.GetBackColor());
	const TextArea& textArea = GetTextArea();
	const EditDoc& doc  = *pEditDoc;
	const TypeConfig& typeConfig = doc.docType.GetDocumentAttribute();

	Rect rcImagePos;
	switch (typeConfig.backImgPos) {
	case BackgroundImagePosType::TopLeft:
	case BackgroundImagePosType::BottomLeft:
	case BackgroundImagePosType::CenterLeft:
		rcImagePos.left = textArea.GetAreaLeft();
		break;
	case BackgroundImagePosType::TopRight:
	case BackgroundImagePosType::BottomRight:
	case BackgroundImagePosType::CenterRight:
		rcImagePos.left = textArea.GetAreaRight() - doc.nBackImgWidth;
		break;
	case BackgroundImagePosType::TopCenter:
	case BackgroundImagePosType::BottomCenter:
	case BackgroundImagePosType::Center:
		rcImagePos.left = textArea.GetAreaLeft() + textArea.GetAreaWidth()/2 - doc.nBackImgWidth/2;
		break;
	default:
		assert_warning(false);
		break;
	}
	switch (typeConfig.backImgPos) {
	case BackgroundImagePosType::TopLeft:
	case BackgroundImagePosType::TopRight:
	case BackgroundImagePosType::TopCenter:
		rcImagePos.top  = textArea.GetAreaTop();
		break;
	case BackgroundImagePosType::BottomLeft:
	case BackgroundImagePosType::BottomRight:
	case BackgroundImagePosType::BottomCenter:
		rcImagePos.top  = textArea.GetAreaBottom() - doc.nBackImgHeight;
		break;
	case BackgroundImagePosType::CenterLeft:
	case BackgroundImagePosType::CenterRight:
	case BackgroundImagePosType::Center:
		rcImagePos.top  = textArea.GetAreaTop() + textArea.GetAreaHeight()/2 - doc.nBackImgHeight/2;
		break;
	default:
		assert_warning(false);
		break;
	}
	rcImagePos.left += typeConfig.backImgPosOffset.x;
	rcImagePos.top  += typeConfig.backImgPosOffset.y;
	// スクロール時の画面の端を作画するときの位置あたりへ移動
	if (typeConfig.backImgScrollX) {
		int tile = typeConfig.backImgRepeatX ? doc.nBackImgWidth : INT_MAX;
		int posX = (textArea.GetViewLeftCol() % tile) * GetTextMetrics().GetHankakuDx();
		rcImagePos.left -= posX % tile;
	}
	if (typeConfig.backImgScrollY) {
		int tile = typeConfig.backImgRepeatY ? doc.nBackImgHeight : INT_MAX;
		int posY = (textArea.GetViewTopLine() % tile) * GetTextMetrics().GetHankakuDy();
		rcImagePos.top -= posY % tile;
	}
	if (typeConfig.backImgRepeatX) {
		if (0 < rcImagePos.left) {
			// rcImagePos.left = rcImagePos.left - (rcImagePos.left / doc.nBackImgWidth + 1) * doc.nBackImgWidth;
			rcImagePos.left = rcImagePos.left % doc.nBackImgWidth - doc.nBackImgWidth;
		}
	}
	if (typeConfig.backImgRepeatY) {
		if (0 < rcImagePos.top) {
			// rcImagePos.top = rcImagePos.top - (rcImagePos.top / doc.nBackImgHeight + 1) * doc.nBackImgHeight;
			rcImagePos.top = rcImagePos.top % doc.nBackImgHeight - doc.nBackImgHeight;
		}
	}
	rcImagePos.SetSize(doc.nBackImgWidth, doc.nBackImgHeight);
	
	RECT rc = rcPaint;
	// rc.left = t_max((int)rc.left, textArea.GetAreaLeft());
	rc.top  = t_max((int)rc.top,  textArea.GetRulerHeight()); // ルーラーを除外
	const int nXEnd = textArea.GetAreaRight();
	const int nYEnd = textArea.GetAreaBottom();
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
			rcImagePos.left  += doc.nBackImgWidth;
			rcImagePos.right += doc.nBackImgWidth;
			if (!typeConfig.backImgRepeatX) {
				break;
			}
		}
		rcImagePos.left  = rcImagePosOrg.left;
		rcImagePos.right = rcImagePosOrg.right;
		rcImagePos.top    += doc.nBackImgHeight;
		rcImagePos.bottom += doc.nBackImgHeight;
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
	int						nLineNum,
	int						nIndex,
	ColorStrategyInfo&	 	csInfo,			// 2010.03.31 ryoji 追加
	bool					bPrev			// 指定位置の色変更直前まで	2010.06.19 ryoji 追加
	)
{
	EColorIndexType eRet = COLORIDX_TEXT;

	if (!pLayout) {
		Color3Setting cColor = { COLORIDX_TEXT, COLORIDX_TEXT, COLORIDX_TEXT };
		return cColor;
	}
	// 2014.12.30 Skipモードの時もCOLORIDX_TEXT
	if (ColorStrategyPool::getInstance().IsSkipBeforeLayout()) {
		Color3Setting cColor = { COLORIDX_TEXT, COLORIDX_TEXT, COLORIDX_TEXT };
		return cColor;
	}

	const LayoutColorInfo* colorInfo;
	const Layout* pLayoutLineFirst = pLayout;
	int nLineNumFirst = nLineNum;
	{
		// 2002/2/10 aroka CMemory変更
		csInfo.pLineOfLogic = pLayout->GetDocLineRef()->GetPtr();

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
		csInfo.nPosInLogic = pLayoutLineFirst->GetLogicOffset();

		// ColorStrategyPool初期化
		auto& pool = ColorStrategyPool::getInstance();
		pool.SetCurrentView(this);
		pool.NotifyOnStartScanLogic();

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
		//pInfo->pDispPos->SetLayoutLineRef(TmpVisitor::CalcLayoutIndex(pLayout));
		// 2013.12.11 Moca カレント行の色替えで必要になりました
		csInfo.pDispPos->SetLayoutLineRef(nLineNumFirst);
	}

	// 文字列参照
	const DocLine* pDocLine = pLayout->GetDocLineRef();
	StringRef lineStr(pDocLine->GetPtr(), pDocLine->GetLengthWithEOL());

	// color strategy
	ColorStrategyPool& pool = ColorStrategyPool::getInstance();
	csInfo.pStrategy = pool.GetStrategyByColor(eRet);
	if (csInfo.pStrategy) {
		csInfo.pStrategy->InitStrategyStatus();
		csInfo.pStrategy->SetStrategyColorInfo(colorInfo);
	}

	const Layout* pLayoutNext = pLayoutLineFirst->GetNextLayout();
	int nLineNumScan = nLineNumFirst;
	int nPosTo = pLayout->GetLogicOffset() + t_min(nIndex, (int)pLayout->GetLengthWithEOL() - 1);
	while (csInfo.nPosInLogic <= nPosTo) {
		if (bPrev && csInfo.nPosInLogic == nPosTo) {
			break;
		}

		// 色切替
		csInfo.CheckChangeColor(lineStr);

		// 1文字進む
		csInfo.nPosInLogic += NativeW::GetSizeOfChar(
									lineStr.GetPtr(),
									lineStr.GetLength(),
									csInfo.nPosInLogic
								);
		if (pLayoutNext && pLayoutNext->GetLogicOffset() <= csInfo.nPosInLogic) {
			++nLineNumScan;
			csInfo.pDispPos->SetLayoutLineRef(nLineNumScan);
			pLayoutNext = pLayoutNext->GetNextLayout();
		}
	}

	Color3Setting cColor;
	csInfo.DoChangeColor(&cColor);

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
	const ColorInfo& info  = pTypeData->colorInfoArr[nColorIdx];
	const ColorInfo& info2 = pTypeData->colorInfoArr[nColorIdx2];
	const ColorInfo& infoBg = pTypeData->colorInfoArr[nColorIdxBg];
	COLORREF fgcolor = GetTextColorByColorInfo2(info, info2);
	gr.SetTextForeColor(fgcolor);
	// 2012.11.21 背景色がテキストとおなじなら背景色はカーソル行背景
	const ColorInfo& info3 = (info2.colorAttr.cBACK == crBack ? infoBg : info2);
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
	if (info.colorAttr.cBACK == crBack) {
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
	if (info.colorAttr.cBACK == crBack) {
		return  info2.colorAttr.cBACK ^ 0x00FFFFFF;
	}
	int alpha = 255*30/100; // 30%
	return MakeColor2(info.colorAttr.cBACK, info2.colorAttr.cBACK, alpha);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           描画                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void EditView::OnPaint(HDC _hdc, PAINTSTRUCT* pPs, BOOL bDrawFromComptibleBmp)
{

	bool bChangeFont = bMiniMap;
	if (bChangeFont) {
		SelectCharWidthCache(CharWidthFontMode::MiniMap, CharWidthCacheMode::Local);
	}
	OnPaint2(_hdc, pPs, bDrawFromComptibleBmp);
	if (bChangeFont) {
		SelectCharWidthCache(CharWidthFontMode::Edit, editWnd.GetLogfontCacheMode());
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
void EditView::OnPaint2(HDC _hdc, PAINTSTRUCT* pPs, BOOL bDrawFromComptibleBmp)
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
		&& hdcCompatDC
		&& hbmpCompatBMP
	) {
		::BitBlt(
			gr,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			pPs->rcPaint.right - pPs->rcPaint.left,
			pPs->rcPaint.bottom - pPs->rcPaint.top,
			hdcCompatDC,
			pPs->rcPaint.left,
			pPs->rcPaint.top,
			SRCCOPY
		);
		if (editWnd.GetActivePane() == nMyIndex) {
			// アクティブペインは、アンダーライン描画
			GetCaret().underLine.CaretUnderLineON(true, false);
		}
		return;
	}
	if (hdcCompatDC && !hbmpCompatBMP
		 || nCompatBMPWidth < (pPs->rcPaint.right - pPs->rcPaint.left)
		 || nCompatBMPHeight < (pPs->rcPaint.bottom - pPs->rcPaint.top)
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
	TypeSupport textType(*this, COLORIDX_TEXT);

//@@@ 2001.11.17 add start MIK
	// 変更があればタイプ設定を行う。
	if (pTypeData->bUseRegexKeyword || pRegexKeyword->bUseRegexKeyword) { // OFFなのに前回のデータが残ってる
		// タイプ別設定をする。設定済みかどうかは呼び先でチェックする。
		pRegexKeyword->RegexKeySetTypes(pTypeData);
	}
//@@@ 2001.11.17 add end MIK

	bool bTransText = IsBkBitmap();
	// メモリＤＣを利用した再描画の場合は描画先のＤＣを切り替える
	HDC hdcOld = 0;
	// 2007.09.09 Moca bUseMemoryDCを有効化。
	// bUseMemoryDC = FALSE;
	bool bUseMemoryDC = (hdcCompatDC != NULL);
	assert_warning(gr != hdcCompatDC);
	if (bUseMemoryDC) {
		hdcOld = gr;
		gr = hdcCompatDC;
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

	EditView& activeView = editWnd.GetActiveView();
	nPageViewTop = activeView.GetTextArea().GetViewTopLine();
	nPageViewBottom = activeView.GetTextArea().GetBottomLine();

	// 背景の表示
	if (bTransText) {
		HDC hdcBgImg = CreateCompatibleDC(gr);
		HBITMAP hOldBmp = (HBITMAP)::SelectObject(hdcBgImg, pEditDoc->hBackImg);
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
		if (bTransText && pTypeData->colorInfoArr[COLORIDX_GYOU].colorAttr.cBACK == textType.GetBackColor()) {
		}else {
			rc.left   = 0;
			rc.top    = GetTextArea().GetRulerHeight();
			rc.right  = GetTextArea().GetLineNumberWidth(); //	Sep. 23 ,2002 genta 余白はテキスト色のまま残す
			rc.bottom = GetTextArea().GetAreaTop();
			gr.SetTextBackColor(pTypeData->colorInfoArr[COLORIDX_GYOU].colorAttr.cBACK);
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
	int nLayoutLine;
	if (0 > nTop - GetTextArea().GetAreaTop()) {
		nLayoutLine = GetTextArea().GetViewTopLine(); // ビュー上部から描画
	}else {
		nLayoutLine = GetTextArea().GetViewTopLine() + (nTop - GetTextArea().GetAreaTop()) / nLineHeight; // ビュー途中から描画
	}

	// ※ ここにあった描画範囲の 260 文字ロールバック処理は GetColorIndex() に吸収	// 2009.02.11 ryoji

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          描画終了レイアウト絶対行 -> nLayoutLineTo            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	int nLayoutLineTo = GetTextArea().GetViewTopLine()
		+ (pPs->rcPaint.bottom - GetTextArea().GetAreaTop() + (nLineHeight - 1)) / nLineHeight - 1;	// 2007.02.17 ryoji 計算を精密化


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         描画座標                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	DispPos pos(GetTextMetrics().GetHankakuDx(), GetTextMetrics().GetHankakuDy());
	pos.InitDrawPos(Point(
		GetTextArea().GetAreaLeft() - GetTextArea().GetViewLeftCol() * nCharDx,
		GetTextArea().GetAreaTop() + (nLayoutLine - GetTextArea().GetViewTopLine()) * nLineHeight
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
		if (!bMiniMap) {
			GetTextDrawer().DispNoteLine(gr, pos.GetDrawPos().y, pPs->rcPaint.bottom, pPs->rcPaint.left, pPs->rcPaint.right);
		}
		// 2006.04.29 行部分は行ごとに作画し、ここでは縦線の残りを作画
		GetTextDrawer().DispVerticalLines(gr, pos.GetDrawPos().y, pPs->rcPaint.bottom, 0, -1);
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
	if (editWnd.GetActivePane() == nMyIndex) {
		// アクティブペインは、アンダーライン描画
		GetCaret().underLine.CaretUnderLineON(true, false);
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
	int				nLineTo			// [in]     作画終了するレイアウト行番号
	)
{
//	MY_RUNNINGTIMER(runningTimer, "EditView::DrawLogicLine");
	bool bDispEOF = false;
	ColorStrategyInfo csInfo(*this);
	csInfo.gr.Init(_hdc);
	csInfo.pDispPos = _pDispPos;

	// ColorStrategyPool初期化
	auto& pool = ColorStrategyPool::getInstance();
	pool.SetCurrentView(this);
	pool.NotifyOnStartScanLogic();
	bool bSkipBeforeLayout = pool.IsSkipBeforeLayout();

	// DispPosを保存しておく
	csInfo.dispPosBegin = *csInfo.pDispPos;

	// 処理する文字位置
	csInfo.nPosInLogic = 0; // ☆開始

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//          論理行データの取得 -> pLine, pLineLen              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 前行の最終設定色
	{
		const Layout* pLayout = csInfo.pDispPos->GetLayoutRef();
		if (bSkipBeforeLayout) {
			EColorIndexType eRet = COLORIDX_TEXT;
			const LayoutColorInfo* colorInfo = nullptr;
			if (pLayout) {
				eRet = pLayout->GetColorTypePrev(); // COLORIDX_TEXTのはず
				colorInfo = pLayout->GetColorInfo();
			}
			csInfo.pStrategy = pool.GetStrategyByColor(eRet);
			if (csInfo.pStrategy) {
				csInfo.pStrategy->InitStrategyStatus();
				csInfo.pStrategy->SetStrategyColorInfo(colorInfo);
			}
		}else {
			Color3Setting cColor = GetColorIndex(pLayout, csInfo.pDispPos->GetLayoutLineRef(), 0, csInfo, true);
			SetCurrentColor(csInfo.gr, cColor.eColorIndex, cColor.eColorIndex2, cColor.eColorIndexBg);
		}
	}

	// 開始ロジック位置を算出
	{
		const Layout* pLayout = csInfo.pDispPos->GetLayoutRef();
		csInfo.nPosInLogic = pLayout ? pLayout->GetLogicOffset() : 0;
	}

	for (;;) {
		// 対象行が描画範囲外だったら終了
		if (GetTextArea().GetBottomLine() < csInfo.pDispPos->GetLayoutLineRef()) {
			csInfo.pDispPos->SetLayoutLineRef(nLineTo + 1);
			break;
		}
		if (nLineTo < csInfo.pDispPos->GetLayoutLineRef()) {
			break;
		}

		// レイアウト行を1行描画
		bDispEOF = DrawLayoutLine(csInfo);

		// 行を進める
		int nOldLogicLineNo = csInfo.pDispPos->GetLayoutRef()->GetLogicLineNo();
		csInfo.pDispPos->ForwardDrawLine(1);		// 描画Y座標＋＋
		csInfo.pDispPos->ForwardLayoutLineRef(1);	// レイアウト行＋＋

		// ロジック行を描画し終わったら抜ける
		if (csInfo.pDispPos->GetLayoutRef()->GetLogicLineNo() != nOldLogicLineNo) {
			break;
		}

		// nLineToを超えたら抜ける
		if (csInfo.pDispPos->GetLayoutLineRef() >= nLineTo + 1) {
			break;
		}
	}

	return bDispEOF;
}

/*!
	レイアウト行を1行描画
*/
// 改行記号を描画した場合はtrueを返す？
bool EditView::DrawLayoutLine(ColorStrategyInfo& csInfo)
{
	bool bDispEOF = false;
	TypeSupport textType(*this, COLORIDX_TEXT);

	const Layout* pLayout = csInfo.pDispPos->GetLayoutRef(); //pEditDoc->m_layoutMgr.SearchLineByLayoutY(pInfo->pDispPos->GetLayoutLineRef());

	// レイアウト情報
	if (pLayout) {
		csInfo.pLineOfLogic = pLayout->GetDocLineRef()->GetPtr();
	}else {
		csInfo.pLineOfLogic = NULL;
	}

	// 文字列参照
	const DocLine* pDocLine = csInfo.GetDocLine();
	StringRef lineStr = pDocLine->GetStringRefWithEOL();

	// 描画範囲外の場合は色切替だけで抜ける
	TextArea& textArea = GetTextArea();
	if (csInfo.pDispPos->GetDrawPos().y < textArea.GetAreaTop()) {
		if (pLayout) {
			bool bChange = false;
			int nPosTo = pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL();
			Color3Setting cColor;
			while (csInfo.nPosInLogic < nPosTo) {
				// 色切替
				bChange |= csInfo.CheckChangeColor(lineStr);

				// 1文字進む
				csInfo.nPosInLogic += NativeW::GetSizeOfChar(
											lineStr.GetPtr(),
											lineStr.GetLength(),
											csInfo.nPosInLogic
										);
			}
			if (bChange) {
				csInfo.DoChangeColor(&cColor);
				SetCurrentColor(csInfo.gr, cColor.eColorIndex, cColor.eColorIndex2, cColor.eColorIndexBg);
			}
		}
		return false;
	}

	// コンフィグ
	int nLineHeight = GetTextMetrics().GetHankakuDy();  // 行の縦幅？
	TypeSupport	caretLineBg(*this, COLORIDX_CARETLINEBG);
	TypeSupport	evenLineBg(*this, COLORIDX_EVENLINEBG);
	TypeSupport	pageViewBg(*this, COLORIDX_PAGEVIEW);
	EditView& activeView = editWnd.GetActiveView();
	TypeSupport&	backType = (caretLineBg.IsDisp() &&
		GetCaret().GetCaretLayoutPos().GetY() == csInfo.pDispPos->GetLayoutLineRef() && !bMiniMap
			? caretLineBg
			: evenLineBg.IsDisp() && csInfo.pDispPos->GetLayoutLineRef() % 2 == 1 && !bMiniMap
				? evenLineBg
				: (pageViewBg.IsDisp() && bMiniMap
					&& activeView.GetTextArea().GetViewTopLine() <= csInfo.pDispPos->GetLayoutLineRef()
					&& csInfo.pDispPos->GetLayoutLineRef() < activeView.GetTextArea().GetBottomLine())
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
		csInfo.gr,
		csInfo.pDispPos->GetLayoutLineRef(),
		csInfo.pDispPos->GetDrawPos().y
	);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       本文描画開始                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	csInfo.pDispPos->ResetDrawCol();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                 行頭(インデント)背景描画                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	if (pLayout && pLayout->GetIndent() != 0) {
		RECT rcClip;
		if (!bTransText && textArea.GenerateClipRect(&rcClip, *csInfo.pDispPos, pLayout->GetIndent())) {
			backType.FillBack(csInfo.gr, rcClip);
		}
		// 描画位置進める
		csInfo.pDispPos->ForwardDrawCol(pLayout->GetIndent());
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
			bSkipRight = ColorStrategyPool::getInstance().IsSkipBeforeLayout();
		}
	}
	// 行終端または折り返しに達するまでループ
	if (pLayout) {
		int nPosTo = pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL();
		auto& figureManager = FigureManager::getInstance();
		while (csInfo.nPosInLogic < nPosTo) {
			// 色切替
			if (csInfo.CheckChangeColor(lineStr)) {
				Color3Setting cColor;
				csInfo.DoChangeColor(&cColor);
				SetCurrentColor(csInfo.gr, cColor.eColorIndex, cColor.eColorIndex2, cColor.eColorIndexBg);
			}

			// 1文字情報取得 $$高速化可能
			Figure& figure = figureManager.GetFigure(&lineStr.GetPtr()[csInfo.GetPosInLogic()],
				(int)lineStr.GetLength() - csInfo.GetPosInLogic());

			// 1文字描画
			figure.DrawImp(csInfo);
			if (bSkipRight && textArea.GetAreaRight() < csInfo.pDispPos->GetDrawPos().x) {
				csInfo.nPosInLogic = nPosTo;
				break;
			}
		}
	}

	// 必要ならEOF描画
	void _DispEOF(Graphics& gr, DispPos* pDispPos, const EditView& view);
	if (pLayout && !pLayout->GetNextLayout() && pLayout->GetLayoutEol().GetLen() == 0) {
		// 有文字行のEOF
		_DispEOF(csInfo.gr, csInfo.pDispPos, *this);
		bDispEOF = true;
	}else if (!pLayout && csInfo.pDispPos->GetLayoutLineRef() == pEditDoc->layoutMgr.GetLineCount()) {
		// 空行のEOF
		const Layout* pBottom = pEditDoc->layoutMgr.GetBottomLayout();
		if (!pBottom || (pBottom && pBottom->GetLayoutEol().GetLen())) {
			_DispEOF(csInfo.gr, csInfo.pDispPos, *this);
			bDispEOF = true;
		}
	}

	// 必要なら折り返し記号描画
	if (pLayout && pLayout->GetLayoutEol().GetLen() == 0 && pLayout->GetNextLayout()) {
		_DispWrap(csInfo.gr, csInfo.pDispPos, *this, csInfo.pDispPos->GetLayoutLineRef());
	}

	// 行末背景描画
	RECT rcClip;
	bool rcClipRet = textArea.GenerateClipRectRight(&rcClip, *csInfo.pDispPos);
	if (rcClipRet) {
		if (!bTransText) {
			backType.FillBack(csInfo.gr, rcClip);
		}
		TypeSupport selectType(*this, COLORIDX_SELECT);
		if (GetSelectionInfo().IsTextSelected() && selectType.IsDisp()) {
			// 選択範囲の指定色：必要ならテキストのない部分の矩形選択を作画
			Range selectArea = GetSelectionInfo().GetSelectAreaLine(csInfo.pDispPos->GetLayoutLineRef(), pLayout);
			// 2010.10.04 スクロール分の足し忘れ
			int nSelectFromPx = GetTextMetrics().GetHankakuDx() * (selectArea.GetFrom().x - textArea.GetViewLeftCol());
			int nSelectToPx   = GetTextMetrics().GetHankakuDx() * (selectArea.GetTo().x - textArea.GetViewLeftCol());
			if (nSelectFromPx < nSelectToPx && selectArea.GetTo().x != INT_MAX) {
				RECT rcSelect; // Pixel
				rcSelect.top    = csInfo.pDispPos->GetDrawPos().y;
				rcSelect.bottom = csInfo.pDispPos->GetDrawPos().y + GetTextMetrics().GetHankakuDy();
				rcSelect.left   = textArea.GetAreaLeft() + nSelectFromPx;
				rcSelect.right  = textArea.GetAreaLeft() + nSelectToPx;
				RECT rcDraw;
				if (::IntersectRect(&rcDraw, &rcClip, &rcSelect)) {
					COLORREF color = GetBackColorByColorInfo2(selectType.GetColorInfo(), backType.GetColorInfo());
					if (color != backType.GetBackColor()) {
						csInfo.gr.FillSolidMyRect(rcDraw, color);
					}
				}
			}
		}
	}

	// ノート線描画
	if (!bMiniMap) {
		GetTextDrawer().DispNoteLine(
			csInfo.gr,
			csInfo.pDispPos->GetDrawPos().y,
			csInfo.pDispPos->GetDrawPos().y + nLineHeight,
			textArea.GetAreaLeft(),
			textArea.GetAreaRight()
		);
	}

	// 指定桁縦線描画
	GetTextDrawer().DispVerticalLines(
		csInfo.gr,
		csInfo.pDispPos->GetDrawPos().y,
		csInfo.pDispPos->GetDrawPos().y + nLineHeight,
		0,
		-1
	);

	// 折り返し桁縦線描画
	if (!bMiniMap) {
		GetTextDrawer().DispWrapLine(
			csInfo.gr,
			csInfo.pDispPos->GetDrawPos().y,
			csInfo.pDispPos->GetDrawPos().y + nLineHeight
		);
	}

	// 反転描画
	if (pLayout && GetSelectionInfo().IsTextSelected()) {
		DispTextSelected(
			csInfo.gr,
			csInfo.pDispPos->GetLayoutLineRef(),
			Point(csInfo.dispPosBegin.GetDrawPos().x, csInfo.pDispPos->GetDrawPos().y),
			pLayout->CalcLayoutWidth(EditDoc::GetInstance(0)->layoutMgr) + pLayout->GetLayoutEol().GetLen() ? 1 : 0
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
	int				nLineNum,	// 反転処理対象レイアウト行番号(0開始)
	const Point&	ptXY,		// (相対レイアウト0桁目の左端座標, 対象行の上端座標)
	int				nX_Layout	// 対象行の終了桁位置。　[ABC\n]なら改行の後ろで4
)
{
	int			nSelectFrom;
	int			nSelectTo;
	RECT		rcClip;
	int			nLineHeight = GetTextMetrics().GetHankakuDy();
	int			nCharWidth = GetTextMetrics().GetHankakuDx();
	HRGN		hrgnDraw;
	const Layout* pLayout = pEditDoc->layoutMgr.SearchLineByLayoutY(nLineNum);
	Range& select = GetSelectionInfo().select;

	// 選択範囲内の行かな
//	if (IsTextSelected()) {
		if (nLineNum >= select.GetFrom().y && nLineNum <= select.GetTo().y) {
			Range selectArea = GetSelectionInfo().GetSelectAreaLine(nLineNum, pLayout);
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
			rcClip.left   = ptXY.x + nSelectFrom * nCharWidth;
			rcClip.right  = ptXY.x + nSelectTo   * nCharWidth;
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
				if (hWnd && (hWnd == editWnd.dlgFind.GetHwnd() || hWnd == editWnd.dlgReplace.GetHwnd())) {
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
			if (!bOMatch && TypeSupport(*this, COLORIDX_SELECT).IsDisp()) {
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
	if (!hdcCompatDC) {
		return false;
	}
	// サイズを64の倍数で整列
	int nBmpWidthNew  = ((cx + 63) & (0x7fffffff - 63));
	int nBmpHeightNew = ((cy + 63) & (0x7fffffff - 63));
	if (nBmpWidthNew != nCompatBMPWidth || nBmpHeightNew != nCompatBMPHeight) {
#if 0
	MYTRACE(_T("EditView::CreateOrUpdateCompatibleBitmap(%d, %d): resized\n"), cx, cy);
#endif
		HDC	hdc = ::GetDC(GetHwnd());
		HBITMAP hBitmapNew = NULL;
		if (hbmpCompatBMP) {
			// BMPの更新
			HDC hdcTemp = ::CreateCompatibleDC(hdc);
			hBitmapNew = ::CreateCompatibleBitmap(hdc, nBmpWidthNew, nBmpHeightNew);
			if (hBitmapNew) {
				HBITMAP hBitmapOld = (HBITMAP)::SelectObject(hdcTemp, hBitmapNew);
				// 前の画面内容をコピーする
				::BitBlt(hdcTemp, 0, 0,
					t_min(nBmpWidthNew, nCompatBMPWidth),
					t_min(nBmpHeightNew, nCompatBMPHeight),
					hdcCompatDC, 0, 0, SRCCOPY);
				::SelectObject(hdcTemp, hBitmapOld);
				::SelectObject(hdcCompatDC, hbmpCompatBMPOld);
				::DeleteObject(hbmpCompatBMP);
			}
			::DeleteDC(hdcTemp);
		}else {
			// BMPの新規作成
			hBitmapNew = ::CreateCompatibleBitmap(hdc, nBmpWidthNew, nBmpHeightNew);
		}
		if (hBitmapNew) {
			hbmpCompatBMP = hBitmapNew;
			nCompatBMPWidth = nBmpWidthNew;
			nCompatBMPHeight = nBmpHeightNew;
			hbmpCompatBMPOld = (HBITMAP)::SelectObject(hdcCompatDC, hbmpCompatBMP);
		}else {
			// 互換BMPの作成に失敗
			// 今後も失敗を繰り返す可能性が高いので
			// hdcCompatDCをNULLにすることで画面バッファ機能をこのウィンドウのみ無効にする。
			//	2007.09.29 genta 関数化．既存のBMPも解放
			UseCompatibleDC(FALSE);
		}
		::ReleaseDC(GetHwnd(), hdc);
	}
	return hbmpCompatBMP != NULL;
}


/*!
	互換メモリBMPを削除

	@note 分割ビューが非表示になった場合と
		親ウィンドウが非表示・最小化された場合に削除される。
	@date 2007.09.09 Moca 新規作成 
*/
void EditView::DeleteCompatibleBitmap()
{
	if (hbmpCompatBMP) {
		::SelectObject(hdcCompatDC, hbmpCompatBMPOld);
		::DeleteObject(hbmpCompatBMP);
		hbmpCompatBMP = NULL;
		hbmpCompatBMPOld = NULL;
		nCompatBMPWidth = -1;
		nCompatBMPHeight = -1;
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
		if (!hdcCompatDC) {
			HDC hdc = ::GetDC(GetHwnd());
			hdcCompatDC = ::CreateCompatibleDC(hdc);
			::ReleaseDC(GetHwnd(), hdc);
			DEBUG_TRACE(_T("EditView::UseCompatibleDC: Created\n"), fCache);
		}else {
			DEBUG_TRACE(_T("EditView::UseCompatibleDC: Reused\n"), fCache);
		}
	}else {
		//	CompatibleBitmapが残っているかもしれないので最初に削除
		DeleteCompatibleBitmap();
		if (hdcCompatDC) {
			::DeleteDC(hdcCompatDC);
			DEBUG_TRACE(_T("EditView::UseCompatibleDC: Deleted.\n"));
			hdcCompatDC = NULL;
		}
	}
}

