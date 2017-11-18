#pragma once

#include "CodeBase.h"
#include "charset/codeutil.h"

struct CommonSetting_StatusBar;

class Utf8 : public CodeBase {
public:

	// CodeBase�C���^�[�t�F�[�X
	CodeConvertResult CodeToUnicode(const Memory& src, NativeW* pDst) {	// ����R�[�h �� UNICODE    �ϊ�
		return UTF8ToUnicode(src, pDst);
	}
	CodeConvertResult UnicodeToCode(const NativeW& src, Memory* pDst) {	// UNICODE    �� ����R�[�h �ϊ�
		return UnicodeToUTF8(src, pDst);
	}
	void GetBom(Memory* pMemBom);										// BOM�f�[�^�擾
	void GetEol(Memory* pMemEol, EolType eolType);
	CodeConvertResult _UnicodeToHex(const wchar_t* src, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar, const bool CESU8Mode);			// UNICODE �� Hex �ϊ�
	CodeConvertResult UnicodeToHex(const wchar_t* ps, size_t nsl, TCHAR* pd, const CommonSetting_StatusBar* psStatusbar){ return _UnicodeToHex(ps, nsl, pd, psStatusbar, false); }

public:
	// UTF-8 / CESU-8 <-> Unicode�R�[�h�ϊ�
	static CodeConvertResult _UTF8ToUnicode( const Memory& src, NativeW* pDstMem, bool bCESU8Mode );
	static CodeConvertResult _UnicodeToUTF8( const NativeW& src, Memory* pDstMem, bool bCESU8Mode );
	inline static CodeConvertResult UTF8ToUnicode( const Memory& src, NativeW* pDst ){ return _UTF8ToUnicode(src, pDst, false); }	// UTF-8 -> Unicode�R�[�h�ϊ�
	inline static CodeConvertResult CESU8ToUnicode( const Memory& src, NativeW* pDst ){ return _UTF8ToUnicode(src, pDst, true); }	// CESU-8 -> Unicode�R�[�h�ϊ�
	inline static CodeConvertResult UnicodeToUTF8( const NativeW& src, Memory* pDst ){ return  _UnicodeToUTF8(src, pDst, false); }	// Unicode �� UTF-8�R�[�h�ϊ�
	inline static CodeConvertResult UnicodeToCESU8( const NativeW& src, Memory* pDst ){ return _UnicodeToUTF8(src, pDst, true); }	// Unicode �� CESU-8�R�[�h�ϊ�

protected:
	// �ϊ��̎���
	inline static size_t _Utf8ToUni_char(const unsigned char*, const size_t, unsigned short*, bool bCESU8Mode);
	static size_t Utf8ToUni(const char*, const size_t, wchar_t*, bool bCESU8Mode);
	inline static size_t _UniToUtf8_char(const unsigned short*, const size_t, unsigned char*, const bool bCSU8Mode);
	static size_t UniToUtf8(const wchar_t*, const size_t, char*, bool* pbError, bool bCSU8Mode);
};

/*!
	UTF-8 �̈ꕶ���ϊ�

	UTF-8 �� CESU-8 �Ƃ��ꍇ�������āA���ꂼ��ϊ�����

	�������̂��߁A�C�����C����

*/
inline size_t Utf8::_Utf8ToUni_char( const unsigned char* pSrc, const size_t nSrcLen, unsigned short* pDst, bool bCESUMode )
{
	size_t nret;

	if (nSrcLen < 1) {
		return 0;
	}

	if (!bCESUMode) {
		// UTF-8 �̏���
		if (nSrcLen < 4) {
			pDst[0] = static_cast<unsigned short>(DecodeUtf8(pSrc, nSrcLen) & 0x0000ffff);
			nret = 1;
		}else if (nSrcLen == 4) {
			// UTF-8 �T���Q�[�g�̈�̏���
			wchar32_t wc32 = DecodeUtf8(pSrc, 4);
			EncodeUtf16Surrog(wc32, pDst);
			nret = 2;
		}else {
			// �ی�R�[�h
			pDst[0] = L'?';
			nret = 1;
		}
	}else {
		// CESU-8 �̏���
		if (nSrcLen < 4) {
			pDst[0] = static_cast<unsigned short>(DecodeUtf8(pSrc, nSrcLen) & 0x0000ffff);
			nret = 1;
		}else if (nSrcLen == 6) {
			// CESU-8 �T���Q�[�g�̈�̏���
			pDst[0] = static_cast<unsigned short>(DecodeUtf8(&pSrc[0], 3) & 0x0000ffff);
			pDst[1] = static_cast<unsigned short>(DecodeUtf8(&pSrc[3], 3) & 0x0000ffff);
			nret = 2;
		}else {
			// �ی�R�[�h
			pDst[0] = L'?';
			nret = 1;
		}
	}

	return nret;
}



/*!
	Unicode -> UTF-8 �̈ꕶ���ϊ�

	nSrcLen �� 1 �܂��� 2

	�������̂��߁A�C�����C����
*/
inline size_t Utf8::_UniToUtf8_char( const unsigned short* pSrc, const size_t nSrcLen, unsigned char* pDst, bool bCESU8Mode )
{
	size_t nret;

	if (nSrcLen < 1) {
		return 0;
	}

	if (!bCESU8Mode) {
		// UTF-8 �̏���
		wchar32_t wc32;
		if (nSrcLen == 2) {
			wc32 = DecodeUtf16Surrog(pSrc[0], pSrc[1]);
		}else if (nSrcLen == 1) {	// nSrcLen == 1
			wc32 = pSrc[0];
		}else {
			wc32 = L'?';
		}
		nret = EncodeUtf8(wc32, &pDst[0]);
	}else {
		// CESU-8 �̏���
		int nclen = 0;
		nclen += EncodeUtf8(pSrc[0], &pDst[0]);
		if (nSrcLen == 2) {
			nclen += EncodeUtf8(pSrc[1], &pDst[nclen]);
		}else {
			;
		}
		nret = nclen;
	}

	return nret;
}

