#include "StdAfx.h"
#include "Utf7.h"
#include "charset/charcode.h"
#include "charset/codecheck.h"
#include "convert/convert_util2.h"
#include "Eol.h"


/*!
	UTF-7 Set D 部分の読み込み。
*/
size_t Utf7::_Utf7SetDToUni_block(const char* pSrc, const size_t nSrcLen, wchar_t* pDst)
{
	const char* pr = pSrc;
	wchar_t* pw = pDst;

	for (; pr<pSrc+nSrcLen; ++pr) {
		if (IsUtf7Direct(*pr)) {
			*pw = *pr;
		}else {
			BinToText(reinterpret_cast<const unsigned char*>(pr), 1, reinterpret_cast<unsigned short*>(pw));
		}
		++pw;
	}
	return pw - pDst;
}

/*!
	UTF-7 Set B 部分の読み込み
*/
size_t Utf7::_Utf7SetBToUni_block(const char* pSrc, const size_t nSrcLen, wchar_t* pDst, bool* pbError)
{
	if (nSrcLen == 0) {
		return 0;
	}

	std::vector<char> buf(nSrcLen);
	char* pbuf = &buf[0];

	size_t ndecoded_len = _DecodeBase64(pSrc, nSrcLen, pbuf);
	size_t nModLen = ndecoded_len % sizeof(wchar_t);
	ndecoded_len = ndecoded_len - nModLen;
	Memory::SwapHLByte(pbuf, ndecoded_len);  // UTF-16 BE を UTF-16 LE に直す
	memcpy(reinterpret_cast<char*>(pDst), pbuf, ndecoded_len);
	bool bError = false;
	if (nModLen) {
		ndecoded_len += BinToText(reinterpret_cast<const unsigned char*>(pbuf) + ndecoded_len,
			nModLen, &reinterpret_cast<unsigned short*>(pDst)[ndecoded_len / sizeof(wchar_t)]) * sizeof(wchar_t);
		bError = true;
	}

	if (pbError) {
		*pbError = bError;
	}

	return ndecoded_len / sizeof(wchar_t);
}

size_t Utf7::Utf7ToUni(const char* pSrc, const size_t nSrcLen, wchar_t* pDst, bool* pbError)
{
	const char *pr, *pr_end;
	char* pr_next;
	wchar_t* pw;
	bool berror_tmp, berror = false;

	pr = pSrc;
	pr_end = pSrc + nSrcLen;
	pw = pDst;

	do {
		// UTF-7 Set D 部分のチェック
		size_t nblocklen = CheckUtf7DPart(pr, pr_end - pr, &pr_next, &berror_tmp);
		if (berror_tmp) {
			berror = true;
		}
		pw += _Utf7SetDToUni_block(pr, nblocklen, pw);

		pr = pr_next;  // 次の読み込み位置を取得
		if (pr_next >= pr_end) {
			break;
		}

		// UTF-7 Set B 部分のチェック
		nblocklen = CheckUtf7BPart(pr, pr_end - pr, &pr_next, &berror_tmp, UC_LOOSE);
		{
			// エラーがあってもできるところまでデコード
			if (berror_tmp) {
				berror = true;
			}
			if (nblocklen < 1 && *(pr_next - 1) == '-') {
				// +- → + 変換
				*pw = L'+';
				++pw;
			}else {
				pw += _Utf7SetBToUni_block(pr, nblocklen, pw, &berror_tmp);
				if (berror_tmp) {
					berror = true;
				}
			}
		}
		pr = pr_next;  // 次の読み込み位置を取得
	} while (pr_next < pr_end);

	if (pbError) {
		*pbError = berror;
	}

	return pw - pDst;
}


// UTF-7→Unicodeコード変換
CodeConvertResult Utf7::UTF7ToUnicode( const Memory& src, NativeW* pDstMem )
{
	// データ取得
	size_t nDataLen;
	const char* pData = reinterpret_cast<const char*>( src.GetRawPtr(&nDataLen) );
	if (nDataLen == 0) {
		pDstMem->Clear();
		return CodeConvertResult::Complete;
	}
	
	// 必要なバッファサイズを調べて確保
	std::vector<wchar_t> dst(nDataLen + 1);
	wchar_t* pDst = &dst[0];

	// 変換
	bool bError;
	size_t nDstLen = Utf7ToUni(pData, nDataLen, pDst, &bError);

	// pDstMem を設定
	pDstMem->_GetMemory()->SetRawDataHoldBuffer( pDst, nDstLen*sizeof(wchar_t) );

	if (!bError) {
		return CodeConvertResult::Complete;
	}else {
		return CodeConvertResult::LoseSome;
	}
}



size_t Utf7::_UniToUtf7SetD_block(const wchar_t* pSrc, const size_t nSrcLen, char* pDst)
{
	if (nSrcLen < 1) {
		return 0;
	}

	size_t i;
	for (i=0; i<nSrcLen; ++i) {
		pDst[i] = static_cast<char>(pSrc[i] & 0x00ff);
	}

	return i;
}



size_t Utf7::_UniToUtf7SetB_block(const wchar_t* pSrc, const size_t nSrcLen, char* pDst)
{
	if (nSrcLen < 1) {
		return 0;
	}

	std::vector<wchar_t> src(nSrcLen);
	wchar_t* psrc = &src[0];

	// // UTF-16 LE → UTF-16 BE
	wcsncpy(&psrc[0], pSrc, nSrcLen);
	Memory::SwapHLByte(reinterpret_cast<char*>(psrc), nSrcLen * sizeof(wchar_t));

	// 書き込み
	char* pw = pDst;
	pw[0] = '+';
	++pw;
	pw += _EncodeBase64(reinterpret_cast<char*>(psrc), nSrcLen * sizeof(wchar_t), pw);
	pw[0] = '-';
	++pw;

	return pw - pDst;
}


size_t Utf7::UniToUtf7(const wchar_t* pSrc, const size_t nSrcLen, char* pDst)
{
	const wchar_t *pr, *pr_base;
	const wchar_t* pr_end;
	char* pw;

	pr = pSrc;
	pr_base = pSrc;
	pr_end = pSrc + nSrcLen;
	pw = pDst;

	do {
		for (; pr<pr_end; ++pr) {
			if (!IsUtf7SetD(*pr)) {
				break;
			}
		}
		pw += _UniToUtf7SetD_block(pr_base, pr - pr_base, pw);
		pr_base = pr;

		if (*pr == L'+') {
			// '+' → "+-"
			pw[0] = '+';
			pw[1] = '-';
			++pr;
			pw += 2;
		}else {
			for (; pr<pr_end; ++pr) {
				if (IsUtf7SetD(*pr)) {
					break;
				}
			}
			pw += _UniToUtf7SetB_block(pr_base, pr - pr_base, pw);
		}
		pr_base = pr;
	} while (pr_base < pr_end);

	return pw - pDst;
}



/*! コード変換 Unicode→UTF-7 */
CodeConvertResult Utf7::UnicodeToUTF7(const NativeW& src, Memory* pDstMem)
{

	// データ取得
	const wchar_t* pSrc = src.GetStringPtr();
	size_t nSrcLen = src.GetStringLength();
	if (nSrcLen == 0) {
		return CodeConvertResult::Failure;
	}

	// 出力先バッファの確保
	// 最大で、変換元のデータ長の５倍。
	std::vector<char> dst(nSrcLen * 5 + 1);  // * → +ACo-
	char* pDst = &dst[0];

	// 変換
	size_t nDstLen = UniToUtf7(pSrc, nSrcLen, pDst);

	// pMem にデータをセット
	pDstMem->SetRawDataHoldBuffer( pDst, nDstLen );

	return CodeConvertResult::Complete;
}

// BOMデータ取得
void Utf7::GetBom(Memory* pMemBom)
{
	static const BYTE UTF7_BOM[]= {'+', '/', 'v', '8', '-'};
	pMemBom->SetRawData(UTF7_BOM, sizeof(UTF7_BOM));
}

void Utf7::GetEol(Memory* pMemEol, EolType eolType)
{
	static const struct{
		const char* szData;
		int nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		{ "",			0 },	// EolType::None
		{ "\x0d\x0a",	2 },	// EolType::CRLF
		{ "\x0a",		1 },	// EolType::LF
		{ "\x0d",		1 },	// EolType::CR
		{ "+AIU-",		5 },	// EolType::NEL
		{ "+ICg-",		5 },	// EolType::LS
		{ "+ICk-",		5 },	// EolType::PS
	};
	pMemEol->SetRawData(aEolTable[(int)eolType].szData, aEolTable[(int)eolType].nLen);
}
