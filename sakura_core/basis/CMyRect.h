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

#include <Windows.h> // RECT
#include "CMyPoint.h"

class Rect : public RECT {
public:
	// コンストラクタ・デストラクタ
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

	// 演算子

	// 代入
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
	
	// 計算
	int Width() const {
		return right - left;
	}
	int Height() const {
		return bottom - top;
	}
	// 左上座標 (TopLeft)
	Point UpperLeft() const {
		return Point(left, top);
	}
	// 右下座標 (BottomRight)
	Point LowerRight() const {
		return Point(right, bottom);
	}

};

// CRect合成。rc1,rc2を含む最小の矩形を生成する。
Rect MergeRect(const Rect& rc1, const Rect& rc2);

