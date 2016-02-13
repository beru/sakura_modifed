#include "StdAfx.h"
#include "CConvert_ZeneisuToHaneisu.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ƒCƒ“ƒ^[ƒtƒF[ƒX                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ‘SŠp‰p”¨”¼Šp‰p”
bool Converter_ZeneisuToHaneisu::DoConvert(NativeW* pData)
{
	// ‘SŠp‰p”¨”¼Šp‰p”
	Convert_ZeneisuToHaneisu(pData->GetStringPtr(), pData->GetStringLength());

	return true;
}

