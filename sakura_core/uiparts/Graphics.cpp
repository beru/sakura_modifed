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

static GDIStock s_gdiStock;	// �B��� GDIStock �I�u�W�F�N�g

void Graphics::Init(HDC hdc)
{
	this->hdc = hdc;
	// �y��
	hpnOrg = NULL;
	// �u���V
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
//                       �N���b�s���O                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


void Graphics::_InitClipping()
{
	if (clippingRgns.empty()) {
		// ���̃N���b�s���O�̈���擾
		RECT rcDummy = {0, 0, 1, 1};
		HRGN hrgnOrg = ::CreateRectRgnIndirect(&rcDummy);
		int nRet = ::GetClipRgn(hdc, hrgnOrg);
		if (nRet != 1) {
			::DeleteObject(hrgnOrg);
			hrgnOrg = NULL;
		}
		// �ۑ�
		clippingRgns.push_back(hrgnOrg);
	}
}

void Graphics::PushClipping(const RECT& rc)
{
	_InitClipping();
	// �V�����쐬��HDC�ɐݒ聨�X�^�b�N�ɕۑ�
	HRGN hrgnNew = CreateRectRgnIndirect(&rc);
	::SelectClipRgn(hdc, hrgnNew);
	clippingRgns.push_back(hrgnNew);
}

void Graphics::PopClipping()
{
	if (clippingRgns.size() >= 2) {
		// �Ō�̗v�f���폜
		::DeleteObject(clippingRgns.back());
		clippingRgns.pop_back();
		// ���̎��_�̍Ō�̗v�f��HDC�ɐݒ�
		::SelectClipRgn(hdc, clippingRgns.back());
	}
}

void Graphics::ClearClipping()
{
	// ���̃N���b�s���O�ɖ߂�
	if (!clippingRgns.empty()) {
		::SelectClipRgn(hdc, clippingRgns[0]);
	}
	// �̈�����ׂč폜
	int nSize = (int)clippingRgns.size();
	for (int i=0; i<nSize; ++i) {
		::DeleteObject(clippingRgns[i]);
	}
	clippingRgns.clear();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �e�L�X�g�����F                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::PushTextForeColor(COLORREF color)
{
	// �ݒ�
	COLORREF cOld = ::SetTextColor(hdc, color);
	// �L�^
	if (textForeColors.empty()) {
		textForeColors.push_back(cOld);
	}
	textForeColors.push_back(color);
}

void Graphics::PopTextForeColor()
{
	// �߂�
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
//                      �e�L�X�g�w�i�F                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::PushTextBackColor(COLORREF color)
{
	// �ݒ�
	COLORREF cOld = ::SetBkColor(hdc, color);
	// �L�^
	if (textBackColors.empty()) {
		textBackColors.push_back(cOld);
	}
	textBackColors.push_back(color);
}

void Graphics::PopTextBackColor()
{
	// �߂�
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
//                         �e�L�X�g                            //
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
//                         �t�H���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::PushMyFont(const Font& font)
{
	// �ݒ�
	HFONT hFontOld = (HFONT)SelectObject(hdc, font.hFont);

	// �L�^
	if (fonts.empty()) {
		Font fontOld = { { false, false }, hFontOld };
		fonts.push_back(fontOld);
	}
	fonts.push_back(font);
}

void Graphics::PopMyFont()
{
	// �߂�
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
//                           �y��                              //
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
	// �I��������
	HPEN hpnNew = NULL;
	if (pens.size() >= 2) {
		hpnNew = pens[pens.size() - 2];
	}else {
		hpnNew = hpnOrg;
	}

	// �I��
	if (hpnNew) {
		SelectObject(hdc, hpnNew);
	}

	// �폜
	if (!pens.empty()) {
		DeleteObject(pens.back());
		pens.pop_back();
	}

	// �I���W�i��
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

//$$note: ������
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
//                          �u���V                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void Graphics::_InitBrushColor()
{
	if (brushes.empty()) {
		// ���̃u���V���擾
		HBRUSH hbrOrg = (HBRUSH)::SelectObject(hdc, ::GetStockObject(NULL_BRUSH));
		::SelectObject(hdc, hbrOrg); // ���ɖ߂�
		// �ۑ�
		brushes.push_back(hbrOrg);
	}
}

void Graphics::PushBrushColor(COLORREF color)
{
	//####�����Ō������ł���

	_InitBrushColor();
	// �V�����쐬��HDC�ɐݒ聨�X�^�b�N�ɕۑ�
	HBRUSH hbrNew = (color != (COLORREF)-1) ? CreateSolidBrush(color) : (HBRUSH)GetStockObject(NULL_BRUSH);
	::SelectObject(hdc, hbrNew);
	brushes.push_back(hbrNew);
}

void Graphics::PopBrushColor()
{
	if (brushes.size() >= 2) {
		// �Ōォ��2�Ԗڂ̗v�f��HDC�ɐݒ�
		::SelectObject(hdc, brushes[brushes.size()-2]);
		// �Ō�̗v�f���폜
		::DeleteObject(brushes.back());
		brushes.pop_back();
	}
}

void Graphics::ClearBrush()
{
	// ���̃u���V�ɖ߂�
	if (!brushes.empty()) {
		::SelectObject(hdc, brushes[0]);
	}
	// �u���V�����ׂč폜 (0�ԗv�f�ȊO)
	int nSize = (int)brushes.size();
	for (int i=1; i<nSize; ++i) {
		::DeleteObject(brushes[i]);
	}
	brushes.resize(t_min(1, (int)brushes.size()));
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


//$$note:������
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
		// �_�`��
		ApiWrap::SetPixelSurely(hdc, x, y, c);

		// �i�߂�
		x += mx;
		y += my;

		// ��������
		if (mx > 0 && x >= x2) break;
		if (mx < 0 && x <= x2) break;
		if (my > 0 && y >= y2) break;
		if (my < 0 && y <= y2) break;
	}
}

// �h���b�v���`�`��p�̃��[�W�������쐬����
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

// �h���b�v���`�`��p�̃u���V���擾����
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
			s_gdiStock.Register(s_hBrush);	// �I�����j���p�ɃX�g�b�N���Ă���
		}
	}
	return s_hBrush;
}

// �h���b�v��̋�`��`�悷��
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

