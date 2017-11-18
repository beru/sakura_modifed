#pragma once

#include <Windows.h> // POINT, LONG

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �Q�����^�̒�`                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
#include "MyPoint.h"
#include "MyRange.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �c�[��                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#include "MyRect.h"

// 2�_��Ίp�Ƃ����`�����߂�
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

// �ϊ��֐�
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
