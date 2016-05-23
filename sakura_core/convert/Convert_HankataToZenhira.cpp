#include "StdAfx.h"
#include "Convert_HankataToZenhira.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 半角カナ→全角ひらがな
bool Converter_HankataToZenhira::DoConvert(NativeW* pData)
{
	// 半角カナ→全角ひらがな
	// 文字数が減ることはあっても増えることは無いので、これでＯＫ
	std::vector<wchar_t> buf(pData->GetStringLength() + 1);
	wchar_t* pBuf = &buf[0];
	size_t nDstLen = 0;
	Convert_HankataToZenhira(pData->GetStringPtr(), pData->GetStringLength(), pBuf, &nDstLen);
	pData->SetString(pBuf, nDstLen);

	return true;
}

