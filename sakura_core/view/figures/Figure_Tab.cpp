#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Figure_Tab.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "types/TypeSupport.h"

// 2007.08.28 kobake 追加
void _DispTab(Graphics& gr, DispPos* pDispPos, const EditView* pView);
// タブ矢印描画関数	//@@@ 2003.03.26 MIK
void _DrawTabArrow(Graphics& gr, int nPosX, int nPosY, int nWidth, int nHeight, bool bBold, COLORREF pColor);

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         Figure_Tab                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Figure_Tab::Match(const wchar_t* pText, int nTextLen) const
{
	return (pText[0] == WCODE::TAB);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         描画実装                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! TAB描画
	@date 2001.03.16 by MIK
	@date 2002.09.22 genta 共通式のくくりだし
	@date 2002.09.23 genta LayoutMgrの値を使う
	@date 2003.03.26 MIK タブ矢印表示
	@date 2013.05.31 novice TAB表示対応(文字指定/短い矢印/長い矢印)
*/
void Figure_Tab::DispSpace(Graphics& gr, DispPos* pDispPos, EditView& view, bool bTrans) const
{
	DispPos& sPos = *pDispPos;

	// 必要なインターフェース
	const TextMetrics* pMetrics = &view.GetTextMetrics();
	const TextArea* pArea = &view.GetTextArea();

	int nLineHeight = pMetrics->GetHankakuDy();
	int nCharWidth = pMetrics->GetHankakuDx();

	TypeSupport tabType(view, COLORIDX_TAB);

	// これから描画するタブ幅
	size_t tabDispWidth = view.pEditDoc->layoutMgr.GetActualTabSpace(sPos.GetDrawCol());

	// タブ記号領域
	RECT rcClip2;
	rcClip2.left = sPos.GetDrawPos().x;
	rcClip2.right = rcClip2.left + nCharWidth * tabDispWidth;
	if (rcClip2.left < pArea->GetAreaLeft()) {
		rcClip2.left = pArea->GetAreaLeft();
	}
	rcClip2.top = sPos.GetDrawPos().y;
	rcClip2.bottom = sPos.GetDrawPos().y + nLineHeight;

	if (pArea->IsRectIntersected(rcClip2)) {
		if (tabType.IsDisp() && TabArrowType::String == pTypeData->bTabArrow) {	// タブ通常表示	//@@@ 2003.03.26 MIK
			//@@@ 2001.03.16 by MIK
			::ExtTextOutW_AnyBuild(
				gr,
				sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
				&rcClip2,
				pTypeData->szTabViewString,
				tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
				pMetrics->GetDxArray_AllHankaku()
			);
		}else {
			// 背景
			::ExtTextOutW_AnyBuild(
				gr,
				sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
				&rcClip2,
				L"        ",
				tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
				pMetrics->GetDxArray_AllHankaku()
			);

			// タブ矢印表示
			if (tabType.IsDisp()) {
				// 文字色や太字かどうかを現在の DC から調べる	// 2009.05.29 ryoji 
				// （検索マッチ等の状況に柔軟に対応するため、ここは記号の色指定には決め打ちしない）
				//	太字かどうか設定も見る様にする 2013/4/11 Uchi
				// 2013.06.21 novice 文字色、太字をGraphicsから取得

				if (TabArrowType::Short == pTypeData->bTabArrow) {
					if (rcClip2.left <= sPos.GetDrawPos().x) { // Apr. 1, 2003 MIK 行番号と重なる
						_DrawTabArrow(
							gr,
							sPos.GetDrawPos().x,
							sPos.GetDrawPos().y,
							pMetrics->GetHankakuWidth(),
							pMetrics->GetHankakuHeight(),
							gr.GetCurrentMyFontBold() || pTypeData->colorInfoArr[COLORIDX_TAB].fontAttr.bBoldFont,
							gr.GetCurrentTextForeColor()
						);
					}
				}else if (TabArrowType::Long == pTypeData->bTabArrow) {
					int	nPosLeft = rcClip2.left > sPos.GetDrawPos().x ? rcClip2.left : sPos.GetDrawPos().x;
					_DrawTabArrow(
						gr,
						nPosLeft,
						sPos.GetDrawPos().y,
						nCharWidth * tabDispWidth - (nPosLeft -  sPos.GetDrawPos().x),	// Tab Area一杯に 2013/4/11 Uchi
						pMetrics->GetHankakuHeight(),
						gr.GetCurrentMyFontBold() || pTypeData->colorInfoArr[COLORIDX_TAB].fontAttr.bBoldFont,
						gr.GetCurrentTextForeColor()
					);
				}
			}
		}
	}

	// Xを進める
	sPos.ForwardDrawCol(tabDispWidth);
}



/*
	タブ矢印描画関数
*/
void _DrawTabArrow(
	Graphics&	gr,
	int			nPosX,   // ピクセルX
	int			nPosY,   // ピクセルY
	int			nWidth,  // ピクセルW
	int			nHeight, // ピクセルH
	bool		bBold,
	COLORREF	pColor
)
{
	// ペン設定
	gr.PushPen(pColor, 0);

	// 矢印の先頭
	int sx = nPosX + nWidth - 2;
	int sy = nPosY + (nHeight / 2);
	int sa = nHeight / 4;								// 鏃のsize

	DWORD pp[] = { 3, 2 };
	POINT pt[5];
	pt[0].x = nPosX;	//「─」左端から右端
	pt[0].y = sy;
	pt[1].x = sx;		//「／」右端から斜め左下
	pt[1].y = sy;
	pt[2].x = sx - sa;	//	矢印の先端に戻る
	pt[2].y = sy + sa;
	pt[3].x = sx;		//「＼」右端から斜め左上
	pt[3].y = sy;
	pt[4].x = sx - sa;
	pt[4].y = sy - sa;
	::PolyPolyline(gr, pt, pp, _countof(pp));

	if (bBold) {
		pt[0].x += 0;	//「─」左端から右端
		pt[0].y += 1;
		pt[1].x += 0;	//「／」右端から斜め左下
		pt[1].y += 1;
		pt[2].x += 0;	//	矢印の先端に戻る
		pt[2].y += 1;
		pt[3].x += 0;	//「＼」右端から斜め左上
		pt[3].y += 1;
		pt[4].x += 0;
		pt[4].y += 1;
		::PolyPolyline(gr, pt, pp, _countof(pp));
	}

	gr.PopPen();
}


