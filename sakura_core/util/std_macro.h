#pragma once

#define SAFE_DELETE(p) { delete p; p = 0; }


/*
	�e���v���[�g�� min �Ƃ� max �Ƃ��B

	�ǂ����̕W���w�b�_�ɁA�����悤�Ȃ��̂��������C�����邯�ǁA
	NOMINMAX ���`����ɂ��Ă��A�Ȃ񂾂� min �Ƃ� max �Ƃ��������O���ƁA
	�e���v���[�g���Ă�ł�񂾂��}�N�����Ă�ł�񂾂��󕪂���Ȃ��̂ŁA
	�����I�Ɂut_�`�v�Ƃ������O�����֐���p�ӁB
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
	���e�����������A�����w��}�N��
*/

// �r���h��Ɋ֌W�Ȃ��AUNICODE�B
#define __LTEXT(A) L##A
#define LTEXT(A) __LTEXT(A)
#define LCHAR(A) __LTEXT(A)

// �r���h��Ɋ֌W�Ȃ��AANSI�B
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

