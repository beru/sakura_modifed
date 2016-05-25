// 2008.11.10 �ϊ����W�b�N����������

#include "StdAfx.h"
#include "Utf8.h"
#include "charset/codecheck.h"

// ��ˑ�����
#include "env/ShareData.h"
#include "env/DllSharedData.h"

// BOM�f�[�^�擾
void Utf8::GetBom(Memory* pMemBom)
{
	static const BYTE UTF8_BOM[] = {0xEF, 0xBB, 0xBF};
	pMemBom->SetRawData(UTF8_BOM, sizeof(UTF8_BOM));
}



void Utf8::GetEol(Memory* pMemEol, EolType eolType){
	static const struct{
		const char* szData;
		int nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		"",					0,	// EolType::None
		"\x0d\x0a",			2,	// EolType::CRLF
		"\x0a",				1,	// EolType::LF
		"\x0d",				1,	// EolType::CR
		"\xc2\x85",			2,	// EolType::NEL
		"\xe2\x80\xa8",		3,	// EolType::LS
		"\xe2\x80\xa9",		3,	// EolType::PS
	};
	pMemEol->SetRawData(aEolTable[(int)eolType].szData, aEolTable[(int)eolType].nLen);
}


/*!
	UTF-8 �� Unicode ����

	@param[in] bCESU8Mode CESU-8 ����������ꍇ true
*/
int Utf8::Utf8ToUni(const char* pSrc, const size_t nSrcLen, wchar_t* pDst, bool bCESU8Mode)
{
	const unsigned char *pr, *pr_end;
	unsigned short* pw;
	int nclen;
	ECharSet echarset;

	if (nSrcLen < 1) {
		return 0;
	}

	pr = reinterpret_cast<const unsigned char*>(pSrc);
	pr_end = reinterpret_cast<const unsigned char*>(pSrc + nSrcLen);
	pw = reinterpret_cast<unsigned short*>(pDst);

	for (;;) {

		// �������`�F�b�N
		if (!bCESU8Mode) {
			nclen = CheckUtf8Char(reinterpret_cast<const char*>(pr), pr_end - pr, &echarset, true, 0);
		}else {
			nclen = CheckCesu8Char(reinterpret_cast<const char*>(pr), pr_end - pr, &echarset, 0);
		}
		if (nclen < 1) {
			break;
		}

		// �ϊ�
		if (echarset != CHARSET_BINARY) {
			pw += _Utf8ToUni_char(pr, nclen, pw, bCESU8Mode);
			pr += nclen;
		}else {
			if (nclen != 1) {	// �ی�R�[�h
				nclen = 1;
			}
			pw += BinToText(pr, 1, pw);
			++pr;
		}
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}



// UTF-8��Unicode�R�[�h�ϊ�
// 2007.08.13 kobake �쐬
CodeConvertResult Utf8::_UTF8ToUnicode( const Memory& src, NativeW* pDstMem, bool bCESU8Mode/*, bool decodeMime*/ )
{
	// �f�[�^�擾
	size_t nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( src.GetRawPtr(&nSrcLen) );
	if (nSrcLen == 0) {
		pDstMem->Clear();
		return CodeConvertResult::Complete;
	}
	
	const char* psrc = pSrc;
	int nsrclen = nSrcLen;

//	Memory mem;
//	// MIME �w�b�_�[�f�R�[�h
//	if (decodeMime) {
//		bool bret = MIMEHeaderDecode(pSrc, nSrcLen, &mem, CODE_UTF8);
//		if (bret) {
//			psrc = reinterpret_cast<char*>(mem.GetRawPtr());
//			nsrclen = mem.GetRawLength();
//		}
//	}

	// �K�v�ȃo�b�t�@�T�C�Y�𒲂ׂĊm�ۂ���
	std::vector<wchar_t> dst(nsrclen);
	wchar_t* pDst = &dst[0];

	// �ϊ�
	int nDstLen = Utf8ToUni(psrc, nsrclen, pDst, bCESU8Mode);

	// pDstMem ���X�V
	pDstMem->_GetMemory()->SetRawDataHoldBuffer( pDst, nDstLen*sizeof(wchar_t) );

	return CodeConvertResult::Complete;
}


/*!
	Unicode -> UTF-8 ����

	@param[in] bCESU8Mode CESU-8 ����������ꍇ true
*/
int Utf8::UniToUtf8(const wchar_t* pSrc, const size_t nSrcLen, char* pDst, bool* pbError, bool bCESU8Mode)
{
	const unsigned short* pr = reinterpret_cast<const unsigned short*>(pSrc);
	const unsigned short* pr_end = reinterpret_cast<const unsigned short*>(pSrc + nSrcLen);
	unsigned char* pw = reinterpret_cast<unsigned char*>(pDst);
	int nclen;
	bool berror=false;
	ECharSet echarset;

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
			pw += _UniToUtf8_char(pr, nclen, pw, bCESU8Mode);
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


// �R�[�h�ϊ� Unicode��UTF-8
CodeConvertResult Utf8::_UnicodeToUTF8( const NativeW& src, Memory* pDstMem, bool bCesu8Mode )
{
	// �\�[�X���擾
	const wchar_t* pSrc = src.GetStringPtr();
	size_t nSrcLen = src.GetStringLength();
	if (nSrcLen == 0) {
		pDstMem->Clear();
		return CodeConvertResult::Complete;
	}
	
	// �K�v�ȃo�b�t�@�T�C�Y�𒲂ׂă��������m��
	std::vector<char> dst(nSrcLen * 4);
	char* pDst = &dst[0];

	// �ϊ�
	bool bError = false;
	int nDstLen = UniToUtf8(pSrc, nSrcLen, pDst, &bError, bCesu8Mode);

	// pDstMem ���X�V
	pDstMem->SetRawDataHoldBuffer( pDst, nDstLen );

	if (!bError) {
		return CodeConvertResult::Complete;
	}else {
		return CodeConvertResult::LoseSome;
	}
}

// �����R�[�h�\���p	UNICODE �� Hex �ϊ�	2008/6/21 Uchi
CodeConvertResult Utf8::_UnicodeToHex(const wchar_t* src, const int iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar, const bool bCESUMode)
{
	NativeW		buff;
	CodeConvertResult	res;
	int				i;
	TCHAR*			pd;
	unsigned char*	ps;
	bool			bbinary=false;

	if (psStatusbar->bDispUtf8Codepoint) {
		// Unicode�ŕ\��
		return CodeBase::UnicodeToHex(src, iSLen, pDst, psStatusbar);
	}
	buff.AllocStringBuffer(4);
	// 1�����f�[�^�o�b�t�@
	if (IsUTF16High(src[0]) && iSLen >= 2 && IsUTF16Low(src[1])) {
		buff._GetMemory()->SetRawDataHoldBuffer(src, 4);
	}else {
		buff._GetMemory()->SetRawDataHoldBuffer(src, 2);
		if (IsBinaryOnSurrogate(src[0])) {
			bbinary = true;
		}
	}

	// UTF-8/CESU-8 �ϊ�
	if (bCESUMode != true) {
		res = UnicodeToUTF8(buff, buff._GetMemory());
	}else {
		res = UnicodeToCESU8(buff, buff._GetMemory());
	}
	if (res != CodeConvertResult::Complete) {
		return res;
	}

	// Hex�ϊ�
	ps = reinterpret_cast<unsigned char*>( buff._GetMemory()->GetRawPtr() );
	pd = pDst;
	if (!bbinary) {
		for (i=buff._GetMemory()->GetRawLength(); i>0; --i, ++ps, pd+=2) {
			auto_sprintf(pd, _T("%02X"), *ps);
		}
	}else {
		auto_sprintf(pd, _T("?%02X"), *ps);
	}

	return CodeConvertResult::Complete;
}
