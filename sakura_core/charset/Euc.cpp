#include "StdAfx.h"
#include "Euc.h"

// 非依存推奨
#include "env/ShareData.h"
#include "env/DllSharedData.h"

/*!
	EUCJP → Unicode 変換関数
*/
size_t Euc::EucjpToUni(const char* pSrc, const size_t nSrcLen, wchar_t* pDst, bool* pbError)
{
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
	size_t nclen;
	for (; (nclen = CheckEucjpChar(reinterpret_cast<const char*>(pr), pr_end - pr, &echarset)) != 0; pr += nclen) {
		switch (echarset) {
		case CHARSET_ASCII7:
			// 保護コード
			if (nclen != 1) {
				nclen = 1;
			}
			// 7-bit ASCII の変換
			*pw = *pr;
			++pw;
			break;
		case CHARSET_JIS_HANKATA:
		case CHARSET_JIS_ZENKAKU:
			// 保護コード
			if (echarset == CHARSET_JIS_HANKATA && nclen != 2) {
				nclen = 2;
			}
			if (echarset == CHARSET_JIS_ZENKAKU && nclen != 2) {
				nclen = 2;
			}
			// 全角文字・半角カタカナ文字の変換
			pw += _EucjpToUni_char(pr, pw, echarset, &berror_tmp);
			if (berror_tmp) {
				berror = true;
			}
			break;
		default:// case CHARSET_BINARY:
			// 保護コード
			if (nclen != 1) {
				nclen = 1;
			}
			// 読み込みエラーになった文字を PUA に対応づける
			pw += BinToText(pr, nclen, pw);
		}
	}

	if (pbError) {
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}


// EUC→Unicodeコード変換
CodeConvertResult Euc::EUCToUnicode(const Memory& src, NativeW* pDstMem)
{
	// ソース取得
	size_t nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( src.GetRawPtr(&nSrcLen) );
	if (nSrcLen == 0) {
		pDstMem->Clear();
		return CodeConvertResult::Complete;
	}

	// 変換先バッファサイズとその確保
	std::vector<wchar_t> dst(nSrcLen);
	wchar_t* pDst = &dst[0];

	// 変換
	bool bError = false;
	size_t nDstLen = EucjpToUni(pSrc, nSrcLen, pDst, &bError);

	// pMem を更新
	pDstMem->_GetMemory()->SetRawDataHoldBuffer( pDst, nDstLen*sizeof(wchar_t) );

	//$$ SJISを介しているので無駄にデータを失うかも？
	// エラーを返すようにする。	2008/5/12 Uchi
	if (!bError) {
		return CodeConvertResult::Complete;
	}else {
		return CodeConvertResult::LoseSome;
	}
}


size_t Euc::UniToEucjp(const wchar_t* pSrc, const size_t nSrcLen, char* pDst, bool* pbError)
{
	bool berror = false, berror_tmp;
	ECharSet echarset;

	auto pr = reinterpret_cast<const unsigned short*>(pSrc);
	auto pr_end = reinterpret_cast<const unsigned short*>(pSrc + nSrcLen);
	auto pw = reinterpret_cast<unsigned char*>(pDst);

	size_t nclen;
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
			pw += _UniToEucjp_char(pr, pw, echarset, &berror_tmp);
			// 保護コード
			if (berror_tmp) {
				berror = true;
			}
			pr += nclen;
		}else {
			if (nclen == 1 && IsBinaryOnSurrogate(static_cast<wchar_t>(*pr))) {
				*pw = static_cast<unsigned char>(TextToBin(*pr) & 0x00ff);
				++pw;
			}else {
				// 保護コード
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


CodeConvertResult Euc::UnicodeToEUC(const NativeW& src, Memory* pDstMem)
{
	// エラー状態
	bool bError = false;

	const wchar_t* pSrc = src.GetStringPtr();
	size_t nSrcLen = src.GetStringLength();

	// 必要なバッファサイズを調べてメモリを確保
	assert(nSrcLen != 0);
	std::vector<char> dst(nSrcLen * 2);
	char* pDst = &dst[0];

	// 変換
	size_t nDstLen = UniToEucjp(pSrc, nSrcLen, pDst, &bError);

	// pMem を更新
	pDstMem->SetRawDataHoldBuffer( pDst, nDstLen );

	if (!bError) {
		return CodeConvertResult::Complete;
	}else {
		return CodeConvertResult::LoseSome;
	}
}

// 文字コード表示用	UNICODE → Hex 変換	2008/6/9 Uchi
CodeConvertResult Euc::UnicodeToHex(const wchar_t* cSrc, size_t iSLen, TCHAR* pDst, const CommonSetting_StatusBar* psStatusbar)
{
	NativeW charBuffer;
	// 2008/6/21 Uchi
	if (psStatusbar->bDispUniInEuc) {
		// Unicodeで表示
		return CodeBase::UnicodeToHex(cSrc, iSLen, pDst, psStatusbar);
	}

	// 1文字データバッファ
	charBuffer.SetString(cSrc, 1);

	bool bbinary = false;
	if (IsBinaryOnSurrogate(cSrc[0])) {
		bbinary = true;
	}

	// EUC-JP 変換
	CodeConvertResult res = UnicodeToEUC(charBuffer, charBuffer._GetMemory());
	if (res != CodeConvertResult::Complete) {
		return res;
	}

	// Hex変換
	unsigned char* ps = reinterpret_cast<unsigned char*>(charBuffer._GetMemory()->GetRawPtr());
	TCHAR* pd = pDst;
	if (!bbinary) {
		for (size_t i=charBuffer._GetMemory()->GetRawLength(); i>0; --i, ++ps, pd+=2) {
			auto_sprintf(pd, _T("%02X"), *ps);
		}
	}else {
		auto_sprintf(pd, _T("?%02X"), *ps);
	}

	return CodeConvertResult::Complete;
}
