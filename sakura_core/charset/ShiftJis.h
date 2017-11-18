#pragma once

#include "CodeBase.h"
#include "charset/codeutil.h"

struct CommonSetting_StatusBar;

class ShiftJis : public CodeBase {

public:
	// CodeBase�C���^�[�t�F�[�X
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst){ return SJISToUnicode(src, pDst); }	// ����R�[�h �� UNICODE    �ϊ�
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst){ return UnicodeToSJIS(src, pDst); }	// UNICODE    �� ����R�[�h �ϊ�
	CodeConvertResult UnicodeToHex(const wchar_t* cSrc, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar);	// UNICODE �� Hex �ϊ�

public:
	// ����
	static CodeConvertResult SJISToUnicode(const Memory& src, NativeW* pDstMem);		// SJIS      �� Unicode�R�[�h�ϊ�
	static CodeConvertResult UnicodeToSJIS(const NativeW& src, Memory* pDstMem);		// Unicode   �� SJIS�R�[�h�ϊ�
	static size_t GetSizeOfChar(const char* pData, size_t nDataLen, size_t nIdx); // �w�肵���ʒu�̕��������o�C�g��������Ԃ�

protected:
	// ����
	inline static size_t _SjisToUni_char(const unsigned char*, unsigned short*, const ECharSet, bool* pbError);
	static size_t SjisToUni(const char*, const size_t, wchar_t*, bool* pbError);
	inline static size_t _UniToSjis_char(const unsigned short*, unsigned char*, const ECharSet, bool* pbError);
	static size_t UniToSjis(const wchar_t*, const size_t, char*, bool* pbError);
};


/*!
	SJIS �̑S�p�ꕶ���܂��͔��p�ꕶ����Unicode�ւ̕ϊ�

	eCharset �� CHARSET_JIS_ZENKAKU �܂��� CHARSET_JIS_HANKATA �B

	�������̂��߁A�C�����C����
*/
inline size_t ShiftJis::_SjisToUni_char(const unsigned char* pSrc, unsigned short* pDst, const ECharSet eCharset, bool* pbError)
{
	size_t nret;
	bool berror = false;

	switch (eCharset) {
	case CHARSET_JIS_HANKATA:
		// ���p�J�^�J�i������
		// �G���[�͋N����Ȃ��B
		nret = MyMultiByteToWideChar_JP(pSrc, 1, pDst);
		// �ی�R�[�h
		if (nret < 1) {
			nret = 1;
		}
		break;
	case CHARSET_JIS_ZENKAKU:
		// �S�p����������
		nret = MyMultiByteToWideChar_JP(pSrc, 2, pDst);
		if (nret < 1) {	// SJIS -> Unicode �ϊ��Ɏ��s
			nret = BinToText(pSrc, 2, pDst);
		}
		break;
	default:
		// �v���I�G���[����R�[�h
		berror = true;
		pDst[0] = L'?';
		nret = 1;
	}

	if (pbError) {
		*pbError = berror;
	}

	return nret;
}




/*!
	UNICODE -> SJIS �ꕶ���ϊ�

	eCharset �� CHARSET_UNI_NORMAL �܂��� CHARSET_UNI_SURROG�B

	�������̂��߁A�C�����C����
*/
inline size_t ShiftJis::_UniToSjis_char(const unsigned short* pSrc, unsigned char* pDst, const ECharSet eCharset, bool* pbError)
{
	size_t nret;
	bool berror = false;

	if (eCharset == CHARSET_UNI_NORMAL) {
		nret = MyWideCharToMultiByte_JP(pSrc, 1, pDst);
		if (nret < 1) {
			// Uni -> SJIS �ϊ��Ɏ��s
			berror = true;
			pDst[0] = '?';
			nret = 1;
		}
	}else if (eCharset == CHARSET_UNI_SURROG) {
		// �T���Q�[�g�y�A�� SJIS �ɕϊ��ł��Ȃ��B
		berror = true;
		pDst[0] = '?';
		nret = 1;
	}else {
		// �ی�R�[�h
		berror = true;
		pDst[0] = '?';
		nret = 1;
	}

	if (pbError) {
		*pbError = berror;
	}

	return nret;
}

