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
//                      �Q�����^�̒�`                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
#include "StrictRange.h"
#include "StrictPoint.h"
#include "StrictRect.h"

// ���W�b�N�P��
struct LogicIntXY { int x; int y; }; // ���\����
typedef StrictPoint<LogicIntXY, int>	LogicPoint;
typedef RangeBase<LogicPoint>			LogicRange;
typedef StrictRect<int, LogicPoint>		LogicRect;

// ���C�A�E�g�P��
struct LayoutIntXY { int x; int y; }; // ���\����
typedef StrictPoint<LayoutIntXY, int>	LayoutPoint;
typedef RangeBase<LayoutPoint>			LayoutRange;
typedef StrictRect<int, LayoutPoint>	LayoutRect;

// ��邢�P��
#include "MyPoint.h"
typedef RangeBase<Point>     SelectionRange;



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �c�[��                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#include "MyRect.h"

// �ϊ��֐�
template <class POINT_T>
inline void TwoPointToRange(
	RangeBase<POINT_T>* prangeDst,
	POINT_T pt1,
	POINT_T pt2
	)
{
	Rect rc;
	TwoPointToRect(&rc, pt1, pt2);
	prangeDst->SetFrom(POINT_T(rc.UpperLeft()));
	prangeDst->SetTo(POINT_T(rc.LowerRight()));
}


// 2�_��Ίp�Ƃ����`�����߂�
template <class T, class INT_TYPE>
inline void TwoPointToRect(
	StrictRect<INT_TYPE, StrictPoint<T, INT_TYPE>>*	pRect,
	StrictPoint<T, INT_TYPE>							pt1,
	StrictPoint<T, INT_TYPE>							pt2
	)
{
	if (pt1.y < pt2.y) {
		pRect->top	= pt1.GetY2();
		pRect->bottom	= pt2.GetY2();
	}else {
		pRect->top	= pt2.GetY2();
		pRect->bottom	= pt1.GetY2();
	}
	if (pt1.x < pt2.x) {
		pRect->left	= pt1.GetX2();
		pRect->right	= pt2.GetX2();
	}else {
		pRect->left	= pt2.GetX2();
		pRect->right	= pt1.GetX2();
	}
}

