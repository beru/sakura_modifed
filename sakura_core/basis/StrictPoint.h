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

// 単位が明示的に区別されたポイント型。※POINTは継承しないことにした
/*
template <int TYPE>
class StrictPoint : public CMyPoint {
public:
	StrictPoint() : CMyPoint() { }
	StrictPoint(int _x, int _y) : CMyPoint(_x, _y) { }
	StrictPoint(const StrictPoint& rhs) : CMyPoint(rhs) { }

	// ※POINTからの変換は、「明示的に指定されたときのみ」許可する。
	explicit StrictPoint(const POINT& rhs) : CMyPoint(rhs) { }
};
*/
template <class SUPER, class INT_TYPE, class SUPER_INT_TYPE = INT_TYPE>
class StrictPoint : public SUPER {
private:
	typedef StrictPoint<SUPER, INT_TYPE> Me;
public:
	typedef INT_TYPE       IntType;
	typedef SUPER_INT_TYPE SuperIntType;
public:
	using SUPER::x;
	using SUPER::y;
	// コンストラクタ・デストラクタ
	StrictPoint() { x = SuperIntType(0); y = SuperIntType(0); }
	StrictPoint(int _x, int _y) { x = SuperIntType(_x); y = SuperIntType(_y); }
	StrictPoint(const SUPER& rhs) { x = rhs.x; y = rhs.y; }

	// 他の型からも、「明示的に指定すれば」変換が可能
	template <class SRC>
	explicit StrictPoint(const SRC& rhs) { x = (SuperIntType)rhs.x; y = (SuperIntType)rhs.y; }

	// 算術演算子
	StrictPoint& operator += (const SUPER& rhs) { x += rhs.x; y += rhs.y; return *this; }
	StrictPoint& operator -= (const SUPER& rhs) { x -= rhs.x; y -= rhs.y; return *this; }
	StrictPoint& operator *= (int n) { x *= n; y *= n; return *this; }
	StrictPoint& operator /= (int n) { x /= n; y /= n; return *this; }

	// 算術演算子２
	StrictPoint operator + (const SUPER& rhs) const { StrictPoint tmp = *this; tmp += rhs; return tmp; }
	StrictPoint operator - (const SUPER& rhs) const { StrictPoint tmp = *this; tmp -= rhs; return tmp; }
	StrictPoint operator * (int n) const { StrictPoint tmp = *this; tmp *= n; return tmp; }
	StrictPoint operator / (int n) const { StrictPoint tmp = *this; tmp /= n; return tmp; }

	// 代入演算子
	StrictPoint& operator = (const SUPER& rhs) { x = rhs.x; y = rhs.y; return *this; }

	// 比較演算子
	bool operator == (const SUPER& rhs) const { return x == rhs.x && y == rhs.y; }
	bool operator != (const SUPER& rhs) const { return !(this->operator == (rhs)); }

	// 設定
	void Clear() { x = SuperIntType(0); y = SuperIntType(0); }
	void Set(INT_TYPE _x, INT_TYPE _y) { x = SuperIntType(_x); y = SuperIntType(_y); }
	void Set(const SUPER& pt) { x = pt.x; y = pt.y; }
	void SetX(INT_TYPE _x) { x = SuperIntType(_x); }
	void SetY(INT_TYPE _y) { y = SuperIntType(_y); }
	void Offset(int _x, int _y) { x += _x; y += _y; }
	void Offset(const SUPER& pt) { x += pt.x; y += pt.y; }

	// 取得
	SuperIntType GetX() const { return x; }
	SuperIntType GetY() const { return y; }
	SUPER Get() const { return *this; }
	INT_TYPE GetX2() const { return INT_TYPE(x); }
	INT_TYPE GetY2() const { return INT_TYPE(y); }

	// x,y いずれかが 0 より小さい場合に true を返す
	bool HasNegative() const {
		return x < 0 || y < 0;
	}

	// x,y どちらも自然数であれば true
	bool BothNatural() const {
		return x >= 0 && y >= 0;
	}

	// 特殊
	POINT GetPOINT() const {
		POINT pt = {x, y};
		return pt;
	}
};

