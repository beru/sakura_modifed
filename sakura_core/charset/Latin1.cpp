/*!	@file
	@brief Latin1 (Latin1, ����, Windows-1252, Windows Codepage 1252 West European) �Ή��N���X

	@author Uchi
	@date 20010/03/20 �V�K�쐬
*/
/*
	Copyright (C) 20010, Uchi

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
#include "StdAfx.h"
#include "Latin1.h"
#include "charset/charcode.h"
#include "charset/codecheck.h"
#include "Eol.h"
//#include "env/ShareData.h"
#include "env/CommonSetting.h"



// �w�肵���ʒu�̕��������o�C�g��������Ԃ�
/*!
	@param[in] pData �ʒu�����߂���������̐擪
	@param[in] nDataLen ������
	@param[in] nIdx �ʒu(0�I���W��)
	@retval 1  1�o�C�g����
	@retval 0  �G���[

	@date 2010/3/20 Uchi �쐬

	�G���[�łȂ����1��Ԃ�
*/
size_t Latin1::GetSizeOfChar(const char* pData, size_t nDataLen, size_t nIdx)
{
	if (nIdx >= nDataLen) {
		return 0;
	}
	return 1;
}




/*!
	Latin1 �� Unicode �ϊ�
*/
int Latin1::Latin1ToUni(const char* pSrc, const int nSrcLen, wchar_t* pDst, bool* pbError)
{
	int nret;
	const unsigned char *pr, *pr_end;
	unsigned short* pw;

	if (pbError) {
		*pbError = false;
	}
	if (nSrcLen < 1) {
		return 0;
	}

	pr = reinterpret_cast<const unsigned char*>(pSrc);
	pr_end = reinterpret_cast<const unsigned char*>(pSrc + nSrcLen);
	pw = reinterpret_cast<unsigned short*>(pDst);

	for (; pr<pr_end; ++pr) {
		if (*pr >= 0x80 && *pr <= 0x9f) {
			// Windows �g����
			nret = ::MultiByteToWideChar(1252, 0, reinterpret_cast<const char*>(pr), 1, reinterpret_cast<wchar_t*>(pw), 4);
			if (nret == 0) {
				*pw = static_cast<unsigned short>(*pr);
			}
			++pw;
		}else {
			*pw++ = static_cast<unsigned short>(*pr);
		}
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}



// �R�[�h�ϊ� Latin1��Unicode
CodeConvertResult Latin1::Latin1ToUnicode( const Memory& src, NativeW* pDstMem )
{
	// �\�[�X�擾
	size_t nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( src.GetRawPtr(&nSrcLen) );
	if (nSrcLen == 0) {
		pDstMem->Clear();
		return CodeConvertResult::Complete;
	}

	// �ϊ���o�b�t�@�T�C�Y��ݒ肵�ă������̈�m��
	std::vector<wchar_t> dst(nSrcLen);
	wchar_t* pDst = &dst[0];

	// �ϊ�
	bool bError;
	int nDstLen = Latin1ToUni(pSrc, nSrcLen, pDst, &bError);

	// pDstMem���X�V
	pDstMem->_GetMemory()->SetRawDataHoldBuffer( pDst, nDstLen*sizeof(wchar_t) );

	if (!bError) {
		return CodeConvertResult::Complete;
	}else {
		return CodeConvertResult::LoseSome;
	}
}


/*
	Unicode -> Latin1
*/
int Latin1::UniToLatin1(const wchar_t* pSrc, const int nSrcLen, char* pDst, bool* pbError)
{
	int nclen;
	const unsigned short *pr, *pr_end;
	unsigned char* pw;
	ECharSet echarset;
	bool berror = false, berror_tmp;

	if (nSrcLen < 1) {
		if (pbError) {
			*pbError = false;
		}
		return 0;
	}

	pr = reinterpret_cast<const unsigned short*>(pSrc);
	pr_end = reinterpret_cast<const unsigned short*>(pSrc + nSrcLen);
	pw = reinterpret_cast<unsigned char*>(pDst);

	while ((nclen = CheckUtf16leChar(reinterpret_cast<const wchar_t*>(pr), pr_end - pr, &echarset, 0)) > 0) {
		// �ی�R�[�h
		switch (echarset) {
		case CHARSET_UNI_NORMAL:
			nclen = 1;
			break;
		case CHARSET_UNI_SURROG:
			nclen = 2;
			break;
		default:
			echarset = CHARSET_BINARY;
			nclen = 1;
		}
		if (echarset != CHARSET_BINARY) {
			pw += _UniToLatin1_char(pr, pw, echarset, &berror_tmp);
			if (berror_tmp) {
				berror = true;
			}
			pr += nclen;
		}else {
			if (nclen == 1 && IsBinaryOnSurrogate(static_cast<wchar_t>(*pr))) {
				*pw = static_cast<unsigned char>(TextToBin(*pr) & 0x000000ff);
				++pw;
			}else {
				berror = true;
				*pw = '?';
				++pw;
			}
			++pr;
		}
	}

	if (pbError) {
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned char*>(pDst);
}




// �R�[�h�ϊ� Unicode��Latin1
CodeConvertResult Latin1::UnicodeToLatin1( const NativeW& src, Memory* pDstMem )
{
	// �\�[�X�擾
	const wchar_t* pSrc = src.GetStringPtr();
	int nSrcLen = src.GetStringLength();
	if (nSrcLen == 0) {
		pDstMem->Clear();
		return CodeConvertResult::Complete;
	}

	// �ϊ���o�b�t�@�T�C�Y��ݒ肵�ăo�b�t�@���m��
	std::vector<char> dst(nSrcLen * 2);
	char* pDst = &dst[0];

	// �ϊ�
	bool berror;
	int nDstLen = UniToLatin1(pSrc, nSrcLen, pDst, &berror);

	// pDstMem���X�V
	pDstMem->SetRawDataHoldBuffer( pDst, nDstLen );

	// ����
	if (berror) {
		return CodeConvertResult::LoseSome;
	}else {
		return CodeConvertResult::Complete;
	}
}


// �����R�[�h�\���p	UNICODE �� Hex �ϊ�	2008/6/9 Uchi
CodeConvertResult Latin1::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar)
{

	// 2008/6/21 Uchi
	if (psStatusbar->bDispUniInSjis) {
		// Unicode�ŕ\��
		return CodeBase::UnicodeToHex(cSrc, iSLen, pDst, psStatusbar);
	}

	NativeW cCharBuffer;
	cCharBuffer.SetString(cSrc, 1);

	bool bbinary = false;
	if (IsBinaryOnSurrogate(cSrc[0])) {
		bbinary = true;
	}

	// Latin1 �ϊ�
	CodeConvertResult res = UnicodeToLatin1(cCharBuffer, cCharBuffer._GetMemory());
	if (res != CodeConvertResult::Complete) {
		return CodeConvertResult::LoseSome;
	}

	// Hex�ϊ�
	unsigned char* ps = reinterpret_cast<unsigned char*>( cCharBuffer._GetMemory()->GetRawPtr() );
	TCHAR* pd = pDst;
	if (!bbinary) {
		for (int i=cCharBuffer._GetMemory()->GetRawLength(); i>0; --i, ++ps, pd+=2) {
			auto_sprintf(pd, _T("%02x"), *ps);
		}
	}else {
		auto_sprintf(pd, _T("?%02x"), *ps);
	}

	return CodeConvertResult::Complete;
}
