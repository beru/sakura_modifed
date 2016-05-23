#include "StdAfx.h"
#include "Convert_ZenkataToHankata.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 全角カナ→半角カナ
bool Converter_ZenkataToHankata::DoConvert(NativeW* pData)
{
	// 全角カタカナ→半角カタカナ
	// 濁点等の影響で、最大2倍にまで膨れ上がる可能性があるので、2倍のバッファを確保
	std::vector<wchar_t> buf(pData->GetStringLength() * 2 + 1);
	wchar_t* pBuf = &buf[0];
	size_t nBufLen = 0;
	Convert_ZenkataToHankata(pData->GetStringPtr(), pData->GetStringLength(), pBuf, &nBufLen);
	pData->SetString(pBuf, nBufLen);

	return true;
}

