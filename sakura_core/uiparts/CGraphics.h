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
#pragma once

/*
2008.05.20 kobake 作成
*/

#include <Windows.h>
#include <vector>

// オリジナル値保存クラス
template <class T>
class TOriginalHolder {
public:
	TOriginalHolder<T>() {
		m_data = 0;
		m_hold = false;
	}
	void Clear() {
		m_data = 0;
		m_hold = false;
	}
	void AssignOnce(const T& t) {
		if (!m_hold) {
			m_data = t;
			m_hold = true;
		}
	}
	const T& Get() const {
		return m_data;
	}
	bool HasData() const {
		return m_hold;
	}
private:
	T		m_data;
	bool	m_hold;
};


// フォント情報管理
struct Font {
	FontAttr	m_fontAttr;
	HFONT		m_hFont;      // フォントハンドル
};

// 描画管理
// 最新実装：ブラシ
class Graphics {
public:
	Graphics(const Graphics& rhs) { Init(rhs.m_hdc); }
	Graphics(HDC hdc = NULL) { Init(hdc); }
	~Graphics();
	void Init(HDC hdc);

	operator HDC() const { return m_hdc; }

	// クリッピング
private:
	void _InitClipping();
public:
	void PushClipping(const RECT& rc);
	void PopClipping();
	void ClearClipping();
	void SetClipping(const RECT& rc) {
		ClearClipping();
		PushClipping(rc);
	}

	// テキスト文字色
public:
	void PushTextForeColor(COLORREF color);
	void PopTextForeColor();
	void ClearTextForeColor();
	void SetTextForeColor(COLORREF color) {
		ClearTextForeColor();
		PushTextForeColor(color);
	}
	COLORREF GetCurrentTextForeColor() {
		assert(!m_vTextForeColors.empty());
		return m_vTextForeColors.back();
	}

	// テキスト背景色
public:
	void PushTextBackColor(COLORREF color);
	void PopTextBackColor();
	void ClearTextBackColor();
	void SetTextBackColor(COLORREF color) {
		ClearTextBackColor();
		PushTextBackColor(color);
	}
	COLORREF GetTextBackColor() {
		assert(!m_vTextBackColors.empty());
		return m_vTextBackColors.back();
	}

	// テキストモード
public:
	void SetTextBackTransparent(bool b) {
		m_nTextModeOrg.AssignOnce(::SetBkMode(m_hdc, b ? TRANSPARENT : OPAQUE));
	}

	// テキスト
public:
	void RestoreTextColors();

	// フォント
public:
	void PushMyFont(HFONT hFont) {
		Font sFont = { { false, false }, hFont };
		PushMyFont(sFont);
	}
	void PushMyFont(const Font& font);
	void PopMyFont();
	void ClearMyFont();
	// フォント設定
	void SetMyFont(const Font& font) {
		ClearMyFont();
		PushMyFont(font);
	}
	bool GetCurrentMyFontBold() {
		assert(!m_vFonts.empty());
		return  m_vFonts.back().m_fontAttr.m_bBoldFont;
	}

	// ペン
public:
	void PushPen(COLORREF color, int nPenWidth, int nStyle = PS_SOLID);
	void PopPen();
	void SetPen(COLORREF color) {
		ClearPen();
		PushPen(color, 1);
	}
	void ClearPen();
	COLORREF GetPenColor() const;

	// ブラシ
public:
	void _InitBrushColor();
	void PushBrushColor(
		COLORREF color	// ブラシの色。(COLORREF)-1 にすると、透明ブラシとなる。
	);
	void PopBrushColor();
	void ClearBrush();

	void SetBrushColor(COLORREF color) {
		ClearBrush();
		PushBrushColor(color);
	}
	HBRUSH GetCurrentBrush() const { return m_vBrushes.size() ? m_vBrushes.back() : NULL; }

	// 描画
public:
	// 直線
	void DrawLine(int x1, int y1, int x2, int y2) {
		::MoveToEx(m_hdc, x1, y1, NULL);
		::LineTo(m_hdc, x2, y2);
	}
	void DrawDotLine(int x1, int y1, int x2, int y2);	// 点線
	// 矩形塗り潰し
	void FillMyRect(const RECT& rc) {
		::FillRect(m_hdc, &rc, GetCurrentBrush());
#ifdef _DEBUG
		::SetPixel(m_hdc, -1, -1, 0); //###########実験
#endif
	}
	// 矩形塗り潰し
	void FillSolidMyRect(const RECT& rc, COLORREF color) {
		PushTextBackColor(color);
		FillMyRectTextBackColor(rc);
		PopTextBackColor();
	}
	// 矩形塗り潰し
	void FillMyRectTextBackColor(const RECT& rc) {
		::ExtTextOut(m_hdc, rc.left, rc.top, ETO_OPAQUE|ETO_CLIPPED, &rc, _T(""), 0, NULL);
	}

	static void DrawDropRect(LPCRECT lpRectNew, SIZE sizeNew, LPCRECT lpRectLast, SIZE sizeLast);	// ドロップ先の矩形を描画する

private:
	// 型
	typedef TOriginalHolder<COLORREF>	OrgColor;
	typedef TOriginalHolder<int>		OrgInt;
private:
	HDC					m_hdc;

	// クリッピング
	std::vector<HRGN>		m_vClippingRgns;

	// テキスト
	std::vector<COLORREF>	m_vTextForeColors;
	std::vector<COLORREF>	m_vTextBackColors;
	std::vector<Font>		m_vFonts;

	// テキスト
	OrgInt				m_nTextModeOrg;

	// ペン
	HPEN				m_hpnOrg;
	std::vector<HPEN>	m_vPens;

	// ブラシ
	std::vector<HBRUSH>	m_vBrushes;
	HBRUSH				m_hbrOrg;
	HBRUSH				m_hbrCurrent;
	bool				m_bDynamicBrush;	// m_hbrCurrentを動的に作成した場合はtrue
};

