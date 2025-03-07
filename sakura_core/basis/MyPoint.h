#pragma once

#include <Windows.h> // POINT

class Point : public POINT {
public:
	// コンストラクタ・デストラクタ
	Point() { x = 0; y = 0; }
	Point(int _x, int _y) { x = _x; y = _y; }
	Point(const POINT& rhs) { x = rhs.x; y = rhs.y; }

	// 算術演算子
	Point& operator += (const POINT& rhs) { x += rhs.x; y += rhs.y; return *this; }
	Point& operator -= (const POINT& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
	Point& operator *= (int n) { x *= n; y *= n; return *this; }
	Point& operator /= (int n) { x /= n; y /= n; return *this; }

	// 算術演算子２
	Point operator + (const POINT& rhs) const { Point tmp = *this; tmp += rhs; return tmp; }
	Point operator - (const POINT& rhs) const { Point tmp = *this; tmp -= rhs; return tmp; }
	Point operator * (int n) const { Point tmp = *this; tmp *= n; return tmp; }
	Point operator / (int n) const { Point tmp = *this; tmp /= n; return tmp; }

	// 代入演算子
	Point& operator = (const POINT& rhs) { x = rhs.x; y = rhs.y; return *this; }

	// 比較演算子
	bool operator == (const POINT& rhs) const { return x == rhs.x && y == rhs.y; }
	bool operator != (const POINT& rhs) const { return !(this->operator == (rhs)); }

	// 設定
	void Set(int _x, int _y) { x = _x; y = _y; }
	void Set(const Point& pt) { x = pt.x; y = pt.y; }
	void SetX(int _x) { x = _x; }
	void SetY(int _y) { y = _y; }
	void Offset(int _x, int _y) { x += _x; y += _y; }
	void Offset(const Point& pt) { x += pt.x; y += pt.y; }
	void Clear() { x = y = 0; }

	// 取得
	int GetX() const { return x; }
	int GetY() const { return y; }
	Point Get() const { return *this; }

	// x,y いずれかが 0 より小さい場合に true を返す
	bool HasNegative() const
	{
		return x < 0 || y < 0;
	}

	// x,y どちらも自然数であれば true
	bool BothNatural() const
	{
		return x >= 0 && y >= 0;
	}
};


/*!
	pt1 - pt2の結果を返す
	Yを優先して比較。Yが同一なら、Xで比較。

	@return <0 pt1 <  pt2
	@return 0  pt1 == pt2
	@return >0 pt1 >  pt2
*/
inline int PointCompare(const Point& pt1, const Point& pt2)
{
	if (pt1.y != pt2.y) return pt1.y - pt2.y;
	return pt1.x - pt2.x;
}

