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
	void Clear() { x = y = 0; }

	// æ“¾
	int GetX() const { return x; }
	int GetY() const { return y; }
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
inline int PointCompare(const Point& pt1, const Point& pt2)
{
	if (pt1.y != pt2.y) return pt1.y - pt2.y;
	return pt1.x - pt2.x;
}

