#pragma once

#include <Windows.h>
#include <vector>

// オリジナル値保存クラス
template <class T>
class TOriginalHolder {
public:
	TOriginalHolder<T>() {
		data = 0;
		hold = false;
	}
	void Clear() {
		data = 0;
		hold = false;
	}
	void AssignOnce(const T& t) {
		if (!hold) {
			data = t;
			hold = true;
		}
	}
	const T& Get() const {
		return data;
	}
	bool HasData() const {
		return hold;
	}
private:
	T		data;
	bool	hold;
};


// フォント情報管理
struct Font {
	FontAttr	fontAttr;
	HFONT		hFont;      // フォントハンドル
};

// 描画管理
// 最新実装：ブラシ
class Graphics {
public:
	Graphics(const Graphics& rhs) { Init(rhs.hdc); }
	Graphics(HDC hdc = NULL) { Init(hdc); }
	~Graphics();
	void Init(HDC hdc);

	operator HDC() const { return hdc; }

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
		assert(!textForeColors.empty());
		return textForeColors.back();
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
		assert(!textBackColors.empty());
		return textBackColors.back();
	}

	// テキストモード
public:
	void SetTextBackTransparent(bool b) {
		nTextModeOrg.AssignOnce(::SetBkMode(hdc, b ? TRANSPARENT : OPAQUE));
	}

	// テキスト
public:
	void RestoreTextColors();

	// フォント
public:
	void PushMyFont(HFONT hFont) {
		Font font = { { false, false }, hFont };
		PushMyFont(font);
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
		assert(!fonts.empty());
		return  fonts.back().fontAttr.bBoldFont;
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
	HBRUSH GetCurrentBrush() const { return brushes.size() ? brushes.back() : NULL; }

	// 描画
public:
	// 直線
	void DrawLine(int x1, int y1, int x2, int y2) {
		::MoveToEx(hdc, x1, y1, NULL);
		::LineTo(hdc, x2, y2);
	}
	void DrawDotLine(int x1, int y1, int x2, int y2);	// 点線
	// 矩形塗り潰し
	void FillMyRect(const RECT& rc) {
		::FillRect(hdc, &rc, GetCurrentBrush());
#ifdef _DEBUG
		::SetPixel(hdc, -1, -1, 0); //###########実験
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
		::ExtTextOut(hdc, rc.left, rc.top, ETO_OPAQUE|ETO_CLIPPED, &rc, _T(""), 0, NULL);
	}

	static void DrawDropRect(LPCRECT lpRectNew, SIZE sizeNew, LPCRECT lpRectLast, SIZE sizeLast);	// ドロップ先の矩形を描画する

private:
	// 型
	typedef TOriginalHolder<COLORREF>	OrgColor;
	typedef TOriginalHolder<int>		OrgInt;
private:
	HDC					hdc;

	// クリッピング
	std::vector<HRGN>		clippingRgns;

	// テキスト
	std::vector<COLORREF>	textForeColors;
	std::vector<COLORREF>	textBackColors;
	std::vector<Font>		fonts;

	// テキスト
	OrgInt				nTextModeOrg;

	// ペン
	HPEN				hpnOrg;
	std::vector<HPEN>	pens;

	// ブラシ
	std::vector<HBRUSH>	brushes;
	HBRUSH				hbrOrg;
	HBRUSH				hbrCurrent;
	bool				bDynamicBrush;	// hbrCurrentを動的に作成した場合はtrue
};

