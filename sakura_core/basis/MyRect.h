#pragma once

#include <Windows.h> // RECT
#include "MyPoint.h"

class Rect : public RECT {
public:
	// �R���X�g���N�^�E�f�X�g���N�^
	Rect() {
		SetLTRB(0, 0, 0, 0);
	}
	Rect(int l, int t, int r, int b) {
		SetLTRB(l, t, r, b);
	}
	Rect(const RECT& rc) {
		RECT* p = this;
		*p = rc;
	}

	// ���Z�q

	// ���
	void SetLTRB(int l, int t, int r, int b) {
		left	= l;
		top		= t;
		right	= r;
		bottom	= b;
	}
	void SetXYWH(int x, int y, int w, int h) {
		left   = x;
		top    = y;
		right  = x + w;
		bottom = y + h;
	}
	void SetPos(int x, int y) {
		int w = Width();
		int h = Height();
		left = x;
		top  = y;
		SetSize(w, h);
	}
	void SetSize(int w, int h) {
		right  = left + w;
		bottom = top  + h;
	}
	
	// �v�Z
	int Width() const {
		return right - left;
	}
	int Height() const {
		return bottom - top;
	}
	// ������W (TopLeft)
	Point UpperLeft() const {
		return Point(left, top);
	}
	// �E�����W (BottomRight)
	Point LowerRight() const {
		return Point(right, bottom);
	}

	// �q�b�g�`�F�b�N
	bool PtInRect(const Point& pt) const {
		return pt.x >= left && pt.x < right && pt.y >= top && pt.y < bottom;
	}

	Rect& Union(const Rect& rc1, const Rect& rc2) {
		this->left		= t_min(rc1.left,	rc2.left);
		this->top		= t_min(rc1.top,	rc2.top);
		this->right		= t_max(rc1.right,	rc2.right);
		this->bottom	= t_max(rc1.bottom,	rc2.bottom);
		return *this;
	}

};

// CRect�����Brc1,rc2���܂ލŏ��̋�`�𐶐�����B
Rect MergeRect(const Rect& rc1, const Rect& rc2);

