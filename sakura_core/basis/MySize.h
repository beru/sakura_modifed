#pragma once

#include <Windows.h> // SIZE

class Size : public SIZE {
public:
	// コンストラクタ・デストラクタ
	Size() {} // ※初期化なし
	Size(int _cx, int _cy) { cx = _cx; cy = _cy; }
	Size(const SIZE& rhs) { cx = rhs.cx; cy = rhs.cy; }

	// 関数
	void Set(int _cx, int _cy) { cx = _cx; cy = _cy; }

	// 演算子
	bool operator == (const SIZE& rhs) const { return cx == rhs.cx && cy == rhs.cy; }
	bool operator != (const SIZE& rhs) const { return !(operator == (rhs)); }
};

