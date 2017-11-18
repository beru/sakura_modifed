#pragma once

#include <Windows.h>
#include <vector>

// �I���W�i���l�ۑ��N���X
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


// �t�H���g���Ǘ�
struct Font {
	FontAttr	fontAttr;
	HFONT		hFont;      // �t�H���g�n���h��
};

// �`��Ǘ�
// �ŐV�����F�u���V
class Graphics {
public:
	Graphics(const Graphics& rhs) { Init(rhs.hdc); }
	Graphics(HDC hdc = NULL) { Init(hdc); }
	~Graphics();
	void Init(HDC hdc);

	operator HDC() const { return hdc; }

	// �N���b�s���O
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

	// �e�L�X�g�����F
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

	// �e�L�X�g�w�i�F
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

	// �e�L�X�g���[�h
public:
	void SetTextBackTransparent(bool b) {
		nTextModeOrg.AssignOnce(::SetBkMode(hdc, b ? TRANSPARENT : OPAQUE));
	}

	// �e�L�X�g
public:
	void RestoreTextColors();

	// �t�H���g
public:
	void PushMyFont(HFONT hFont) {
		Font font = { { false, false }, hFont };
		PushMyFont(font);
	}
	void PushMyFont(const Font& font);
	void PopMyFont();
	void ClearMyFont();
	// �t�H���g�ݒ�
	void SetMyFont(const Font& font) {
		ClearMyFont();
		PushMyFont(font);
	}
	bool GetCurrentMyFontBold() {
		assert(!fonts.empty());
		return  fonts.back().fontAttr.bBoldFont;
	}

	// �y��
public:
	void PushPen(COLORREF color, int nPenWidth, int nStyle = PS_SOLID);
	void PopPen();
	void SetPen(COLORREF color) {
		ClearPen();
		PushPen(color, 1);
	}
	void ClearPen();
	COLORREF GetPenColor() const;

	// �u���V
public:
	void _InitBrushColor();
	void PushBrushColor(
		COLORREF color	// �u���V�̐F�B(COLORREF)-1 �ɂ���ƁA�����u���V�ƂȂ�B
	);
	void PopBrushColor();
	void ClearBrush();

	void SetBrushColor(COLORREF color) {
		ClearBrush();
		PushBrushColor(color);
	}
	HBRUSH GetCurrentBrush() const { return brushes.size() ? brushes.back() : NULL; }

	// �`��
public:
	// ����
	void DrawLine(int x1, int y1, int x2, int y2) {
		::MoveToEx(hdc, x1, y1, NULL);
		::LineTo(hdc, x2, y2);
	}
	void DrawDotLine(int x1, int y1, int x2, int y2);	// �_��
	// ��`�h��ׂ�
	void FillMyRect(const RECT& rc) {
		::FillRect(hdc, &rc, GetCurrentBrush());
#ifdef _DEBUG
		::SetPixel(hdc, -1, -1, 0); //###########����
#endif
	}
	// ��`�h��ׂ�
	void FillSolidMyRect(const RECT& rc, COLORREF color) {
		PushTextBackColor(color);
		FillMyRectTextBackColor(rc);
		PopTextBackColor();
	}
	// ��`�h��ׂ�
	void FillMyRectTextBackColor(const RECT& rc) {
		::ExtTextOut(hdc, rc.left, rc.top, ETO_OPAQUE|ETO_CLIPPED, &rc, _T(""), 0, NULL);
	}

	static void DrawDropRect(LPCRECT lpRectNew, SIZE sizeNew, LPCRECT lpRectLast, SIZE sizeLast);	// �h���b�v��̋�`��`�悷��

private:
	// �^
	typedef TOriginalHolder<COLORREF>	OrgColor;
	typedef TOriginalHolder<int>		OrgInt;
private:
	HDC					hdc;

	// �N���b�s���O
	std::vector<HRGN>		clippingRgns;

	// �e�L�X�g
	std::vector<COLORREF>	textForeColors;
	std::vector<COLORREF>	textBackColors;
	std::vector<Font>		fonts;

	// �e�L�X�g
	OrgInt				nTextModeOrg;

	// �y��
	HPEN				hpnOrg;
	std::vector<HPEN>	pens;

	// �u���V
	std::vector<HBRUSH>	brushes;
	HBRUSH				hbrOrg;
	HBRUSH				hbrCurrent;
	bool				bDynamicBrush;	// hbrCurrent�𓮓I�ɍ쐬�����ꍇ��true
};

