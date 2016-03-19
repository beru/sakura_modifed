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
#include "view/EditView.h" // SColorStrategyInfo
#include "Figure_Eol.h"
#include "types/TypeSupport.h"
#include "env/ShareData.h"
#include "env/DllSharedData.h"
#include "window/EditWnd.h"

// 折り返し描画
void _DispWrap(Graphics& gr, DispPos* pDispPos, const EditView* pView);

// EOF描画関数
// 実際には pX と nX が更新される。
// 2004.05.29 genta
// 2007.08.25 kobake 戻り値を void に変更。引数 x, y を DispPos に変更
// 2007.08.25 kobake 引数から nCharWidth, nLineHeight を削除
// 2007.08.28 kobake 引数 fuOptions を削除
//void _DispEOF(Graphics& gr, DispPos* pDispPos, const EditView* pView, bool bTrans);

// 改行記号描画
// 2007.08.30 kobake 追加
void _DispEOL(Graphics& gr, DispPos* pDispPos, Eol eol, const EditView* pView, bool bTrans);


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        Figure_Eol                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool Figure_Eol::Match(const wchar_t* pText, int nTextLen) const
{
	// 2014.06.18 折り返し・最終行だとDrawImpでeol.GetLen()==0になり無限ループするので
	// もしも行の途中に改行コードがあった場合はMatchさせない
	if (nTextLen == 2 && pText[0] == L'\r' && pText[1] == L'\n') return true;
	if (nTextLen == 1 && WCODE::IsLineDelimiterExt(pText[0])) return true;
	return false;
}

// 2006.04.29 Moca 選択処理のため縦線処理を追加
//$$ 高速化可能。
bool Figure_Eol::DrawImp(ColorStrategyInfo* pInfo)
{
	EditView* pView = pInfo->pView;

	// 改行取得
	const Layout* pLayout = pInfo->pDispPos->GetLayoutRef();
	Eol eol = pLayout->GetLayoutEol();
	if (eol.GetLen()) {
		// CFigureSpace::DrawImp_StyleSelectもどき。選択・検索色を優先する
		TypeSupport currentType(pView, pInfo->GetCurrentColor());	// 周辺の色（現在の指定色/選択色）
		TypeSupport currentType2(pView, pInfo->GetCurrentColor2());	// 周辺の色（現在の指定色）
		TypeSupport textType(pView, COLORIDX_TEXT);				// テキストの指定色
		TypeSupport spaceType(pView, GetDispColorIdx());	// 空白の指定色
		TypeSupport searchType(pView, COLORIDX_SEARCH);	// 検索色(EOL固有)
		TypeSupport currentTypeBg(pView, pInfo->GetCurrentColorBg());
		TypeSupport& currentType3 = (currentType2.GetBackColor() == textType.GetBackColor() ? currentTypeBg: currentType2);
		COLORREF crText;
		COLORREF crBack;
		bool bSelecting = pInfo->GetCurrentColor() != pInfo->GetCurrentColor2();
		bool blendColor = bSelecting && currentType.GetTextColor() == currentType.GetBackColor(); // 選択混合色
		TypeSupport& currentStyle = blendColor ? currentType2 : currentType;
		TypeSupport *pcText, *pcBack;
		if (bSelecting && !blendColor) {
			// 選択文字色固定指定
			pcText = &currentType;
			pcBack = &currentType;
		}else if (pInfo->GetCurrentColor2() == COLORIDX_SEARCH) {
			// 検索色優先
			pcText = &searchType;
			pcBack = &searchType;
		}else {
			pcText = spaceType.GetTextColor() == textType.GetTextColor() ? &currentType2 : &spaceType;
			pcBack = spaceType.GetBackColor() == textType.GetBackColor() ? &currentType3 : &spaceType;
		}
		if (blendColor) {
			// 混合色(検索色を優先しつつ)
			crText = pView->GetTextColorByColorInfo2(currentType.GetColorInfo(), pcText->GetColorInfo());
			crBack = pView->GetBackColorByColorInfo2(currentType.GetColorInfo(), pcBack->GetColorInfo());
		}else {
			crText = pcText->GetTextColor();
			crBack = pcBack->GetBackColor();
		}
		pInfo->gr.PushTextForeColor(crText);
		pInfo->gr.PushTextBackColor(crBack);
		bool bTrans = pView->IsBkBitmap() && textType.GetBackColor() == crBack;
		Font font;
		font.fontAttr.bBoldFont = spaceType.IsBoldFont() || currentStyle.IsBoldFont();
		font.fontAttr.bUnderLine = spaceType.HasUnderLine();
		font.hFont = pInfo->pView->GetFontset().ChooseFontHandle(font.fontAttr);
		pInfo->gr.PushMyFont(font);

		DispPos pos(*pInfo->pDispPos);	// 現在位置を覚えておく
		_DispEOL(pInfo->gr, pInfo->pDispPos, eol, pView, bTrans);
		DrawImp_StylePop(pInfo);
		DrawImp_DrawUnderline(pInfo, pos);

		pInfo->nPosInLogic += eol.GetLen();
	}else {
		// 無限ループ対策
		pInfo->nPosInLogic += 1;
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
	const EditView*	pView,
	LayoutYInt		nLineNum
	)
{
	RECT rcClip2;
	if (pView->GetTextArea().GenerateClipRect(&rcClip2, *pDispPos, 1)) {
		// サポートクラス
		TypeSupport wrapType(pView, COLORIDX_WRAP);
		TypeSupport textType(pView, COLORIDX_TEXT);
		TypeSupport bgLineType(pView, COLORIDX_CARETLINEBG);
		TypeSupport evenBgLineType(pView, COLORIDX_EVENLINEBG);
		TypeSupport pageViewBgLineType(pView,COLORIDX_PAGEVIEW);
		bool bBgcolor = wrapType.GetBackColor() == textType.GetBackColor();
		EColorIndexType bgColorOverwrite = COLORIDX_WRAP;
		bool bTrans = pView->IsBkBitmap();
		if (wrapType.IsDisp()) {
			EditView& activeView = pView->m_pEditWnd->GetActiveView();
			if (bgLineType.IsDisp() && pView->GetCaret().GetCaretLayoutPos().GetY2() == nLineNum) {
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
				pView->m_bMiniMap
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
				gr.PushTextBackColor(TypeSupport(pView, bgColorOverwrite).GetBackColor());
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
			pView->GetTextMetrics().GetDxArray_AllHankaku()
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
@date 2004.05.29 genta  MIKさんのアドバイスにより関数にくくりだし
@date 2007.08.28 kobake 引数 nCharWidth 削除
@date 2007.08.28 kobake 引数 fuOptions 削除
@date 2007.08.30 kobake 引数 EofColInfo 削除
*/
void _DispEOF(
	Graphics&			gr,			// [in] 描画対象のDevice Context
	DispPos*			pDispPos,	// [in] 表示座標
	const EditView*		pView
	)
{
	// 描画に使う色情報
	TypeSupport eofType(pView, COLORIDX_EOF);
	if (!eofType.IsDisp()) {
		return;
	}
	TypeSupport textType(pView, COLORIDX_TEXT);
	bool bTrans = pView->IsBkBitmap() && eofType.GetBackColor() == textType.GetBackColor();

	// 必要なインターフェースを取得
	const TextMetrics* pMetrics = &pView->GetTextMetrics();
	const TextArea* pArea = &pView->GetTextArea();

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
// May 23, 2000 genta
//@@@ 2001.12.21 YAZAKI 改行記号の書きかたが変だったので修正
void _DrawEOL(
	Graphics&		gr,
	const Rect&		rcEol,
	Eol				eol,
	bool			bBold,
	COLORREF		pColor
);

// 2007.08.30 kobake 追加
void _DispEOL(Graphics& gr, DispPos* pDispPos, Eol eol, const EditView* pView, bool bTrans)
{
	RECT rcClip2;
	if (pView->GetTextArea().GenerateClipRect(&rcClip2, *pDispPos, 2)) {
		// 2003.08.17 ryoji 改行文字が欠けないように
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClip2,
			L"  ",
			2,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);

		// 改行記号の表示
		if (TypeSupport(pView, COLORIDX_EOL).IsDisp()) {
			// From Here 2003.08.17 ryoji 改行文字が欠けないように

			// リージョン作成、選択。
			gr.SetClipping(rcClip2);
			
			// 描画領域
			Rect rcEol;
			rcEol.SetPos(pDispPos->GetDrawPos().x + 1, pDispPos->GetDrawPos().y);
			rcEol.SetSize(pView->GetTextMetrics().GetHankakuWidth(), pView->GetTextMetrics().GetHankakuHeight());

			// 描画
			// 文字色や太字かどうかを現在の DC から調べる	// 2009.05.29 ryoji 
			// （検索マッチ等の状況に柔軟に対応するため、ここは記号の色指定には決め打ちしない）
			// 2013.06.21 novice 文字色、太字をGraphicsから取得
			_DrawEOL(gr, rcEol, eol, gr.GetCurrentMyFontBold(), gr.GetCurrentTextForeColor());

			// リージョン破棄
			gr.ClearClipping();

			// To Here 2003.08.17 ryoji 改行文字が欠けないように
		}
	}

	// 描画位置を進める(2桁)
	pDispPos->ForwardDrawCol(2);
}


//	May 23, 2000 genta
/*!
画面描画補助関数:
行末の改行マークを改行コードによって書き分ける（メイン）

@note bBoldがtrueの時は横に1ドットずらして重ね書きを行うが、
あまり太く見えない。

@date 2001.12.21 YAZAKI 改行記号の描きかたを変更。ペンはこの関数内で作るようにした。
						矢印の先頭を、sx, syにして描画ルーチン書き直し。
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
	case EolType::CR:	// 左向き矢印	// 2007.08.17 ryoji EolType::LF -> EolType::CR
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
	case EolType::LF:	// 下向き矢印	// 2007.08.17 ryoji EolType::CR -> EolType::LF
	// 2013.04.22 Moca NEL,LS,PS対応。暫定でLFと同じにする
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

