#include "StdAfx.h"
#include "ShiftJis.h"
#include "charset/charcode.h"
#include "charset/codecheck.h"

// ��ˑ�����
#include "env/ShareData.h"
#include "env/DllSharedData.h"


// �w�肵���ʒu�̕��������o�C�g��������Ԃ�
/*!
	@param[in] pData �ʒu�����߂���������̐擪
	@param[in] nDataLen ������
	@param[in] nIdx �ʒu(0�I���W��)
	@retval 1  1�o�C�g����
	@retval 2  2�o�C�g����
	@retval 0  �G���[

	@date 2005-09-02 D.S.Koba �쐬

	@note nIdx�͗\�ߕ����̐擪�ʒu�Ƃ킩���Ă��Ȃ���΂Ȃ�Ȃ��D
	2�o�C�g������2�o�C�g�ڂ�nIdx�ɗ^����Ɛ��������ʂ������Ȃ��D
*/
int ShiftJis::GetSizeOfChar(const char* pData, int nDataLen, int nIdx)
{
	if (nIdx >= nDataLen) {
		return 0;
	}else if (nIdx == (nDataLen - 1)) {
		return 1;
	}
	
	if (_IS_SJIS_1(reinterpret_cast<const unsigned char*>(pData)[nIdx])
		&& _IS_SJIS_2(reinterpret_cast<const unsigned char*>(pData)[nIdx + 1])
	) {
		return 2;
	}
	return 1;
}


/*!
	SJIS �� Unicode �ϊ�
*/
int ShiftJis::SjisToUni(
	const char* __restrict pSrc, const int nSrcLen,
	wchar_t* __restrict pDst, bool* pbError
	)
{
	ECharSet echarset;
	int nclen;
	const unsigned char *pr, *pr_end;
	unsigned short* pw;
	bool berror_tmp, berror = false;

	if (nSrcLen < 1) {
		if (pbError) {
			*pbError = false;
		}
		return 0;
	}

	pr = reinterpret_cast<const unsigned char*>(pSrc);
	pr_end = reinterpret_cast<const unsigned char*>(pSrc + nSrcLen);
	pw = reinterpret_cast<unsigned short*>(pDst);

	for (; (nclen=CheckSjisChar(reinterpret_cast<const char*>(pr), pr_end - pr, &echarset))!=0; pr+=nclen) {
		switch (echarset) {
		case CHARSET_ASCII7:
			// �ی�R�[�h
			if (nclen != 1) {
				nclen = 1;
			}
			// 7-bit ASCII ������ϊ�
			*pw = static_cast<unsigned short>(*pr);
			pw += 1;
			break;
		case CHARSET_JIS_ZENKAKU:
		case CHARSET_JIS_HANKATA:
			// �ی�R�[�h
			if (echarset == CHARSET_JIS_ZENKAKU && nclen != 2) {
				nclen = 2;
			}
			if (echarset == CHARSET_JIS_HANKATA && nclen != 1) {
				nclen = 1;
			}
			// �S�p�����܂��͔��p�J�^�J�i������ϊ�
			pw += _SjisToUni_char(pr, pw, echarset, &berror_tmp);
			if (berror_tmp) {
				berror = true;
			}
			break;
		default:// CHARSET_BINARY:
			if (nclen != 1) {	// �ی�R�[�h
				nclen = 1;
			}
			// �G���[����������
			pw += BinToText(pr, nclen, pw);
		}
	}

	if (pbError) {
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}


// �R�[�h�ϊ� SJIS��Unicode
CodeConvertResult ShiftJis::SJISToUnicode(
	const Memory& src,
	NativeW* pDstMem
	)
{
	// �\�[�X�擾
	int nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>(src.GetRawPtr(&nSrcLen));
	if (nSrcLen == 0) {
		pDstMem->_SetStringLength(0);
		return CodeConvertResult::Complete;
	}

	// �ϊ���o�b�t�@�T�C�Y��ݒ肵�ă������̈�m��
	pDstMem->AllocStringBuffer(nSrcLen + 1);
	wchar_t* pDst = pDstMem->GetStringPtr();
	
	// �ϊ�
	bool bError;
	int nDstLen = SjisToUni(pSrc, nSrcLen, pDst, &bError);
	pDstMem->_SetStringLength(nDstLen);

	if (!bError) {
		return CodeConvertResult::Complete;
	}else {
		return CodeConvertResult::LoseSome;
	}
}


/*
	Unicode -> SJIS
*/
int ShiftJis::UniToSjis(const wchar_t* pSrc, const int nSrcLen, char* pDst, bool* pbError)
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
			pw += _UniToSjis_char(pr, pw, echarset, &berror_tmp);
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




// �R�[�h�ϊ� Unicode��SJIS
CodeConvertResult ShiftJis::UnicodeToSJIS( const NativeW& src, Memory* pDstMem )
{
	// ���
	const Memory* pMem = src._GetMemory();

	// �\�[�X�擾
	const wchar_t* pSrc = reinterpret_cast<const wchar_t*>(pMem->GetRawPtr());
	int nSrcLen = pMem->GetRawLength() / sizeof(wchar_t);
	if (nSrcLen == 0) {
		pDstMem->Clear();
		return CodeConvertResult::Complete;
	}

	// �ϊ���o�b�t�@�T�C�Y��ݒ肵�ăo�b�t�@���m��
	std::vector<char> dst(nSrcLen * 2);
	char* pDst = &dst[0];

	// �ϊ�
	bool berror;
	int nDstLen = UniToSjis(pSrc, nSrcLen, pDst, &berror);

	// pMem���X�V
	pDstMem->SetRawDataHoldBuffer( pDst, nDstLen );

	// ����
	if (berror) {
		return CodeConvertResult::LoseSome;
	}else {
		return CodeConvertResult::Complete;
	}
}


// �����R�[�h�\���p	UNICODE �� Hex �ϊ�	2008/6/9 Uchi
CodeConvertResult ShiftJis::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar)
{
	NativeW		cCharBuffer;
	CodeConvertResult	res;
	int				i;
	unsigned char*	ps;
	TCHAR*			pd;
	bool			bbinary = false;

	// 2008/6/21 Uchi
	if (psStatusbar->bDispUniInSjis) {
		// Unicode�ŕ\��
		return CodeBase::UnicodeToHex(cSrc, iSLen, pDst, psStatusbar);
	}

	cCharBuffer.AppendString(cSrc, 1);

	if (IsBinaryOnSurrogate(cSrc[0])) {
		bbinary = true;
	}

	// SJIS �ϊ�
	res = UnicodeToSJIS(cCharBuffer, cCharBuffer._GetMemory());
	if (res != CodeConvertResult::Complete) {
		return CodeConvertResult::LoseSome;
	}

	// Hex�ϊ�
	ps = reinterpret_cast<unsigned char*>( cCharBuffer._GetMemory()->GetRawPtr() );
	pd = pDst;
	if (!bbinary) {
		for (i=cCharBuffer._GetMemory()->GetRawLength(); i>0; --i, ++ps, pd+=2) {
			auto_sprintf(pd, _T("%02X"), *ps);
		}
	}else {
		auto_sprintf(pd, _T("?%02X"), *ps);
	}

	return CodeConvertResult::Complete;
}
