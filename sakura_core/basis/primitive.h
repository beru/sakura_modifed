#pragma once

// -- -- -- -- �_���^ -- -- -- -- //

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef BOOL
#define BOOL	int
#endif


// -- -- -- -- �萔 -- -- -- -- //

#ifndef NULL
#define NULL 0
#endif


// -- -- -- -- ���� -- -- -- -- //

// TCHAR�ǉ��@�\
// TCHAR�Ƌt�̕����^��NOT_TCHAR�Ƃ��Ē�`����
typedef char NOT_TCHAR;

// WIN_CHAR (WinAPI�ɓn���̂ŁA�K��TCHAR�łȂ���΂Ȃ�Ȃ�����)
typedef TCHAR WIN_CHAR;
#define _WINT(A) _T(A)


// EDIT_CHAR
typedef wchar_t EDIT_CHAR;
#define _EDITL(A) LTEXT(A)


// �����R�[�h�ʁA�����^
typedef unsigned char	uchar_t;		// unsigned char �̕ʖ��D
typedef unsigned short	uchar16_t;		// UTF-16 �p�D
typedef unsigned long	uchar32_t;		// UTF-32 �p�D
typedef long			wchar32_t;


// -- -- -- -- ���̑� -- -- -- -- //

typedef char KEYCODE;

