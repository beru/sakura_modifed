#include "StdAfx.h"
#include "Graphics.h"
#include "util/std_macro.h"

class GDIStock {
public:
	GDIStock() {}
	~GDIStock() {
		while (!objects.empty()) {
			::DeleteObject(objects.back());
			objects.pop_back();
		}
	}
	bool Register(HGDIOBJ hObject) {
		if (hObject) {
			objects.push_back(hObject);
			return true;
		}
		return false;
	}
protected:
	std::vector<HGDIOBJ> objects;
};

static GDIStock s_gdiStock;	// 唯一の GDIStock オブジェクト

void Graphics::Init(HDC hdc)
{
	this->hdc = hdc;
	// ペン
	hpnOrg = NULL;
	// ブラシ
	hbrOrg = NULL;
	hbrCurrent = NULL;
	bDynamicBrush = false;
}

Graphics::~Graphics()
{
	ClearClipping();
	ClearMyFont();
	ClearPen();
	ClearBrush();
	RestoreTextColors();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       クリッピング                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


void Graphics::_InitClipping()
{
	if (clippingRgns.empty()) {
		// 元のクリッピング領域を取得
		RECT rcDummy = {0, 0, 1, 1};
		HRGN hrgnOrg = ::CreateRectRgnIndirect(&rcDummy);
		int nRet = ::GetClipRgn(hdc, hrgnOrg);
		if (nRet != 1) {
			::DeleteObject(hrgnOrg);
			hrgnOrg = NULL;
		}
		// 保存
		clippingRgns.push_back(hrgnOrg);
	}
}

void Graphics::PushClipping(const RECT& rc)
{
	_InitClipping();
	// 新しく作成→HDCに設定→スタックに保存
	HRGN hrgnNew = CreateRectRgnIndirect(&rc);
	::SelectClipRgn(hdc, hrgnNew);
	clippingRgns.push_back(hrgnNew);
}

void Graphics::PopClipping()
{
	if (clippingRgns.size() >= 2) {
		// 最後の要素を削除
		::DeleteObject(clippingRgns.back());
		clippingRgns.pop_back();
		// この時点の最後の要素をHDCに設定
		::SelectClipRgn(hdc, clippingRgns.back());
	}
}

void Graphics::ClearClipping()
{
	// 元のクリッピングに戻す
	if (!clippingRgns.empty()) {
		::SelectClipRgn(hdc, clippingRgns[0]);
	}
	// 領域をすべて削除
	int nSize = (int)clippingRgns.size();
	for (int i=0; i<nSize; ++i) {
		::DeleteObject(clippingRgns[i]);
	}
	clippingRgns.clear();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      テキスト文字色                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::PushTextForeColor(COLORREF color)
{
	// 設定
	COLORREF cOld = ::SetTextColor(hdc, color);
	// 記録
	if (textForeColors.empty()) {
		textForeColors.push_back(cOld);
	}
	textForeColors.push_back(color);
}

void Graphics::PopTextForeColor()
{
	// 戻す
	if (textForeColors.size() >= 2) {
		textForeColors.pop_back();
		::SetTextColor(hdc, textForeColors.back());
	}
}

void Graphics::ClearTextForeColor()
{
	if (!textForeColors.empty()) {
		::SetTextColor(hdc, textForeColors[0]);
		textForeColors.clear();
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      テキスト背景色                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::PushTextBackColor(COLORREF color)
{
	// 設定
	COLORREF cOld = ::SetBkColor(hdc, color);
	// 記録
	if (textBackColors.empty()) {
		textBackColors.push_back(cOld);
	}
	textBackColors.push_back(color);
}

void Graphics::PopTextBackColor()
{
	// 戻す
	if (textBackColors.size() >= 2) {
		textBackColors.pop_back();
		::SetBkColor(hdc, textBackColors.back());
	}
}

void Graphics::ClearTextBackColor()
{
	if (!textBackColors.empty()) {
		::SetBkColor(hdc, textBackColors[0]);
		textBackColors.clear();
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         テキスト                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::RestoreTextColors()
{
	PopTextForeColor();
	PopTextBackColor();
	if (nTextModeOrg.HasData()) {
		::SetBkMode(hdc, nTextModeOrg.Get());
		nTextModeOrg.Clear();
	}
}
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         フォント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::PushMyFont(const Font& font)
{
	// 設定
	HFONT hFontOld = (HFONT)SelectObject(hdc, font.hFont);

	// 記録
	if (fonts.empty()) {
		Font fontOld = { { false, false }, hFontOld };
		fonts.push_back(fontOld);
	}
	fonts.push_back(font);
}

void Graphics::PopMyFont()
{
	// 戻す
	if (fonts.size() >= 2) {
		fonts.pop_back();
		SelectObject(hdc, fonts.back().hFont);
	}
}

void Graphics::ClearMyFont()
{
	if (!fonts.empty()) {
		SelectObject(hdc, fonts[0].hFont);
		fonts.clear();
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ペン                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::PushPen(COLORREF color, int nPenWidth, int nStyle)
{
	HPEN hpnNew = CreatePen(nStyle, nPenWidth, color);
	HPEN hpnOld = (HPEN)SelectObject(hdc, hpnNew);
	pens.push_back(hpnNew);
	if (!hpnOrg) {
		hpnOrg = hpnOld;
	}
}

void Graphics::PopPen()
{
	// 選択する候補
	HPEN hpnNew = NULL;
	if (pens.size() >= 2) {
		hpnNew = pens[pens.size() - 2];
	}else {
		hpnNew = hpnOrg;
	}

	// 選択
	if (hpnNew) {
		SelectObject(hdc, hpnNew);
	}

	// 削除
	if (!pens.empty()) {
		DeleteObject(pens.back());
		pens.pop_back();
	}

	// オリジナル
	if (pens.empty()) {
		hpnOrg = NULL;
	}
}


void Graphics::ClearPen()
{
	if (hpnOrg) {
		SelectObject(hdc, hpnOrg);
		hpnOrg = NULL;
	}
	int nSize = (int)pens.size();
	for (int i=0; i<nSize; ++i) {
		DeleteObject(pens[i]);
	}
	pens.clear();
}

//$$note: 高速化
COLORREF Graphics::GetPenColor() const
{
	if (pens.size()) {
		LOGPEN logpen;
		if (GetObject(pens.back(), sizeof(logpen), &logpen)) {
			return logpen.lopnColor;
		}
	}else {
		return 0;
	}
	return 0;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ブラシ                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::_InitBrushColor()
{
	if (brushes.empty()) {
		// 元のブラシを取得
		HBRUSH hbrOrg = (HBRUSH)::SelectObject(hdc, ::GetStockObject(NULL_BRUSH));
		::SelectObject(hdc, hbrOrg); // 元に戻す
		// 保存
		brushes.push_back(hbrOrg);
	}
}

void Graphics::PushBrushColor(COLORREF color)
{
	//####ここで効率化できる

	_InitBrushColor();
	// 新しく作成→HDCに設定→スタックに保存
	HBRUSH hbrNew = (color != (COLORREF)-1) ? CreateSolidBrush(color) : (HBRUSH)GetStockObject(NULL_BRUSH);
	::SelectObject(hdc, hbrNew);
	brushes.push_back(hbrNew);
}

void Graphics::PopBrushColor()
{
	if (brushes.size() >= 2) {
		// 最後から2番目の要素をHDCに設定
		::SelectObject(hdc, brushes[brushes.size()-2]);
		// 最後の要素を削除
		::DeleteObject(brushes.back());
		brushes.pop_back();
	}
}

void Graphics::ClearBrush()
{
	// 元のブラシに戻す
	if (!brushes.empty()) {
		::SelectObject(hdc, brushes[0]);
	}
	// ブラシをすべて削除 (0番要素以外)
	int nSize = (int)brushes.size();
	for (int i=1; i<nSize; ++i) {
		::DeleteObject(brushes[i]);
	}
	brushes.resize(t_min(1, (int)brushes.size()));
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           直線                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


//$$note:高速化
void Graphics::DrawDotLine(int x1, int y1, int x2, int y2)
{
	COLORREF c = GetPenColor();
	int my = t_unit(y2 - y1) * 2;
	int mx = t_unit(x2 - x1) * 2;
	if (!mx && !my) return;
	int x = x1;
	int y = y1;
	if (!mx && !my) return;
	for (;;) {
		// 点描画
		ApiWrap::SetPixelSurely(hdc, x, y, c);

		// 進める
		x += mx;
		y += my;

		// 条件判定
		if (mx > 0 && x >= x2) break;
		if (mx < 0 && x <= x2) break;
		if (my > 0 && y >= y2) break;
		if (my < 0 && y <= y2) break;
	}
}

// ドロップ先矩形描画用のリージョンを作成する
static
HRGN CreateDropRectRgn(LPCRECT lpRect, SIZE size)
{
	HRGN hRgnOutside = ::CreateRectRgnIndirect(lpRect);
	RECT rc = *lpRect;
	::InflateRect(&rc, -size.cx, -size.cy);
	::IntersectRect(&rc, &rc, lpRect);
	HRGN hRgnInside = ::CreateRectRgnIndirect(&rc);
	HRGN hRgn = ::CreateRectRgn(0, 0, 0, 0);
	::CombineRgn(hRgn, hRgnOutside, hRgnInside, RGN_XOR);
	if (hRgnOutside) ::DeleteObject(hRgnOutside);
	if (hRgnInside) ::DeleteObject(hRgnInside);
	return hRgn;
}

// ドロップ先矩形描画用のブラシを取得する
static
HBRUSH GetDropRectBrush()
{
	static HBRUSH s_hBrush = NULL;
	if (!s_hBrush) {
		WORD wBits[8] = {0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA};
		HBITMAP hBitmap = ::CreateBitmap(8, 8, 1, 1, wBits);
		if (hBitmap) {
			s_hBrush = ::CreatePatternBrush(hBitmap);
			::DeleteObject(hBitmap);
			s_gdiStock.Register(s_hBrush);	// 終了時破棄用にストックしておく
		}
	}
	return s_hBrush;
}

// ドロップ先の矩形を描画する
void Graphics::DrawDropRect(
	LPCRECT lpRectNew,
	SIZE sizeNew,
	LPCRECT lpRectLast,
	SIZE sizeLast
	)
{
	if (!lpRectNew && !lpRectLast)
		return;

	HWND hwndDt = ::GetDesktopWindow();
	HDC hdc = ::GetDCEx(hwndDt, NULL, DCX_WINDOW | DCX_CACHE | DCX_LOCKWINDOWUPDATE);

	HRGN hRgnNew = NULL;
	HRGN hRgnUpdate = NULL;
	if (lpRectNew) {
		hRgnNew = CreateDropRectRgn(lpRectNew, sizeNew);
	}
	if (lpRectLast) {
		HRGN hRgnLast = CreateDropRectRgn(lpRectLast, sizeLast);
		if (lpRectNew) {
			hRgnUpdate = ::CreateRectRgn(0, 0, 0, 0);
			::CombineRgn(hRgnUpdate, hRgnLast, hRgnNew, RGN_XOR);
			::DeleteObject(hRgnLast);
		}else {
			hRgnUpdate = hRgnLast;
		}
	}

	RECT rc;
	::SelectClipRgn(hdc, hRgnUpdate? hRgnUpdate: hRgnNew);
	::GetClipBox(hdc, &rc);

	HBRUSH hBrush = GetDropRectBrush();
	HBRUSH hBrushOld = (HBRUSH)::SelectObject(hdc, hBrush);

	::PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATINVERT);

	::SelectObject(hdc, hBrushOld);
	::SelectClipRgn(hdc, NULL);

	if (hRgnNew) ::DeleteObject(hRgnNew);
	if (hRgnUpdate) ::DeleteObject(hRgnUpdate);

	::ReleaseDC(hwndDt, hdc);
}

