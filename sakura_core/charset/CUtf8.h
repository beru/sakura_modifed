/*
	Copyright (C) 2008, kobake

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

#include "CCodeBase.h"
#include "charset/codeutil.h"

struct CommonSetting_Statusbar;

class Utf8 : public CodeBase {
public:

	// CodeBase�C���^�[�t�F�[�X
	CodeConvertResult CodeToUnicode(const Memory& cSrc, NativeW* pDst) {	//!< ����R�[�h �� UNICODE    �ϊ�
		return UTF8ToUnicode(cSrc, pDst);
	}
	CodeConvertResult UnicodeToCode(const NativeW& cSrc, Memory* pDst) {	//!< UNICODE    �� ����R�[�h �ϊ�
		return UnicodeToUTF8(cSrc, pDst);
	}
	void GetBom(Memory* pcmemBom);																			//!< BOM�f�[�^�擾
	void GetEol(Memory* pcmemEol, EolType eEolType);
	CodeConvertResult _UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar, const bool CESU8Mode);			//!< UNICODE �� Hex �ϊ�
	CodeConvertResult UnicodeToHex(const wchar_t* ps, const int nsl, TCHAR* pd, const CommonSetting_Statusbar* psStatusbar){ return _UnicodeToHex(ps, nsl, pd, psStatusbar, false); }

public:
	// UTF-8 / CESU-8 <-> Unicode�R�[�h�ϊ�
	// 2007.08.13 kobake �ǉ�
	// 2009.01.08        CESU-8 �ɑΉ�
	static CodeConvertResult _UTF8ToUnicode( const Memory& cSrc, NativeW* pDstMem, bool bCESU8Mode );
	static CodeConvertResult _UnicodeToUTF8( const NativeW& cSrc, Memory* pDstMem, bool bCESU8Mode );
	inline static CodeConvertResult UTF8ToUnicode( const Memory& cSrc, NativeW* pDst ){ return _UTF8ToUnicode(cSrc, pDst, false); }	// UTF-8 -> Unicode�R�[�h�ϊ�
	inline static CodeConvertResult CESU8ToUnicode( const Memory& cSrc, NativeW* pDst ){ return _UTF8ToUnicode(cSrc, pDst, true); }	// CESU-8 -> Unicode�R�[�h�ϊ�
	inline static CodeConvertResult UnicodeToUTF8( const NativeW& cSrc, Memory* pDst ){ return  _UnicodeToUTF8(cSrc, pDst, false); }	// Unicode �� UTF-8�R�[�h�ϊ�
	inline static CodeConvertResult UnicodeToCESU8( const NativeW& cSrc, Memory* pDst ){ return _UnicodeToUTF8(cSrc, pDst, true); }	// Unicode �� CESU-8�R�[�h�ϊ�

protected:
	// �ϊ��̎���
	// 2008.11.10 �ϊ����W�b�N����������
	inline static int _Utf8ToUni_char(const unsigned char*, const int, unsigned short*, bool bCESU8Mode);
	static int Utf8ToUni(const char*, const int, wchar_t*, bool bCESU8Mode);
	inline static int _UniToUtf8_char(const unsigned short*, const int, unsigned char*, const bool bCSU8Mode);
	static int UniToUtf8(const wchar_t*, const int, char*, bool* pbError, bool bCSU8Mode);
};

/*!
	UTF-8 �̈ꕶ���ϊ�

	UTF-8 �� CESU-8 �Ƃ��ꍇ�������āA���ꂼ��ϊ�����

	�������̂��߁A�C�����C����

*/
inline int Utf8::_Utf8ToUni_char( const unsigned char* pSrc, const int nSrcLen, unsigned short* pDst, bool bCESUMode )
{
	int nret;

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
inline int Utf8::_UniToUtf8_char( const unsigned short* pSrc, const int nSrcLen, unsigned char* pDst, bool bCESU8Mode )
{
	int nret;

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

