#include "StdAfx.h"
#include "ShiftJis.h"
#include "charset/charcode.h"
#include "charset/codecheck.h"

// 非依存推奨
#include "env/ShareData.h"
#include "env/DllSharedData.h"


// 指定した位置の文字が何バイト文字かを返す
/*!
	@param[in] pData 位置を求めたい文字列の先頭
	@param[in] nDataLen 文字列長
	@param[in] nIdx 位置(0オリジン)
	@retval 1  1バイト文字
	@retval 2  2バイト文字
	@retval 0  エラー

	@note nIdxは予め文字の先頭位置とわかっていなければならない．
	2バイト文字の2バイト目をnIdxに与えると正しい結果が得られない．
*/
size_t ShiftJis::GetSizeOfChar(const char* pData, size_t nDataLen, size_t nIdx)
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
	SJIS → Unicode 変換
*/
size_t ShiftJis::SjisToUni(
	const char* __restrict pSrc, const size_t nSrcLen,
	wchar_t* __restrict pDst, bool* pbError
	)
{
	ECharSet echarset;
	size_t nclen;
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
			// 保護コード
			if (nclen != 1) {
				nclen = 1;
			}
			// 7-bit ASCII 文字を変換
			*pw = static_cast<unsigned short>(*pr);
			pw += 1;
			break;
		case CHARSET_JIS_ZENKAKU:
		case CHARSET_JIS_HANKATA:
			// 保護コード
			if (echarset == CHARSET_JIS_ZENKAKU && nclen != 2) {
				nclen = 2;
			}
			if (echarset == CHARSET_JIS_HANKATA && nclen != 1) {
				nclen = 1;
			}
			// 全角文字または半角カタカナ文字を変換
			pw += _SjisToUni_char(pr, pw, echarset, &berror_tmp);
			if (berror_tmp) {
				berror = true;
			}
			break;
		default:// CHARSET_BINARY:
			if (nclen != 1) {	// 保護コード
				nclen = 1;
			}
			// エラーが見つかった
			pw += BinToText(pr, nclen, pw);
		}
	}

	if (pbError) {
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}


// コード変換 SJIS→Unicode
CodeConvertResult ShiftJis::SJISToUnicode(
	const Memory& src,
	NativeW* pDstMem
	)
{
	// ソース取得
	size_t nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>(src.GetRawPtr(&nSrcLen));
	if (nSrcLen == 0) {
		pDstMem->_SetStringLength(0);
		return CodeConvertResult::Complete;
	}

	// 変換先バッファサイズを設定してメモリ領域確保
	pDstMem->AllocStringBuffer(nSrcLen + 1);
	wchar_t* pDst = pDstMem->GetStringPtr();
	
	// 変換
	bool bError;
	size_t nDstLen = SjisToUni(pSrc, nSrcLen, pDst, &bError);
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
size_t ShiftJis::UniToSjis(const wchar_t* pSrc, const size_t nSrcLen, char* pDst, bool* pbError)
{
	size_t nclen;
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




// コード変換 Unicode→SJIS
CodeConvertResult ShiftJis::UnicodeToSJIS( const NativeW& src, Memory* pDstMem )
{
	// 状態
	const Memory* pMem = src._GetMemory();

	// ソース取得
	const wchar_t* pSrc = reinterpret_cast<const wchar_t*>(pMem->GetRawPtr());
	size_t nSrcLen = pMem->GetRawLength() / sizeof(wchar_t);
	if (nSrcLen == 0) {
		pDstMem->Clear();
		return CodeConvertResult::Complete;
	}

	// 変換先バッファサイズを設定してバッファを確保
	std::vector<char> dst(nSrcLen * 2);
	char* pDst = &dst[0];

	// 変換
	bool berror;
	size_t nDstLen = UniToSjis(pSrc, nSrcLen, pDst, &berror);

	// pMemを更新
	pDstMem->SetRawDataHoldBuffer(pDst, nDstLen);

	// 結果
	if (berror) {
		return CodeConvertResult::LoseSome;
	}else {
		return CodeConvertResult::Complete;
	}
}


// 文字コード表示用	UNICODE → Hex 変換
CodeConvertResult ShiftJis::UnicodeToHex(const wchar_t* cSrc, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar)
{
	NativeW		cCharBuffer;
	CodeConvertResult	res;
	unsigned char*	ps;
	TCHAR*			pd;
	bool			bbinary = false;

	if (psStatusbar->bDispUniInSjis) {
		// Unicodeで表示
		return CodeBase::UnicodeToHex(cSrc, iSLen, pDst, psStatusbar);
	}

	cCharBuffer.AppendString(cSrc, 1);

	if (IsBinaryOnSurrogate(cSrc[0])) {
		bbinary = true;
	}

	// SJIS 変換
	res = UnicodeToSJIS(cCharBuffer, cCharBuffer._GetMemory());
	if (res != CodeConvertResult::Complete) {
		return CodeConvertResult::LoseSome;
	}

	// Hex変換
	ps = reinterpret_cast<unsigned char*>( cCharBuffer._GetMemory()->GetRawPtr() );
	pd = pDst;
	if (!bbinary) {
		for (size_t i=cCharBuffer._GetMemory()->GetRawLength(); i>0; --i, ++ps, pd+=2) {
			auto_sprintf(pd, _T("%02X"), *ps);
		}
	}else {
		auto_sprintf(pd, _T("?%02X"), *ps);
	}

	return CodeConvertResult::Complete;
}
