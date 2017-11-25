#include "StdAfx.h"
#include "TextDrawer.h"
#include <vector>
#include "TextMetrics.h"
#include "TextArea.h"
#include "ViewFont.h"
#include "Eol.h"
#include "view/EditView.h"
#include "doc/EditDoc.h"
#include "types/TypeSupport.h"
#include "charset/charcode.h"
#include "doc/layout/Layout.h"

const TextArea& TextDrawer::GetTextArea() const
{
	return editView.GetTextArea();
}

using namespace std;


/* テキスト表示 */
void TextDrawer::DispText(
	HDC			hdc,
	DispPos*	pDispPos,
	const wchar_t* pData,
	size_t		nLength,
	bool		bTransparent
	) const
{
	if (nLength == 0) {
		return;
	}
	int x = pDispPos->GetDrawPos().x;
	int y = pDispPos->GetDrawPos().y;

	// 必要なインターフェースを取得
	const TextMetrics* pMetrics = &editView.GetTextMetrics();
	const TextArea& textArea = GetTextArea();

	// 文字間隔配列を生成
	static vector<int> vDxArray(1);
	const int* pDxArray = pMetrics->GenerateDxArray(&vDxArray, pData, nLength, editView.GetTextMetrics().GetHankakuDx());

	// 文字列のピクセル幅
	size_t nTextWidth = pMetrics->CalcTextWidth(pData, nLength, pDxArray);

	// テキストの描画範囲の矩形を求める -> rcClip
	Rect rcClip;
	rcClip.left   = x;
	rcClip.right  = x + (int)nTextWidth;
	rcClip.top    = y;
	rcClip.bottom = y + editView.GetTextMetrics().GetHankakuDy();
	if (rcClip.left < textArea.GetAreaLeft()) {
		rcClip.left = textArea.GetAreaLeft();
	}

	// 文字間隔
	size_t nDx = editView.GetTextMetrics().GetHankakuDx();

	if (textArea.IsRectIntersected(rcClip) && rcClip.top >= textArea.GetAreaTop()) {

		if (rcClip.Width() > textArea.GetAreaWidth()) {
			rcClip.right = rcClip.left + textArea.GetAreaWidth();
		}

		// ウィンドウの左にあふれた文字数 -> nBefore
		size_t nBeforeLogic = 0;
		size_t nBeforeLayout = 0;
		if (x < 0) {
			size_t nLeftLayout = (0 - x) / nDx - 1;
			while (nBeforeLayout < nLeftLayout) {
				nBeforeLayout += NativeW::GetKetaOfChar(pData, nLength, nBeforeLogic);
				nBeforeLogic  += NativeW::GetSizeOfChar(pData, nLength, nBeforeLogic);
			}
		}

		/*
		// ウィンドウの右にあふれた文字数 -> nAfter
		int nAfterLayout = 0;
		if (rcClip.right < x + nTextWidth) {
			//	-1してごまかす（うしろはいいよね？）
			nAfterLayout = (x + nTextWidth - rcClip.right) / nDx - 1;
		}
		*/

		// 描画開始位置
		int nDrawX = x + nBeforeLayout * nDx;

		// 実際の描画文字列ポインタ
		const wchar_t*	pDrawData			= &pData[nBeforeLogic];
		int				nDrawDataMaxLength	= nLength - nBeforeLogic;

		// 実際の文字間隔配列
		const int* pDrawDxArray = &pDxArray[nBeforeLogic];

		// 描画する文字列長を求める -> nDrawLength
		int nRequiredWidth = rcClip.right - nDrawX; // 埋めるべきピクセル幅
		if (nRequiredWidth <= 0) {
			goto end;
		}
		int nWorkWidth = 0;
		int nDrawLength = 0;
		while (nWorkWidth < nRequiredWidth) {
			if (nDrawLength >= nDrawDataMaxLength) {
				break;
			}
			nWorkWidth += pDrawDxArray[nDrawLength++];
		}
		// サロゲートペア対策	2008/7/5 Uchi	Update 7/8 Uchi
		if (nDrawLength < nDrawDataMaxLength && pDrawDxArray[nDrawLength] == 0) {
			++nDrawLength;
		}

		// 描画
		::ExtTextOutW_AnyBuild(
			hdc,
			nDrawX,					// X
			y,						// Y
			ExtTextOutOption() & ~(bTransparent? ETO_OPAQUE: 0),
			&rcClip,
			pDrawData,				// 文字列
			nDrawLength,			// 文字列長
			pDrawDxArray			// 文字間隔の入った配列
		);
	}

end:
	// 描画位置を進める
	pDispPos->ForwardDrawCol(nTextWidth / nDx);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        指定桁縦線                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	指定桁縦線の描画
	@note Common::nVertLineOffsetにより、指定桁の前の文字の上に作画されることがある。
*/
void TextDrawer::DispVerticalLines(
	Graphics&	gr,			// 作画するウィンドウのDC
	int			nTop,		// 線を引く上端のクライアント座標y
	int			nBottom,	// 線を引く下端のクライアント座標y
	int			nLeftCol,	// 線を引く範囲の左桁の指定
	int	nRightCol	// 線を引く範囲の右桁の指定(-1で未指定)
	) const
{
	auto& view = editView;
	
	const TypeConfig& typeData = view.pEditDoc->docType.GetDocumentAttribute();
	
	TypeSupport vertType(view, COLORIDX_VERTLINE);
	TypeSupport textType(view, COLORIDX_TEXT);
	
	if (!vertType.IsDisp()) {
		return;
	}
	
	auto& textArea = view.GetTextArea();
	nLeftCol = t_max((int)textArea.GetViewLeftCol(), nLeftCol);
	
	const size_t nWrapKetas  = view.pEditDoc->layoutMgr.GetMaxLineKetas();
	const size_t nCharDx  = view.GetTextMetrics().GetHankakuDx();
	if (nRightCol < 0) {
		nRightCol = nWrapKetas;
	}
	const int nPosXOffset = GetDllShareData().common.window.nVertLineOffset + textArea.GetAreaLeft();
	const int nPosXLeft   = t_max(textArea.GetAreaLeft() + (nLeftCol - (int)textArea.GetViewLeftCol()) * (int)nCharDx, textArea.GetAreaLeft());
	const int nPosXRight  = t_min(textArea.GetAreaLeft() + (nRightCol - (int)textArea.GetViewLeftCol()) * (int)nCharDx, textArea.GetAreaRight());
	const size_t nLineHeight = view.GetTextMetrics().GetHankakuDy();
	bool bOddLine = ((((nLineHeight % 2) ? textArea.GetViewTopLine() : 0) + textArea.GetAreaTop() + nTop) % 2 == 1);

	// 太線
	const bool bBold = vertType.IsBoldFont();
	// ドット線(下線属性を転用/テスト用)
	const bool bDot = vertType.HasUnderLine();
	const bool bExorPen = (vertType.GetTextColor() == textType.GetBackColor());
	int nROP_Old = 0;
	if (bExorPen) {
		gr.SetPen(vertType.GetBackColor());
		nROP_Old = ::SetROP2(gr, R2_NOTXORPEN);
	}else {
		gr.SetPen(vertType.GetTextColor());
	}

	for (int k=0; k<MAX_VERTLINES && typeData.nVertLineIdx[k]!=0; ++k) {
		// nXColは1開始。GetTextArea().GetViewLeftCol()は0開始なので注意。
		int nXCol = typeData.nVertLineIdx[k];
		int nXColEnd = nXCol;
		int nXColAdd = 1;
		// nXColがマイナスだと繰り返し。k+1を終了値、k+2をステップ幅として利用する
		if (nXCol < 0) {
			if (k < MAX_VERTLINES - 2) {
				nXCol = -nXCol;
				nXColEnd = typeData.nVertLineIdx[++k];
				nXColAdd = typeData.nVertLineIdx[++k];
				if (nXColEnd < nXCol || nXColAdd <= 0) {
					continue;
				}
				// 作画範囲の始めまでスキップ
				if (nXCol < textArea.GetViewLeftCol()) {
					nXCol = textArea.GetViewLeftCol() + nXColAdd - (textArea.GetViewLeftCol() - nXCol) % nXColAdd;
				}
			}else {
				k += 2;
				continue;
			}
		}
		for (; nXCol<=nXColEnd; nXCol+=nXColAdd) {
			if ((int)nWrapKetas < nXCol) {
				break;
			}
			int nPosX = nPosXOffset + (nXCol - 1 - textArea.GetViewLeftCol()) * nCharDx;
			// 太線の場合、半分だけ作画する可能性がある。
			int nPosXBold = nPosX;
			if (bBold) {
				nPosXBold -= 1;
			}
			if (nPosXRight <= nPosXBold) {
				break;
			}
			if (nPosXLeft <= nPosX) {
				if (bDot) {
					// 点線で作画。1ドットの線を作成
					int y = nTop;
					// スクロールしても線が切れないように座標を調整
					if (bOddLine) {
						++y;
					}
					for (; y<nBottom; y+=2) {
						if (nPosX < nPosXRight) {
							::MoveToEx(gr, nPosX, y, NULL);
							::LineTo(gr, nPosX, y + 1);
						}
						if (bBold && nPosXLeft <= nPosXBold) {
							::MoveToEx(gr, nPosXBold, y, NULL);
							::LineTo(gr, nPosXBold, y + 1);
						}
					}
				}else {
					if (nPosX < nPosXRight) {
						::MoveToEx(gr, nPosX, nTop, NULL);
						::LineTo(gr, nPosX, nBottom);
					}
					if (bBold && nPosXLeft <= nPosXBold) {
						::MoveToEx(gr, nPosXBold, nTop, NULL);
						::LineTo(gr, nPosXBold, nBottom);
					}
				}
			}
		}
	}
	if (bExorPen) {
		::SetROP2(gr, nROP_Old);
	}
}

void TextDrawer::DispNoteLine(
	Graphics&	gr,			// 作画するウィンドウのDC
	int			nTop,		// 線を引く上端のクライアント座標y
	int			nBottom,	// 線を引く下端のクライアント座標y
	int			nLeft,		// 線を引く左端
	int			nRight		// 線を引く右端
	) const
{
	auto& view = editView;

	TypeSupport noteLine(view, COLORIDX_NOTELINE);
	if (noteLine.IsDisp()) {
		gr.SetPen(noteLine.GetTextColor());
		const size_t nLineHeight = view.GetTextMetrics().GetHankakuDy();
		const int left = nLeft;
		const int right = nRight;
		int userOffset = view.pTypeData->nNoteLineOffset;
		int offset = view.GetTextArea().GetAreaTop() + userOffset - 1;
		while (offset < 0) {
			offset += nLineHeight;
		}
		int offsetMod = offset % nLineHeight;
		int y = ((nTop - offset) / nLineHeight * nLineHeight) + offsetMod;
		for (; y<nBottom; y+=nLineHeight) {
			if (nTop <= y) {
				::MoveToEx( gr, left, y, NULL );
				::LineTo( gr, right, y );
			}
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        折り返し桁縦線                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	折り返し桁縦線の描画 */
void TextDrawer::DispWrapLine(
	Graphics&	gr,			// 作画するウィンドウのDC
	int			nTop,		// 線を引く上端のクライアント座標y
	int			nBottom		// 線を引く下端のクライアント座標y
	) const
{
	auto& view = editView;
	TypeSupport wrapType(view, COLORIDX_WRAP);
	if (!wrapType.IsDisp()) {
		return;
	}

	const TextArea& textArea = GetTextArea();
	const size_t nWrapKetas = view.pEditDoc->layoutMgr.GetMaxLineKetas();
	const size_t nCharDx = view.GetTextMetrics().GetHankakuDx();
	int nXPos = textArea.GetAreaLeft() + (nWrapKetas - textArea.GetViewLeftCol()) * nCharDx;
	if (textArea.GetAreaLeft() < nXPos && nXPos < textArea.GetAreaRight()) {
		/// 折り返し記号の色のペンを設定
		gr.PushPen(wrapType.GetTextColor(), 0);

		::MoveToEx(gr, nXPos, nTop, NULL);
		::LineTo(gr, nXPos, nBottom);

		gr.PopPen();
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          行番号                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void TextDrawer::DispLineNumber(
	Graphics&		gr,
	int				nLineNum,
	int				y
	) const
{
	//$$ 高速化：SearchLineByLayoutYにキャッシュを持たせる
	const Layout*	pLayout = EditDoc::GetInstance(0)->layoutMgr.SearchLineByLayoutY(nLineNum);

	auto& view = editView;
	const TypeConfig& typeConfig = view.pEditDoc->docType.GetDocumentAttribute();

	unsigned int nLineHeight = (unsigned int)view.GetTextMetrics().GetHankakuDy();
	size_t nCharWidth = view.GetTextMetrics().GetHankakuDx();
	// 行番号表示部分X幅	Sep. 23, 2002 genta 共通式のくくりだし
	//int nLineNumAreaWidth = pView->GetTextArea().nViewAlignLeftCols * nCharWidth;
	int nLineNumAreaWidth = view.GetTextArea().GetAreaLeft() - GetDllShareData().common.window.nLineNumRightSpace;

	TypeSupport textType(view, COLORIDX_TEXT);
	TypeSupport caretLineBg(view, COLORIDX_CARETLINEBG);
	TypeSupport evenLineBg(view, COLORIDX_EVENLINEBG);
	// 行がないとき・行の背景が透明のときの色
	TypeSupport& backType = (caretLineBg.IsDisp() &&
		view.GetCaret().GetCaretLayoutPos().GetY() == nLineNum
			? caretLineBg
			: evenLineBg.IsDisp() && nLineNum % 2 == 1
				? evenLineBg
				: textType);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     nColorIndexを決定                       //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	EColorIndexType nColorIndex = COLORIDX_GYOU;	// 行番号
	const DocLine* pDocLine = nullptr;
	bool bGyouMod = false;
	if (pLayout) {
		pDocLine = pLayout->GetDocLineRef();
		if (1
			&& view.GetDocument().docEditor.IsModified()
			&& ModifyVisitor().IsLineModified(
				pDocLine,
				view.GetDocument().docEditor.opeBuf.GetNoModifiedSeq()
			)
		) {
			// 変更フラグ
			if (TypeSupport(view, COLORIDX_GYOU_MOD).IsDisp()) {
				nColorIndex = COLORIDX_GYOU_MOD;	// 行番号（変更行）
				bGyouMod = true;
			}
		}
	}

	if (pDocLine) {
		// DIFF色設定
		DiffLineGetter(pDocLine).GetDiffColor(&nColorIndex);

		// ブックマークの表示
		if (BookmarkGetter(pDocLine).IsBookmarked()) {
			if (TypeSupport(view, COLORIDX_MARK).IsDisp()) {
				nColorIndex = COLORIDX_MARK;
			}
		}
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             決定されたnColorIndexを使って描画               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	TypeSupport colorType(view, nColorIndex);
	TypeSupport markType(view, COLORIDX_MARK);

	// 該当行の行番号エリア矩形
	RECT rcLineNum;
	rcLineNum.left = 0;
	rcLineNum.right = nLineNumAreaWidth;
	rcLineNum.top = y;
	rcLineNum.bottom = y + nLineHeight;
	
	bool bTrans = view.IsBkBitmap() && textType.GetBackColor() == colorType.GetBackColor();
	bool bTransText = view.IsBkBitmap() && textType.GetBackColor() == backType.GetBackColor();
	bool bDispLineNumTrans = false;

	COLORREF fgcolor = colorType.GetTextColor();
	COLORREF bgcolor = colorType.GetBackColor();
	TypeSupport gyouType(view, COLORIDX_GYOU);
	TypeSupport gyouModType(view, COLORIDX_GYOU_MOD);
	if (bGyouMod && nColorIndex != COLORIDX_GYOU_MOD) {
		if (gyouType.GetTextColor() == colorType.GetTextColor()) {
			fgcolor = gyouModType.GetTextColor();
		}
		if (gyouType.GetBackColor() == colorType.GetBackColor()) {
			bgcolor = gyouModType.GetBackColor();
			bTrans = view.IsBkBitmap() && textType.GetBackColor() == gyouModType.GetBackColor();
		}
	}
	// 背景色がテキストと同じなら、透過色として行背景色を適用
	if (bgcolor == textType.GetBackColor()) {
		bgcolor = backType.GetBackColor();
		bTrans = view.IsBkBitmap() && textType.GetBackColor() == bgcolor;
		bDispLineNumTrans = true;
	}
	if (!pLayout) {
		// 行が存在しない場合は、テキスト描画色で塗りつぶし
		if (!bTransText) {
			textType.FillBack(gr, rcLineNum);
		}
		bDispLineNumTrans = true;
	}else if (TypeSupport(view, COLORIDX_GYOU).IsDisp()) { // 行番号表示／非表示
		Font font = colorType.GetTypeFont();
	 	// 2013.12.30 変更行の色・フォント属性をDIFFブックマーク行に継承するように
		if (bGyouMod && nColorIndex != COLORIDX_GYOU_MOD) {
			bool bChange = true;
			if (gyouType.IsBoldFont() == colorType.IsBoldFont()) {
		 		font.fontAttr.bBoldFont = gyouModType.IsBoldFont();
				bChange = true;
			}
			if (gyouType.HasUnderLine() == colorType.HasUnderLine()) {
				font.fontAttr.bUnderLine = gyouModType.HasUnderLine();
				bChange = true;
			}
			if (bChange) {
				font.hFont = view.GetFontset().ChooseFontHandle(font.fontAttr);
			}
		}
		gr.PushTextForeColor(fgcolor);	// テキスト：行番号の色
		gr.PushTextBackColor(bgcolor);	// テキスト：行番号背景の色
		gr.PushMyFont(font);	// フォント：行番号のフォント

		// 描画文字列
		wchar_t szLineNum[18];
		size_t nLineCols;
		size_t nLineNumCols;
		{
			// 行番号の表示 false=折り返し単位／true=改行単位
			if (typeConfig.bLineNumIsCRLF) {
				// 論理行番号表示モード
				if (!pLayout || pLayout->GetLogicOffset() != 0) { // 折り返しレイアウト行
					wcscpy(szLineNum, L" ");
				}else {
					_itow(pLayout->GetLogicLineNo() + 1, szLineNum, 10);	// 対応する論理行番号
//###デバッグ用
//					_itow(ModifyVisitor().GetLineModifiedSeq(pDocLine), szLineNum, 10);	// 行の変更番号
				}
			}else {
				// 物理行（レイアウト行）番号表示モード
				_itow(nLineNum + 1, szLineNum, 10);
			}
			nLineCols = wcslen(szLineNum);
			nLineNumCols = nLineCols;

			// 行番号区切り 0=なし 1=縦線 2=任意
			if (typeConfig.nLineTermType == 2) {
				//	Sep. 22, 2002 genta
				szLineNum[nLineCols] = typeConfig.cLineTermChar;
				szLineNum[++nLineCols] = '\0';
			}
		}

		//	Sep. 23, 2002 genta
		int drawNumTop = (view.GetTextArea().nViewAlignLeftCols - nLineNumCols - 1) * (nCharWidth);
		::ExtTextOutW_AnyBuild(gr,
			drawNumTop,
			y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcLineNum,
			szLineNum,
			nLineCols,
			view.GetTextMetrics().GetDxArray_AllHankaku()
		);

		// 行番号区切り 0=なし 1=縦線 2=任意
		if (typeConfig.nLineTermType == 1) {
			RECT rc;
			rc.left = nLineNumAreaWidth - 2;
			rc.top = y;
			rc.right = nLineNumAreaWidth - 1;
			rc.bottom = y + nLineHeight;
			gr.FillSolidMyRect(rc, fgcolor);
		}

		gr.PopTextForeColor();
		gr.PopTextBackColor();
		gr.PopMyFont();
	}else {
		// 行番号エリアの背景描画
		if (!bTrans) {
			gr.FillSolidMyRect(rcLineNum, bgcolor);
		}
		bDispLineNumTrans = true;
	}

	// 行属性描画 ($$$分離予定)
	if (pDocLine) {
		// とりあえずブックマークに縦線
		if (BookmarkGetter(pDocLine).IsBookmarked() && !markType.IsDisp()) {
			gr.PushPen(colorType.GetTextColor(), 2);
			::MoveToEx(gr, 1, y, NULL);
			::LineTo(gr, 1, y + (int)nLineHeight);
			gr.PopPen();
		}

		// DIFFマーク描画
		DiffLineGetter(pDocLine).DrawDiffMark(gr, y, nLineHeight, fgcolor);
	}

	// 行番号とテキストの隙間の描画
	if (!bTransText) {
		RECT rcRest;
		rcRest.left   = rcLineNum.right;
		rcRest.right  = view.GetTextArea().GetAreaLeft();
		rcRest.top    = y;
		rcRest.bottom = y + nLineHeight;
		textType.FillBack(gr, rcRest);
	}
	
	// 行番号部分のノート線描画
	if (!view.bMiniMap) {
		int left   = bDispLineNumTrans ? 0 : rcLineNum.right;
		int right  = view.GetTextArea().GetAreaLeft();
		int top    = y;
		int bottom = y + nLineHeight;
		DispNoteLine( gr, top, bottom, left, right );
	}
}

