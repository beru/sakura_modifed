#include "StdAfx.h"
#include "Convert_HaneisuToZeneisu.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ƒCƒ“ƒ^[ƒtƒF[ƒX                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ”¼Šp‰p”¨‘SŠp‰p”
bool Converter_HaneisuToZeneisu::DoConvert(NativeW* pData)
{
	// ”¼Šp‰p”¨‘SŠp‰p”
	Convert_HaneisuToZeneisu(pData->GetStringPtr(), pData->GetStringLength());

	return true;
}

