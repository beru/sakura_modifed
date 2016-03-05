/*
2008.05.20 kobake 作成
*/

#include "StdAfx.h"
#include "Graphics.h"
#include "util/std_macro.h"

class GDIStock {
public:
	GDIStock() {}
	~GDIStock() {
		while (!m_objects.empty()) {
			::DeleteObject(m_objects.back());
			m_objects.pop_back();
		}
	}
	bool Register(HGDIOBJ hObject) {
		if (hObject) {
			m_objects.push_back(hObject);
			return true;
		}
		return false;
	}
protected:
	std::vector<HGDIOBJ> m_objects;
};

static GDIStock s_gdiStock;	// 唯一の GDIStock オブジェクト

void Graphics::Init(HDC hdc)
{
	m_hdc = hdc;
	// ペン
	m_hpnOrg = NULL;
	// ブラシ
	m_hbrOrg = NULL;
	m_hbrCurrent = NULL;
	m_bDynamicBrush = false;
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
	if (m_clippingRgns.empty()) {
		// 元のクリッピング領域を取得
		RECT rcDummy = {0, 0, 1, 1};
		HRGN hrgnOrg = ::CreateRectRgnIndirect(&rcDummy);
		int nRet = ::GetClipRgn(m_hdc, hrgnOrg);
		if (nRet != 1) {
			::DeleteObject(hrgnOrg);
			hrgnOrg = NULL;
		}
		// 保存
		m_clippingRgns.push_back(hrgnOrg);
	}
}

void Graphics::PushClipping(const RECT& rc)
{
	_InitClipping();
	// 新しく作成→HDCに設定→スタックに保存
	HRGN hrgnNew = CreateRectRgnIndirect(&rc);
	::SelectClipRgn(m_hdc, hrgnNew);
	m_clippingRgns.push_back(hrgnNew);
}

void Graphics::PopClipping()
{
	if (m_clippingRgns.size() >= 2) {
		// 最後の要素を削除
		::DeleteObject(m_clippingRgns.back());
		m_clippingRgns.pop_back();
		// この時点の最後の要素をHDCに設定
		::SelectClipRgn(m_hdc, m_clippingRgns.back());
	}
}

void Graphics::ClearClipping()
{
	// 元のクリッピングに戻す
	if (!m_clippingRgns.empty()) {
		::SelectClipRgn(m_hdc, m_clippingRgns[0]);
	}
	// 領域をすべて削除
	int nSize = (int)m_clippingRgns.size();
	for (int i=0; i<nSize; ++i) {
		::DeleteObject(m_clippingRgns[i]);
	}
	m_clippingRgns.clear();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      テキスト文字色                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::PushTextForeColor(COLORREF color)
{
	// 設定
	COLORREF cOld = ::SetTextColor(m_hdc, color);
	// 記録
	if (m_textForeColors.empty()) {
		m_textForeColors.push_back(cOld);
	}
	m_textForeColors.push_back(color);
}

void Graphics::PopTextForeColor()
{
	// 戻す
	if (m_textForeColors.size() >= 2) {
		m_textForeColors.pop_back();
		::SetTextColor(m_hdc, m_textForeColors.back());
	}
}

void Graphics::ClearTextForeColor()
{
	if (!m_textForeColors.empty()) {
		::SetTextColor(m_hdc, m_textForeColors[0]);
		m_textForeColors.clear();
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      テキスト背景色                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::PushTextBackColor(COLORREF color)
{
	// 設定
	COLORREF cOld = ::SetBkColor(m_hdc, color);
	// 記録
	if (m_textBackColors.empty()) {
		m_textBackColors.push_back(cOld);
	}
	m_textBackColors.push_back(color);
}

void Graphics::PopTextBackColor()
{
	// 戻す
	if (m_textBackColors.size() >= 2) {
		m_textBackColors.pop_back();
		::SetBkColor(m_hdc, m_textBackColors.back());
	}
}

void Graphics::ClearTextBackColor()
{
	if (!m_textBackColors.empty()) {
		::SetBkColor(m_hdc, m_textBackColors[0]);
		m_textBackColors.clear();
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         テキスト                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::RestoreTextColors()
{
	PopTextForeColor();
	PopTextBackColor();
	if (m_nTextModeOrg.HasData()) {
		::SetBkMode(m_hdc, m_nTextModeOrg.Get());
		m_nTextModeOrg.Clear();
	}
}
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         フォント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::PushMyFont(const Font& font)
{
	// 設定
	HFONT hFontOld = (HFONT)SelectObject(m_hdc, font.hFont);

	// 記録
	if (m_fonts.empty()) {
		Font fontOld = { { false, false }, hFontOld };
		m_fonts.push_back(fontOld);
	}
	m_fonts.push_back(font);
}

void Graphics::PopMyFont()
{
	// 戻す
	if (m_fonts.size() >= 2) {
		m_fonts.pop_back();
		SelectObject(m_hdc, m_fonts.back().hFont);
	}
}

void Graphics::ClearMyFont()
{
	if (!m_fonts.empty()) {
		SelectObject(m_hdc, m_fonts[0].hFont);
		m_fonts.clear();
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ペン                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::PushPen(COLORREF color, int nPenWidth, int nStyle)
{
	HPEN hpnNew = CreatePen(nStyle, nPenWidth, color);
	HPEN hpnOld = (HPEN)SelectObject(m_hdc, hpnNew);
	m_vPens.push_back(hpnNew);
	if (!m_hpnOrg) {
		m_hpnOrg = hpnOld;
	}
}

void Graphics::PopPen()
{
	// 選択する候補
	HPEN hpnNew = NULL;
	if (m_vPens.size() >= 2) {
		hpnNew = m_vPens[m_vPens.size() - 2];
	}else {
		hpnNew = m_hpnOrg;
	}

	// 選択
	if (hpnNew) {
		SelectObject(m_hdc, hpnNew);
	}

	// 削除
	if (!m_vPens.empty()) {
		DeleteObject(m_vPens.back());
		m_vPens.pop_back();
	}

	// オリジナル
	if (m_vPens.empty()) {
		m_hpnOrg = NULL;
	}
}


void Graphics::ClearPen()
{
	if (m_hpnOrg) {
		SelectObject(m_hdc, m_hpnOrg);
		m_hpnOrg = NULL;
	}
	int nSize = (int)m_vPens.size();
	for (int i=0; i<nSize; ++i) {
		DeleteObject(m_vPens[i]);
	}
	m_vPens.clear();
}

//$$note: 高速化
COLORREF Graphics::GetPenColor() const
{
	if (m_vPens.size()) {
		LOGPEN logpen;
		if (GetObject(m_vPens.back(), sizeof(logpen), &logpen)) {
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
	if (m_brushes.empty()) {
		// 元のブラシを取得
		HBRUSH hbrOrg = (HBRUSH)::SelectObject(m_hdc, ::GetStockObject(NULL_BRUSH));
		::SelectObject(m_hdc, hbrOrg); // 元に戻す
		// 保存
		m_brushes.push_back(hbrOrg);
	}
}

void Graphics::PushBrushColor(COLORREF color)
{
	//####ここで効率化できる

	_InitBrushColor();
	// 新しく作成→HDCに設定→スタックに保存
	HBRUSH hbrNew = (color != (COLORREF)-1) ? CreateSolidBrush(color) : (HBRUSH)GetStockObject(NULL_BRUSH);
	::SelectObject(m_hdc, hbrNew);
	m_brushes.push_back(hbrNew);
}

void Graphics::PopBrushColor()
{
	if (m_brushes.size() >= 2) {
		// 最後から2番目の要素をHDCに設定
		::SelectObject(m_hdc, m_brushes[m_brushes.size()-2]);
		// 最後の要素を削除
		::DeleteObject(m_brushes.back());
		m_brushes.pop_back();
	}
}

void Graphics::ClearBrush()
{
	// 元のブラシに戻す
	if (!m_brushes.empty()) {
		::SelectObject(m_hdc, m_brushes[0]);
	}
	// ブラシをすべて削除 (0番要素以外)
	int nSize = (int)m_brushes.size();
	for (int i=1; i<nSize; ++i) {
		::DeleteObject(m_brushes[i]);
	}
	m_brushes.resize(t_min(1, (int)m_brushes.size()));
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
		ApiWrap::SetPixelSurely(m_hdc, x, y, c);

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

