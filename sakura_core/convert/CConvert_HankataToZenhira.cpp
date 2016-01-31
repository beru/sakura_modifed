#include "StdAfx.h"
#include "CConvert_HankataToZenhira.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 半角カナ→全角ひらがな
bool CConvert_HankataToZenhira::DoConvert(CNativeW* pcData)
{
	// 半角カナ→全角ひらがな
	// 文字数が減ることはあっても増えることは無いので、これでＯＫ
	std::vector<wchar_t> buf(pcData->GetStringLength() + 1);
	wchar_t* pBuf = &buf[0];
	int nDstLen = 0;
	Convert_HankataToZenhira(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nDstLen);
	pcData->SetString(pBuf, nDstLen);

	return true;
}

