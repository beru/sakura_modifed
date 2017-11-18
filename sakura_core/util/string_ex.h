#pragma once

// 2007.10.19 kobake
// string.h で定義されている関数を拡張したようなモノ達


/*
	++ ++ 命名参考(規則では無い) ++ ++

	標準関数から引用
	～_s:  バッファオーバーフロー考慮版 (例: strcpy_s)
	～i～: 大文字小文字区別無し版       (例: stricmp)

	独自
	auto_～:  引数の型により、自動で処理が決定される版 (例: auto_strcpy)
*/

#include "util/tchar_printf.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          メモリ                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// 文字列コピーや文字列比較の際に、mem系関数が使われている箇所が多々ありますが、
// mem系関数はvoidポインタを受け取り、型チェックが行われないので危険です。
// ここに、型チェック付きのmem系互換の関数を作成しました。…と書いたけど、実際のプロトタイプはもっと下のほうに。。(auto_mem～)
// (※対象がメモリなので、そもそも文字という概念は無いが、
//    便宜上、char系では1バイト単位を、wchar_t系では2バイト単位を、
//    文字とみなして処理を行う、ということで)

// メモリ比較
inline int amemcmp(const char* p1, const char* p2, size_t count) { return ::memcmp(p1, p2, count); }

// 大文字小文字を区別せずにメモリ比較
inline int amemicmp(const char* p1, const char* p2, size_t count) { return ::memicmp(p1, p2, count); }
       int wmemicmp(const wchar_t* p1, const wchar_t* p2, size_t count);
       int wmemicmp(const wchar_t* p1, const wchar_t* p2);
       int wmemicmp_ascii(const wchar_t* p1, const wchar_t* p2, size_t count);

// 元の関数と同じシグニチャ版。
// 文字列以外のメモリ処理でmem～系関数を使う場面では、この関数を使っておくと、意味合いがはっきりして良い。
inline void* memset_raw(void* dest, int c, size_t size) { return ::memset(dest, c, size); }
inline void* memcpy_raw(void* dest, const void* src, size_t size) { return ::memcpy(dest, src, size); }


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           文字                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 文字変換
inline int my_toupper(int c) { return ((c >= 'a') && (c <= 'z')) ? (c - 'a' + 'A') : c; }
inline int my_tolower(int c) { return ((c >= 'A') && (c <= 'Z')) ? (c - 'A' + 'a') : c; }
inline int my_towupper(int c) { return ((c >= L'a') && (c <= L'z')) ? (c - L'a' + L'A') : c; }
inline int my_towlower(int c) { return ((c >= L'A') && (c <= L'Z')) ? (c - L'A' + L'a') : c; }
inline wchar_t my_towupper2( wchar_t c ){ return my_towupper(c); }
inline wchar_t my_towlower2( wchar_t c ){ return my_towlower(c); }
int skr_towupper(int c);
int skr_towlower(int c);
#define _tcs_toupper skr_towupper
#define _tcs_tolower skr_towlower

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           拡張・独自実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 文字数の上限付きコピー
LPWSTR wcscpyn(LPWSTR lpString1, LPCWSTR lpString2, size_t iMaxLength); // iMaxLengthは文字単位。

// Apr. 03, 2003 genta
char* strncpy_ex(char* dst, size_t dst_count, const char* src, size_t src_count);

// 大文字小文字を区別せずに文字列を検索
const wchar_t* wcsistr(const wchar_t* s1, const wchar_t* s2);
const char* stristr(const char* s1, const char* s2);
inline wchar_t* wcsistr(wchar_t* s1, const wchar_t* s2) { return const_cast<wchar_t*>(wcsistr(static_cast<const wchar_t*>(s1), s2)); }
inline char* stristr(char* s1, const char* s2) { return const_cast<char*>(stristr(static_cast<const char*>(s1), s2)); }
#define _tcsistr wcsistr

// 大文字小文字を区別せずに文字列を検索（日本語対応版）
const char* strchr_j(const char* s1, char c);				// strchr の日本語対応版。
const char* strichr_j(const char* s1, char c);				// strchr の大文字小文字同一視＆日本語対応版。
const char* strstr_j(const char* s1, const char* s2);		// strstr の日本語対応版。
const char* stristr_j(const char* s1, const char* s2);		// strstr の大文字小文字同一視＆日本語対応版。
inline char* strchr_j (char* s1, char c        ) { return const_cast<char*>(strchr_j ((const char*)s1, c)); }
inline char* strichr_j(char* s1, char c        ) { return const_cast<char*>(strichr_j((const char*)s1, c)); }
inline char* strstr_j (char* s1, const char* s2) { return const_cast<char*>(strstr_j ((const char*)s1, s2)); }
inline char* stristr_j(char* s1, const char* s2) { return const_cast<char*>(stristr_j((const char*)s1, s2)); }
#define _tcsistr_j wcsistr

template <class CHAR_TYPE>
CHAR_TYPE* my_strtok(
	CHAR_TYPE*			pBuffer,	// [in] 文字列バッファ(終端があること)
	size_t				nLen,		// [in] 文字列の長さ
	size_t*				pnOffset,	// [in/out] オフセット
	const CHAR_TYPE*	pDelimiter	// [in] 区切り文字
);


// ▽ シグニチャおよび動作仕様は変わらないけど、
// コンパイラと言語指定によって不正動作をしてしまうことを回避するために
// 独自に実装し直したもの。
int my_stricmp(const char* s1, const char* s2);
int my_strnicmp(const char* s1, const char* s2, size_t n);

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//        auto系（_UNICODE 定義に依存しない関数）              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// char型にするかwchar_t型にするか確定しない変数があります。
// 下記関数群を使って文字列操作を行った場合、
// 将来、その変数の型が変わっても、その操作箇所を書き直さなくても
// 済むことになります。
//
// 強制キャストによる使用は推奨しません。
// そもそも、この関数呼び出しに限らず、強制キャストは最低限に留めてください。
// せっかくの、C++の厳格な型チェックの恩恵を受けることができなくなります。


// 転送系
inline char* auto_memcpy(char* dest, const char* src, size_t count) {        ::memcpy (dest, src, count); return dest; }
inline wchar_t* auto_memcpy(wchar_t* dest, const wchar_t* src, size_t count) { return ::wmemcpy(dest, src, count);              }
inline char* auto_strcpy(char* dst, const char* src) { return strcpy(dst, src); }
inline wchar_t* auto_strcpy(wchar_t* dst, const wchar_t* src) { return wcscpy(dst, src); }
inline errno_t auto_strcpy_s(char* dst, size_t nDstCount, const char* src) { return strcpy_s(dst, nDstCount, src); }
inline errno_t auto_strcpy_s(wchar_t* dst, size_t nDstCount, const wchar_t* src) { return wcscpy_s(dst, nDstCount, src); }
inline char* auto_strncpy(char* dst, const char* src, size_t count) { return strncpy(dst, src, count); }
inline wchar_t* auto_strncpy(wchar_t* dst, const wchar_t* src, size_t count) { return wcsncpy(dst, src, count); }
inline char* auto_memset(char* dest, char c, size_t count) { memset(dest, c, count); return dest; }
inline wchar_t* auto_memset(wchar_t* dest, wchar_t c, size_t count) { return wmemset(dest, c, count);              }
inline char* auto_strcat(char* dst, const char* src) { return strcat(dst, src); }
inline wchar_t* auto_strcat(wchar_t* dst, const wchar_t* src) { return wcscat(dst, src); }
inline errno_t auto_strcat_s(char* dst, size_t nDstCount, const char* src) { return strcat_s(dst, nDstCount, src); }
inline errno_t auto_strcat_s(wchar_t* dst, size_t nDstCount, const wchar_t* src) { return wcscat_s(dst, nDstCount, src); }

// 比較系
inline int auto_memcmp (const char* p1, const char* p2, size_t count) { return amemcmp(p1, p2, count); }
inline int auto_memcmp (const wchar_t* p1, const wchar_t* p2, size_t count) { return wmemcmp(p1, p2, count); }
inline int auto_strcmp (const char* p1, const char* p2) { return strcmp(p1, p2); }
inline int auto_strcmp (const wchar_t* p1, const wchar_t* p2) { return wcscmp(p1, p2); }
inline int auto_strncmp(const char* str1, const char* str2, size_t count) { return strncmp(str1, str2, count); }
inline int auto_strncmp(const wchar_t* str1, const wchar_t* str2, size_t count) { return wcsncmp(str1, str2, count); }

// 比較系（ASCII, UCS2 専用）
inline int auto_memicmp(const char* p1, const char* p2, size_t count) { return amemicmp(p1, p2, count); }
inline int auto_memicmp(const wchar_t* p1, const wchar_t* p2, size_t count) { return wmemicmp(p1, p2, count); }

// 比較系（SJIS, UTF-16 専用)
inline int auto_strnicmp(const char* p1, const char* p2, size_t count) { return my_strnicmp(p1, p2, count); }
inline int auto_strnicmp(const wchar_t* p1, const wchar_t* p2, size_t count) { return wmemicmp(p1, p2, count); } // Stub.
inline int auto_stricmp(const char* p1, const char* p2) { return my_stricmp(p1, p2); }
inline int auto_stricmp(const wchar_t* p1, const wchar_t* p2) { return wmemicmp(p1, p2); } // Stub.

// 長さ計算系
inline size_t auto_strlen(const char* str) { return strlen(str); }
inline size_t auto_strlen(const wchar_t* str) { return wcslen(str); }
inline size_t auto_strnlen(const char* str, size_t count) { return strnlen(str, count); }
inline size_t auto_strnlen(const wchar_t* str, size_t count) { return wcsnlen(str, count); }

// 検索系（SJIS, UCS2 専用）
inline const char* auto_strstr(const char* str, const char* strSearch) { return ::strstr_j(str, strSearch); }
inline const wchar_t* auto_strstr(const wchar_t* str, const wchar_t* strSearch) { return ::wcsstr  (str, strSearch); }
inline       char* auto_strstr(char* str, const char* strSearch) { return ::strstr_j(str, strSearch); }
inline       wchar_t* auto_strstr(wchar_t* str, const wchar_t* strSearch) { return ::wcsstr  (str, strSearch); }
inline const char* auto_strchr(const char* str, char c) { return ::strchr_j(str, c); }
inline const wchar_t* auto_strchr(const wchar_t* str, wchar_t c) { return ::wcschr  (str, c); }
inline       char* auto_strchr(char* str, char c) { return ::strchr_j(str, c); }
inline       wchar_t* auto_strchr(wchar_t* str, wchar_t c) { return ::wcschr  (str, c); }

// 変換系
inline long auto_atol(const char* str) { return atol(str);  }
inline long auto_atol(const wchar_t* str) { return _wtol(str); }
char* tcstostr(char* dest, const TCHAR* src, size_t count);
wchar_t* tcstostr(wchar_t* dest, const TCHAR* src, size_t count);
TCHAR* strtotcs(TCHAR* dest, const char* src, size_t count);
TCHAR* strtotcs(TCHAR* dest, const wchar_t* src, size_t count);

// 印字系

template <size_t len>
inline
int auto_sprintf_s(char (&buff)[len], const char* format, ...)
{
	va_list v;
	va_start(v, format);
	int ret = tchar_vsprintf_s(buff, len, format, v);
	va_end(v);
	return ret; 
}

template <size_t len>
inline
int auto_sprintf_s(wchar_t (&buff)[len], const wchar_t* format, ...)
{
	va_list v;
	va_start(v, format);
	int ret = tchar_vsprintf_s(buff, len, format, v);
	va_end(v);
	return ret; 
}

inline
int auto_sprintf(wchar_t* buf, const wchar_t* format, ...)
{
	va_list v;
	va_start(v, format);
	int ret = tchar_vsprintf(buf, format, v);
	va_end(v);
	return ret;
}

inline int auto_snprintf_s(char* buf, size_t count, const char* format, ...)   { va_list v; va_start(v, format); int ret = tchar_vsnprintf_s (buf, count, format, v); va_end(v); return ret; }
inline int auto_snprintf_s(wchar_t* buf, size_t count, const wchar_t* format, ...)   { va_list v; va_start(v, format); int ret = tchar_vsnprintf_s(buf, count, format, v); va_end(v); return ret; }
inline int auto_sprintf(char* buf, const char* format, ...)                    { va_list v; va_start(v, format); int ret = tchar_vsprintf (buf, format, v); va_end(v); return ret; }
inline int auto_sprintf_s(char* buf, size_t nBufCount, const char* format, ...) { va_list v; va_start(v, format); int ret = tchar_vsprintf_s (buf, nBufCount, format, v); va_end(v); return ret; }
inline int auto_sprintf_s(wchar_t* buf, size_t nBufCount, const wchar_t* format, ...) { va_list v; va_start(v, format); int ret = tchar_vsprintf_s(buf, nBufCount, format, v); va_end(v); return ret; }
inline int auto_vsprintf(char* buf, const char* format, va_list& v) { return tchar_vsprintf (buf, format, v); }
inline int auto_vsprintf(wchar_t* buf, const wchar_t* format, va_list& v) { return tchar_vsprintf(buf, format, v); }
inline int auto_vsprintf_s(char* buf, size_t nBufCount, const char* format, va_list& v) { return tchar_vsprintf_s (buf, nBufCount, format, v); }
inline int auto_vsprintf_s(wchar_t* buf, size_t nBufCount, const wchar_t* format, va_list& v) { return tchar_vsprintf_s(buf, nBufCount, format, v); }

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      文字コード変換                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#include <vector>

// SJIS→UNICODE。終端にL'\0'を付けてくれる版。
size_t mbstowcs2(wchar_t* dst, const char* src, size_t dst_count);
size_t mbstowcs2(wchar_t* pDst, int nDstCount, const char* pSrc, int nSrcCount);

// UNICODE→SJIS。終端に'\0'を付けてくれる版。
size_t wcstombs2(char* dst,const wchar_t* src,size_t dst_count);

// SJIS→UNICODE。
wchar_t*	mbstowcs_new(const char* pszSrc);								// 戻り値はnew[]で確保して返す。使い終わったらdelete[]すること。
wchar_t*	mbstowcs_new(const char* pSrc, size_t nSrcLen, int* pnDstLen);		// 戻り値はnew[]で確保して返す。使い終わったらdelete[]すること。
void		mbstowcs_vector(const char* src, std::vector<wchar_t>* ret);	// 戻り値はvectorとして返す。
void		mbstowcs_vector(const char* pSrc, size_t nSrcLen, std::vector<wchar_t>* ret);	// 戻り値はvectorとして返す。

// UNICODE→SJIS
char*	wcstombs_new(const wchar_t* src); // 戻り値はnew[]で確保して返す。
char*	wcstombs_new(const wchar_t* pSrc, size_t nSrcLen); // 戻り値はnew[]で確保して返す。
void	wcstombs_vector(const wchar_t* pSrc, std::vector<char>* ret); // 戻り値はvectorとして返す。
void	wcstombs_vector(const wchar_t* pSrc, size_t nSrcLen, std::vector<char>* ret); // 戻り値はvectorとして返す。

// TCHAR
size_t _tcstowcs(wchar_t* wszDst, const TCHAR* tszSrc, size_t nDstCount);
size_t _tcstombs(CHAR*  szDst,  const TCHAR* tszSrc, size_t nDstCount);
size_t _wcstotcs(TCHAR* tszDst, const wchar_t* wszSrc, size_t nDstCount);
size_t _mbstotcs(TCHAR* tszDst, const CHAR*  szSrc,  size_t nDstCount);
int _tctomb(const TCHAR* p, char* mb);
int _tctowc(const TCHAR* p, wchar_t* wc);



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       リテラル比較                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// リテラルとの文字列比較の際に、手打ちで文字数を入力するのは
// 手間が掛かる上に、保守性が損なわれるので、
// カプセル化された関数やマクロに処理を任せるのが望ましい。

// wcsncmpの文字数指定をszData2からwcslenで取得してくれる版
inline int wcsncmp_auto(const wchar_t* strData1, const wchar_t* szData2)
{
	return wcsncmp(strData1, szData2, wcslen(szData2));
}

// wcsncmpの文字数指定をliteralData2の大きさで取得してくれる版
#define wcsncmp_literal(strData1, literalData2) \
	::wcsncmp(strData1, literalData2, _countof(literalData2) - 1) // ※終端ヌルを含めないので、_countofからマイナス1する

// strncmpの文字数指定をliteralData2の大きさで取得してくれる版
#define strncmp_literal(strData1, literalData2) \
	::strncmp(strData1, literalData2, _countof(literalData2) - 1) // ※終端ヌルを含めないので、_countofからマイナス1する

// TCHAR
#define _tcsncmp_literal wcsncmp_literal

