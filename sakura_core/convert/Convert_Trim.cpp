#include "StdAfx.h"
#include "Convert_Trim.h"
#include "convert_util.h"
#include "Eol.h"
#include "charset/charcode.h"
#include "util/string_ex2.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! TRIM Step2
	ConvMemory から 戻ってきた後の処理．
	CMemory.cppのなかに置かないほうが良いかなと思ってこちらに置きました．
*/
bool Converter_Trim::DoConvert(NativeW* pData)
{
	const wchar_t*	pLine;
	size_t nLineLen;
	size_t nBgn;
	size_t i, j;
	size_t nPosDes;
	Eol			eol;
	size_t nCharChars;

	nBgn = 0;
	nPosDes = 0;
	// 変換後に必要なバイト数を調べる
	while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &eol, bExtEol))) {
		if (0 < nLineLen) {
			nPosDes += nLineLen;
		}
		nPosDes += eol.GetLen();
	}
	if (0 >= nPosDes) {
		return true;
	}
	std::vector<wchar_t> des(nPosDes + 1);
	wchar_t* pDes = &des[0];
	nBgn = 0;
	nPosDes = 0;
	// LTRIM
	if (bLeft) {
		while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &eol, bExtEol))) {
			if (0 < nLineLen) {
				for (i=0; i<=nLineLen; ++i) {
					if (WCODE::IsBlank(pLine[i])) {
						continue;
					}else {
						break;
					}
				}
				if (nLineLen-i>0) {
					wmemcpy(&pDes[nPosDes], &pLine[i], nLineLen);
					nPosDes += nLineLen - i;
				}
			}
			wmemcpy(&pDes[nPosDes], eol.GetValue2(), eol.GetLen());
			nPosDes += eol.GetLen();
		}
	}else {
		// RTRIM
		while ((pLine = GetNextLineW(pData->GetStringPtr(), pData->GetStringLength(), &nLineLen, &nBgn, &eol, bExtEol))) {
			if (0 < nLineLen) {
				// 右から遡るのではなく左から探す（"ａ@" の右２バイトが全角空白と判定される問題の対処）
				i = j = 0;
				while (i < nLineLen) {
					nCharChars = NativeW::GetSizeOfChar(pLine, nLineLen, i);
					if (!WCODE::IsBlank(pLine[i])) {
						j = i + nCharChars;
					}
					i += nCharChars;
				}
				if (j > 0) {
					wmemcpy(&pDes[nPosDes], &pLine[0], j);
					nPosDes += j;
				}
			}
			wmemcpy(&pDes[nPosDes], eol.GetValue2(), eol.GetLen());
			nPosDes += eol.GetLen();
		}
	}
	pDes[nPosDes] = L'\0';

	pData->SetString(pDes, nPosDes);
	return true;
}


