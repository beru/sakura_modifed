#include "StdAfx.h"
#include "CConvert_HaneisuToZeneisu.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ƒCƒ“ƒ^[ƒtƒF[ƒX                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ”¼Šp‰p”¨‘SŠp‰p”
bool Converter_HaneisuToZeneisu::DoConvert(NativeW* pcData)
{
	// ”¼Šp‰p”¨‘SŠp‰p”
	Convert_HaneisuToZeneisu(pcData->GetStringPtr(), pcData->GetStringLength());

	return true;
}

