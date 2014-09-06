//2007.10.18 kobake �쐬
/*
	Copyright (C) 2007, kobake

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

#define SAFE_DELETE(p) { delete p; p=0; }


/*
	2007.10.18 kobake
	�e���v���[�g�� min �Ƃ� max �Ƃ��B

	�ǂ����̕W���w�b�_�ɁA�����悤�Ȃ��̂��������C�����邯�ǁA
	NOMINMAX ���`����ɂ��Ă��A�Ȃ񂾂� min �Ƃ� max �Ƃ��������O���ƁA
	�e���v���[�g���Ă�ł�񂾂��}�N�����Ă�ł�񂾂��󕪂���Ȃ��̂ŁA
	�����I�Ɂut_�`�v�Ƃ������O�����֐���p�ӁB
*/

template <class T>
inline T t_min(T t1,T t2)
{
	return t1<t2?t1:t2;
}

template <class T>
inline T t_max(T t1,T t2)
{
	return t1>t2?t1:t2;
}

template <class T>
T t_abs(T t)
{
	return t>=T(0)?t:T(-t);
}

template <class T>
void t_swap(T& t1, T& t2)
{
	T tmp = t1;
	t1 = t2;
	t2 = tmp;
}

template <class T>
T t_unit(T t)
{
	return
		t>T(0)?1:
		t<T(0)?-1:
		0;
}

//sizeof
#define sizeof_raw(V)  sizeof(V)
#define sizeof_type(V) sizeof(V)


/*
	2007.10.19 kobake

	���e�����������A�����w��}�N��
*/

//�r���h��Ɋ֌W�Ȃ��AUNICODE�B
#define __LTEXT(A) L##A
#define LTEXT(A) __LTEXT(A)
#define LCHAR(A) __LTEXT(A)

//�r���h��Ɋ֌W�Ȃ��AANSI�B
#define ATEXT(A) A

// http://bits.stephan-brumme.com/roundUpToNextPowerOfTwo.html
static inline
unsigned int roundUpToNextPowerOfTwo(unsigned int x)
{
	x--;
	x |= x >> 1;  // handle  2 bit numbers
	x |= x >> 2;  // handle  4 bit numbers
	x |= x >> 4;  // handle  8 bit numbers
	x |= x >> 8;  // handle 16 bit numbers
	x |= x >> 16; // handle 32 bit numbers
	x++;

	return x;
}

