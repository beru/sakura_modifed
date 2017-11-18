#include "StdAfx.h"
#include "convert/Decode_Base64Decode.h"
#include "charset/charcode.h"
#include "convert/convert_util2.h"
#include "charset/codecheck.h"

// Base64デコード
bool Decode_Base64Decode::DoDecode(const NativeW& src, Memory* pDst)
{
	using namespace WCODE;

	const size_t BUFFER_SIZE = 1024;  // バッファサイズ。１以上の整数かつ４の倍数で。
	const size_t _BUFSIZE = ((BUFFER_SIZE + 3) / 4) * 4;

	const wchar_t* pSrc;
	size_t nSrcLen;
	char *pw, *pw_base;
	wchar_t buffer[_BUFSIZE];
	size_t i, j;
	wchar_t c = 0;

	pSrc = src.GetStringPtr();
	nSrcLen = src.GetStringLength();
	pDst->AllocBuffer(nSrcLen);  // 書き込みバッファを確保
	pw_base = pw = reinterpret_cast<char*>(pDst->GetRawPtr());

	i = 0;  // pSrc の添え字
	do {
		j = 0;
		for (; i<nSrcLen; ++i) {
		// バッファに文字をためるループ
			c = pSrc[i];
			if (IsLineDelimiterBasic(c) || c == TAB || c == SPACE) {
				continue;
			}
			if (j == _BUFSIZE || c == LTEXT('=')) {
				break;
			}
			if (!IsBase64(c)) {
				return false;
			}
			buffer[j] = static_cast<char>(c & 0xff);
			++j;
		}
		pw += _DecodeBase64(&buffer[0], j, pw);
	}while (i < nSrcLen && c != LTEXT('='));

	//if (!CheckBase64Padbit(&buffer[0], j)) {
	//	return false;
	//}

	pDst->_SetRawLength(pw - pw_base);
	return true;
}

