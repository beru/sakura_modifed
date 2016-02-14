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

#include <Windows.h> // POINT

class Point : public POINT {
public:
	// ƒRƒ“ƒXƒgƒ‰ƒNƒ^EƒfƒXƒgƒ‰ƒNƒ^
	Point() { x = 0; y = 0; }
	Point(int _x, int _y) { x = _x; y = _y; }
	Point(const POINT& rhs) { x = rhs.x; y = rhs.y; }

	// Zp‰‰Zq
	Point& operator += (const POINT& rhs) { x += rhs.x; y += rhs.y; return *this; }
	Point& operator -= (const POINT& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
	Point& operator *= (int n) { x *= n; y *= n; return *this; }
	Point& operator /= (int n) { x /= n; y /= n; return *this; }

	// Zp‰‰Zq‚Q
	Point operator + (const POINT& rhs) const { Point tmp = *this; tmp += rhs; return tmp; }
	Point operator - (const POINT& rhs) const { Point tmp = *this; tmp -= rhs; return tmp; }
	Point operator * (int n) const { Point tmp = *this; tmp *= n; return tmp; }
	Point operator / (int n) const { Point tmp = *this; tmp /= n; return tmp; }

	// ‘ã“ü‰‰Zq
	Point& operator = (const POINT& rhs) { x = rhs.x; y = rhs.y; return *this; }

	// ”äŠr‰‰Zq
	bool operator == (const POINT& rhs) const { return x == rhs.x && y == rhs.y; }
	bool operator != (const POINT& rhs) const { return !(this->operator == (rhs)); }

	// İ’è
	void Set(int _x, int _y) { x = _x; y = _y; }
	void Set(const Point& pt) { x = pt.x; y = pt.y; }
	void SetX(int _x) { x = _x; }
	void SetY(int _y) { y = _y; }
	void Offset(int _x, int _y) { x += _x; y += _y; }
	void Offset(const Point& pt) { x += pt.x; y += pt.y; }

	// æ“¾
	int GetX() const { return (int)x; }
	int GetY() const { return (int)y; }
	Point Get() const { return *this; }

	// x,y ‚¢‚¸‚ê‚©‚ª 0 ‚æ‚è¬‚³‚¢ê‡‚É true ‚ğ•Ô‚·
	bool HasNegative() const
	{
		return x < 0 || y < 0;
	}

	// x,y ‚Ç‚¿‚ç‚à©‘R”‚Å‚ ‚ê‚Î true
	bool BothNatural() const
	{
		return x >= 0 && y >= 0;
	}
};


/*!
	pt1 - pt2‚ÌŒ‹‰Ê‚ğ•Ô‚·
	Y‚ğ—Dæ‚µ‚Ä”äŠrBY‚ª“¯ˆê‚È‚çAX‚Å”äŠrB

	@return <0 pt1 <  pt2
	@return 0  pt1 == pt2
	@return >0 pt1 >  pt2
*/
template <class POINT_T>
inline int PointCompare(const POINT_T& pt1, const POINT_T& pt2)
{
	if (pt1.y != pt2.y) return (Int)(pt1.y - pt2.y);
	return (Int)(pt1.x - pt2.x);
}


// 2“_‚ğ‘ÎŠp‚Æ‚·‚é‹éŒ`‚ğ‹‚ß‚é
template <class POINT_T>
inline void TwoPointToRect(
	RECT*	pRect,
	POINT_T	pt1,
	POINT_T	pt2
)
{
	if (pt1.y < pt2.y) {
		pRect->top	= (Int)pt1.y;
		pRect->bottom	= (Int)pt2.y;
	}else {
		pRect->top	= (Int)pt2.y;
		pRect->bottom	= (Int)pt1.y;
	}
	if (pt1.x < pt2.x) {
		pRect->left	= (Int)pt1.x;
		pRect->right	= (Int)pt2.x;
	}else {
		pRect->left	= (Int)pt2.x;
		pRect->right	= (Int)pt1.x;
	}
}

