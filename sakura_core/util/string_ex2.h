#pragma once

class Eol;

// Aug. 16, 2007 kobake
wchar_t* wcsncpy_ex(wchar_t* dst, size_t dst_count, const wchar_t* src, size_t src_count);
wchar_t* wcs_pushW(wchar_t* dst, size_t dst_count, const wchar_t* src, size_t src_count);
wchar_t* wcs_pushW(wchar_t* dst, size_t dst_count, const wchar_t* src);
wchar_t* wcs_pushA(wchar_t* dst, size_t dst_count, const char* src, size_t src_count);
wchar_t* wcs_pushA(wchar_t* dst, size_t dst_count, const char* src);
#define wcs_pushT wcs_pushW

int AddLastChar(TCHAR*, size_t, TCHAR); // 2003.06.24 Moca �Ō�̕������w�肳�ꂽ�����łȂ��Ƃ��͕t������
size_t LimitStringLength(const wchar_t*, size_t, size_t, NativeW&); // �f�[�^���w��u�������v�ȓ��ɐ؂�l�߂�

const char* GetNextLimitedLengthText(const char*, size_t, size_t, size_t*, size_t*); // �w�蒷�ȉ��̃e�L�X�g�ɐ؂蕪����
const char* GetNextLine(const char*, size_t, size_t*, size_t*, Eol*); // CR0LF0,CRLF,LF,CR�ŋ�؂���u�s�v��Ԃ��B���s�R�[�h�͍s���ɉ����Ȃ�
const wchar_t* GetNextLineW(const wchar_t*, size_t, size_t*, size_t*, Eol*, bool); // GetNextLine��wchar_t��
//wchar_t* GetNextLineWB(const wchar_t*, int, int*, int*, Eol*); // GetNextLine��wchar_t��(�r�b�N�G���f�B�A���p)  // ���g�p
void GetLineColumn(const wchar_t*, int*, int*);

size_t cescape(const TCHAR* org, TCHAR* buf, TCHAR cesc, TCHAR cwith);

/*!	&�̓�d��
	���j���[�Ɋ܂܂��&��&&�ɒu��������
	@author genta
	@date 2002/01/30 cescape�Ɋg�����C
	@date 2004/06/19 genta Generic mapping
*/
inline void dupamp(const TCHAR* org, TCHAR* out)
{ cescape(org, out, _T('&'), _T('&')); }


/*
	scanf�I���S�X�L����

	�g�p��:
		int a[3];
		scan_ints("1,23,4,5", "%d,%d,%d", a);
		// ����: a[0]=1, a[1]=23, a[2]=4 �ƂȂ�B
*/
int scan_ints(
	const wchar_t*	pszData,	// [in]  �f�[�^������
	const wchar_t*	pszFormat,	// [in]  �f�[�^�t�H�[�}�b�g
	int*			anBuf		// [out] �擾�������l (�v�f���͍ő�32�܂�)
);

