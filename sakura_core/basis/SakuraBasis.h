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
//                      １次元型の定義                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
#ifdef USE_STRICT_INT
	// -- -- 厳格なintで単位型を定義 -- -- //

	#include "CStrictInteger.h"

	// ロジック単位
	typedef StrictInteger <
		0,		// 型を分けるための数値。
		true,	// intとの比較を許すかどうか
		true,	// intとの加減算を許すかどうか
		true,	// intへの暗黙の変換を許すかどうか
		true	// intの代入を許すかどうか
	>
	LogicInt;

	// レイアウト単位
	typedef StrictInteger <
		1,		// 型を分けるための数値。
		true,	// intとの比較を許すかどうか
		true,	// intとの加減算を許すかどうか
		false,	// intへの暗黙の変換を許すかどうか
		true	// intの代入を許すかどうか
	>
	LayoutInt;

#else
	// -- -- 通常のintで単位型を定義

	// ロジック単位
	typedef int LogicInt;

	// レイアウト単位
	typedef int LayoutInt;

#endif

typedef LogicInt  LogicXInt;
typedef LogicInt  LogicYInt;
typedef LayoutInt LayoutXInt;
typedef LayoutInt LayoutYInt;
typedef int PixelYInt;
typedef int PixelXInt;

#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
typedef LayoutXInt HabaXInt;
typedef int        KetaXInt;
#else
typedef PixelXInt  HabaXInt;
typedef LayoutXInt KetaXInt;
#endif


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ２次元型の定義                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
#include "CStrictRange.h"
#include "CStrictPoint.h"
#include "CStrictRect.h"

// ロジック単位
struct LogicIntXY { LogicInt x; LogicInt y; }; // 基底構造体
typedef StrictPoint<LogicIntXY, LogicInt>	LogicPoint;
typedef RangeBase<LogicPoint>				LogicRange;
typedef StrictRect<LogicInt, LogicPoint>	LogicRect;

// レイアウト単位
struct LayoutIntXY { LayoutInt x; LayoutInt y; }; // 基底構造体
typedef StrictPoint<LayoutIntXY, LayoutInt>	LayoutPoint;
typedef RangeBase<LayoutPoint>				LayoutRange;
typedef StrictRect<LayoutInt, LayoutPoint>	LayoutRect;

// ゆるい単位
#include "CMyPoint.h"
typedef RangeBase<Point>     SelectionRange;



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ツール                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#include "CMyRect.h"

// 変換関数
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


// 2点を対角とする矩形を求める
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

