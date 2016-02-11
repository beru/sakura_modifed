#include "StdAfx.h"
#include "CConvert_HankataToZenkata.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 半角カナ→全角カナ
bool Converter_HankataToZenkata::DoConvert(NativeW* pcData)
{
	// 半角カナ→全角カナ
	// 文字数が減ることはあっても増えることは無いので、これでＯＫ
	std::vector<wchar_t> buf(pcData->GetStringLength() + 1);
	wchar_t* pBuf = &buf[0];
	int nDstLen = 0;
	Convert_HankataToZenkata(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nDstLen);
	pcData->SetString(pBuf, nDstLen);

	return true;
}

