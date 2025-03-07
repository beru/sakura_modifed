#include "StdAfx.h"
#include "view/EditView.h" // SColorStrategyInfo
#include "Figure_Eol.h"
#include "types/TypeSupport.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "window/EditWnd.h"

// 折り返し描画
void _DispWrap(Graphics& gr, DispPos* pDispPos, const EditView& view, int nLineNum);

// EOF描画関数
// 実際には pX と nX が更新される。
//void _DispEOF(Graphics& gr, DispPos* pDispPos, const EditView* pView, bool bTrans);

// 改行記号描画
void _DispEOL(Graphics& gr, DispPos* pDispPos, Eol eol, const EditView& view, bool bTrans);


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        Figure_Eol                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Figure_Eol::Match(const wchar_t* pText, int nTextLen) const
{
	// 折り返し・最終行だとDrawImpでeol.GetLen()==0になり無限ループするので
	// もしも行の途中に改行コードがあった場合はMatchさせない
	if (nTextLen == 2 && pText[0] == L'\r' && pText[1] == L'\n') return true;
	if (nTextLen == 1 && WCODE::IsLineDelimiterExt(pText[0])) return true;
	return false;
}

//$$ 高速化可能。
bool Figure_Eol::DrawImp(ColorStrategyInfo& csInfo)
{
	auto& view = csInfo.view;

	// 改行取得
	const Layout* pLayout = csInfo.pDispPos->GetLayoutRef();
	Eol eol = pLayout->GetLayoutEol();
	if (eol.GetLen()) {
		// CFigureSpace::DrawImp_StyleSelectもどき。選択・検索色を優先する
		TypeSupport currentType(view, csInfo.GetCurrentColor());	// 周辺の色（現在の指定色/選択色）
		TypeSupport currentType2(view, csInfo.GetCurrentColor2());	// 周辺の色（現在の指定色）
		TypeSupport textType(view, COLORIDX_TEXT);				// テキストの指定色
		TypeSupport spaceType(view, GetDispColorIdx());	// 空白の指定色
		TypeSupport searchType(view, COLORIDX_SEARCH);	// 検索色(EOL固有)
		TypeSupport currentTypeBg(view, csInfo.GetCurrentColorBg());
		TypeSupport& currentType3 = (currentType2.GetBackColor() == textType.GetBackColor() ? currentTypeBg: currentType2);
		COLORREF crText;
		COLORREF crBack;
		bool bSelecting = csInfo.GetCurrentColor() != csInfo.GetCurrentColor2();
		bool blendColor = bSelecting && currentType.GetTextColor() == currentType.GetBackColor(); // 選択混合色
		TypeSupport& currentStyle = blendColor ? currentType2 : currentType;
		TypeSupport *pcText, *pcBack;
		if (bSelecting && !blendColor) {
			// 選択文字色固定指定
			pcText = &currentType;
			pcBack = &currentType;
		}else if (csInfo.GetCurrentColor2() == COLORIDX_SEARCH) {
			// 検索色優先
			pcText = &searchType;
			pcBack = &searchType;
		}else {
			pcText = spaceType.GetTextColor() == textType.GetTextColor() ? &currentType2 : &spaceType;
			pcBack = spaceType.GetBackColor() == textType.GetBackColor() ? &currentType3 : &spaceType;
		}
		if (blendColor) {
			// 混合色(検索色を優先しつつ)
			crText = view.GetTextColorByColorInfo2(currentType.GetColorInfo(), pcText->GetColorInfo());
			crBack = view.GetBackColorByColorInfo2(currentType.GetColorInfo(), pcBack->GetColorInfo());
		}else {
			crText = pcText->GetTextColor();
			crBack = pcBack->GetBackColor();
		}
		csInfo.gr.PushTextForeColor(crText);
		csInfo.gr.PushTextBackColor(crBack);
		bool bTrans = view.IsBkBitmap() && textType.GetBackColor() == crBack;
		Font font;
		font.fontAttr.bBoldFont = spaceType.IsBoldFont() || currentStyle.IsBoldFont();
		font.fontAttr.bUnderLine = spaceType.HasUnderLine();
		font.hFont = csInfo.view.GetFontset().ChooseFontHandle(font.fontAttr);
		csInfo.gr.PushMyFont(font);

		DispPos pos(*csInfo.pDispPos);	// 現在位置を覚えておく
		_DispEOL(csInfo.gr, csInfo.pDispPos, eol, view, bTrans);
		DrawImp_StylePop(csInfo);
		DrawImp_DrawUnderline(csInfo, pos);

		csInfo.nPosInLogic += eol.GetLen();
	}else {
		// 無限ループ対策
		csInfo.nPosInLogic += 1;
		assert_warning( 1 );
	}

	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     折り返し描画実装                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 折り返し描画
void _DispWrap(
	Graphics&		gr,
	DispPos*		pDispPos,
	const EditView&	view,
	int				nLineNum
	)
{
	RECT rcClip2;
	if (view.GetTextArea().GenerateClipRect(&rcClip2, *pDispPos, 1)) {
		// サポートクラス
		TypeSupport wrapType(view, COLORIDX_WRAP);
		TypeSupport textType(view, COLORIDX_TEXT);
		TypeSupport bgLineType(view, COLORIDX_CARETLINEBG);
		TypeSupport evenBgLineType(view, COLORIDX_EVENLINEBG);
		TypeSupport pageViewBgLineType(view,COLORIDX_PAGEVIEW);
		bool bBgcolor = wrapType.GetBackColor() == textType.GetBackColor();
		EColorIndexType bgColorOverwrite = COLORIDX_WRAP;
		bool bTrans = view.IsBkBitmap();
		if (wrapType.IsDisp()) {
			EditView& activeView = view.editWnd.GetActiveView();
			if (bgLineType.IsDisp() && view.GetCaret().GetCaretLayoutPos().y == nLineNum) {
				if (bBgcolor) {
					bgColorOverwrite = COLORIDX_CARETLINEBG;
					bTrans = bTrans && bgLineType.GetBackColor() == textType.GetBackColor();
				}
			}else if (evenBgLineType.IsDisp() && nLineNum % 2 == 1) {
				if (bBgcolor) {
					bgColorOverwrite = COLORIDX_EVENLINEBG;
					bTrans = bTrans && evenBgLineType.GetBackColor() == textType.GetBackColor();
				}
			}else if (
				view.bMiniMap
				&& activeView.GetTextArea().GetViewTopLine() <= nLineNum
				&& nLineNum < activeView.GetTextArea().GetBottomLine()
			) {
				bgColorOverwrite = COLORIDX_PAGEVIEW;
				bTrans = bTrans && pageViewBgLineType.GetBackColor() == textType.GetBackColor();
			}
		}
		bool bChangeColor = false;

		// 描画文字列と色の決定
		const wchar_t* szText;
		if (wrapType.IsDisp()) {
			szText = L"<";
			wrapType.SetGraphicsState_WhileThisObj(gr);
			if (bgColorOverwrite != COLORIDX_WRAP) {
				bChangeColor = true;
				gr.PushTextBackColor(TypeSupport(view, bgColorOverwrite).GetBackColor());
			}
		}else {
			szText = L" ";
		}

		// 描画
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClip2,
			szText,
			wcslen(szText),
			view.GetTextMetrics().GetDxArray_AllHankaku()
		);
		if (bChangeColor) {
			gr.PopTextBackColor();
		}
	}
	pDispPos->ForwardDrawCol(1);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       EOF描画実装                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*!
EOF記号の描画
*/
void _DispEOF(
	Graphics&			gr,			// [in] 描画対象のDevice Context
	DispPos*			pDispPos,	// [in] 表示座標
	const EditView&		view
	)
{
	// 描画に使う色情報
	TypeSupport eofType(view, COLORIDX_EOF);
	if (!eofType.IsDisp()) {
		return;
	}
	TypeSupport textType(view, COLORIDX_TEXT);
	bool bTrans = view.IsBkBitmap() && eofType.GetBackColor() == textType.GetBackColor();

	// 必要なインターフェースを取得
	const TextMetrics* pMetrics = &view.GetTextMetrics();
	const TextArea* pArea = &view.GetTextArea();

	// 定数
	static const wchar_t	szEof[] = L"[EOF]";
	const int		nEofLen = _countof(szEof) - 1;

	// クリッピング領域を計算
	RECT rcClip;
	if (pArea->GenerateClipRect(&rcClip, *pDispPos, nEofLen)) {
		// 色設定
		eofType.SetGraphicsState_WhileThisObj(gr);

		// 描画
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClip,
			szEof,
			nEofLen,
			pMetrics->GetDxArray_AllHankaku()
		);
	}

	// 描画位置を進める
	pDispPos->ForwardDrawCol(nEofLen);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       改行描画実装                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 画面描画補助関数
void _DrawEOL(
	Graphics&		gr,
	const Rect&		rcEol,
	Eol				eol,
	bool			bBold,
	COLORREF		pColor
);

void _DispEOL(Graphics& gr, DispPos* pDispPos, Eol eol, const EditView& view, bool bTrans)
{
	RECT rcClip2;
	if (view.GetTextArea().GenerateClipRect(&rcClip2, *pDispPos, 2)) {
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClip2,
			L"  ",
			2,
			view.GetTextMetrics().GetDxArray_AllHankaku()
		);

		// 改行記号の表示
		if (TypeSupport(view, COLORIDX_EOL).IsDisp()) {
			// リージョン作成、選択。
			gr.SetClipping(rcClip2);
			
			// 描画領域
			Rect rcEol;
			rcEol.SetPos(pDispPos->GetDrawPos().x + 1, pDispPos->GetDrawPos().y);
			rcEol.SetSize(view.GetTextMetrics().GetHankakuWidth(), view.GetTextMetrics().GetHankakuHeight());

			// 描画
			// 文字色や太字かどうかを現在の DC から調べる	
			// （検索マッチ等の状況に柔軟に対応するため、ここは記号の色指定には決め打ちしない）
			_DrawEOL(gr, rcEol, eol, gr.GetCurrentMyFontBold(), gr.GetCurrentTextForeColor());

			// リージョン破棄
			gr.ClearClipping();
		}
	}

	// 描画位置を進める(2桁)
	pDispPos->ForwardDrawCol(2);
}


/*!
画面描画補助関数:
行末の改行マークを改行コードによって書き分ける（メイン）

@note bBoldがtrueの時は横に1ドットずらして重ね書きを行うが、
あまり太く見えない。
*/
void _DrawEOL(
	Graphics&		gr,			// Device Context Handle
	const Rect&		rcEol,		// 描画領域
	Eol				eol,		// 行末コード種別
	bool			bBold,		// true: 太字
	COLORREF		pColor		// 色
	)
{
	int sx, sy;	// 矢印の先頭
	gr.SetPen(pColor);

	switch (eol.GetType()) {
	case EolType::CRLF:	// 下左矢印
		{
			sx = rcEol.left;						// X左端
			sy = rcEol.top + (rcEol.Height() / 2);	// Y中心
			DWORD pp[] = { 3, 3 };
			POINT pt[6];
			pt[0].x = sx + rcEol.Width();	// 上へ
			pt[0].y = sy - rcEol.Height() / 4;
			pt[1].x = sx + rcEol.Width();	// 下へ
			pt[1].y = sy;
			pt[2].x = sx;	// 先頭へ
			pt[2].y = sy;
			pt[3].x = sx + rcEol.Height() / 4;	// 先頭から下へ
			pt[3].y = sy + rcEol.Height() / 4;
			pt[4].x = sx;	// 先頭へ戻り
			pt[4].y = sy;
			pt[5].x = sx + rcEol.Height() / 4;	// 先頭から上へ
			pt[5].y = sy - rcEol.Height() / 4;
			::PolyPolyline(gr, pt, pp, _countof(pp));

			if (bBold) {
				pt[0].x += 1;	// 上へ（右へずらす）
				pt[0].y += 0;
				pt[1].x += 1;	// 右へ（右にひとつずれている）
				pt[1].y += 1;
				pt[2].x += 0;	// 先頭へ
				pt[2].y += 1;
				pt[3].x += 0;	// 先頭から下へ
				pt[3].y += 1;
				pt[4].x += 0;	// 先頭へ戻り
				pt[4].y += 1;
				pt[5].x += 0;	// 先頭から上へ
				pt[5].y += 1;
				::PolyPolyline(gr, pt, pp, _countof(pp));
			}
		}
		break;
	case EolType::CR:	// 左向き矢印
		{
			sx = rcEol.left;
			sy = rcEol.top + (rcEol.Height() / 2);
			DWORD pp[] = { 3, 2 };
			POINT pt[5];
			pt[0].x = sx + rcEol.Width();	// 右へ
			pt[0].y = sy;
			pt[1].x = sx;	// 先頭へ
			pt[1].y = sy;
			pt[2].x = sx + rcEol.Height() / 4;	// 先頭から下へ
			pt[2].y = sy + rcEol.Height() / 4;
			pt[3].x = sx;	// 先頭へ戻り
			pt[3].y = sy;
			pt[4].x = sx + rcEol.Height() / 4;	// 先頭から上へ
			pt[4].y = sy - rcEol.Height() / 4;
			::PolyPolyline(gr, pt, pp, _countof(pp));

			if (bBold) {
				pt[0].x += 0;	// 右へ
				pt[0].y += 1;
				pt[1].x += 0;	// 先頭へ
				pt[1].y += 1;
				pt[2].x += 0;	// 先頭から下へ
				pt[2].y += 1;
				pt[3].x += 0;	// 先頭へ戻り
				pt[3].y += 1;
				pt[4].x += 0;	// 先頭から上へ
				pt[4].y += 1;
				::PolyPolyline(gr, pt, pp, _countof(pp));
			}
		}
		break;
	case EolType::LF:	// 下向き矢印
		{
			sx = rcEol.left + (rcEol.Width() / 2);
			sy = rcEol.top + (rcEol.Height() * 3 / 4);
			DWORD pp[] = { 3, 2 };
			POINT pt[5];
			pt[0].x = sx;	// 上へ
			pt[0].y = rcEol.top + rcEol.Height() / 4 + 1;
			pt[1].x = sx;	// 上から下へ
			pt[1].y = sy;
			pt[2].x = sx - rcEol.Height() / 4;	// そのまま左上へ
			pt[2].y = sy - rcEol.Height() / 4;
			pt[3].x = sx;	// 矢印の先端に戻る
			pt[3].y = sy;
			pt[4].x = sx + rcEol.Height() / 4;	// そして右上へ
			pt[4].y = sy - rcEol.Height() / 4;
			::PolyPolyline(gr, pt, pp, _countof(pp));

			if (bBold) {
				pt[0].x += 1;	// 上へ
				pt[0].y += 0;
				pt[1].x += 1;	// 上から下へ
				pt[1].y += 0;
				pt[2].x += 1;	// そのまま左上へ
				pt[2].y += 0;
				pt[3].x += 1;	// 矢印の先端に戻る
				pt[3].y += 0;
				pt[4].x += 1;	// そして右上へ
				pt[4].y += 0;
				::PolyPolyline(gr, pt, pp, _countof(pp));
			}
		}
		break;
	case EolType::NEL:
	case EolType::LS:
	case EolType::PS:
		{
			// 左下矢印(折れ曲がりなし)
			sx = rcEol.left;			//X左端
			sy = rcEol.top + ( rcEol.Height() * 3 / 4 );	//Y上から3/4
			DWORD pp[] = { 2, 3 };
			POINT pt[5];
			int nWidth = t_min(rcEol.Width(), rcEol.Height() / 2);
			pt[0].x = sx + nWidth;	//	右上から
			pt[0].y = sy - nWidth;
			pt[1].x = sx;	//	先頭へ
			pt[1].y = sy;
			pt[2].x = sx + nWidth;	//	右から
			pt[2].y = sy;
			pt[3].x = sx;	//	先頭へ戻り
			pt[3].y = sy;
			pt[4].x = sx;	//	先頭から上へ
			pt[4].y = sy - nWidth;
			::PolyPolyline( gr, pt, pp, _countof(pp));

			if ( bBold ) {
				pt[0].x += 0;	//	右上から
				pt[0].y += 1;
				pt[1].x += 0;	//	先頭へ
				pt[1].y += 1;
				pt[2].x += 0;	//	右から
				pt[2].y -= 1;
				pt[3].x += 1;	//	先頭へ戻り
				pt[3].y -= 1;
				pt[4].x += 1;	//	先頭から上へ
				pt[4].y += 0;
				::PolyPolyline( gr, pt, pp, _countof(pp));
			}
		}
		break;
	}
}

