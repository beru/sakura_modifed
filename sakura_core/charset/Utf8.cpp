#include "StdAfx.h"
#include "Utf8.h"
#include "charset/codecheck.h"

// 非依存推奨
#include "env/ShareData.h"
#include "env/DllSharedData.h"

// BOMデータ取得
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
	UTF-8 → Unicode 実装

	@param[in] bCESU8Mode CESU-8 を処理する場合 true
*/
size_t Utf8::Utf8ToUni(const char* pSrc, const size_t nSrcLen, wchar_t* pDst, bool bCESU8Mode)
{
	const unsigned char *pr, *pr_end;
	unsigned short* pw;
	size_t nclen;
	ECharSet echarset;

	if (nSrcLen < 1) {
		return 0;
	}

	pr = reinterpret_cast<const unsigned char*>(pSrc);
	pr_end = reinterpret_cast<const unsigned char*>(pSrc + nSrcLen);
	pw = reinterpret_cast<unsigned short*>(pDst);

	for (;;) {

		// 文字をチェック
		if (!bCESU8Mode) {
			nclen = CheckUtf8Char(reinterpret_cast<const char*>(pr), pr_end - pr, &echarset, true, 0);
		}else {
			nclen = CheckCesu8Char(reinterpret_cast<const char*>(pr), pr_end - pr, &echarset, 0);
		}
		if (nclen < 1) {
			break;
		}

		// 変換
		if (echarset != CHARSET_BINARY) {
			pw += _Utf8ToUni_char(pr, nclen, pw, bCESU8Mode);
			pr += nclen;
		}else {
			if (nclen != 1) {	// 保護コード
				nclen = 1;
			}
			pw += BinToText(pr, 1, pw);
			++pr;
		}
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}



// UTF-8→Unicodeコード変換
CodeConvertResult Utf8::_UTF8ToUnicode( const Memory& src, NativeW* pDstMem, bool bCESU8Mode/*, bool decodeMime*/ )
{
	// データ取得
	size_t nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( src.GetRawPtr(&nSrcLen) );
	if (nSrcLen == 0) {
		pDstMem->Clear();
		return CodeConvertResult::Complete;
	}
	
	const char* psrc = pSrc;
	size_t nsrclen = nSrcLen;

//	Memory mem;
//	// MIME ヘッダーデコード
//	if (decodeMime) {
//		bool bret = MIMEHeaderDecode(pSrc, nSrcLen, &mem, CODE_UTF8);
//		if (bret) {
//			psrc = reinterpret_cast<char*>(mem.GetRawPtr());
//			nsrclen = mem.GetRawLength();
//		}
//	}

	// 必要なバッファサイズを調べて確保する
	std::vector<wchar_t> dst(nsrclen);
	wchar_t* pDst = &dst[0];

	// 変換
	size_t nDstLen = Utf8ToUni(psrc, nsrclen, pDst, bCESU8Mode);

	// pDstMem を更新
	pDstMem->_GetMemory()->SetRawDataHoldBuffer(pDst, nDstLen*sizeof(wchar_t));

	return CodeConvertResult::Complete;
}


/*!
	Unicode -> UTF-8 実装

	@param[in] bCESU8Mode CESU-8 を処理する場合 true
*/
size_t Utf8::UniToUtf8(const wchar_t* pSrc, const size_t nSrcLen, char* pDst, bool* pbError, bool bCESU8Mode)
{
	const unsigned short* pr = reinterpret_cast<const unsigned short*>(pSrc);
	const unsigned short* pr_end = reinterpret_cast<const unsigned short*>(pSrc + nSrcLen);
	unsigned char* pw = reinterpret_cast<unsigned char*>(pDst);
	size_t nclen;
	bool berror = false;
	ECharSet echarset;

	while ((nclen = CheckUtf16leChar(reinterpret_cast<const wchar_t*>(pr), pr_end - pr, &echarset, 0)) > 0) {
		// 保護コード
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


// コード変換 Unicode→UTF-8
CodeConvertResult Utf8::_UnicodeToUTF8( const NativeW& src, Memory* pDstMem, bool bCesu8Mode )
{
	// ソースを取得
	const wchar_t* pSrc = src.GetStringPtr();
	size_t nSrcLen = src.GetStringLength();
	if (nSrcLen == 0) {
		pDstMem->Clear();
		return CodeConvertResult::Complete;
	}
	
	// 必要なバッファサイズを調べてメモリを確保
	std::vector<char> dst(nSrcLen * 4);
	char* pDst = &dst[0];

	// 変換
	bool bError = false;
	size_t nDstLen = UniToUtf8(pSrc, nSrcLen, pDst, &bError, bCesu8Mode);

	// pDstMem を更新
	pDstMem->SetRawDataHoldBuffer(pDst, nDstLen);

	if (!bError) {
		return CodeConvertResult::Complete;
	}else {
		return CodeConvertResult::LoseSome;
	}
}

// 文字コード表示用	UNICODE → Hex 変換
CodeConvertResult Utf8::_UnicodeToHex(const wchar_t* src, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar, const bool bCESUMode)
{
	NativeW		buff;
	CodeConvertResult	res;
	TCHAR*			pd;
	unsigned char*	ps;
	bool			bbinary=false;

	if (psStatusbar->bDispUtf8Codepoint) {
		// Unicodeで表示
		return CodeBase::UnicodeToHex(src, iSLen, pDst, psStatusbar);
	}
	buff.AllocStringBuffer(4);
	// 1文字データバッファ
	if (IsUTF16High(src[0]) && iSLen >= 2 && IsUTF16Low(src[1])) {
		buff._GetMemory()->SetRawDataHoldBuffer(src, 4);
	}else {
		buff._GetMemory()->SetRawDataHoldBuffer(src, 2);
		if (IsBinaryOnSurrogate(src[0])) {
			bbinary = true;
		}
	}

	// UTF-8/CESU-8 変換
	if (bCESUMode != true) {
		res = UnicodeToUTF8(buff, buff._GetMemory());
	}else {
		res = UnicodeToCESU8(buff, buff._GetMemory());
	}
	if (res != CodeConvertResult::Complete) {
		return res;
	}

	// Hex変換
	ps = reinterpret_cast<unsigned char*>( buff._GetMemory()->GetRawPtr() );
	pd = pDst;
	if (!bbinary) {
		for (size_t i=buff._GetMemory()->GetRawLength(); i>0; --i, ++ps, pd+=2) {
			auto_sprintf(pd, _T("%02X"), *ps);
		}
	}else {
		auto_sprintf(pd, _T("?%02X"), *ps);
	}

	return CodeConvertResult::Complete;
}
