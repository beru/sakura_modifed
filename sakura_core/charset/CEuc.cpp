#include "StdAfx.h"
#include "CEuc.h"

// ��ˑ�����
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"

/*!
	EUCJP �� Unicode �ϊ��֐�
*/
int Euc::EucjpToUni(const char* pSrc, const int nSrcLen, wchar_t* pDst, bool* pbError)
{
	int nclen;
	ECharSet echarset;
	bool berror_tmp, berror = false;

	if (nSrcLen < 1) {
		if (pbError) {
			*pbError = false;
		}
		return 0;
	}

	auto pr = reinterpret_cast<const unsigned char*>(pSrc);
	auto pr_end = reinterpret_cast<const unsigned char*>(pSrc + nSrcLen);
	auto pw = reinterpret_cast<unsigned short*>(pDst);

	for (; (nclen = CheckEucjpChar(reinterpret_cast<const char*>(pr), pr_end - pr, &echarset)) != 0; pr += nclen) {
		switch (echarset) {
		case CHARSET_ASCII7:
			// �ی�R�[�h
			if (nclen != 1) {
				nclen = 1;
			}
			// 7-bit ASCII �̕ϊ�
			*pw = *pr;
			++pw;
			break;
		case CHARSET_JIS_HANKATA:
		case CHARSET_JIS_ZENKAKU:
			// �ی�R�[�h
			if (echarset == CHARSET_JIS_HANKATA && nclen != 2) {
				nclen = 2;
			}
			if (echarset == CHARSET_JIS_ZENKAKU && nclen != 2) {
				nclen = 2;
			}
			// �S�p�����E���p�J�^�J�i�����̕ϊ�
			pw += _EucjpToUni_char(pr, pw, echarset, &berror_tmp);
			if (berror_tmp) {
				berror = true;
			}
			break;
		default:// case CHARSET_BINARY:
			// �ی�R�[�h
			if (nclen != 1) {
				nclen = 1;
			}
			// �ǂݍ��݃G���[�ɂȂ��������� PUA �ɑΉ��Â���
			pw += BinToText(pr, nclen, pw);
		}
	}

	if (pbError) {
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}


// EUC��Unicode�R�[�h�ϊ�
// 2007.08.13 kobake �ǉ�
CodeConvertResult Euc::EUCToUnicode(const Memory& cSrc, NativeW* pDstMem)
{
	// �\�[�X�擾
	int nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( cSrc.GetRawPtr(&nSrcLen) );
	if (nSrcLen == 0) {
		pDstMem->Clear();
		return CodeConvertResult::Complete;
	}

	// �ϊ���o�b�t�@�T�C�Y�Ƃ��̊m��
	std::vector<wchar_t> dst(nSrcLen);
	wchar_t* pDst = &dst[0];

	// �ϊ�
	bool bError = false;
	int nDstLen = EucjpToUni(pSrc, nSrcLen, pDst, &bError);

	// pMem ���X�V
	pDstMem->_GetMemory()->SetRawDataHoldBuffer( pDst, nDstLen*sizeof(wchar_t) );

	//$$ SJIS����Ă���̂Ŗ��ʂɃf�[�^�����������H
	// �G���[��Ԃ��悤�ɂ���B	2008/5/12 Uchi
	if (!bError) {
		return CodeConvertResult::Complete;
	}else {
		return CodeConvertResult::LoseSome;
	}
}


int Euc::UniToEucjp(const wchar_t* pSrc, const int nSrcLen, char* pDst, bool* pbError)
{
	int nclen;
	bool berror = false, berror_tmp;
	ECharSet echarset;

	auto pr = reinterpret_cast<const unsigned short*>(pSrc);
	auto pr_end = reinterpret_cast<const unsigned short*>(pSrc + nSrcLen);
	auto pw = reinterpret_cast<unsigned char*>(pDst);

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
			pw += _UniToEucjp_char(pr, pw, echarset, &berror_tmp);
			// �ی�R�[�h
			if (berror_tmp) {
				berror = true;
			}
			pr += nclen;
		}else {
			if (nclen == 1 && IsBinaryOnSurrogate(static_cast<wchar_t>(*pr))) {
				*pw = static_cast<unsigned char>(TextToBin(*pr) & 0x00ff);
				++pw;
			}else {
				// �ی�R�[�h
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


CodeConvertResult Euc::UnicodeToEUC(const NativeW& cSrc, Memory* pDstMem)
{
	// �G���[���
	bool bError = false;

	const wchar_t* pSrc = cSrc.GetStringPtr();
	int nSrcLen = cSrc.GetStringLength();

	// �K�v�ȃo�b�t�@�T�C�Y�𒲂ׂă��������m��
	assert(nSrcLen != 0);
	std::vector<char> dst(nSrcLen * 2);
	char* pDst = &dst[0];

	// �ϊ�
	int nDstLen = UniToEucjp(pSrc, nSrcLen, pDst, &bError);

	// pMem ���X�V
	pDstMem->SetRawDataHoldBuffer( pDst, nDstLen );

	if (!bError) {
		return CodeConvertResult::Complete;
	}else {
		return CodeConvertResult::LoseSome;
	}
}

// �����R�[�h�\���p	UNICODE �� Hex �ϊ�	2008/6/9 Uchi
CodeConvertResult Euc::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar)
{
	NativeW cCharBuffer;
	// 2008/6/21 Uchi
	if (psStatusbar->m_bDispUniInEuc) {
		// Unicode�ŕ\��
		return CodeBase::UnicodeToHex(cSrc, iSLen, pDst, psStatusbar);
	}

	// 1�����f�[�^�o�b�t�@
	cCharBuffer.SetString(cSrc, 1);

	bool bbinary = false;
	if (IsBinaryOnSurrogate(cSrc[0])) {
		bbinary = true;
	}

	// EUC-JP �ϊ�
	CodeConvertResult res = UnicodeToEUC(cCharBuffer, cCharBuffer._GetMemory());
	if (res != CodeConvertResult::Complete) {
		return res;
	}

	// Hex�ϊ�
	unsigned char* ps = reinterpret_cast<unsigned char*>(cCharBuffer._GetMemory()->GetRawPtr());
	TCHAR* pd = pDst;
	if (!bbinary) {
		for (int i=cCharBuffer._GetMemory()->GetRawLength(); i>0; --i, ++ps, pd+=2) {
			auto_sprintf(pd, _T("%02X"), *ps);
		}
	}else {
		auto_sprintf(pd, _T("?%02X"), *ps);
	}

	return CodeConvertResult::Complete;
}
