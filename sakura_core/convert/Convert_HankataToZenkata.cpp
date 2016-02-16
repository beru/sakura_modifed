#include "StdAfx.h"
#include "Convert_HankataToZenkata.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 半角カナ→全角カナ
bool Converter_HankataToZenkata::DoConvert(NativeW* pData)
{
	// 半角カナ→全角カナ
	// 文字数が減ることはあっても増えることは無いので、これでＯＫ
	std::vector<wchar_t> buf(pData->GetStringLength() + 1);
	wchar_t* pBuf = &buf[0];
	int nDstLen = 0;
	Convert_HankataToZenkata(pData->GetStringPtr(), pData->GetStringLength(), pBuf, &nDstLen);
	pData->SetString(pBuf, nDstLen);

	return true;
}

