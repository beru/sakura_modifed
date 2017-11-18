#pragma once

#include "charset/charcode.h"

// ディレクトリを除いた、ファイル名だけを取得する
class CharPointerA {
public:
	CharPointerA() { }
	CharPointerA(const char* p) : p(p) { }
	CharPointerA(const CharPointerA& rhs) : p(rhs.p) { }

	// 進む
	const char* operator ++ ()   { _forward(); return this->p; }							// ++p;
	const char* operator ++ (int) { CharPointerA tmp; _forward(); return tmp.p;  }		// p++;
	const char* operator += (size_t n) { while (n-- > 0) _forward(); return this->p; }	// p+=n;
	
	// 進んだ値
	const char* operator + (size_t n) const { CharPointerA tmp = *this; return tmp += n; }
	WORD operator[](size_t n) const { CharPointerA tmp = *this; tmp += n; return *tmp; }

	// 代入
	const char* operator = (const char* p) { this->p = p; return this->p; }

	// 文字取得
	WORD operator * () const { return _get(); }

	// ポインタ取得
//	operator const char*() const { return m_p; } // ※operator + と競合するので、このキャスト演算子は提供しない
	const char* GetPointer() const { return p; }

protected:
	void _forward() { // 1文字進む
		if (_IS_SJIS_1(p[0]) && _IS_SJIS_2(p[1])) p += 2;
		else p += 1;
	}
	WORD _get() const { // 1文字取得する
		if (_IS_SJIS_1(p[0]) && _IS_SJIS_2(p[1])) return *((WORD*)p);
		else return *p;
	}

private:
	const char* p;
};


class CharPointerW {
public:
	CharPointerW() { }
	CharPointerW(const wchar_t* p) : p(p) { }
	CharPointerW(const CharPointerW& rhs) : p(rhs.p) { }

	// 進む
	const wchar_t* operator ++ ()   { _forward(); return this->p; }							// ++p;
	const wchar_t* operator ++ (int) { CharPointerW tmp; _forward(); return tmp.p;   }		// p++;
	const wchar_t* operator += (size_t n) { while (n-- > 0) _forward(); return this->p;  }	// p+=n;
	
	// 進んだ値
	const wchar_t* operator + (size_t n) const { CharPointerW tmp = *this; return tmp += n; }
	WORD operator[](size_t n) const { CharPointerW tmp = *this; tmp += n; return *tmp; }

	// 代入
	const wchar_t* operator = (const wchar_t* p) { this->p = p; return this->p; }

	// 文字取得
	WORD operator * () const { return _get(); }

	// ポインタ取得
//	operator const wchar_t*() const { return p; } // ※operator + と競合するので、このキャスト演算子は提供しない
	const wchar_t* GetPointer() const { return p; }

protected:
	void _forward() { // 1文字進む
		++p;
	}
	WORD _get() const { // 1文字取得する
		return *p;
	}

private:
	const wchar_t* p;
};

typedef CharPointerW CharPointerT;

