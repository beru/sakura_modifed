#pragma once

// 2007.10.19 kobake
// string.h �Œ�`����Ă���֐����g�������悤�ȃ��m�B


/*
	++ ++ �����Q�l(�K���ł͖���) ++ ++

	�W���֐�������p
	�`_s:  �o�b�t�@�I�[�o�[�t���[�l���� (��: strcpy_s)
	�`i�`: �啶����������ʖ�����       (��: stricmp)

	�Ǝ�
	auto_�`:  �����̌^�ɂ��A�����ŏ��������肳���� (��: auto_strcpy)
*/

#include "util/tchar_printf.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ������                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// ������R�s�[�╶�����r�̍ۂɁAmem�n�֐����g���Ă���ӏ������X����܂����A
// mem�n�֐���void�|�C���^���󂯎��A�^�`�F�b�N���s���Ȃ��̂Ŋ댯�ł��B
// �����ɁA�^�`�F�b�N�t����mem�n�݊��̊֐����쐬���܂����B�c�Ə��������ǁA���ۂ̃v���g�^�C�v�͂����Ɖ��̂ق��ɁB�B(auto_mem�`)
// (���Ώۂ��������Ȃ̂ŁA�������������Ƃ����T�O�͖������A
//    �֋X��Achar�n�ł�1�o�C�g�P�ʂ��Awchar_t�n�ł�2�o�C�g�P�ʂ��A
//    �����Ƃ݂Ȃ��ď������s���A�Ƃ������Ƃ�)

// ��������r
inline int amemcmp(const char* p1, const char* p2, size_t count) { return ::memcmp(p1, p2, count); }

// �啶������������ʂ����Ƀ�������r
inline int amemicmp(const char* p1, const char* p2, size_t count) { return ::memicmp(p1, p2, count); }
       int wmemicmp(const wchar_t* p1, const wchar_t* p2, size_t count);
       int wmemicmp(const wchar_t* p1, const wchar_t* p2);
       int wmemicmp_ascii(const wchar_t* p1, const wchar_t* p2, size_t count);

// ���̊֐��Ɠ����V�O�j�`���ŁB
// ������ȊO�̃�����������mem�`�n�֐����g����ʂł́A���̊֐����g���Ă����ƁA�Ӗ��������͂����肵�ėǂ��B
inline void* memset_raw(void* dest, int c, size_t size) { return ::memset(dest, c, size); }
inline void* memcpy_raw(void* dest, const void* src, size_t size) { return ::memcpy(dest, src, size); }


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �����ϊ�
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
//                           �g���E�Ǝ�����                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �������̏���t���R�s�[
LPWSTR wcscpyn(LPWSTR lpString1, LPCWSTR lpString2, size_t iMaxLength); // iMaxLength�͕����P�ʁB

// Apr. 03, 2003 genta
char* strncpy_ex(char* dst, size_t dst_count, const char* src, size_t src_count);

// �啶������������ʂ����ɕ����������
const wchar_t* wcsistr(const wchar_t* s1, const wchar_t* s2);
const char* stristr(const char* s1, const char* s2);
inline wchar_t* wcsistr(wchar_t* s1, const wchar_t* s2) { return const_cast<wchar_t*>(wcsistr(static_cast<const wchar_t*>(s1), s2)); }
inline char* stristr(char* s1, const char* s2) { return const_cast<char*>(stristr(static_cast<const char*>(s1), s2)); }
#define _tcsistr wcsistr

// �啶������������ʂ����ɕ�����������i���{��Ή��Łj
const char* strchr_j(const char* s1, char c);				// strchr �̓��{��Ή��ŁB
const char* strichr_j(const char* s1, char c);				// strchr �̑啶�����������ꎋ�����{��Ή��ŁB
const char* strstr_j(const char* s1, const char* s2);		// strstr �̓��{��Ή��ŁB
const char* stristr_j(const char* s1, const char* s2);		// strstr �̑啶�����������ꎋ�����{��Ή��ŁB
inline char* strchr_j (char* s1, char c        ) { return const_cast<char*>(strchr_j ((const char*)s1, c)); }
inline char* strichr_j(char* s1, char c        ) { return const_cast<char*>(strichr_j((const char*)s1, c)); }
inline char* strstr_j (char* s1, const char* s2) { return const_cast<char*>(strstr_j ((const char*)s1, s2)); }
inline char* stristr_j(char* s1, const char* s2) { return const_cast<char*>(stristr_j((const char*)s1, s2)); }
#define _tcsistr_j wcsistr

template <class CHAR_TYPE>
CHAR_TYPE* my_strtok(
	CHAR_TYPE*			pBuffer,	// [in] ������o�b�t�@(�I�[�����邱��)
	size_t				nLen,		// [in] ������̒���
	size_t*				pnOffset,	// [in/out] �I�t�Z�b�g
	const CHAR_TYPE*	pDelimiter	// [in] ��؂蕶��
);


// �� �V�O�j�`������ѓ���d�l�͕ς��Ȃ����ǁA
// �R���p�C���ƌ���w��ɂ���ĕs����������Ă��܂����Ƃ�������邽�߂�
// �Ǝ��Ɏ��������������́B
int my_stricmp(const char* s1, const char* s2);
int my_strnicmp(const char* s1, const char* s2, size_t n);

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//        auto�n�i_UNICODE ��`�Ɉˑ����Ȃ��֐��j              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// char�^�ɂ��邩wchar_t�^�ɂ��邩�m�肵�Ȃ��ϐ�������܂��B
// ���L�֐��Q���g���ĕ����񑀍���s�����ꍇ�A
// �����A���̕ϐ��̌^���ς���Ă��A���̑���ӏ������������Ȃ��Ă�
// �ςނ��ƂɂȂ�܂��B
//
// �����L���X�g�ɂ��g�p�͐������܂���B
// ���������A���̊֐��Ăяo���Ɍ��炸�A�����L���X�g�͍Œ���ɗ��߂Ă��������B
// ���������́AC++�̌��i�Ȍ^�`�F�b�N�̉��b���󂯂邱�Ƃ��ł��Ȃ��Ȃ�܂��B


// �]���n
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

// ��r�n
inline int auto_memcmp (const char* p1, const char* p2, size_t count) { return amemcmp(p1, p2, count); }
inline int auto_memcmp (const wchar_t* p1, const wchar_t* p2, size_t count) { return wmemcmp(p1, p2, count); }
inline int auto_strcmp (const char* p1, const char* p2) { return strcmp(p1, p2); }
inline int auto_strcmp (const wchar_t* p1, const wchar_t* p2) { return wcscmp(p1, p2); }
inline int auto_strncmp(const char* str1, const char* str2, size_t count) { return strncmp(str1, str2, count); }
inline int auto_strncmp(const wchar_t* str1, const wchar_t* str2, size_t count) { return wcsncmp(str1, str2, count); }

// ��r�n�iASCII, UCS2 ��p�j
inline int auto_memicmp(const char* p1, const char* p2, size_t count) { return amemicmp(p1, p2, count); }
inline int auto_memicmp(const wchar_t* p1, const wchar_t* p2, size_t count) { return wmemicmp(p1, p2, count); }

// ��r�n�iSJIS, UTF-16 ��p)
inline int auto_strnicmp(const char* p1, const char* p2, size_t count) { return my_strnicmp(p1, p2, count); }
inline int auto_strnicmp(const wchar_t* p1, const wchar_t* p2, size_t count) { return wmemicmp(p1, p2, count); } // Stub.
inline int auto_stricmp(const char* p1, const char* p2) { return my_stricmp(p1, p2); }
inline int auto_stricmp(const wchar_t* p1, const wchar_t* p2) { return wmemicmp(p1, p2); } // Stub.

// �����v�Z�n
inline size_t auto_strlen(const char* str) { return strlen(str); }
inline size_t auto_strlen(const wchar_t* str) { return wcslen(str); }
inline size_t auto_strnlen(const char* str, size_t count) { return strnlen(str, count); }
inline size_t auto_strnlen(const wchar_t* str, size_t count) { return wcsnlen(str, count); }

// �����n�iSJIS, UCS2 ��p�j
inline const char* auto_strstr(const char* str, const char* strSearch) { return ::strstr_j(str, strSearch); }
inline const wchar_t* auto_strstr(const wchar_t* str, const wchar_t* strSearch) { return ::wcsstr  (str, strSearch); }
inline       char* auto_strstr(char* str, const char* strSearch) { return ::strstr_j(str, strSearch); }
inline       wchar_t* auto_strstr(wchar_t* str, const wchar_t* strSearch) { return ::wcsstr  (str, strSearch); }
inline const char* auto_strchr(const char* str, char c) { return ::strchr_j(str, c); }
inline const wchar_t* auto_strchr(const wchar_t* str, wchar_t c) { return ::wcschr  (str, c); }
inline       char* auto_strchr(char* str, char c) { return ::strchr_j(str, c); }
inline       wchar_t* auto_strchr(wchar_t* str, wchar_t c) { return ::wcschr  (str, c); }

// �ϊ��n
inline long auto_atol(const char* str) { return atol(str);  }
inline long auto_atol(const wchar_t* str) { return _wtol(str); }
char* tcstostr(char* dest, const TCHAR* src, size_t count);
wchar_t* tcstostr(wchar_t* dest, const TCHAR* src, size_t count);
TCHAR* strtotcs(TCHAR* dest, const char* src, size_t count);
TCHAR* strtotcs(TCHAR* dest, const wchar_t* src, size_t count);

// �󎚌n

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
//                      �����R�[�h�ϊ�                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#include <vector>

// SJIS��UNICODE�B�I�[��L'\0'��t���Ă����ŁB
size_t mbstowcs2(wchar_t* dst, const char* src, size_t dst_count);
size_t mbstowcs2(wchar_t* pDst, int nDstCount, const char* pSrc, int nSrcCount);

// UNICODE��SJIS�B�I�[��'\0'��t���Ă����ŁB
size_t wcstombs2(char* dst,const wchar_t* src,size_t dst_count);

// SJIS��UNICODE�B
wchar_t*	mbstowcs_new(const char* pszSrc);								// �߂�l��new[]�Ŋm�ۂ��ĕԂ��B�g���I�������delete[]���邱�ƁB
wchar_t*	mbstowcs_new(const char* pSrc, size_t nSrcLen, int* pnDstLen);		// �߂�l��new[]�Ŋm�ۂ��ĕԂ��B�g���I�������delete[]���邱�ƁB
void		mbstowcs_vector(const char* src, std::vector<wchar_t>* ret);	// �߂�l��vector�Ƃ��ĕԂ��B
void		mbstowcs_vector(const char* pSrc, size_t nSrcLen, std::vector<wchar_t>* ret);	// �߂�l��vector�Ƃ��ĕԂ��B

// UNICODE��SJIS
char*	wcstombs_new(const wchar_t* src); // �߂�l��new[]�Ŋm�ۂ��ĕԂ��B
char*	wcstombs_new(const wchar_t* pSrc, size_t nSrcLen); // �߂�l��new[]�Ŋm�ۂ��ĕԂ��B
void	wcstombs_vector(const wchar_t* pSrc, std::vector<char>* ret); // �߂�l��vector�Ƃ��ĕԂ��B
void	wcstombs_vector(const wchar_t* pSrc, size_t nSrcLen, std::vector<char>* ret); // �߂�l��vector�Ƃ��ĕԂ��B

// TCHAR
size_t _tcstowcs(wchar_t* wszDst, const TCHAR* tszSrc, size_t nDstCount);
size_t _tcstombs(CHAR*  szDst,  const TCHAR* tszSrc, size_t nDstCount);
size_t _wcstotcs(TCHAR* tszDst, const wchar_t* wszSrc, size_t nDstCount);
size_t _mbstotcs(TCHAR* tszDst, const CHAR*  szSrc,  size_t nDstCount);
int _tctomb(const TCHAR* p, char* mb);
int _tctowc(const TCHAR* p, wchar_t* wc);



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       ���e������r                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// ���e�����Ƃ̕������r�̍ۂɁA��ł��ŕ���������͂���̂�
// ��Ԃ��|�����ɁA�ێ琫�����Ȃ���̂ŁA
// �J�v�Z�������ꂽ�֐���}�N���ɏ�����C����̂��]�܂����B

// wcsncmp�̕������w���szData2����wcslen�Ŏ擾���Ă�����
inline int wcsncmp_auto(const wchar_t* strData1, const wchar_t* szData2)
{
	return wcsncmp(strData1, szData2, wcslen(szData2));
}

// wcsncmp�̕������w���literalData2�̑傫���Ŏ擾���Ă�����
#define wcsncmp_literal(strData1, literalData2) \
	::wcsncmp(strData1, literalData2, _countof(literalData2) - 1) // ���I�[�k�����܂߂Ȃ��̂ŁA_countof����}�C�i�X1����

// strncmp�̕������w���literalData2�̑傫���Ŏ擾���Ă�����
#define strncmp_literal(strData1, literalData2) \
	::strncmp(strData1, literalData2, _countof(literalData2) - 1) // ���I�[�k�����܂߂Ȃ��̂ŁA_countof����}�C�i�X1����

// TCHAR
#define _tcsncmp_literal wcsncmp_literal

