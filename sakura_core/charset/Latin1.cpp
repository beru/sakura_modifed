#include "StdAfx.h"
#include "Latin1.h"
#include "charset/charcode.h"
#include "charset/codecheck.h"
#include "Eol.h"
//#include "env/ShareData.h"
#include "env/CommonSetting.h"


// 指定した位置の文字が何バイト文字かを返す
/*!
	@param[in] pData 位置を求めたい文字列の先頭
	@param[in] nDataLen 文字列長
	@param[in] nIdx 位置(0オリジン)
	@retval 1  1バイト文字
	@retval 0  エラー

	エラーでなければ1を返す
*/
size_t Latin1::GetSizeOfChar(const char* pData, size_t nDataLen, size_t nIdx)
{
	if (nIdx >= nDataLen) {
		return 0;
	}
	return 1;
}


/*!
	Latin1 → Unicode 変換
*/
size_t Latin1::Latin1ToUni(const char* pSrc, const size_t nSrcLen, wchar_t* pDst, bool* pbError)
{
	if (pbError) {
		*pbError = false;
	}
	if (nSrcLen < 1) {
		return 0;
	}

	const unsigned char* pr = reinterpret_cast<const unsigned char*>(pSrc);
	const unsigned char* pr_end = reinterpret_cast<const unsigned char*>(pSrc + nSrcLen);
	unsigned short* pw = reinterpret_cast<unsigned short*>(pDst);
	for (; pr<pr_end; ++pr) {
		if (*pr >= 0x80 && *pr <= 0x9f) {
			// Windows 拡張部
			int nret = ::MultiByteToWideChar(1252, 0, reinterpret_cast<const char*>(pr), 1, reinterpret_cast<wchar_t*>(pw), 4);
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

// コード変換 Latin1→Unicode
CodeConvertResult Latin1::Latin1ToUnicode( const Memory& src, NativeW* pDstMem )
{
	// ソース取得
	size_t nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( src.GetRawPtr(&nSrcLen) );
	if (nSrcLen == 0) {
		pDstMem->Clear();
		return CodeConvertResult::Complete;
	}

	// 変換先バッファサイズを設定してメモリ領域確保
	std::vector<wchar_t> dst(nSrcLen);
	wchar_t* pDst = &dst[0];

	// 変換
	bool bError;
	size_t nDstLen = Latin1ToUni(pSrc, nSrcLen, pDst, &bError);

	// pDstMemを更新
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
size_t Latin1::UniToLatin1(const wchar_t* pSrc, const size_t nSrcLen, char* pDst, bool* pbError)
{

	if (nSrcLen < 1) {
		if (pbError) {
			*pbError = false;
		}
		return 0;
	}

	bool berror = false, berror_tmp;
	const unsigned short* pr = reinterpret_cast<const unsigned short*>(pSrc);
	const unsigned short* pr_end = reinterpret_cast<const unsigned short*>(pSrc + nSrcLen);
	unsigned char* pw = reinterpret_cast<unsigned char*>(pDst);
	size_t nclen;
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




// コード変換 Unicode→Latin1
CodeConvertResult Latin1::UnicodeToLatin1( const NativeW& src, Memory* pDstMem )
{
	// ソース取得
	const wchar_t* pSrc = src.GetStringPtr();
	size_t nSrcLen = src.GetStringLength();
	if (nSrcLen == 0) {
		pDstMem->Clear();
		return CodeConvertResult::Complete;
	}

	// 変換先バッファサイズを設定してバッファを確保
	std::vector<char> dst(nSrcLen * 2);
	char* pDst = &dst[0];

	// 変換
	bool berror;
	size_t nDstLen = UniToLatin1(pSrc, nSrcLen, pDst, &berror);

	// pDstMemを更新
	pDstMem->SetRawDataHoldBuffer( pDst, nDstLen );

	// 結果
	if (berror) {
		return CodeConvertResult::LoseSome;
	}else {
		return CodeConvertResult::Complete;
	}
}


// 文字コード表示用	UNICODE → Hex 変換
CodeConvertResult Latin1::UnicodeToHex(const wchar_t* cSrc, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar)
{
	if (psStatusbar->bDispUniInSjis) {
		// Unicodeで表示
		return CodeBase::UnicodeToHex(cSrc, iSLen, pDst, psStatusbar);
	}

	NativeW cCharBuffer;
	cCharBuffer.SetString(cSrc, 1);

	bool bbinary = false;
	if (IsBinaryOnSurrogate(cSrc[0])) {
		bbinary = true;
	}

	// Latin1 変換
	CodeConvertResult res = UnicodeToLatin1(cCharBuffer, cCharBuffer._GetMemory());
	if (res != CodeConvertResult::Complete) {
		return CodeConvertResult::LoseSome;
	}

	// Hex変換
	unsigned char* ps = reinterpret_cast<unsigned char*>( cCharBuffer._GetMemory()->GetRawPtr() );
	TCHAR* pd = pDst;
	if (!bbinary) {
		for (size_t i=cCharBuffer._GetMemory()->GetRawLength(); i>0; --i, ++ps, pd+=2) {
			auto_sprintf(pd, _T("%02x"), *ps);
		}
	}else {
		auto_sprintf(pd, _T("?%02x"), *ps);
	}

	return CodeConvertResult::Complete;
}
