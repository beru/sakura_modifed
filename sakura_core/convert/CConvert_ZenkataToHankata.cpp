#include "StdAfx.h"
#include "CConvert_ZenkataToHankata.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 全角カナ→半角カナ
bool Converter_ZenkataToHankata::DoConvert(NativeW* pcData)
{
	// 全角カタカナ→半角カタカナ
	// 濁点等の影響で、最大2倍にまで膨れ上がる可能性があるので、2倍のバッファを確保
	std::vector<wchar_t> buf(pcData->GetStringLength() * 2 + 1);
	wchar_t* pBuf = &buf[0];
	int nBufLen = 0;
	Convert_ZenkataToHankata(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nBufLen);
	pcData->SetString(pBuf, nBufLen);

	return true;
}

