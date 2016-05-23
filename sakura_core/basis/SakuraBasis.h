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

#include <Windows.h> // POINT, LONG

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ２次元型の定義                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
#include "MyPoint.h"
#include "MyRange.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ツール                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#include "MyRect.h"

// 2点を対角とする矩形を求める
inline void TwoPointToRect(
	RECT* pRect,
	POINT pt1,
	POINT pt2
	)
{
	if (pt1.y < pt2.y) {
		pRect->top	= pt1.y;
		pRect->bottom	= pt2.y;
	}else {
		pRect->top	= pt2.y;
		pRect->bottom	= pt1.y;
	}
	if (pt1.x < pt2.x) {
		pRect->left	= pt1.x;
		pRect->right	= pt2.x;
	}else {
		pRect->left	= pt2.x;
		pRect->right	= pt1.x;
	}
}

// 変換関数
inline void TwoPointToRange(
	Range* prangeDst,
	Point pt1,
	Point pt2
)
{
	Rect rc;
	TwoPointToRect(&rc, pt1, pt2);
	prangeDst->SetFrom(rc.UpperLeft());
	prangeDst->SetTo(rc.LowerRight());
}
