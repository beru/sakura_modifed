#pragma once

#include "CodeBase.h"

// Latin1 (Latin1, ����, Windows-1252, Windows Codepage 1252 West European) �Ή��N���X
class Latin1 : public CodeBase {

public:
	// CodeBase�C���^�[�t�F�[�X
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst){ return Latin1ToUnicode(src, pDst); }	// ����R�[�h �� UNICODE    �ϊ�
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst){ return UnicodeToLatin1(src, pDst); }	// UNICODE    �� ����R�[�h �ϊ�
	CodeConvertResult UnicodeToHex(const wchar_t* pSrc, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar);	// UNICODE �� Hex �ϊ�

public:
	// ����
	static CodeConvertResult Latin1ToUnicode(const Memory& src, NativeW* pDstMem);		// Latin1   �� Unicode�R�[�h�ϊ�
	static CodeConvertResult UnicodeToLatin1(const NativeW& src, Memory* pDstMem);		// Unicode  �� Latin1�R�[�h�ϊ�
	static size_t GetSizeOfChar(const char* pData, size_t nDataLen, size_t nIdx); // �w�肵���ʒu�̕��������o�C�g��������Ԃ�

protected:
	// ����
	static size_t Latin1ToUni(const char*, const size_t, wchar_t*, bool* pbError);
	inline static int _UniToLatin1_char(const unsigned short*, unsigned char*, const ECharSet, bool* pbError);
	static size_t UniToLatin1(const wchar_t*, const size_t, char*, bool* pbError);
};

/*!
	UNICODE -> Latin1 �ꕶ���ϊ�

	eCharset �� CHARSET_UNI_NORMAL �܂��� CHARSET_UNI_SURROG�B

	�������̂��߁A�C�����C����
*/
inline int Latin1::_UniToLatin1_char(const unsigned short* pSrc, unsigned char* pDst, const ECharSet eCharset, bool* pbError)
{
	int nret;
	bool berror = false;
	BOOL blost;

	if (eCharset == CHARSET_UNI_NORMAL) {
		if ((pSrc[0] >= 0 && pSrc[0] <= 0x7f) || (pSrc[0] >= 0xa0 && pSrc[0] <= 0xff)) {
			// ISO 58859-1�͈̔�
			pDst[0] = (unsigned char)pSrc[0];
			nret = 1;
		} else {
			// ISO 8859-1�ȊO
			nret = ::WideCharToMultiByte(1252, 0, reinterpret_cast<const wchar_t*>(pSrc), 1, reinterpret_cast<char*>(pDst), 4, NULL, &blost);
			if (blost != FALSE) {
				// Uni -> Latin1 �ϊ��Ɏ��s
				berror = true;
				pDst[0] = '?';
				nret = 1;
			}
		}
	}else if (eCharset == CHARSET_UNI_SURROG) {
		// �T���Q�[�g�y�A�� Latin1 �ɕϊ��ł��Ȃ��B
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

