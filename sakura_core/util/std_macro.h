#pragma once

#define SAFE_DELETE(p) { delete p; p = 0; }


/*
	テンプレート式 min とか max とか。

	どっかの標準ヘッダに、同じようなものがあった気がするけど、
	NOMINMAX を定義するにしても、なんだか min とか max とかいう名前だと、
	テンプレートを呼んでるんだかマクロを呼んでるんだか訳分かんないので、
	明示的に「t_～」という名前を持つ関数を用意。
*/

template <class T>
inline T t_min(T t1, T t2) {
	return t1 < t2 ? t1 : t2;
}

template <class T>
inline T t_max(T t1, T t2) {
	return t1 > t2 ? t1 : t2;
}

template <class T>
T t_abs(T t) {
	return t >= T(0) ? t : T(-t);
}

template <class T>
void t_swap(T& t1, T& t2) {
	T tmp = t1;
	t1 = t2;
	t2 = tmp;
}

template <class T>
T t_unit(T t) {
	return
		t > T(0) ? 1:
		t <T(0) ? -1:
		0;
}

// sizeof
#define sizeof_raw(V)  sizeof(V)
#define sizeof_type(V) sizeof(V)


/*
	リテラル文字列種、明示指定マクロ
*/

// ビルド種に関係なく、UNICODE。
#define __LTEXT(A) L##A
#define LTEXT(A) __LTEXT(A)
#define LCHAR(A) __LTEXT(A)

// ビルド種に関係なく、ANSI。
#define ATEXT(A) A

// http://bits.stephan-brumme.com/roundUpToNextPowerOfTwo.html
static inline
unsigned int roundUpToNextPowerOfTwo(unsigned int x) {
	--x;
	x |= x >> 1;  // handle  2 bit numbers
	x |= x >> 2;  // handle  4 bit numbers
	x |= x >> 4;  // handle  8 bit numbers
	x |= x >> 8;  // handle 16 bit numbers
	x |= x >> 16; // handle 32 bit numbers
	++x;

	return x;
}

