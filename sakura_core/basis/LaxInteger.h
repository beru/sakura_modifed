#pragma once

// 型チェックの緩い整数型
class LaxInteger {
private:
	typedef LaxInteger Me;

public:
	// コンストラクタ・デストラクタ
	LaxInteger() { value = 0; }
	LaxInteger(const Me& rhs) { value = rhs.value; }
	LaxInteger(int value) { this->value = value; }

	// 暗黙の変換
	operator const int&() const { return value; }
	operator       int&()       { return value; }

private:
	int value;
};

