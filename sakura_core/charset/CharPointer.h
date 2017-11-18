#pragma once

#include "charset/charcode.h"

// �f�B���N�g�����������A�t�@�C�����������擾����
class CharPointerA {
public:
	CharPointerA() { }
	CharPointerA(const char* p) : p(p) { }
	CharPointerA(const CharPointerA& rhs) : p(rhs.p) { }

	// �i��
	const char* operator ++ ()   { _forward(); return this->p; }							// ++p;
	const char* operator ++ (int) { CharPointerA tmp; _forward(); return tmp.p;  }		// p++;
	const char* operator += (size_t n) { while (n-- > 0) _forward(); return this->p; }	// p+=n;
	
	// �i�񂾒l
	const char* operator + (size_t n) const { CharPointerA tmp = *this; return tmp += n; }
	WORD operator[](size_t n) const { CharPointerA tmp = *this; tmp += n; return *tmp; }

	// ���
	const char* operator = (const char* p) { this->p = p; return this->p; }

	// �����擾
	WORD operator * () const { return _get(); }

	// �|�C���^�擾
//	operator const char*() const { return m_p; } // ��operator + �Ƌ�������̂ŁA���̃L���X�g���Z�q�͒񋟂��Ȃ�
	const char* GetPointer() const { return p; }

protected:
	void _forward() { // 1�����i��
		if (_IS_SJIS_1(p[0]) && _IS_SJIS_2(p[1])) p += 2;
		else p += 1;
	}
	WORD _get() const { // 1�����擾����
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

	// �i��
	const wchar_t* operator ++ ()   { _forward(); return this->p; }							// ++p;
	const wchar_t* operator ++ (int) { CharPointerW tmp; _forward(); return tmp.p;   }		// p++;
	const wchar_t* operator += (size_t n) { while (n-- > 0) _forward(); return this->p;  }	// p+=n;
	
	// �i�񂾒l
	const wchar_t* operator + (size_t n) const { CharPointerW tmp = *this; return tmp += n; }
	WORD operator[](size_t n) const { CharPointerW tmp = *this; tmp += n; return *tmp; }

	// ���
	const wchar_t* operator = (const wchar_t* p) { this->p = p; return this->p; }

	// �����擾
	WORD operator * () const { return _get(); }

	// �|�C���^�擾
//	operator const wchar_t*() const { return p; } // ��operator + �Ƌ�������̂ŁA���̃L���X�g���Z�q�͒񋟂��Ȃ�
	const wchar_t* GetPointer() const { return p; }

protected:
	void _forward() { // 1�����i��
		++p;
	}
	WORD _get() const { // 1�����擾����
		return *p;
	}

private:
	const wchar_t* p;
};

typedef CharPointerW CharPointerT;

