#include "StdAfx.h"
#include "CConvert_ToZenkata.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// できる限り全角カタカナにする
bool Converter_ToZenkata::DoConvert(NativeW* pData)
{
	// 半角カナ→全角カナ
	// 文字数が減ることはあっても増えることは無いので、これでＯＫ
	std::vector<wchar_t> buf(pData->GetStringLength() + 1);
	wchar_t* pBuf = &buf[0];
	int nBufLen = 0;
	Convert_HankataToZenkata(pData->GetStringPtr(), pData->GetStringLength(), pBuf, &nBufLen);

	// 全角ひら→全角カナ
	Convert_ZenhiraToZenkata(pBuf, nBufLen);

	// 半角英数→全角英数
	Convert_HaneisuToZeneisu(pBuf, nBufLen);

	// 設定
	pData->SetString(pBuf, nBufLen);

	return true;
}

