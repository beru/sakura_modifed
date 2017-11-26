#include "StdAfx.h"
#include "string_ex.h"
#include "charset/charcode.h"
#include "util/std_macro.h"
#include <limits.h>

int __cdecl my_internal_icmp(const char* s1, const char* s2, unsigned int n, unsigned int dcount, bool flag);



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �g���E�Ǝ�����                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	�啶���������𓯈ꎋ���镶�����r������B
	@param s1 [in] ������P
	@param s2 [in] ������Q

	@retval 0	��v
 */
int __cdecl my_stricmp(const char* s1, const char* s2)
{
	// �`�F�b�N���镶������uint�ő�ɐݒ肷��
	//return my_internal_icmp(s1, s2, (unsigned int)(~0), 0, true);
	return my_internal_icmp(s1, s2, UINT_MAX, 0, true);
}

/*!	�啶���������𓯈ꎋ���镶���񒷂�������r������B
	@param s1 [in] ������P
	@param s2 [in] ������Q
	@param n [in] ������

	@retval 0	��v
 */
int __cdecl my_strnicmp(const char* s1, const char* s2, size_t n)
{
	return my_internal_icmp(s1, s2, (unsigned int)n, 1, true);
}

LPWSTR wcscpyn(LPWSTR lpString1, LPCWSTR lpString2, size_t iMaxLength)
{
	ASSERT_GE(iMaxLength, 1);
	size_t len2 = wcslen(lpString2);
	if (len2 > iMaxLength-1) len2 = iMaxLength-1;
	wmemcpy(lpString1, lpString2, len2);
	lpString1[len2] = L'\0';
	return lpString1;
}


/*
	TCHAR �� wchar_t �܂��� char �̕ϊ��֐�
*/

char* tcstostr(char* dest, const TCHAR* src, size_t count) {
	TCHAR* pr = const_cast<TCHAR*>(src);
	char* pw = dest;
	for (; pr<src+count; ++pr) {
		*pw = static_cast<char>(*pr);
		++pw;
	}
	return pw;
}
wchar_t* tcstostr(wchar_t* dest, const TCHAR* src, size_t count) {
	TCHAR* pr = const_cast<TCHAR*>(src);
	wchar_t* pw = dest;
	for (; pr<src+count; ++pr) {
		*pw = static_cast<wchar_t>(*pr);
		++pw;
	}
	return pw;
}

TCHAR* strtotcs(TCHAR* dest, const char* src, size_t count)
{
	char* pr = const_cast<char*>(src);
	TCHAR* pw = dest;
	for (; pr<src+count; ++pr) {
		*pw = static_cast<TCHAR>(*pr);
		++pw;
	}
	return pw;
}

TCHAR* strtotcs(TCHAR* dest, const wchar_t* src, size_t count)
{
	wchar_t* pr = const_cast<wchar_t*>(src);
	TCHAR* pw = dest;
	for (; pr<src+count; ++pr) {
		*pw = static_cast<TCHAR>(*pr);
		++pw;
	}
	return pw;
}


/*! �����������@�\�t��strncpy

	�R�s�[��̃o�b�t�@�T�C�Y������Ȃ��悤��strncpy����B
	�o�b�t�@���s������ꍇ�ɂ�2�o�C�g�����̐ؒf�����蓾��B
	������\0�͕t�^����Ȃ����A�R�s�[�̓R�s�[��o�b�t�@�T�C�Y-1�܂łɂ��Ă����B

	@param dst [in] �R�s�[��̈�ւ̃|�C���^
	@param dst_count [in] �R�s�[��̈�̃T�C�Y
	@param src [in] �R�s�[��
	@param src_count [in] �R�s�[���镶����̖���

	@retval ���ۂɃR�s�[���ꂽ�R�s�[��̈��1����w���|�C���^
*/
char* strncpy_ex(char* dst, size_t dst_count, const char* src, size_t src_count)
{
	if (src_count >= dst_count) {
		src_count = dst_count - 1;
	}
	auto_memcpy(dst, src, src_count);
	return dst + src_count;
}

const wchar_t* wcsistr(const wchar_t* s1, const wchar_t* s2)
{
	size_t len2 = wcslen(s2);
	const wchar_t* p = s1;
	const wchar_t* q = wcschr(s1, L'\0') - len2;
	while (p <= q) {
		if (auto_memicmp(p, s2, len2) == 0) return p;
		++p;
	}
	return NULL;
}

const char* stristr(const char* s1, const char* s2)
{
	//$ ���{��l�����ĂȂ��̂ŁA����܂���ɗ����Ȃ��ŁBstristr_j���g���̂��]�܂����B
	size_t len2 = strlen(s2);
	const char* p = s1;
	const char* q = strchr(s1, L'\0')-len2;
	while (p <= q) {
		if (auto_memicmp(p, s2, len2) == 0) return p;
		++p;
	}
	return NULL;
}

const char* strichr_j(const char* s1, char c2)
{
	if (c2 == 0) return ::strchr(s1, 0); // ������I�[��T�����߂�c2��0��n�����ꍇ���A���������������悤��

	int C2 = my_toupper(c2);
	for (const char* p1=s1; *p1; ++p1) {
		if (my_toupper(*p1) == C2) return p1;
		if (my_iskanji1(*(const unsigned char*)p1) && *(p1 + 1) != 0) ++p1;
	}
	return NULL;
}

const char* strchr_j(const char* str, char c)
{
	if (c == 0) return ::strchr(str, 0); // ������I�[��T�����߂�c��0��n�����ꍇ���A���������������悤��

	for (const char* p1=str; *p1; ++p1) {
		if (*p1 == c) return p1;
		if (my_iskanji1(*(const unsigned char*)p1) && *(p1 + 1) != 0) ++p1;
	}
	return NULL;
}

/*!
	strstr()��2byte code�Ή���
*/
const char* strstr_j(const char* s1, const char* s2)
{
	size_t n = strlen(s2);
	for (const char* p1=s1; *p1; ++p1) {
		if (strncmp(p1, s2, n) == 0) return p1;
		if (my_iskanji1(*(const unsigned char*)p1) && *(p1 + 1) != 0) ++p1;
	}
	return NULL;
}

/*!
	strstr()�̑啶�����������ꎋ��

	@note
	Windows API�ɂ���StrStrI��IE4�������Ă��Ȃ�PC�ł͎g�p�s�̂���
	�Ǝ��ɍ쐬
*/
const char* stristr_j(const char* s1, const char* s2)
{
	size_t n = strlen(s2);
	for (const char* p1=s1; *p1; ++p1) {
		if (my_strnicmp(p1, s2, n) == 0) return p1;
		if (my_iskanji1(*(const unsigned char*)p1) && *(p1 + 1) != 0) ++p1;
	}
	return NULL;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �����R�[�h�ϊ�                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// SJIS��UNICODE�B�I�[��L'\0'��t���Ă����ŁB
size_t mbstowcs2(wchar_t* dst, const char* src, size_t dst_count)
{
	size_t ret = ::mbstowcs(dst, src, dst_count-1);
	dst[ret] = L'\0';
	return ret;
}
size_t mbstowcs2(wchar_t* pDst, int nDstCount, const char* pSrc, int nSrcCount)
{
	int ret = MultiByteToWideChar(
		CP_SJIS,
		0,
		pSrc,
		nSrcCount,
		pDst,
		nDstCount-1
	);
	pDst[ret] = L'\0';
	return (size_t)ret;
}

// UNICODE��SJIS�B�I�[��'\0'��t���Ă����ŁB
size_t wcstombs2(char* dst, const wchar_t* src, size_t dst_count)
{
	size_t ret = ::wcstombs(dst, src, dst_count-1);
	dst[ret] = '\0';
	return ret;
}

// SJIS��UNICODE�B�߂�l��new[]�Ŋm�ۂ��ĕԂ��B
wchar_t* mbstowcs_new(const char* src)
{
	size_t new_length = mbstowcs(NULL, src, 0);
	wchar_t* ret = new wchar_t[new_length + 1];
	mbstowcs(ret, src, new_length);
	ret[new_length] = L'\0';
	return ret;
}
wchar_t* mbstowcs_new(const char* pSrc, size_t nSrcLen, int* pnDstLen)
{
	// �K�v�ȗ̈�T�C�Y
	int nNewLength = MultiByteToWideChar(
		CP_SJIS,
		0,
		pSrc,
		(int)nSrcLen,
		NULL,
		0
	);
	
	// �m��
	wchar_t* pNew = new wchar_t[nNewLength + 1];

	// �ϊ�
	nNewLength = MultiByteToWideChar(
		CP_SJIS,
		0,
		pSrc,
		(int)nSrcLen,
		pNew,
		nNewLength
	);
	pNew[nNewLength] = L'\0';
	if (pnDstLen) {
		*pnDstLen = nNewLength;
	}
	return pNew;
}

// UNICODE��SJIS�B�߂�l��new[]�Ŋm�ۂ��ĕԂ��B
char* wcstombs_new(const wchar_t* src)
{
	return wcstombs_new(src, wcslen(src));
}
// �߂�l��new[]�Ŋm�ۂ��ĕԂ��B
char* wcstombs_new(const wchar_t* pSrc, size_t nSrcLen)
{
	// �K�v�ȗ̈�T�C�Y
	int nNewLength = WideCharToMultiByte(
		CP_SJIS,
		0,
		pSrc,
		(int)nSrcLen,
		NULL,
		0,
		NULL,
		NULL
	);

	// �m��
	char* pNew = new char[nNewLength + 1];

	// �ϊ�
	nNewLength = WideCharToMultiByte(
		CP_SJIS,
		0,
		pSrc,
		(int)nSrcLen,
		pNew,
		nNewLength,
		NULL,
		NULL
	);
	pNew[nNewLength] = '\0';

	return pNew;
}

// SJIS��UNICODE�B�߂�l��vector�Ƃ��ĕԂ��B
void mbstowcs_vector(const char* src, std::vector<wchar_t>* ret)
{
	mbstowcs_vector(src, strlen(src), ret);
}

// ���߂�lret�ɂ����āAret->size()�������񒷂ł͂Ȃ����Ƃɒ��ӁB�������́A(ret->size()-1)�������񒷂ƂȂ�B
void mbstowcs_vector(const char* pSrc, size_t nSrcLen, std::vector<wchar_t>* ret)
{
	// �K�v�ȗe��
	int nNewLen = MultiByteToWideChar(
		CP_SJIS,
		0,
		pSrc,
		(int)nSrcLen,
		NULL,
		0
	);

	// �m��
	ret->resize(nNewLen + 1);

	// �ϊ�
	nNewLen = MultiByteToWideChar(
		CP_SJIS,
		0,
		pSrc,
		(int)nSrcLen,
		&(*ret)[0],
		nNewLen
	);
	(*ret)[nNewLen] = L'\0';
}


// UNICODE��SJIS�B�߂�l��vector�Ƃ��ĕԂ��B
void wcstombs_vector(const wchar_t* src, std::vector<char>* ret)
{
	wcstombs_vector(src, wcslen(src), ret);
}
void wcstombs_vector(const wchar_t* pSrc, size_t nSrcLen, std::vector<char>* ret)
{
	// �K�v�ȗe��
	int nNewLen = WideCharToMultiByte(
		CP_SJIS,
		0,
		pSrc,
		(int)nSrcLen,
		NULL,
		0,
		NULL,
		NULL
	);

	// �m��
	ret->resize(nNewLen + 1);

	// �ϊ�
	nNewLen = WideCharToMultiByte(
		CP_SJIS,
		0,
		pSrc,
		(int)nSrcLen,
		&(*ret)[0],
		nNewLen,
		NULL,
		NULL
	);
	(*ret)[nNewLen] = '\0';
}

size_t _tcstowcs(wchar_t* wszDst, const TCHAR* tszSrc, size_t nDstCount)
{
	wcsncpy_s(wszDst, nDstCount, tszSrc, _TRUNCATE);
	return wcslen(wszDst);
}
size_t _tcstombs(CHAR*  szDst,  const TCHAR* tszSrc, size_t nDstCount)
{
	return wcstombs2(szDst, tszSrc, nDstCount);
}
size_t _wcstotcs(TCHAR* tszDst, const wchar_t* wszSrc, size_t nDstCount)
{
	wcsncpy_s(tszDst, nDstCount, wszSrc, _TRUNCATE);
	return wcslen(tszDst);
}
size_t _mbstotcs(TCHAR* tszDst, const CHAR*  szSrc,  size_t nDstCount)
{
	return mbstowcs2(tszDst, szSrc, nDstCount);
}
int _tctomb(const TCHAR* p, char* mb)
{
	return wctomb(mb, *p);
}
int _tctowc(const TCHAR* p, wchar_t* wc)
{
	*wc = *p;
	return 1;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ������                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int wmemicmp(const wchar_t* p1, const wchar_t* p2, size_t count)
{
	for (size_t i=0; i<count; ++i) {
		int n = skr_towlower(*p1++) - skr_towlower(*p2++);	// ��ASCII���ϊ�
		if (n != 0) return n;
	}
	return 0;
}

int wmemicmp(const wchar_t* p1, const wchar_t* p2)
{
	return wmemicmp(p1, p2, t_max(wcslen(p1), wcslen(p2)));
}

int wmemicmp_ascii(const wchar_t* p1, const wchar_t* p2, size_t count)
{
	for (size_t i=0; i<count; ++i) {
		int n = my_towlower(*p1++) - my_towlower(*p2++);	// ASCII�̂ݕϊ��i�����j
		if (n != 0) return n;
	}
	return 0;
}


/*!
	�󔒂��܂ރt�@�C�������l�������g�[�N���̕���
	
	�擪�ɂ���A��������؂蕶���͖�������D
	
	@return �g�[�N��
*/
//$ ����������Ԃ�����B�B
namespace {
	template <class T> struct Charset {};
	template <> struct Charset<char>{ static const char QUOT = '"'; };
	template <> struct Charset<wchar_t>{ static const wchar_t QUOT = L'"'; };
}
template <class CHAR_TYPE>
CHAR_TYPE* my_strtok(
	CHAR_TYPE*			pBuffer,	// [in] ������o�b�t�@(�I�[�����邱��)
	size_t				nLen,		// [in] ������̒���
	size_t*				pnOffset,	// [in/out] �I�t�Z�b�g
	const CHAR_TYPE*	pDelimiter	// [in] ��؂蕶��
)
{
	size_t i = *pnOffset;
	CHAR_TYPE* p;

	do {
		bool bFlag = false;	// �_�u���R�[�e�[�V�����̒����H
		if (i >= nLen) return NULL;
		p = &pBuffer[i];
		for (; i<nLen; ++i) {
			if (pBuffer[i] == Charset<CHAR_TYPE>::QUOT) bFlag = ! bFlag;
			if (!bFlag) {
				if (auto_strchr(pDelimiter, pBuffer[i])) {
					pBuffer[i++] = _T('\0');
					break;
				}
			}
		}
		*pnOffset = i;
	}while (!*p);	// ��̃g�[�N���Ȃ玟��T��
	return p;
}
// �C���X�^���X��
template char* my_strtok(char*, size_t, size_t*, const char*);
template wchar_t* my_strtok(wchar_t*, size_t, size_t*, const wchar_t*);

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#ifdef MY_ICMP_MBS
int my_mbtoupper2(int c);
int my_mbtolower2(int c);
int my_mbisalpha2(int c);
#endif  // MY_ICMP_MBS

#ifdef MY_ICMP_MBS
/*!	�S�p�A���t�@�x�b�g�̂Q�����ڂ�啶���ɕϊ�����B
	@param c [in] �ϊ����镶���R�[�h

	@note
		0x8260 - 0x8279 : �`...�y
		0x8281 - 0x829a : ��...��

	@return �ϊ����ꂽ�����R�[�h
*/
int my_mbtoupper2(int c)
{
	if (c >= 0x81 && c <= 0x9a) return c - (0x81 - 0x60);
	return c;
}
#endif  /* MY_ICMP_MBS */


#ifdef MY_ICMP_MBS
/*!	�S�p�A���t�@�x�b�g�̂Q�����ڂ��������ɕϊ�����B
	@param c [in] �ϊ����镶���R�[�h

	@return �ϊ����ꂽ�����R�[�h
*/
int my_mbtolower2(int c)
{
	if (c >= 0x60 && c <= 0x79) return c + (0x81 - 0x60);
	return c;
}
#endif  /* MY_ICMP_MBS */


#ifdef MY_ICMP_MBS
/*!	�S�p�A���t�@�x�b�g�̂Q�����ڂ����ׂ�B
	@param c [in] �������镶���R�[�h

	@retval 1	�S�p�A���t�@�x�b�g�Q�o�C�g�ڂł���
	@retval 0	������
*/
int my_mbisalpha2(int c)
{
	if ((c >= 0x60 && c <= 0x79) || (c >= 0x81 && c <= 0x9a)) return 1;
	return 0;
}
#endif  /* MY_ICMP_MBS */


/*!	�啶���������𓯈ꎋ���镶���񒷂�������r������B
	@param s1   [in] ������P
	@param s2   [in] ������Q
	@param n    [in] ������
	@param dcount  [in] �X�e�b�v�l (1=strnicmp,memicmp, 0=stricmp)
	@param flag [in] ������I�[�`�F�b�N (true=stricmp,strnicmp, false=memicmp)

	@retval 0	��v
 */
int __cdecl my_internal_icmp(const char* s1, const char* s2, unsigned int n, unsigned int dcount, bool flag)
{
	int 	c1, c2;
	bool	prev1, prev2; // �O�̕����� SJIS�̂P�o�C�g�ڂ�
#ifdef MY_ICMP_MBS
	bool	mba1, mba2;
#endif  // MY_ICMP_MBS

	unsigned char* p1 = (unsigned char*) s1;
	unsigned char* p2 = (unsigned char*) s2;
	prev1 = prev2 = false;
#ifdef MY_ICMP_MBS
	mba1 = mba2 = false;
#endif  // MY_ICMP_MBS

	// �w�蒷�����J��Ԃ�
	for (unsigned int i=n; i>0; i-=dcount) {
		// ��r�ΏۂƂȂ镶�����擾����
//		c1 = c1_lo = c1_up = (int)((unsigned int)*p1);
//		c2 = c2_lo = c2_up = (int)((unsigned int)*p2);
		c1 = (int)((unsigned int)*p1);
		c2 = (int)((unsigned int)*p2);

		// �����P�̓��{��`�F�b�N���s����r�p�̑啶�����������Z�b�g����
		if (prev1) {	// �O�̕��������{��P�o�C�g��
			// ����͓��{��Q�o�C�g�ڂȂ̂ŕϊ����Ȃ�
			prev1 = false;
#ifdef MY_ICMP_MBS
			// �S�p�����̃A���t�@�x�b�g
			if (mba1) {
				mba1 = false;
				if (my_mbisalpha2(c1)) {
					c1 = my_mbtoupper2(c1);
				}
			}
#endif  // MY_ICMP_MBS
		}else if (my_iskanji1(c1)) {
			// ����͓��{��P�o�C�g�ڂȂ̂ŕϊ����Ȃ�
			prev1 = true;
#ifdef MY_ICMP_MBS
			if (c1 == 0x82) mba1 = true;
#endif  // MY_ICMP_MBS
		}else {
			c1 = my_toupper(c1);
		}

		// �����Q�̓��{��`�F�b�N���s����r�p�̑啶�����������Z�b�g����
		if (prev2) {	// �O�̕��������{��P�o�C�g��
			// ����͓��{��Q�o�C�g�ڂȂ̂ŕϊ����Ȃ�
			prev2 = false;
#ifdef MY_ICMP_MBS
			// �S�p�����̃A���t�@�x�b�g
			if (mba2) {
				mba2 = false;
				if (my_mbisalpha2(c2)) {
					c2 = my_mbtoupper2(c2);
				}
			}
#endif  // MY_ICMP_MBS
		}else if (my_iskanji1(c2)) {
			// ����͓��{��P�o�C�g�ڂȂ̂ŕϊ����Ȃ�
			prev2 = true;
#ifdef MY_ICMP_MBS
			if (c2 == 0x82) mba2 = true;
#endif  // MY_ICMP_MBS
		}else {
			c2 = my_toupper(c2);
		}

		// ��r����
//		if ((c1_lo - c2_lo) && (c1_up - c2_up)) return c1 - c2;	// �߂�l�͌��̕����̍�
		if (c1 - c2) return c1 - c2;	// �߂�l�͑啶���ɕϊ����������̍�

		if (flag) {
			// ������̏I�[�ɒB���������ׂ�
			if (!c1) return 0;
		}
		// �|�C���^��i�߂�
		++p1;
		++p2;
	}
	return 0;
}

// skr_towupper() / skr_tolower()
//
// BugReport/64: towupper(c) �ɂ���� U+00e0-U+00fc �� U+0020 �����ꎋ�������̑΍�
// VC �̃����^�C���� c < 256 �̏����ł͂Ȃ��� locale �ɑΉ����� "ANSI �n��" �ϊ��e�[�u���������s���Ă���͗l
// �iUnicode �n�ϊ��֐��Ȃ̂� locale �� "Japanese" ���� c < 256 �͈̔͂ł� SJIS �p�炵���ϊ��e�[�u�����g����j
// ����ł͓s���������̂� c < 256 �͈͂̕ϊ��� "English"(Windows-1252) locale �𗘗p����B
//   �EUnicode �̍ŏ��� 256 �̕����ʒu�� Windows-1252 �̐e�ʂ� ISO-8859-1 �R���B
//   �E����� 0x80-0x9F �̋�ԂŁAWindows-1252 �ł͐}�`�����AISO-8859-1(Unicode) �ł͐��䕶���B
// �� �����^�C���� towupper(c)/tolower(c) ���������҂��铮��ɂȂ����Ƃ��Ă����̕��@���g�������Ė�薳���͂�
int skr_towupper(int c)
{
#if defined(_MSC_VER) && _MSC_VER >= 1400 // VS2005�ȍ~�Ȃ�
	static wchar_t szMap[256];	// c < 256 �p�̕ϊ��e�[�u��
	static bool bInit = false;
	if (!bInit) {
		int i;
		_locale_t locale = _create_locale(LC_CTYPE, "English");
		for (i=0; i<0x80; ++i) szMap[i] = (wchar_t)my_towupper(i);	// ���O�ŕϊ�
		for (; i<0xA0; ++i) szMap[i] = (wchar_t)i;						// ���ϊ��i����R�[�h���j
		for (; i<255; ++i) szMap[i] = _towupper_l((wchar_t)i, locale);	// "English"locale�ŕϊ�
		szMap[255] = 0x0178;	// Windows-1252 ���� 0x9f(���䕶����) �Ƀ}�b�v���Ă��܂��̂�
		_free_locale(locale);
		bInit = true;
	}

	if (c < 256) return szMap[c];
#endif
	return towupper((wchar_t)c);
}

int skr_towlower(int c)
{
#if defined(_MSC_VER) && _MSC_VER >= 1400 // VS2005�ȍ~�Ȃ�
	static wchar_t szMap[256];	// c < 256 �p�̕ϊ��e�[�u��
	static bool bInit = false;
	if (!bInit) {
		int i;
		_locale_t locale = _create_locale(LC_CTYPE, "English");
		for (i=0; i<0x80; ++i) szMap[i] = (wchar_t)my_towlower(i);	// ���O�ŕϊ�
		for (; i<0xA0; ++i) szMap[i] = (wchar_t)i;						// ���ϊ��i����R�[�h���j
		for (; i<256; ++i) szMap[i] = _towlower_l((wchar_t)i, locale);	// "English"locale�ŕϊ�
		_free_locale(locale);
		bInit = true;
	}

	if (c < 256) return szMap[c];
#endif
	return towlower((wchar_t)c);
}

