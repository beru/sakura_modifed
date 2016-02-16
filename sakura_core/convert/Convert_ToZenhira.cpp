#include "StdAfx.h"
#include "Convert_ToZenhira.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// できる限り全角ひらがなにする
bool Converter_ToZenhira::DoConvert(NativeW* pData)
{
	// 半カナ→全角カナ
	// 文字数が減ることはあっても増えることは無いので、これでＯＫ
	std::vector<wchar_t> buf(pData->GetStringLength() + 1);
	wchar_t* pBuf = &buf[0];
	int nBufLen = 0;
	Convert_HankataToZenkata(pData->GetStringPtr(), pData->GetStringLength(), pBuf, &nBufLen);

	// 全カナ→全角ひらがな
	Convert_ZenkataToZenhira(pBuf, nBufLen);

	// 半角英数→全角英数
	Convert_HaneisuToZeneisu(pBuf, nBufLen);

	// 設定
	pData->SetString(pBuf, nBufLen);

	return true;
}

