#pragma once

// -- -- -- -- 論理型 -- -- -- -- //

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef BOOL
#define BOOL	int
#endif


// -- -- -- -- 定数 -- -- -- -- //

#ifndef NULL
#define NULL 0
#endif


// -- -- -- -- 文字 -- -- -- -- //

// TCHAR追加機能
// TCHARと逆の文字型をNOT_TCHARとして定義する
typedef char NOT_TCHAR;

// WIN_CHAR (WinAPIに渡すので、必ずTCHARでなければならないもの)
typedef TCHAR WIN_CHAR;
#define _WINT(A) _T(A)


// EDIT_CHAR
typedef wchar_t EDIT_CHAR;
#define _EDITL(A) LTEXT(A)


// 文字コード別、文字型
typedef unsigned char	uchar_t;		// unsigned char の別名．
typedef unsigned short	uchar16_t;		// UTF-16 用．
typedef unsigned long	uchar32_t;		// UTF-32 用．
typedef long			wchar32_t;


// -- -- -- -- その他 -- -- -- -- //

typedef char KEYCODE;

