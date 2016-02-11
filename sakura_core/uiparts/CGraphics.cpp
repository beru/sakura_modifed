/*
2008.05.20 kobake 作成
*/

#include "StdAfx.h"
#include "CGraphics.h"
#include "util/std_macro.h"

class GDIStock {
public:
	GDIStock() {}
	~GDIStock() {
		while (!m_vObjects.empty()) {
			::DeleteObject(m_vObjects.back());
			m_vObjects.pop_back();
		}
	}
	bool Register(HGDIOBJ hObject) {
		if (hObject) {
			m_vObjects.push_back(hObject);
			return true;
		}
		return false;
	}
protected:
	std::vector<HGDIOBJ> m_vObjects;
};

static GDIStock s_cGDIStock;	// 唯一の GDIStock オブジェクト

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
	if (m_vClippingRgns.empty()) {
		// 元のクリッピング領域を取得
		RECT rcDummy = {0, 0, 1, 1};
		HRGN hrgnOrg = ::CreateRectRgnIndirect(&rcDummy);
		int nRet = ::GetClipRgn(m_hdc, hrgnOrg);
		if (nRet != 1) {
			::DeleteObject(hrgnOrg);
			hrgnOrg = NULL;
		}
		// 保存
		m_vClippingRgns.push_back(hrgnOrg);
	}
}

void Graphics::PushClipping(const RECT& rc)
{
	_InitClipping();
	// 新しく作成→HDCに設定→スタックに保存
	HRGN hrgnNew = CreateRectRgnIndirect(&rc);
	::SelectClipRgn(m_hdc, hrgnNew);
	m_vClippingRgns.push_back(hrgnNew);
}

void Graphics::PopClipping()
{
	if (m_vClippingRgns.size() >= 2) {
		// 最後の要素を削除
		::DeleteObject(m_vClippingRgns.back());
		m_vClippingRgns.pop_back();
		// この時点の最後の要素をHDCに設定
		::SelectClipRgn(m_hdc, m_vClippingRgns.back());
	}
}

void Graphics::ClearClipping()
{
	// 元のクリッピングに戻す
	if (!m_vClippingRgns.empty()) {
		::SelectClipRgn(m_hdc, m_vClippingRgns[0]);
	}
	// 領域をすべて削除
	int nSize = (int)m_vClippingRgns.size();
	for (int i=0; i<nSize; ++i) {
		::DeleteObject(m_vClippingRgns[i]);
	}
	m_vClippingRgns.clear();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      テキスト文字色                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::PushTextForeColor(COLORREF color)
{
	// 設定
	COLORREF cOld = ::SetTextColor(m_hdc, color);
	// 記録
	if (m_vTextForeColors.empty()) {
		m_vTextForeColors.push_back(cOld);
	}
	m_vTextForeColors.push_back(color);
}

void Graphics::PopTextForeColor()
{
	// 戻す
	if (m_vTextForeColors.size() >= 2) {
		m_vTextForeColors.pop_back();
		::SetTextColor(m_hdc, m_vTextForeColors.back());
	}
}

void Graphics::ClearTextForeColor()
{
	if (!m_vTextForeColors.empty()) {
		::SetTextColor(m_hdc, m_vTextForeColors[0]);
		m_vTextForeColors.clear();
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
	if (m_vTextBackColors.empty()) {
		m_vTextBackColors.push_back(cOld);
	}
	m_vTextBackColors.push_back(color);
}

void Graphics::PopTextBackColor()
{
	// 戻す
	if (m_vTextBackColors.size() >= 2) {
		m_vTextBackColors.pop_back();
		::SetBkColor(m_hdc, m_vTextBackColors.back());
	}
}

void Graphics::ClearTextBackColor()
{
	if (!m_vTextBackColors.empty()) {
		::SetBkColor(m_hdc, m_vTextBackColors[0]);
		m_vTextBackColors.clear();
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

void Graphics::PushMyFont(const Font& sFont)
{
	// 設定
	HFONT hFontOld = (HFONT)SelectObject(m_hdc, sFont.m_hFont);

	// 記録
	if (m_vFonts.empty()) {
		Font sFontOld = { { false, false }, hFontOld };
		m_vFonts.push_back(sFontOld);
	}
	m_vFonts.push_back(sFont);
}

void Graphics::PopMyFont()
{
	// 戻す
	if (m_vFonts.size() >= 2) {
		m_vFonts.pop_back();
		SelectObject(m_hdc, m_vFonts.back().m_hFont);
	}
}

void Graphics::ClearMyFont()
{
	if (!m_vFonts.empty()) {
		SelectObject(m_hdc, m_vFonts[0].m_hFont);
		m_vFonts.clear();
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
	if (m_vBrushes.empty()) {
		// 元のブラシを取得
		HBRUSH hbrOrg = (HBRUSH)::SelectObject(m_hdc, ::GetStockObject(NULL_BRUSH));
		::SelectObject(m_hdc, hbrOrg); // 元に戻す
		// 保存
		m_vBrushes.push_back(hbrOrg);
	}
}

void Graphics::PushBrushColor(COLORREF color)
{
	//####ここで効率化できる

	_InitBrushColor();
	// 新しく作成→HDCに設定→スタックに保存
	HBRUSH hbrNew = (color != (COLORREF)-1) ? CreateSolidBrush(color) : (HBRUSH)GetStockObject(NULL_BRUSH);
	::SelectObject(m_hdc, hbrNew);
	m_vBrushes.push_back(hbrNew);
}

void Graphics::PopBrushColor()
{
	if (m_vBrushes.size() >= 2) {
		// 最後から2番目の要素をHDCに設定
		::SelectObject(m_hdc, m_vBrushes[m_vBrushes.size()-2]);
		// 最後の要素を削除
		::DeleteObject(m_vBrushes.back());
		m_vBrushes.pop_back();
	}
}

void Graphics::ClearBrush()
{
	// 元のブラシに戻す
	if (!m_vBrushes.empty()) {
		::SelectObject(m_hdc, m_vBrushes[0]);
	}
	// ブラシをすべて削除 (0番要素以外)
	int nSize = (int)m_vBrushes.size();
	for (int i=1; i<nSize; ++i) {
		::DeleteObject(m_vBrushes[i]);
	}
	m_vBrushes.resize(t_min(1, (int)m_vBrushes.size()));
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
			s_cGDIStock.Register(s_hBrush);	// 終了時破棄用にストックしておく
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

