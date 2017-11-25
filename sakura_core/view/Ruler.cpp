#include "StdAfx.h"
#include "Ruler.h"
#include "TextArea.h"
#include "view/EditView.h"
#include "doc/EditDoc.h"
#include "types/TypeSupport.h"

Ruler::Ruler(const EditView& editView, const EditDoc& editDoc)
	:
	editView(editView),
	editDoc(editDoc)
{
	nOldRulerDrawX = 0;	// 前回描画したルーラーのキャレット位置
	nOldRulerWidth = 0;	// 前回描画したルーラーのキャレット幅
}

Ruler::~Ruler()
{
}

void Ruler::_DrawRulerCaret(
	Graphics& gr,
	int nCaretDrawPosX,
	int nCaretWidth
	)
{
	// 描画領域 -> hRgn
	RECT rc;
	rc.left = nCaretDrawPosX + 1;
	rc.right = rc.left + editView.GetTextMetrics().GetHankakuDx() - 1;
	rc.top = 0;
	rc.bottom = editView.GetTextArea().GetAreaTop() - editView.GetTextArea().GetTopYohaku() - 1;
	HRGN hRgn = ::CreateRectRgnIndirect(&rc);

	// ブラシ作成 -> hBrush
	HBRUSH hBrush;
	if (nCaretWidth == 0) {
		hBrush = ::CreateSolidBrush(RGB(128, 128, 128));
	}else {
		hBrush = ::CreateSolidBrush(RGB(0, 0, 0));
	}

	// 領域を描画 (色を反転させる)
	int nROP_Old  = ::SetROP2(gr, R2_NOTXORPEN);
	HBRUSH hBrushOld = (HBRUSH)::SelectObject(gr, hBrush);
	::SelectObject(gr, hBrush);
	::PaintRgn(gr, hRgn);
	::SelectObject(gr, hBrushOld);
	::SetROP2(gr, nROP_Old);

	// 描画オブジェクト破棄
	::DeleteObject(hRgn);
	::DeleteObject(hBrush);
}

/*! 
	ルーラーのキャレットを再描画
	@param hdc [in] デバイスコンテキスト
	DispRulerの内容を元に作成
*/
void Ruler::DrawRulerCaret(Graphics& gr)
{
	auto& textArea = editView.GetTextArea();
	auto& caret = editView.GetCaret();
	if (1
		&& textArea.GetViewLeftCol() <= caret.GetCaretLayoutPos().GetX()
		&& textArea.GetRightCol() + 2 >= caret.GetCaretLayoutPos().GetX()
	) {
		auto& ruler = editView.GetRuler();
		if (1
			&& ruler.nOldRulerDrawX == caret.CalcCaretDrawPos(caret.GetCaretLayoutPos()).x
			&& caret.GetCaretSize().cx == ruler.nOldRulerWidth
		) {
			// 前描画した位置画同じ かつ ルーラーのキャレット幅が同じ 
			return;
		}

		// 元位置をクリア nOldRulerWidth
		this->_DrawRulerCaret(gr, nOldRulerDrawX, nOldRulerWidth);

		// 新しい位置で描画
		this->_DrawRulerCaret(
			gr,
			caret.CalcCaretDrawPos(caret.GetCaretLayoutPos()).x,
			caret.GetCaretSize().cx
		);
	}
}

// ルーラーの背景のみ描画
void Ruler::DrawRulerBg(Graphics& gr)
{
	// 必要なインターフェース
	CommonSetting* pCommon = &GetDllShareData().common;

	// サポート
	TypeSupport rulerType(editView, COLORIDX_RULER);

	// フォント設定 (ルーラー上の数字用)
	LOGFONT	lf = {0};
	lf.lfHeight			= 1 - pCommon->window.nRulerHeight;
	lf.lfWidth			= 5;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= 400;
	lf.lfItalic			= 0;
	lf.lfUnderline		= 0;
	lf.lfStrikeOut		= 0;
	lf.lfCharSet		= 0;
	lf.lfOutPrecision	= 3;
	lf.lfClipPrecision	= 2;
	lf.lfQuality		= 1;
	lf.lfPitchAndFamily	= 34;
	_tcscpy(lf.lfFaceName, _T("Arial"));
	HFONT hFont = ::CreateFontIndirect(&lf);
	HFONT hFontOld = (HFONT)::SelectObject(gr, hFont);
	::SetBkMode(gr, TRANSPARENT);
	
	auto& textArea = editView.GetTextArea();
	// 背景塗りつぶし
	RECT rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = textArea.GetAreaRight();
	rc.bottom = textArea.GetAreaTop() - textArea.GetTopYohaku();
	rulerType.FillBack(gr, rc);
	
	// ルーラー色設定
	gr.PushPen(rulerType.GetTextColor(), 0);
	gr.PushTextForeColor(rulerType.GetTextColor());
	
	// 描画開始位置
	int nX = textArea.GetAreaLeft();
	int nY = textArea.GetRulerHeight() - 2;
	
	// 下線 (ルーラーと本文の境界)
	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	size_t nMaxLineKetas = editDoc.layoutMgr.GetMaxLineKetas();
	int nToX = textArea.GetAreaLeft() + ((int)nMaxLineKetas - textArea.GetViewLeftCol()) * editView.GetTextMetrics().GetHankakuDx() + 1;
	if (nToX > textArea.GetAreaRight()) {
		nToX = textArea.GetAreaRight();
	}
	::MoveToEx(gr, textArea.GetAreaLeft(), nY + 1, NULL);
	::LineTo(gr, nToX, nY + 1);

	// 目盛を描画
	int i = textArea.GetViewLeftCol();
	while (i <= textArea.GetRightCol() + 1 && i <= nMaxLineKetas) {
		// ルーラー終端の区切り(大)
		if (i == nMaxLineKetas) {
			::MoveToEx(gr, nX, nY, NULL);
			::LineTo(gr, nX, 0);
		// 10目盛おきの区切り(大)と数字
		}else if (i % 10 == 0) {
			wchar_t szColumn[32];
			::MoveToEx(gr, nX, nY, NULL);
			::LineTo(gr, nX, 0);
			_itow(i / 10, szColumn, 10);
			::TextOutW_AnyBuild(gr, nX + 2 + 0, -1 + 0, szColumn, wcslen(szColumn));
		// 5目盛おきの区切り(中)
		}else if (i % 5 == 0) {
			::MoveToEx(gr, nX, nY, NULL);
			::LineTo(gr, nX, nY - 6);
		// 毎目盛の区切り(小)
		}else {
			::MoveToEx(gr, nX, nY, NULL);
			::LineTo(gr, nX, nY - 3);
		}

		nX += (int)editView.GetTextMetrics().GetHankakuDx();
		++i;
	}

	// 色戻す
	gr.PopTextForeColor();
	gr.PopPen();

	// フォント戻す
	::SelectObject(gr, hFontOld);
	::DeleteObject(hFont);
}

/*! ルーラー描画 */
void Ruler::DispRuler(HDC hdc)
{
	// サポート
	TypeSupport rulerType(editView, COLORIDX_RULER);

	if (!editView.GetDrawSwitch()) {
		return;
	}
	if (!rulerType.IsDisp()) {
		return;
	}

	// 描画対象
	Graphics gr(hdc);
	auto& caret = editView.GetCaret();
	// ルーラー全体を描き直す必要がない場合は、ルーラ上のキャレットのみ描きなおす 
	if (!bRedrawRuler) {
		DrawRulerCaret(gr);
	}else {
		// 背景描画
		DrawRulerBg(gr);
		auto& textArea = editView.GetTextArea();
		// キャレット描画
		if (1
			&& textArea.GetViewLeftCol() <= caret.GetCaretLayoutPos().GetX()
			&& textArea.GetRightCol() + 2 >= caret.GetCaretLayoutPos().GetX()
		) {
			_DrawRulerCaret(gr, caret.CalcCaretDrawPos(caret.GetCaretLayoutPos()).x, caret.GetCaretSize().cx);
		}

		bRedrawRuler = false;	// bRedrawRuler = true で指定されるまで、ルーラのキャレットのみを再描画
	}

	// 描画したルーラーのキャレット位置・幅を保存
	nOldRulerDrawX = caret.CalcCaretDrawPos(caret.GetCaretLayoutPos()).x;
	nOldRulerWidth = caret.GetCaretSize().cx ;
}

